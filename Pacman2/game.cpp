#include "game.h"
#include "net.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Grid tiles OB 0..14, in the order the original pacman.exe lists them. */
static const char* const WALL_FILES[15] = {
	"blank", "up", "down", "up_down", "up_left", "up_right", "down_left",
	"down_right", "left", "right", "right_left", "no_up", "no_down",
	"no_right", "no_left",
};
static const char* const GHOST_NAMES[4] = { "blinky", "pinky", "inky", "clyde" };

/* Start cells on the grid shared by Grid_1/2/4. */
static const int PAC_START_COL = 9, PAC_START_ROW = 15;
static const int GHOST_START[4][2] = { { 2, 3 }, { 9, 7 }, { 16, 3 }, { 9, 3 } };

static const float PAC_SPEED[3]    = { 1.5f, 2.0f, 2.5f };    /* px per tick */
static const float GHOST_FACTOR[3] = { 0.80f, 0.92f, 1.0f };  /* vs. Pac-Man */
static const int   FRIGHT_SECS[3]  = { 8, 6, 4 };
static const float RANDOM_TURN[3]  = { 0.30f, 0.10f, 0.0f };  /* chance of a random turn */

/* Alternating scatter / chase phases, in seconds; the last chase never ends. */
static const int SCHEDULE[] = { 7, 20, 7, 20, 5 };
static const int SCHEDULE_LEN = sizeof(SCHEDULE) / sizeof(SCHEDULE[0]);

static const int SIMPLE_MARK_POINTS = 10;
static const int POWER_MARK_POINTS = 50;
static const int LIVES = 4;            /* current life + 3 spare */
static const int READY_TICKS = 2 * TICKS_PER_SEC;
static const int DEATH_FRAME_TICKS = 8;
static const int DEATH_TICKS = 6 * DEATH_FRAME_TICKS + TICKS_PER_SEC / 2;

static inline int FloorDiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

int Actor::Col() const { return FloorDiv(x + TILE / 2, TILE); }
int Actor::Row() const { return FloorDiv(y + TILE / 2, TILE); }
bool Actor::AtCenter() const { return x % TILE == 0 && y % TILE == 0; }

Game::Game()
	: net(NULL), difficulty(NORMAL), speed(MEDIUM), num_players(1), my_id(1),
	  wanted(DIR_NONE), pac_anim(0), phase(PHASE_READY), phase_ticks(0),
	  fright_ticks(0), ghost_combo(0), schedule_index(0), schedule_ticks(0),
	  scatter(true), dead_bmp(NULL), life_bmp(NULL), blue_bmp(NULL), white_bmp(NULL),
	  snd_dot(NULL), snd_ghost(NULL), snd_attack(NULL), snd_dead(NULL),
	  dot_voice(-1), attack_playing(false)
{
	memset(players, 0, sizeof(players));
}

Game::~Game() {}

static BITMAP* Load(const std::string& path)
{
	BITMAP* b = load_bitmap(path.c_str(), NULL);
	if (!b) allegro_message("Missing image %s", path.c_str());
	return b;
}

bool Game::LoadAssets()
{
	for (int i = 0; i < 15; i++)
		if (!(wall_bmp[i] = Load(std::string("./images/obstacles/") + WALL_FILES[i] + ".bmp"))) return false;
	for (int d = 0; d < 4; d++) {
		if (!(pac_bmp[d] = Load(std::string("./images/pacman/pacman_") + DIR_NAME[d] + ".bmp"))) return false;
		if (!(eyes_bmp[d] = Load(std::string("./images/ghosts/eyes_") + DIR_NAME[d] + ".bmp"))) return false;
		for (int c = 0; c < 4; c++)
			if (!(ghost_bmp[c][d] = Load(std::string("./images/ghosts/") + GHOST_NAMES[c] + "_" + DIR_NAME[d] + ".bmp"))) return false;
	}
	dead_bmp = Load("./images/pacman/dead_pacman.bmp");
	life_bmp = Load("./images/pacman/life.bmp");
	blue_bmp = Load("./images/ghosts/blue_ghost.bmp");
	white_bmp = Load("./images/ghosts/white_ghost.bmp");
	mark_bmp[0] = Load("./images/marks/simple_mark.bmp");
	mark_bmp[1] = Load("./images/marks/power_mark.bmp");
	team_mark_bmp[0] = Load("./images/marks/teammate_simple_mark.bmp");
	team_mark_bmp[1] = Load("./images/marks/teammate_power_mark.bmp");
	if (!dead_bmp || !life_bmp || !blue_bmp || !white_bmp || !mark_bmp[0] || !mark_bmp[1] ||
	    !team_mark_bmp[0] || !team_mark_bmp[1])
		return false;

	/* Sound is optional; a missing device just means silence. */
	snd_dot = load_sample("./sounds/eatdot.wav");
	snd_ghost = load_sample("./sounds/eatghost.wav");
	snd_attack = load_sample("./sounds/attack.wav");
	snd_dead = load_sample("./sounds/dead.wav");
	dot_voice = allocate_voice(snd_dot);
	return true;
}

