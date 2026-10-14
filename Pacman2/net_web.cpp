/*
 * Browser build of Net: online games through the relay (relay/server.js)
 * over a WebSocket. The relay hands out player ids and passes messages
 * around like the TCP host does on a LAN, so the game sees the same
 * protocol either way (see net.h).
 */
#include "net.h"

#include <emscripten.h>
#include <emscripten/websocket.h>
#include <string.h>

struct NetWebCallbacks {
	static bool OnOpen(int, const EmscriptenWebSocketOpenEvent*, void* user)
	{
		((Net*)user)->opened = true;
		return true;
	}
	static bool OnMessage(int, const EmscriptenWebSocketMessageEvent* e, void* user)
	{
		if (e->isText && e->numBytes > 0)
			((Net*)user)->inbox.push_back(std::string((const char*)e->data, strnlen((const char*)e->data, e->numBytes)));
		return true;
	}
	static bool OnClose(int, const EmscriptenWebSocketCloseEvent* e, void* user)
	{
		Net* n = (Net*)user;
		n->closed = true;
		if (n->error.empty() && e->reason[0]) n->error = e->reason;
		return true;
	}
	static bool OnError(int, const EmscriptenWebSocketErrorEvent*, void* user)
	{
		Net* n = (Net*)user;
		if (n->error.empty()) n->error = "Cannot reach the game server";
		n->closed = true;
		return true;
	}
};

Net::Net()
	: lobby_open(true), server_lost(false), socket(0), is_host(false), opened(false),
	  closed(false), lobby_players(1) {}

Net::~Net()
{
	if (socket > 0) {
		emscripten_websocket_close(socket, 1000, "bye");
		emscripten_websocket_delete(socket);
	}
}

bool Net::Open(const std::string& url)
{
	if (!emscripten_websocket_is_supported()) { error = "This browser has no WebSockets"; return false; }
	EmscriptenWebSocketCreateAttributes attr;
	emscripten_websocket_init_create_attributes(&attr);
	attr.url = url.c_str();
	attr.createOnMainThread = true;
	socket = emscripten_websocket_new(&attr);
	if (socket <= 0) { error = "Cannot open a connection to " + url; return false; }
	emscripten_websocket_set_onopen_callback(socket, this, NetWebCallbacks::OnOpen);
	emscripten_websocket_set_onmessage_callback(socket, this, NetWebCallbacks::OnMessage);
	emscripten_websocket_set_onclose_callback(socket, this, NetWebCallbacks::OnClose);
	emscripten_websocket_set_onerror_callback(socket, this, NetWebCallbacks::OnError);
	/* Callbacks run while we yield to the browser. A relay on a free host
	   may be asleep and take most of a minute to wake up. */
	for (int waited = 0; !opened && !closed && waited < 75000; waited += 10) emscripten_sleep(10);
	if (!opened) {
		if (error.empty()) error = "Cannot reach the game server";
		return false;
	}
	return true;
}

/* Waits for the relay's answer: the first message, which must start with
   `prefix` (otherwise it's an [ERROR ...] and becomes the error text). The
   answer stays in the inbox for the game to read. */
bool Net::WaitFor(const char* prefix, int ms)
{
	for (int waited = 0; inbox.empty() && !closed && waited < ms; waited += 10) emscripten_sleep(10);
	if (inbox.empty()) {
		if (error.empty()) error = "No answer from the game server";
		return false;
	}
	const std::string& first = inbox.front();
	if (first.compare(0, strlen(prefix), prefix) == 0) return true;
	if (first.compare(0, 7, "[ERROR ") == 0) error = first.substr(7, first.size() - 8);
	else error = "Unexpected answer: " + first;
	return false;
}

bool Net::HostRelay(const std::string& url)
{
	if (!Open(url)) return false;
	emscripten_websocket_send_utf8_text(socket, "HOST");
	if (!WaitFor("[ROOM ", 15000)) return false;
	room = inbox.front().substr(6, 4);
	inbox.erase(inbox.begin());
	is_host = true;
	return true;
}

bool Net::JoinRelay(const std::string& url, const std::string& code)
{
	if (!Open(url)) return false;
	std::string join = "JOIN " + code;
	emscripten_websocket_send_utf8_text(socket, join.c_str());
	return WaitFor("[WELCOME ", 15000);  /* [WELCOME id] is left for the lobby screen */
}

void Net::Send(const std::string& line)
{
	if (socket > 0 && opened && !closed) emscripten_websocket_send_utf8_text(socket, line.c_str());
}

std::vector<Net::Message> Net::Poll()
{
	std::vector<Message> got;
	std::vector<std::string> in;
	in.swap(inbox);
	for (size_t i = 0; i < in.size(); i++) {
		const std::string& t = in[i];
		int id;
		/* Relay notices for the host: who joined and who left. */
		if (sscanf(t.c_str(), "[@JOIN %d]", &id) == 1) { joined.push_back(id); continue; }
		if (sscanf(t.c_str(), "[@LEFT %d]", &id) == 1) {
			for (size_t k = 0; k < joined.size(); k++)
				if (joined[k] == id) { joined.erase(joined.begin() + k); break; }
			disconnected.push_back(id);
			continue;
		}
		if (sscanf(t.c_str(), "[LOBBY %d]", &id) == 1) lobby_players = id;
		Message m;
		m.from = 0;
		m.text = t;
		got.push_back(m);
	}
	if (closed) {
		if (is_host) {
			/* Lost the relay: everyone else is gone too. */
			for (size_t k = 0; k < joined.size(); k++) disconnected.push_back(joined[k]);
			joined.clear();
		} else {
			server_lost = true;
		}
	}
	return got;
}

std::vector<int> Net::TakeDisconnected()
{
	std::vector<int> d;
	d.swap(disconnected);
	return d;
}

int Net::PlayerCount() const { return is_host ? lobby_players : 1; }
