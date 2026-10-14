/*
 * Pacman (LAN edition): a rebuild of the later 2008 pacman.exe, whose
 * source was lost, on the grid levels and assets that shipped with it.
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>
#include "common.h"
#include "game.h"
#include "net.h"
#if WEB_BUILD
#include <emscripten.h>
/* The relay's address comes from the web page (window.PACMAN_RELAY). */
EM_JS(char*, RelayUrlFromPage, (), {
	var url = (typeof window !== "undefined" && window.PACMAN_RELAY) || "";
	return stringToNewUTF8(url);
});
#endif

static volatile int ticks = 0;
static void Tick(void) { ticks++; }
END_OF_FUNCTION(Tick);

static BITMAP* buffer;
static Input input;

static void Show(void) { blit(buffer, screen, 0, 0, 0, 0, GAME_W, GAME_H); }

static void Text(int x, int y, const char* s)
{
	textprintf_ex(buffer, font, x, y, makecol(255, 255, 255), -1, "%s", s);
}

/* Waits for the next 60 Hz tick and refreshes the keyboard state. */
static void NextFrame(void)
{
	while (ticks == 0) rest(1);
	ticks = 0;
	input.Update();
}

/* Numbered menu; 1-9 or arrows to pick, Enter to confirm. -1 on Escape. */
static int Menu(const char* title, const char* const* options, int count, int selected)
{
	for (;;) {
		NextFrame();
		if (input.Pressed(KEY_ESC)) return -1;
		if (input.Pressed(KEY_UP) && selected > 0) selected--;
		if (input.Pressed(KEY_DOWN) && selected < count - 1) selected++;
		for (int i = 0; i < count && i < 9; i++)
			if (input.Pressed(KEY_1 + i)) selected = i;
		if (input.Pressed(KEY_ENTER) || input.Pressed(KEY_ENTER_PAD)) return selected;

		clear_to_color(buffer, makecol(0, 0, 0));
		Text(220, 183, title);
		for (int i = 0; i < count; i++) {
			char line[80];
			snprintf(line, sizeof(line), "%s%d.%s", i == selected ? "> " : "  ", i + 1, options[i]);
			Text(212, 193 + 10 * i, line);
		}
		Text(220, 193 + 10 * count + 20, "Use 1-3 or the arrow keys, then Enter.");
		Text(440, 590, ESCAPE_HINT);
		Show();
	}
}

/* Toggles for the optional features. Returns -1 on Escape. */
static int OptionsMenu(int options, bool multiplayer)
{
	static const char* const NAMES[] = {
		"Moving obstacles", "Dots run away from Pac-Man",
		"Tunnel leads to the next player's maze (multiplayer)", "Cornered: send a ghost away (multiplayer)",
	};
	static const int FLAGS[] = { OPT_MOVING_WALLS, OPT_MOVING_DOTS, OPT_TERRAINS, OPT_AUTO_SEND };
	const int count = multiplayer || !WEB_BUILD ? 4 : 2;  /* no point showing multiplayer options to a lone web player */
	int selected = 0;
	for (;;) {
		NextFrame();
		if (input.Pressed(KEY_ESC)) return -1;
		if (input.Pressed(KEY_UP) && selected > 0) selected--;
		if (input.Pressed(KEY_DOWN) && selected < count - 1) selected++;
		for (int i = 0; i < count; i++)
			if (input.Pressed(KEY_1 + i)) { selected = i; options ^= FLAGS[i]; }
		if (input.Pressed(KEY_SPACE)) options ^= FLAGS[selected];
		if (input.Pressed(KEY_ENTER) || input.Pressed(KEY_ENTER_PAD)) return options;

		clear_to_color(buffer, makecol(0, 0, 0));
		Text(220, 183, count == 2 ? "Options (1-2 or Space to toggle, Enter to play):"
		                          : "Options (1-4 or Space to toggle, Enter to play):");
		for (int i = 0; i < count; i++) {
			char line[96];
			snprintf(line, sizeof(line), "%s%d.[%s] %s", i == selected ? "> " : "  ", i + 1,
			         (options & FLAGS[i]) ? "x" : " ", NAMES[i]);
			Text(212, 203 + 10 * i, line);
		}
		Text(440, 590, ESCAPE_HINT);
		Show();
	}
}

/* Shows a message until Enter or Escape. */
static void Notice(const char* line1, const char* line2)
{
	for (;;) {
		NextFrame();
		if (input.Pressed(KEY_ENTER) || input.Pressed(KEY_ENTER_PAD) || input.Pressed(KEY_ESC)) return;
		clear_to_color(buffer, makecol(0, 0, 0));
		Text(220, 183, line1);
		if (line2) Text(220, 193, line2);
		Text(220, 213, "Press Enter to continue.");
		Show();
	}
}