bool Game::Init(int _difficulty, int _speed, int _players, int _my_id, Net* _net)
{
	difficulty = _difficulty;
	speed = _speed;
	num_players = _players;
	my_id = _my_id;
	net = _net;

	/* 2 players split the maze in two, 3-4 players in four. */
	int grid = num_players <= 1 ? 1 : num_players == 2 ? 2 : 4;
	char path[64];
	snprintf(path, sizeof(path), "./init/Grid/Grid_%d.txt", grid);
	if (!level.Load(path) || !LoadAssets())
		return false;

	for (int i = 1; i <= MAX_PLAYERS; i++) {
		players[i].present = i <= num_players;
		players[i].state = PS_PLAYING;
		players[i].score = 0;
		players[i].lives = LIVES;
	}
	/* In multiplayer everyone starts paused and presses P to join,
	   like the original. */
	if (num_players > 1)
		for (int i = 1; i <= num_players; i++)
			players[i].state = PS_PAUSED;

	for (int c = 0; c < 4; c++)
		AddGhost(c, my_id, GHOST_START[c][0], GHOST_START[c][1], GHOST_NORMAL);
	ResetActors();

	/* With three players the fourth territory is shared out. */
	if (net && net->IsServer() && num_players == 3)
		Redistribute(4);
	return true;
}

void Game::AddGhost(int color, int label, int col, int row, GhostMode mode)
{
	Ghost g;
	g.Place(col, row);
	g.color = color;
	g.label = label;
	g.mode = mode;
	g.start_col = col;
	g.start_row = row;
	ghosts.push_back(g);
}

void Game::ResetActors()
{
	pac.Place(PAC_START_COL, PAC_START_ROW);
	pac.dir = DIR_LEFT;
	wanted = DIR_NONE;
	pac_anim = 0;
	for (size_t i = 0; i < ghosts.size(); i++) {
		Ghost& g = ghosts[i];
		g.Place(g.start_col, g.start_row);
		/* Ghosts received from teammates start in the house. */
		g.mode = (g.start_row == level.HomeRow()) ? GHOST_LEAVING : GHOST_NORMAL;
	}
	fright_ticks = 0;
	if (attack_playing) { stop_sample(snd_attack); attack_playing = false; }
	schedule_index = 0;
	schedule_ticks = SCHEDULE[0] * TICKS_PER_SEC;
	scatter = true;
	phase = PHASE_READY;
	phase_ticks = READY_TICKS;
}

/* ---- movement ------------------------------------------------------ */

float Game::PacSpeed() const { return PAC_SPEED[speed]; }

float Game::GhostSpeed(const Ghost& g) const
{
	float base = PacSpeed() * GHOST_FACTOR[difficulty];
	switch (g.mode) {
	case GHOST_EYES: return 4.0f;
	case GHOST_FRIGHTENED: base = PacSpeed() * 0.55f; break;
	case GHOST_LEAVING: base *= 0.6f; break;
	default: break;
	}
	if (level.InTunnel(g.Col(), g.Row())) base *= 0.5f;
	return base;
}

bool Game::PacCanEnter(int col, int row) const
{
	return !level.IsWall(col, row) && !level.IsDoor(col, row);
}

