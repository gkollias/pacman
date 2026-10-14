#include "net.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLIENTS 3

static void Configure(int fd)
{
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	int one = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
#ifdef SO_NOSIGPIPE
	setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif
}

Net::Net() : lobby_open(true), server_lost(false), listen_fd(-1) {}

Net::~Net()
{
	for (size_t i = 0; i < conns.size(); i++) close(conns[i].fd);
	if (listen_fd >= 0) close(listen_fd);
}

bool Net::Host(int port)
{
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) { error = strerror(errno); return false; }
	int one = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0 || listen(listen_fd, 4) < 0) {
		error = std::string("Cannot listen on port: ") + strerror(errno);
		close(listen_fd);
		listen_fd = -1;
		return false;
	}
	fcntl(listen_fd, F_SETFL, fcntl(listen_fd, F_GETFL, 0) | O_NONBLOCK);
	return true;
}

bool Net::Connect(const char* host, int port)
{
	addrinfo hints, *res = NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	char port_str[16];
	snprintf(port_str, sizeof(port_str), "%d", port);
	if (getaddrinfo(host, port_str, &hints, &res) != 0 || !res) {
		error = std::string("Unknown host ") + host;
		return false;
	}
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	Configure(fd);
	int rc = connect(fd, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);
	if (rc < 0 && errno == EINPROGRESS) {
		/* Don't hang on an unreachable address. */
		fd_set w;
		FD_ZERO(&w);
		FD_SET(fd, &w);
		timeval tv = { 3, 0 };
		rc = select(fd + 1, NULL, &w, NULL, &tv) > 0 ? 0 : -1;
		if (rc == 0) {
			int err = 0;
			socklen_t len = sizeof(err);
			getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
			if (err) { errno = err; rc = -1; }
		} else {
			errno = ETIMEDOUT;
		}
	}
	if (rc < 0) {
		error = std::string("Cannot connect to ") + host + ": " + strerror(errno);
		close(fd);
		return false;
	}
	Conn c;
	c.fd = fd;
	c.id = 1;
	conns.push_back(c);
	return true;
}

int Net::PlayerCount() const { return 1 + (int)conns.size(); }

void Net::Queue(Conn& c, const std::string& line)
{
	c.out += line;
	c.out += '\n';
}

bool Net::Flush(Conn& c)
{
	while (!c.out.empty()) {
		ssize_t n = send(c.fd, c.out.data(), c.out.size(), 0);
		if (n > 0) { c.out.erase(0, n); continue; }
		if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return true;
		return false;
	}
	return true;
}

void Net::Send(const std::string& line)
{
	for (size_t i = 0; i < conns.size(); i++) Queue(conns[i], line);
}

void Net::Accept()
{
	int fd;
	while ((fd = accept(listen_fd, NULL, NULL)) >= 0) {
		if (!lobby_open || conns.size() >= MAX_CLIENTS) { close(fd); continue; }
		Configure(fd);
		/* Lowest free player id 2..4. */
		int id = 2;
		for (bool taken = true; taken; ) {
			taken = false;
			for (size_t i = 0; i < conns.size(); i++)
				if (conns[i].id == id) { id++; taken = true; }
		}
		Conn c;
		c.fd = fd;
		c.id = id;
		conns.push_back(c);
		char buf[32];
		snprintf(buf, sizeof(buf), "[WELCOME %d]", id);
		Queue(conns.back(), buf);
		snprintf(buf, sizeof(buf), "[LOBBY %d]", PlayerCount());
		Send(buf);
	}
}

void Net::Drop(size_t i)
{
	close(conns[i].fd);
	if (IsServer()) disconnected.push_back(conns[i].id);
	else server_lost = true;
	conns.erase(conns.begin() + i);
	if (IsServer() && lobby_open) {
		char buf[32];
		snprintf(buf, sizeof(buf), "[LOBBY %d]", PlayerCount());
		Send(buf);
	}
}

std::vector<Net::Message> Net::Poll()
{
	std::vector<Message> got;
	if (IsServer()) Accept();

	for (size_t i = 0; i < conns.size(); ) {
		Conn& c = conns[i];
		bool alive = true;
		char buf[4096];
		for (;;) {
			ssize_t n = recv(c.fd, buf, sizeof(buf), 0);
			if (n > 0) { c.in.append(buf, n); continue; }
			if (n == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) alive = false;
			break;
		}
		size_t nl;
		while ((nl = c.in.find('\n')) != std::string::npos) {
			Message m;
			m.from = c.id;
			m.text = c.in.substr(0, nl);
			c.in.erase(0, nl + 1);
			if (!m.text.empty() && m.text[m.text.size() - 1] == '\r') m.text.erase(m.text.size() - 1);
			if (m.text.empty()) continue;
			got.push_back(m);
			/* Relay a client's message to the other clients. */
			if (IsServer())
				for (size_t j = 0; j < conns.size(); j++)
					if (j != i) Queue(conns[j], m.text);
		}
		if (!alive) { Drop(i); continue; }
		i++;
	}
	for (size_t i = 0; i < conns.size(); ) {
		if (!Flush(conns[i])) { Drop(i); continue; }
		i++;
	}
	return got;
}

std::vector<int> Net::TakeDisconnected()
{
	std::vector<int> d;
	d.swap(disconnected);
	return d;
}
