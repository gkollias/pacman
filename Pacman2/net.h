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
 * Lobby:   [WELCOME id]  [LOBBY players]  [START players difficulty speed]
 * Game:    [EAT_SM idx id] [EAT_PM idx id] [SCORE id n] [LIVES id n]
 *          [STATE id state] [PAUSE id] [UNPAUSE id] [LEFT id]
 *          [SEND_SM from to] [GET_SM from to] [SEND_PM from to] [GET_PM from to]
 *          [MOVE_MARK idx owner]   (server's answer to the four above)
 *          [GHOST to color label] [GET_GHOST from to]
 */

#define PACMAN_PORT 10000

class Net {
public:
	struct Message { int from; std::string text; };

	Net();
	virtual ~Net();

	bool Host(int port);
	bool Connect(const char* host, int port);
	virtual bool IsServer() const { return listen_fd >= 0; }
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
	struct Conn { int fd; int id; std::string in, out; };
	int listen_fd;
	std::vector<Conn> conns;  /* server: clients; client: the server */
	std::vector<int> disconnected;
	bool lobby_open, server_lost;
	std::string error;

	void Accept();
	void Queue(Conn& c, const std::string& line);
	bool Flush(Conn& c);
	void Drop(size_t i);
};

#endif