bool Game::GhostCanEnter(const Ghost& g, int col, int row) const
{
	if (level.IsWall(col, row)) return false;
	if (level.IsDoor(col, row)) return g.mode == GHOST_EYES || g.mode == GHOST_LEAVING;
	return true;
}

static void Advance(Actor& a, int width)
{
	a.x += DIR_DX[a.dir];
	a.y += DIR_DY[a.dir];
	/* Wrap through the side tunnel. */
	if (a.x <= -TILE) a.x += width * TILE;
	else if (a.x >= width * TILE) a.x -= width * TILE;
}

void Game::MovePacman(const Input& in)
{
	if (in.Down(KEY_UP)) wanted = DIR_UP;
	else if (in.Down(KEY_DOWN)) wanted = DIR_DOWN;
	else if (in.Down(KEY_LEFT)) wanted = DIR_LEFT;
	else if (in.Down(KEY_RIGHT)) wanted = DIR_RIGHT;

	/* Reversing is allowed anywhere, not just on cell centres. */
	if (wanted != DIR_NONE && wanted == Opposite(pac.dir) && pac.moving) {
		pac.dir = wanted;
	}

	pac.acc += PacSpeed();
	while (pac.acc >= 1.0f) {
		pac.acc -= 1.0f;
		if (pac.AtCenter()) {
			int c = pac.Col(), r = pac.Row();
			EatAt(c, r);
			if (wanted != DIR_NONE && PacCanEnter(c + DIR_DX[wanted], r + DIR_DY[wanted]))
				pac.dir = wanted;
			if (pac.dir == DIR_NONE || !PacCanEnter(c + DIR_DX[pac.dir], r + DIR_DY[pac.dir])) {
				pac.moving = false;
				pac.acc = 0;
				break;
			}
		}
		pac.moving = true;
		Advance(pac, level.Width());
	}
	if (pac.moving) pac_anim++;
}

void Game::GhostTarget(const Ghost& g, int& tc, int& tr) const
{
	int pc = pac.Col(), pr = pac.Row();
	int pdx = pac.dir == DIR_NONE ? 0 : DIR_DX[pac.dir];
	int pdy = pac.dir == DIR_NONE ? 0 : DIR_DY[pac.dir];
	int w = level.Width(), h = level.Height();

	if (scatter) {
		/* Each colour retreats to its own corner. */
		switch (g.color) {
		case 0: tc = w - 2; tr = -3; return;
		case 1: tc = 1; tr = -3; return;
		case 2: tc = w - 1; tr = h + 1; return;
		default: tc = 0; tr = h + 1; return;
		}
	}
	switch (g.color) {
	case 1: /* pinky: four cells ahead of Pac-Man */
		tc = pc + 4 * pdx; tr = pr + 4 * pdy;
		return;
	case 2: { /* inky: mirror of the first blinky around two cells ahead */
		int bc = pc, br = pr;
		for (size_t i = 0; i < ghosts.size(); i++)
			if (ghosts[i].color == 0) { bc = ghosts[i].Col(); br = ghosts[i].Row(); break; }
		tc = 2 * (pc + 2 * pdx) - bc;
		tr = 2 * (pr + 2 * pdy) - br;
		return;
	}
	case 3: { /* clyde: chases from afar, retreats when close */
		int dc = g.Col() - pc, dr = g.Row() - pr;
		if (dc * dc + dr * dr > 64) { tc = pc; tr = pr; }
		else { tc = 0; tr = h + 1; }
		return;
	}
	default: /* blinky */
		tc = pc; tr = pr;
		return;
	}
}

