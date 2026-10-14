import { test, before, after } from 'node:test';
import assert from 'node:assert/strict';
import { createRelay } from '../server.js';

let relay;
before(async () => { relay = await createRelay({ port: 0, log: () => {} }); });
after(async () => { await relay.close(); });

/* A test client that collects every message it receives. */
function connect() {
  return new Promise((resolve, reject) => {
    const ws = new WebSocket(`ws://localhost:${relay.port}`);
    const c = { ws, got: [], closed: false, waiters: [] };
    ws.onmessage = (e) => {
      c.got.push(e.data);
      c.waiters = c.waiters.filter((w) => !w());
    };
    ws.onclose = () => { c.closed = true; c.waiters = c.waiters.filter((w) => !w()); };
    ws.onopen = () => resolve(c);
    ws.onerror = reject;
  });
}

/* Waits until a message matching `re` has arrived (or the connection closed). */
function next(c, re, ms = 2000) {
  return new Promise((resolve, reject) => {
    const find = () => {
      const i = c.got.findIndex((m) => re.test(m));
      if (i >= 0) { resolve(c.got.splice(i, 1)[0]); return true; }
      if (c.closed) { reject(new Error(`closed before ${re}; got ${JSON.stringify(c.got)}`)); return true; }
      return false;
    };
    if (find()) return;
    c.waiters.push(find);
    setTimeout(() => reject(new Error(`timeout waiting for ${re}; got ${JSON.stringify(c.got)}`)), ms);
  });
}

const settle = () => new Promise((r) => setTimeout(r, 50));

async function hostRoom() {
  const host = await connect();
  host.ws.send('HOST');
  const code = (await next(host, /^\[ROOM \d{4}\]$/)).slice(6, 10);
  return { host, code };
}

async function join(code) {
  const c = await connect();
  c.ws.send(`JOIN ${code}`);
  return c;
}

test('host gets a room code; players get ids 2, 3, 4 and lobby counts', async () => {
  const { host, code } = await hostRoom();
  const a = await join(code);
  assert.equal(await next(a, /^\[WELCOME/), '[WELCOME 2]');
  assert.equal(await next(host, /^\[@JOIN/), '[@JOIN 2]');
  assert.equal(await next(host, /^\[LOBBY/), '[LOBBY 2]');
  const b = await join(code);
  assert.equal(await next(b, /^\[WELCOME/), '[WELCOME 3]');
  assert.equal(await next(a, /^\[LOBBY 3\]/), '[LOBBY 3]');
  const c = await join(code);
  assert.equal(await next(c, /^\[WELCOME/), '[WELCOME 4]');
  const d = await join(code);
  assert.match(await next(d, /^\[ERROR/), /full/);
  for (const x of [host, a, b, c]) x.ws.close();
});

test('messages are routed like the LAN host does', async () => {
  const { host, code } = await hostRoom();
  const a = await join(code);
  const b = await join(code);
  await next(a, /^\[WELCOME 2\]/);
  await next(b, /^\[WELCOME 3\]/);
  await settle();
  for (const x of [host, a, b]) x.got.length = 0;

  host.ws.send('[START 3 1 1 15]');
  assert.equal(await next(a, /^\[START/), '[START 3 1 1 15]');
  assert.equal(await next(b, /^\[START/), '[START 3 1 1 15]');

  a.ws.send('[EAT_SM 7 2]');                       // player -> host and the others
  assert.equal(await next(host, /^\[EAT_SM/), '[EAT_SM 7 2]');
  assert.equal(await next(b, /^\[EAT_SM/), '[EAT_SM 7 2]');
  await settle();
  assert.ok(!a.got.some((m) => m.startsWith('[EAT_SM')), 'no echo to the sender');

  host.ws.send('[MOVE_MARK 7 3]');                 // host -> every player
  assert.equal(await next(a, /^\[MOVE_MARK/), '[MOVE_MARK 7 3]');
  assert.equal(await next(b, /^\[MOVE_MARK/), '[MOVE_MARK 7 3]');

  const late = await join(code);                   // started: closed to newcomers
  assert.match(await next(late, /^\[ERROR/), /already started/);
  for (const x of [host, a, b]) x.ws.close();
});

test('the host hears when a player leaves', async () => {
  const { host, code } = await hostRoom();
  const a = await join(code);
  await next(a, /^\[WELCOME 2\]/);
  await next(host, /^\[@JOIN 2\]/);
  a.ws.close();
  assert.equal(await next(host, /^\[@LEFT/), '[@LEFT 2]');
  assert.equal(await next(host, /^\[LOBBY 1\]/), '[LOBBY 1]');
  const b = await join(code);                      // the freed id is reused
  assert.equal(await next(b, /^\[WELCOME/), '[WELCOME 2]');
  host.ws.close();
  b.ws.close();
});

test('the room closes when the host leaves', async () => {
  const { host, code } = await hostRoom();
  const a = await join(code);
  await next(a, /^\[WELCOME 2\]/);
  host.ws.close();
  await new Promise((r) => { if (a.closed) r(); else a.waiters.push(() => { if (a.closed) { r(); return true; } return false; }); });
  assert.ok(a.closed);
  await settle();
  assert.ok(!relay.rooms.has(code));
  const b = await join(code);
  assert.match(await next(b, /^\[ERROR/), /No game/);
});

test('bad first messages are refused', async () => {
  const x = await connect();
  x.ws.send('HELLO');
  assert.match(await next(x, /^\[ERROR/), /Expected HOST or JOIN/);
  const y = await connect();
  y.ws.send('JOIN 0000');
  assert.match(await next(y, /^\[ERROR/), /No game/);
});
