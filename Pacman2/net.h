#ifndef NET_H_
#define NET_H_

#include <string>
#include <vector>

/*
 * LAN multiplayer over TCP (POSIX sockets), one text line per message in
 * the bracketed style the original pacman.exe used, e.g. "[EAT_SM 42 2]".
 *
 * The server is player 1 and the hub: a client's message goes to the
 * server, which handles it and relays it to every other client. Server
 * messages go to all clients.
 *
 * Lobby:   [WELCOME id]  [LOBBY players]  [START players difficulty speed options]
 * Game:    [EAT_SM mark id] [EAT_PM mark id terrain] [SCORE id n] [LIVES id n]
 *          [STATE id state] [PAUSE id] [UNPAUSE id] [LEFT id]
 *          [SEND_SM from to] [GET_SM from to] [SEND_PM from to] [GET_PM from to]
 *          [MOVE_MARK mark owner]   (server's answer to the four above)
 *          [MOVE_DOT mark col row]  (server: a power dot wandered)
 *          [GHOST to color label col row dir] [GET_GHOST from to]
 * Terrains (the side tunnel leads into the next player's maze):
 *          [PAC_TERRAIN id terrain] [PAC_POS id terrain x y dir anim]
 *          [SNAP terrain fright walls... ghosts...]  (host -> visitors)
 *          [PAC_HIT id] [GHOST_EATEN id points]      (host decides collisions)
 */

#define PACMAN_PORT 10000

class Net {
public:
	struct Message { int from; std::string text; };

	Net();
	virtual ~Net();

#ifdef __EMSCRIPTEN__
	/* Browser build: online games through the relay (relay/server.js)
	   over a WebSocket, since a web page can't listen for connections. */
	bool HostRelay(const std::string& url);                         /* get a room code */
	bool JoinRelay(const std::string& url, const std::string& code);
	const std::string& RoomCode() const { return room; }
	virtual bool IsServer() const { return is_host; }
#else
	bool Host(int port);
	bool Connect(const char* host, int port);
	virtual bool IsServer() const { return listen_fd >= 0; }
#endif
	const std::string& Error() const { return error; }

	/* Client: to the server. Server: to every client. */
	virtual void Send(const std::string& line);
	/* Reads whatever arrived. The server also accepts new players while
	   the lobby is open and relays client messages to the others. */
	std::vector<Message> Poll();
	/* Server: ids of players that disconnected since the last call. */
	std::vector<int> TakeDisconnected();
	bool ServerLost() const { return server_lost; }

	int PlayerCount() const;        /* server: 1 + connected clients */
	void CloseLobby() { lobby_open = false; }

private:
	std::vector<int> disconnected;
	bool lobby_open, server_lost;
	std::string error;
#ifdef __EMSCRIPTEN__
	int socket;                      /* EMSCRIPTEN_WEBSOCKET_T */
	bool is_host, opened, closed;
	std::string room;
	int lobby_players;
	std::vector<int> joined;         /* host: players in the room */
	std::vector<std::string> inbox;  /* filled by the WebSocket callbacks */
	friend struct NetWebCallbacks;   /* net_web.cpp */
	bool Open(const std::string& url);
	bool WaitFor(const char* prefix, int ms);
#else
	struct Conn { int fd; int id; std::string in, out; };
	int listen_fd;
	std::vector<Conn> conns;  /* server: clients; client: the server */

	void Accept();
	void Queue(Conn& c, const std::string& line);
	bool Flush(Conn& c);
	void Drop(size_t i);
#endif
};

#endif