Dir Game::ChooseGhostDir(Ghost& g)
{
	int c = g.Col(), r = g.Row();
	Dir options[4];
	int n = 0;
	/* Classic tie-break order: up, left, down, right. */
	static const Dir ORDER[4] = { DIR_UP, DIR_LEFT, DIR_DOWN, DIR_RIGHT };
	for (int i = 0; i < 4; i++) {
		Dir d = ORDER[i];
		if (g.dir != DIR_NONE && d == Opposite(g.dir)) continue;
		if (GhostCanEnter(g, c + DIR_DX[d], r + DIR_DY[d])) options[n++] = d;
	}
	if (n == 0) return Opposite(g.dir);  /* dead end */

	if (g.mode == GHOST_FRIGHTENED ||
	    (g.mode == GHOST_NORMAL && (float)rand() / RAND_MAX < RANDOM_TURN[difficulty]))
		return options[rand() % n];

	int tc, tr;
	if (g.mode == GHOST_EYES) { tc = level.DoorCol(); tr = level.HomeRow(); }
	else if (g.mode == GHOST_LEAVING) { tc = level.DoorCol(); tr = level.ExitRow(); }
	else GhostTarget(g, tc, tr);

	Dir best = options[0];
	int best_dist = -1;
	for (int i = 0; i < n; i++) {
		int nc = c + DIR_DX[options[i]], nr = r + DIR_DY[options[i]];
		int dist = (nc - tc) * (nc - tc) + (nr - tr) * (nr - tr);
		if (best_dist < 0 || dist < best_dist) { best_dist = dist; best = options[i]; }
	}
	return best;
}

void Game::MoveGhost(Ghost& g)
{
	g.acc += GhostSpeed(g);
	while (g.acc >= 1.0f) {
		g.acc -= 1.0f;
		if (g.AtCenter()) {
			int c = g.Col(), r = g.Row();
			if (g.mode == GHOST_EYES && c == level.DoorCol() && r == level.HomeRow()) {
				g.mode = GHOST_LEAVING;  /* revived */
				g.dir = DIR_NONE;
			} else if (g.mode == GHOST_LEAVING && c == level.DoorCol() && r == level.ExitRow()) {
				g.mode = GHOST_NORMAL;
				g.dir = DIR_NONE;
			}
			g.dir = ChooseGhostDir(g);
			if (g.dir == DIR_NONE) { g.acc = 0; break; }
		}
		g.moving = true;
		Advance(g, level.Width());
	}
}

/* ---- rules --------------------------------------------------------- */

void Game::EatAt(int col, int row)
{
	if (row < 0 || row >= level.Height()) return;
	Cell& c = level.At(col, row);
	if (!level.IsMark(c) || c.eaten || c.owner != my_id) return;
	c.eaten = true;
	int idx = level.Index(col, row);
	if (c.type == CELL_SIMPLE_MARK) {
		players[my_id].score += SIMPLE_MARK_POINTS;
		if (dot_voice >= 0 && voice_get_position(dot_voice) < 0) voice_start(dot_voice);
		Send("[EAT_SM %d %d]", idx, my_id);
	} else {
		players[my_id].score += POWER_MARK_POINTS;
		StartFright();
		Send("[EAT_PM %d %d]", idx, my_id);
	}
	Send("[SCORE %d %d]", my_id, players[my_id].score);
	CheckCompleted();
}

void Game::StartFright()
{
	fright_ticks = FRIGHT_SECS[difficulty] * TICKS_PER_SEC;
	ghost_combo = 0;
	for (size_t i = 0; i < ghosts.size(); i++) {
		Ghost& g = ghosts[i];
		if (g.mode == GHOST_NORMAL || g.mode == GHOST_FRIGHTENED) {
			if (g.mode == GHOST_NORMAL && g.dir != DIR_NONE) g.dir = Opposite(g.dir);
			g.mode = GHOST_FRIGHTENED;
		}
	}
	if (!attack_playing && snd_attack) {
		play_sample(snd_attack, 255, 128, 1000, 1);
		attack_playing = true;
	}
}

void Game::UpdateFright()
{
	if (fright_ticks <= 0) return;
	if (--fright_ticks > 0) return;
	for (size_t i = 0; i < ghosts.size(); i++)
		if (ghosts[i].mode == GHOST_FRIGHTENED) ghosts[i].mode = GHOST_NORMAL;
	if (attack_playing) { stop_sample(snd_attack); attack_playing = false; }
}

