/*
 * Pacman relay: lets browser players host and join online games.
 *
 * Browsers can't listen for connections, so the host's game connects
 * here too. The relay plays the part the Mac host's TCP server plays on
 * a LAN: it hands out player ids and passes the game's text messages
 * around. The host's game stays in charge of the game itself.
 *
 * Every WebSocket text frame is one game message. A connection's first
 * frame is "HOST" or "JOIN <code>":
 *
 *   HOST        -> [ROOM <code>]         you are player 1 of a new room
 *   JOIN <code> -> [WELCOME <id>]         you are player <id> (2-4)
 *               -> [ERROR <reason>]       and the connection closes
 *
 * Then, within a room:
 *   - the host's messages go to every player; "[START ...]" closes the
 *     room to new players;
 *   - a player's messages go to the host and the other players;
 *   - everyone gets [LOBBY <players>] while the room is open;
 *   - the host gets [@JOIN <id>] and [@LEFT <id>] as players come and go;
 *   - when the host leaves, the room closes and so do the players'
 *     connections.
 */
import http from 'node:http';
import { WebSocketServer } from 'ws';

const MAX_PLAYERS = 4;          // host + 3
const MAX_ROOMS = 500;
const MAX_MESSAGE = 4096;       // bytes; game messages are far smaller
const HEARTBEAT_MS = 30000;

export function createRelay({ port = Number(process.env.PORT) || 8787, log = console.log } = {}) {
  const rooms = new Map();      // code -> { host, players: Map<id, ws>, open }

  const server = http.createServer((req, res) => {
    res.writeHead(200, { 'content-type': 'text/plain' });
    res.end(`pacman relay: ${rooms.size} room(s)\n`);
  });
  const wss = new WebSocketServer({ server, maxPayload: MAX_MESSAGE });

  function send(ws, text) {
    if (ws.readyState === ws.OPEN) ws.send(text);
  }

  function lobby(room) {
    const text = `[LOBBY ${1 + room.players.size}]`;
    send(room.host, text);
    for (const p of room.players.values()) send(p, text);
  }

  function newCode() {
    for (let i = 0; i < 1000; i++) {
      const code = String(1000 + Math.floor(Math.random() * 9000));
      if (!rooms.has(code)) return code;
    }
    return null;
  }

  function refuse(ws, reason) {
    send(ws, `[ERROR ${reason}]`);
    ws.close(1008, reason);
  }

  function onFirstMessage(ws, text) {
    if (text === 'HOST') {
      const code = rooms.size < MAX_ROOMS ? newCode() : null;
      if (!code) return refuse(ws, 'The relay is full, try again later');
      rooms.set(code, { host: ws, players: new Map(), open: true });
      ws.room = code;
      ws.playerId = 1;
      send(ws, `[ROOM ${code}]`);
      log(`room ${code} opened (${rooms.size} rooms)`);
      return;
    }
    const m = /^JOIN (\d{4})$/.exec(text);
    if (!m) return refuse(ws, 'Expected HOST or JOIN <code>');
    const room = rooms.get(m[1]);
    if (!room) return refuse(ws, 'No game with that code');
    if (!room.open) return refuse(ws, 'That game has already started');
    if (room.players.size >= MAX_PLAYERS - 1) return refuse(ws, 'That game is full');
    let id = 2;
    while (room.players.has(id)) id++;
    room.players.set(id, ws);
    ws.room = m[1];
    ws.playerId = id;
    send(ws, `[WELCOME ${id}]`);
    send(room.host, `[@JOIN ${id}]`);
    lobby(room);
  }

  function onGameMessage(ws, text) {
    const room = rooms.get(ws.room);
    if (!room) return;
    if (ws.playerId === 1) {
      if (text.startsWith('[START')) room.open = false;
      for (const p of room.players.values()) send(p, text);
    } else {
      send(room.host, text);
      for (const [id, p] of room.players) if (id !== ws.playerId) send(p, text);
    }
  }

  function onClose(ws) {
    const room = rooms.get(ws.room);
    if (!room) return;
    if (ws.playerId === 1) {
      rooms.delete(ws.room);
      for (const p of room.players.values()) p.close(1001, 'The host left');
      log(`room ${ws.room} closed (${rooms.size} rooms)`);
    } else if (room.players.get(ws.playerId) === ws) {
      room.players.delete(ws.playerId);
      send(room.host, `[@LEFT ${ws.playerId}]`);
      if (room.open) lobby(room);
    }
  }

  wss.on('connection', (ws) => {
    ws.isAlive = true;
    ws.on('pong', () => { ws.isAlive = true; });
    ws.on('message', (data, isBinary) => {
      if (isBinary) return;
      const text = data.toString();
      if (ws.room === undefined) onFirstMessage(ws, text);
      else onGameMessage(ws, text);
    });
    ws.on('close', () => onClose(ws));
    ws.on('error', () => {});
  });

  /* Drop connections that stopped answering. */
  const heartbeat = setInterval(() => {
    for (const ws of wss.clients) {
      if (!ws.isAlive) { ws.terminate(); continue; }
      ws.isAlive = false;
      ws.ping();
    }
  }, HEARTBEAT_MS);

  return new Promise((resolve) => {
    server.listen(port, () => {
      resolve({
        port: server.address().port,
        rooms,
        close: () => new Promise((done) => {
          clearInterval(heartbeat);
          for (const ws of wss.clients) ws.terminate();
          wss.close(() => server.close(() => done()));
        }),
      });
    });
  });
}

if (import.meta.url === `file://${process.argv[1]}`) {
  const relay = await createRelay();
  console.log(`pacman relay listening on port ${relay.port}`);
}