/* Edits a line of digits (and dots, if allowed). Returns false on Escape. */
static bool AskText(const char* prompt, const char* help, std::string& value, bool dots, size_t max_len)
{
	for (;;) {
		NextFrame();
		if (input.Pressed(KEY_ESC)) return false;
		if (input.Pressed(KEY_ENTER) || input.Pressed(KEY_ENTER_PAD)) return !value.empty();
		if (input.Pressed(KEY_BACKSPACE) && !value.empty()) value.erase(value.size() - 1);
		if (dots && input.Pressed(KEY_STOP) && value.size() < max_len) value += '.';
		for (int d = 0; d <= 9; d++) {
			int k = d == 0 ? KEY_0 : KEY_1 + d - 1;
			if (input.Pressed(k) && value.size() < max_len) value += (char)('0' + d);
		}
		clear_to_color(buffer, makecol(0, 0, 0));
		Text(220, 183, prompt);
		std::string shown = value + ((ticks / 30) % 2 ? " " : "_");
		Text(220, 203, shown.c_str());
		Text(220, 223, help);
		Show();
	}
}

/* Server lobby: wait for clients until Enter. Returns players, 0 on Escape. */
static int Lobby(Net& net)
{
	for (;;) {
		NextFrame();
		net.Poll();
		if (input.Pressed(KEY_ESC)) return 0;
		if (input.Pressed(KEY_ENTER) || input.Pressed(KEY_ENTER_PAD)) return net.PlayerCount();
		clear_to_color(buffer, makecol(0, 0, 0));
		char line[80];
#if WEB_BUILD
		snprintf(line, sizeof(line), "Room code: %s", net.RoomCode().c_str());
		Text(220, 173, line);
		Text(220, 183, "Share this code with your friends; they join from this page.");
#else
		Text(220, 183, "Hosting a game on port 10000.");
#endif
		snprintf(line, sizeof(line), "Players connected: %d (max 4)", net.PlayerCount());
		Text(220, 203, line);
		Text(220, 223, "Press Enter to start.");
		Show();
	}
}

/* Client lobby: wait for [START]. Fills in the game settings. */
static bool WaitForStart(Net& net, int& my_id, int& players, int& difficulty, int& speed,
                         int& options, std::vector<std::string>& after_start)
{
	int lobby = 0;
	for (;;) {
		NextFrame();
		if (input.Pressed(KEY_ESC)) return false;
		std::vector<Net::Message> msgs = net.Poll();
		for (size_t i = 0; i < msgs.size(); i++) {
			const char* t = msgs[i].text.c_str();
			int a, b, c, d;
			if (sscanf(t, "[WELCOME %d]", &a) == 1) my_id = a;
			else if (sscanf(t, "[LOBBY %d]", &a) == 1) lobby = a;
			else if (sscanf(t, "[START %d %d %d %d]", &a, &b, &c, &d) == 4) {
				players = a; difficulty = b; speed = c; options = d;
				/* Game messages may arrive in the same read as START. */
				for (size_t j = i + 1; j < msgs.size(); j++) after_start.push_back(msgs[j].text);
				return my_id > 1;
			}
		}
		if (net.ServerLost()) {
			Notice("The server closed the connection.", NULL);
			return false;
		}
		clear_to_color(buffer, makecol(0, 0, 0));
		char line[80];
		snprintf(line, sizeof(line), "Connected as player %d.", my_id);
		Text(220, 183, line);
		snprintf(line, sizeof(line), "Players in lobby: %d", lobby);
		Text(220, 193, line);
		Text(220, 213, "Waiting for the host to start the game...");
		Show();
	}
}

/* Runs one game until Escape. */
static void Play(Game& game, Net* net)
{
	int fps = 0, frames = 0, second = 0;
	for (;;) {
		while (ticks == 0) rest(1);
		int pending = ticks;
		ticks = 0;
		if (pending > 5) pending = 5;  /* don't spiral after a stall */
		for (int t = 0; t < pending; t++) {
			input.Update();
			if (input.Pressed(KEY_ESC)) return;
			if (net) {
				std::vector<Net::Message> msgs = net->Poll();
				for (size_t i = 0; i < msgs.size(); i++) game.HandleMessage(msgs[i].text);
				std::vector<int> gone = net->TakeDisconnected();
				for (size_t i = 0; i < gone.size(); i++) game.PlayerLeft(gone[i]);
				if (net->ServerLost()) { game.ServerLost(); net = NULL; }
			}
			game.Tick(input);
			if (++second >= TICKS_PER_SEC) { fps = frames; frames = 0; second = 0; }
		}
		game.Draw(buffer, fps);
		Show();
		frames++;
	}
}