void Game::UpdateSchedule()
{
	if (fright_ticks > 0 || schedule_index >= SCHEDULE_LEN) return;
	if (--schedule_ticks > 0) return;
	schedule_index++;
	scatter = !scatter;
	if (schedule_index < SCHEDULE_LEN) schedule_ticks = SCHEDULE[schedule_index] * TICKS_PER_SEC;
	/* Ghosts turn around when the mode changes. */
	for (size_t i = 0; i < ghosts.size(); i++)
		if (ghosts[i].mode == GHOST_NORMAL && ghosts[i].dir != DIR_NONE)
			ghosts[i].dir = Opposite(ghosts[i].dir);
}

void Game::CheckCollisions()
{
	for (size_t i = 0; i < ghosts.size(); i++) {
		Ghost& g = ghosts[i];
		if (abs(g.x - pac.x) >= TILE / 2 || abs(g.y - pac.y) >= TILE / 2) continue;
		if (g.mode == GHOST_FRIGHTENED) {
			g.mode = GHOST_EYES;
			players[my_id].score += 200 << (ghost_combo < 3 ? ghost_combo : 3);
			ghost_combo++;
			if (snd_ghost) play_sample(snd_ghost, 255, 128, 1000, 0);
			Send("[SCORE %d %d]", my_id, players[my_id].score);
		} else if (g.mode == GHOST_NORMAL || g.mode == GHOST_LEAVING) {
			Die();
			return;
		}
	}
}

/* Starts the death animation. The life is taken exactly once, when the
   animation ends (the original lost every life on a single hit). */
void Game::Die()
{
	phase = PHASE_DYING;
	phase_ticks = DEATH_TICKS;
	fright_ticks = 0;
	if (attack_playing) { stop_sample(snd_attack); attack_playing = false; }
	if (snd_dead) play_sample(snd_dead, 255, 128, 1000, 0);
}

void Game::CheckCompleted()
{
	/* Single player: clear your maze. Multiplayer: the team clears
	   every territory together. */
	int left = num_players > 1 ? level.RemainingMarks(0) : level.RemainingMarks(my_id);
	if (left == 0 && phase != PHASE_GAMEOVER) {
		phase = PHASE_COMPLETED;
		if (attack_playing) { stop_sample(snd_attack); attack_playing = false; }
	}
}

bool Game::LocallyPaused() const { return players[my_id].state == PS_PAUSED; }

bool Game::AnyonePaused() const
{
	for (int i = 1; i <= MAX_PLAYERS; i++)
		if (players[i].present && players[i].state == PS_PAUSED) return true;
	return false;
}

void Game::Tick(const Input& in)
{
	if (in.Pressed(KEY_P) && phase != PHASE_GAMEOVER && phase != PHASE_COMPLETED) {
		if (num_players == 1) {
			players[my_id].state = LocallyPaused() ? PS_PLAYING : PS_PAUSED;
		} else if (LocallyPaused()) {
			players[my_id].state = PS_PLAYING;
			Send("[UNPAUSE %d]", my_id);
		} else {
			/* Pause everyone; each player rejoins with P. */
			for (int i = 1; i <= MAX_PLAYERS; i++)
				if (players[i].present && players[i].state == PS_PLAYING) players[i].state = PS_PAUSED;
			Send("[PAUSE %d]", my_id);
		}
	}

	if (num_players > 1 && phase != PHASE_GAMEOVER) {
		int next = NextPlayer();
		if (next) {
			if (in.Pressed(KEY_H)) SendGhostTo(next);
			if (in.Pressed(KEY_G)) Send("[GET_GHOST %d %d]", my_id, next);
			if (in.Pressed(KEY_D)) RequestMark("SEND_SM", my_id, next);
			if (in.Pressed(KEY_F)) RequestMark("GET_SM", my_id, next);
			if (in.Pressed(KEY_C)) RequestMark("SEND_PM", my_id, next);
			if (in.Pressed(KEY_V)) RequestMark("GET_PM", my_id, next);
		}
	}

	if (AnyonePaused() || Finished()) {
		if (Finished() && attack_playing) { stop_sample(snd_attack); attack_playing = false; }
		return;
	}

	switch (phase) {
	case PHASE_READY:
		/* Arrow keys pressed during "Ready?" are remembered. */
		if (in.Down(KEY_UP)) wanted = DIR_UP;
		else if (in.Down(KEY_DOWN)) wanted = DIR_DOWN;
		else if (in.Down(KEY_LEFT)) wanted = DIR_LEFT;
		else if (in.Down(KEY_RIGHT)) wanted = DIR_RIGHT;
		if (--phase_ticks <= 0) phase = PHASE_PLAYING;
		break;

	case PHASE_PLAYING:
		MovePacman(in);
		CheckCollisions();
		if (phase != PHASE_PLAYING) break;
		for (size_t i = 0; i < ghosts.size(); i++) MoveGhost(ghosts[i]);
		CheckCollisions();
		UpdateFright();
		UpdateSchedule();
		break;

	case PHASE_DYING:
		if (--phase_ticks > 0) break;
		players[my_id].lives--;
		Send("[LIVES %d %d]", my_id, players[my_id].lives);
		if (players[my_id].lives <= 0) {
			phase = PHASE_GAMEOVER;
			players[my_id].state = PS_GAMEOVER;
			Send("[STATE %d %d]", my_id, (int)PS_GAMEOVER);
			if (net && net->IsServer()) Redistribute(my_id);
		} else {
			ResetActors();
		}
		break;

	default:
		break;
	}
}

/* ---- multiplayer --------------------------------------------------- */

void Game::Send(const char* fmt, ...)
{
	if (!net) return;
	char buf[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	net->Send(buf);
}

/* The next player still in the game, after me, in id order. */
int Game::NextPlayer() const
{
	for (int k = 1; k < MAX_PLAYERS; k++) {
		int id = (my_id - 1 + k) % MAX_PLAYERS + 1;
		if (players[id].present && (players[id].state == PS_PLAYING || players[id].state == PS_PAUSED))
			return id;
	}
	return 0;
}

void Game::SendGhostTo(int to)
{
	/* Keep at least one ghost; never send one that is only a pair of eyes. */
	if (ghosts.size() <= 1) return;
	for (size_t i = ghosts.size(); i-- > 0; ) {
		if (ghosts[i].mode == GHOST_EYES) continue;
		Send("[GHOST %d %d %d]", to, ghosts[i].color, ghosts[i].label);
		ghosts.erase(ghosts.begin() + i);
		return;
	}
}

/* Mark transfers are decided by the server so two players can never
   claim the same mark; clients just ask. */
void Game::RequestMark(const char* what, int from, int to)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "[%s %d %d]", what, from, to);
	if (net && net->IsServer()) HandleMessage(buf);
	else Send("%s", buf);
}

void Game::MoveMark(int idx, int owner)
{
	if (idx < 0 || idx >= level.CellCount()) return;
	level.AtIndex(idx).owner = owner;
}

/* Server only: hand one uneaten mark of type `type` from `from` to `to`. */
void Game::TransferMark(CellType type, int from, int to)
{
	std::vector<int> candidates;
	for (int i = 0; i < level.CellCount(); i++) {
		const Cell& c = level.AtIndex(i);
		if (c.type == type && !c.eaten && c.owner == from) candidates.push_back(i);
	}
	if (candidates.empty()) return;
	int idx = candidates[rand() % candidates.size()];
	MoveMark(idx, to);
	Send("[MOVE_MARK %d %d]", idx, to);
	CheckCompleted();
}

/* Server only: share out a player's remaining marks among those still playing. */
void Game::Redistribute(int from)
{
	std::vector<int> heirs;
	for (int i = 1; i <= MAX_PLAYERS; i++)
		if (i != from && players[i].present && (players[i].state == PS_PLAYING || players[i].state == PS_PAUSED))
			heirs.push_back(i);
	if (heirs.empty()) return;
	size_t next = 0;
	for (int i = 0; i < level.CellCount(); i++) {
		const Cell& c = level.AtIndex(i);
		if (!level.IsMark(c) || c.eaten || c.owner != from) continue;
		int to = heirs[next++ % heirs.size()];
		MoveMark(i, to);
		Send("[MOVE_MARK %d %d]", i, to);
	}
}