int main()
{
	srand((unsigned)time(NULL));
	allegro_init();
	install_keyboard();
	install_timer();
	LOCK_VARIABLE(ticks);
	LOCK_FUNCTION(Tick);
	install_int_ex(Tick, BPS_TO_TIMER(TICKS_PER_SEC));
	set_color_depth(16);
	if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, GAME_W, GAME_H, 0, 0) != 0) {
		allegro_message("Unable to set graphics mode");
		return 1;
	}
	if (install_sound(DIGI_AUTODETECT, MIDI_NONE, "") != 0)
		fprintf(stderr, "No sound device; continuing without sound.\n");
	buffer = create_bitmap(GAME_W, GAME_H);

#if WEB_BUILD
	static const char* const MODES[] = { "Single player", "Host an online game", "Join an online game" };
	char* relay_c = RelayUrlFromPage();
	std::string relay = relay_c ? relay_c : "";
	free(relay_c);
	std::string room_code;
#else
	static const char* const MODES[] = { "Single player", "Server (host a LAN game)", "Client (join a LAN game)" };
#endif
	static const char* const DIFFICULTIES[] = { "Easy", "Normal", "Hard" };
	static const char* const SPEEDS[] = { "Slow", "Normal", "Fast" };
	std::string address = "127.0.0.1";

	for (;;) {
		int mode = Menu("Enter your choice:", MODES, 3, 0);
#if WEB_BUILD
		if (mode < 0) continue;  /* a web page has nothing to quit to */
		if (mode > 0 && relay.empty()) {
			Notice("Online play isn't set up on this page.", "(No game server address.)");
			continue;
		}
#else
		if (mode < 0) break;
#endif

		Net* net = NULL;
		int my_id = 1, players = 1, difficulty = NORMAL, speed = MEDIUM, options = OPT_ALL;
		std::vector<std::string> after_start;

		if (mode == 2) {
#if WEB_BUILD
			if (!AskText("Enter the 4-digit room code and press enter:", "(digits, Backspace)", room_code, false, 4)) continue;
			net = new Net();
			clear_to_color(buffer, makecol(0, 0, 0));
			Text(220, 183, ("Joining game " + room_code + "...").c_str());
			Show();
			if (!net->JoinRelay(relay, room_code)) {
#else
			if (!AskText("Enter the server's IP address and press enter:", "(digits, '.', Backspace)", address, true, 40)) continue;
			net = new Net();
			clear_to_color(buffer, makecol(0, 0, 0));
			Text(220, 183, ("Connecting to " + address + "...").c_str());
			Show();
			if (!net->Connect(address.c_str(), PACMAN_PORT)) {
#endif
				Notice("Could not join the game.", net->Error().c_str());
				delete net;
				continue;
			}
			if (!WaitForStart(*net, my_id, players, difficulty, speed, options, after_start)) { delete net; continue; }
		} else {
			difficulty = Menu("Select the difficulty of the Game and press enter:", DIFFICULTIES, 3, NORMAL);
			if (difficulty < 0) continue;
			speed = Menu("Select the speed of the Game and press enter:", SPEEDS, 3, MEDIUM);
			if (speed < 0) continue;
			options = OptionsMenu(options, mode == 1);
			if (options < 0) continue;
			if (mode == 1) {
				net = new Net();
				clear_to_color(buffer, makecol(0, 0, 0));
				Text(220, 183, "Setting up the game...");
				Show();
#if WEB_BUILD
				if (!net->HostRelay(relay)) {
#else
				if (!net->Host(PACMAN_PORT)) {
#endif
					Notice("Could not host the game.", net->Error().c_str());
					delete net;
					continue;
				}
				players = Lobby(*net);
				if (players == 0) { delete net; continue; }
				if (players == 1) { delete net; net = NULL; }  /* nobody joined */
				else {
					net->CloseLobby();
					char start[64];
					snprintf(start, sizeof(start), "[START %d %d %d %d]", players, difficulty, speed, options);
					net->Send(start);
				}
			}
		}

		Game* game = new Game();
		if (game->Init(difficulty, speed, players, my_id, net, options)) {
			for (size_t i = 0; i < after_start.size(); i++) game->HandleMessage(after_start[i]);
			Play(*game, net);
		}
		delete game;
		delete net;  /* closing the socket tells the others we left */
	}

	destroy_bitmap(buffer);
	allegro_exit();
	return 0;
}
END_OF_MAIN()