void Game::HandleMessage(const std::string& msg)
{
	char cmd[32];
	int a = 0, b = 0, c = 0;
	if (sscanf(msg.c_str(), "[%31s %d %d %d", cmd, &a, &b, &c) < 1) return;
	bool server = net && net->IsServer();
	std::string m = cmd;

	if (m == "EAT_SM" || m == "EAT_PM") {
		if (a >= 0 && a < level.CellCount()) level.AtIndex(a).eaten = true;
		CheckCompleted();
	} else if (m == "MOVE_MARK") {
		MoveMark(a, b);
		CheckCompleted();
	} else if (m == "SCORE") {
		if (a >= 1 && a <= MAX_PLAYERS) players[a].score = b;
	} else if (m == "LIVES") {
		if (a >= 1 && a <= MAX_PLAYERS) players[a].lives = b;
	} else if (m == "STATE") {
		if (a >= 1 && a <= MAX_PLAYERS) players[a].state = (PlayerState)b;
		if (server && b == PS_GAMEOVER) Redistribute(a);
	} else if (m == "PAUSE") {
		for (int i = 1; i <= MAX_PLAYERS; i++)
			if (players[i].present && players[i].state == PS_PLAYING) players[i].state = PS_PAUSED;
	} else if (m == "UNPAUSE") {
		if (a >= 1 && a <= MAX_PLAYERS && players[a].state == PS_PAUSED) players[a].state = PS_PLAYING;
	} else if (m == "GHOST") {
		if (a == my_id) {
			AddGhost(b, c, level.DoorCol(), level.HomeRow(), GHOST_LEAVING);
			ghosts.back().dir = DIR_NONE;
		}
	} else if (m == "GET_GHOST") {
		if (b == my_id) SendGhostTo(a);
	} else if (server && (m == "SEND_SM" || m == "SEND_PM")) {
		TransferMark(m == "SEND_SM" ? CELL_SIMPLE_MARK : CELL_POWER_MARK, a, b);
	} else if (server && (m == "GET_SM" || m == "GET_PM")) {
		TransferMark(m == "GET_SM" ? CELL_SIMPLE_MARK : CELL_POWER_MARK, b, a);
	} else if (m == "LEFT") {
		PlayerLeft(a);
	}
}

void Game::PlayerLeft(int id)
{
	if (id < 1 || id > MAX_PLAYERS || !players[id].present) return;
	players[id].state = PS_LEFT;
	if (net && net->IsServer()) {
		Send("[LEFT %d]", id);
		Redistribute(id);
	}
}

/* Client only: the server is gone, so finish the game alone with every
   remaining mark. */
void Game::ServerLost()
{
	for (int i = 1; i <= MAX_PLAYERS; i++)
		if (i != my_id && players[i].present) players[i].state = PS_LEFT;
	if (players[my_id].state == PS_PAUSED) players[my_id].state = PS_PLAYING;
	for (int i = 0; i < level.CellCount(); i++)
		if (level.IsMark(level.AtIndex(i))) level.AtIndex(i).owner = my_id;
	net = NULL;
	CheckCompleted();
}

/* ---- drawing ------------------------------------------------------- */

void Game::DrawText(BITMAP* buf, int x, int y, const char* fmt, ...)
{
	char text[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);
	textprintf_ex(buf, font, x, y, makecol(255, 255, 255), -1, "%s", text);
}

/* Messages go below the maze so they never cover it. */
void Game::DrawCentered(BITMAP* buf, int y, const char* text)
{
	int x = ORIGIN_X + level.Width() * TILE / 2 - (int)strlen(text) * 4;
	textprintf_ex(buf, font, x, y, makecol(255, 255, 255), -1, "%s", text);
}

static void DrawSprite(BITMAP* buf, BITMAP* bmp, int sx, int sy, int x, int y)
{
	masked_blit(bmp, buf, sx, sy, ORIGIN_X + x, ORIGIN_Y + y, TILE, TILE);
}

void Game::Draw(BITMAP* buf, int fps)
{
	clear_to_color(buf, makecol(0, 0, 0));

	for (int r = 0; r < level.Height(); r++)
		for (int c = 0; c < level.Width(); c++) {
			const Cell& cell = level.At(c, r);
			int x = ORIGIN_X + c * TILE, y = ORIGIN_Y + r * TILE;
			if (cell.type == CELL_WALL) {
				/* Tile 0 is the inside of a wall block; leave it black. */
				if (cell.wall_tile > 0 && cell.wall_tile < 15)
					blit(wall_bmp[cell.wall_tile], buf, 0, 0, x, y, TILE, TILE);
			} else if (level.IsMark(cell) && !cell.eaten) {
				int kind = cell.type == CELL_POWER_MARK ? 1 : 0;
				BITMAP* b = cell.owner == my_id ? mark_bmp[kind] : team_mark_bmp[kind];
				masked_blit(b, buf, 0, 0, x + (TILE - b->w) / 2, y + (TILE - b->h) / 2, b->w, b->h);
			}
		}

	for (size_t i = 0; i < ghosts.size(); i++) {
		const Ghost& g = ghosts[i];
		int d = g.dir == DIR_NONE ? DIR_UP : g.dir;
		if (phase == PHASE_DYING) continue;
		if (g.mode == GHOST_EYES) {
			DrawSprite(buf, eyes_bmp[d], 0, 0, g.x, g.y);
			continue;
		}
		if (g.mode == GHOST_FRIGHTENED) {
			bool flash = fright_ticks < 2 * TICKS_PER_SEC && (fright_ticks / 12) % 2 == 0;
			DrawSprite(buf, flash ? white_bmp : blue_bmp, 0, 0, g.x, g.y);
		} else {
			DrawSprite(buf, ghost_bmp[g.color][d], 0, 0, g.x, g.y);
		}
		textprintf_ex(buf, font, ORIGIN_X + g.x + 8, ORIGIN_Y + g.y + 12, makecol(255, 255, 255), -1, "%d", g.label);
	}

	if (phase == PHASE_DYING) {
		int frame = (DEATH_TICKS - phase_ticks) / DEATH_FRAME_TICKS;
		if (frame < 6) DrawSprite(buf, dead_bmp, 0, frame * TILE, pac.x, pac.y);
	} else {
		static const int CYCLE[4] = { 0, 1, 2, 1 };
		int d = pac.dir == DIR_NONE ? DIR_LEFT : pac.dir;
		DrawSprite(buf, pac_bmp[d], 0, CYCLE[(pac_anim / 4) % 4] * TILE, pac.x, pac.y);
	}

	/* HUD */
	DrawText(buf, 0, 0, "Player %d", my_id);
	DrawText(buf, 0, 10, "Score %d", players[my_id].score);
	DrawText(buf, 0, 104, "FPS: %d", fps);
	for (int i = 0; i < players[my_id].lives - 1; i++)
		masked_blit(life_bmp, buf, 0, 0, 30 * i, 459, life_bmp->w, life_bmp->h);

	if (num_players > 1) {
		static const char* const STATE_NAME[] = { "Playing", "Paused", "Game Over", "Left" };
		DrawText(buf, 600, 4, "Players States");
		int y = 14;
		for (int i = 1; i <= MAX_PLAYERS; i++) {
			if (!players[i].present) continue;
			DrawText(buf, 600, y, "Player %d %-9s %d", i, STATE_NAME[players[i].state], players[i].score);
			y += 10;
		}
		DrawText(buf, 600, y + 10, "Marks left %d", level.RemainingMarks(0));
		DrawText(buf, 600, y + 20, "Yours      %d", level.RemainingMarks(my_id));
		DrawText(buf, 600, y + 30, "Ghosts     %d", (int)ghosts.size());
		DrawText(buf, 600, 540, "H/G send/get ghost");
		DrawText(buf, 600, 550, "D/F send/get dot");
		DrawText(buf, 600, 560, "C/V send/get power");
	}

	if (phase == PHASE_GAMEOVER) DrawCentered(buf, 546, "Game Over - press Escape");
	else if (phase == PHASE_COMPLETED) DrawCentered(buf, 546, "Game Completed - press Escape");
	else if (AnyonePaused())
		DrawCentered(buf, 546, LocallyPaused() && num_players > 1 ? "Game Paused - press P to rejoin" : "Game Paused");
	else if (phase == PHASE_READY)
		DrawCentered(buf, 546, phase_ticks > TICKS_PER_SEC ? "Ready?" : "Go!!!");

	DrawText(buf, 440, 590, "Press Escape to quit.");
}
