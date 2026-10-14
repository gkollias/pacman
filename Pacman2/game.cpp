#include "game.h"
#include "net.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>

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
static const int   FLEE_RADIUS[3]  = { 3, 4, 5 };             /* dots run when Pac-Man is this close */
static const float FLEE_SLOWNESS[3] = { 1.8f, 1.35f, 1.12f }; /* a power dot hops a cell this much slower
                                                                 than Pac-Man crosses one (ordinary: x2) */

/* Alternating scatter / chase phases, in seconds; the last chase never ends. */
static const int SCHEDULE[] = { 7, 20, 7, 20, 5 };
static const int SCHEDULE_LEN = sizeof(SCHEDULE) / sizeof(SCHEDULE[0]);

static const int SIMPLE_MARK_POINTS = 10;
static const int POWER_MARK_POINTS = 50;
static const int LIVES = 4;            /* current life + 3 spare */
static const int READY_TICKS = 2 * TICKS_PER_SEC;
static const int DEATH_FRAME_TICKS = 8;
static const int DEATH_TICKS = 6 * DEATH_FRAME_TICKS + TICKS_PER_SEC / 2;
static const int WALL_SLIDE_TICKS = 15;
static const int CORNER_SCAN = 5;      /* cells a cornered Pac-Man looks ahead */
static const int AUTO_SEND_COOLDOWN = 5 * TICKS_PER_SEC;
static const int REMOTE_STALE = 15;    /* ticks before a silent remote Pac-Man is hidden */
static const int HIT_COOLDOWN = 3 * TICKS_PER_SEC;

static inline int FloorDiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

int Actor::Col() const { return FloorDiv(x + TILE / 2, TILE); }
int Actor::Row() const { return FloorDiv(y + TILE / 2, TILE); }
bool Actor::AtCenter() const { return x % TILE == 0 && y % TILE == 0; }

/* Does a 24x24 box at (x, y) cover cell (col, row)? Handles the tunnel wrap. */
static bool BoxCovers(int x, int y, int col, int row, int width)
{
	if (y >= (row + 1) * TILE || y + TILE <= row * TILE) return false;
	for (int shift = -1; shift <= 1; shift++) {
		int cx = (col + shift * width) * TILE;
		if (x < cx + TILE && x + TILE > cx) return true;
	}
	return false;
}

Game::Game()
	: net(NULL), difficulty(NORMAL), speed(MEDIUM), num_players(1), my_id(1), options(OPT_ALL),
	  wanted(DIR_NONE), pac_anim(0), pac_terrain(1), remote_fright(0),
	  phase(PHASE_READY), phase_ticks(0), fright_ticks(0), ghost_combo(0),
	  schedule_index(0), schedule_ticks(0), scatter(true),
	  auto_send_cooldown(0), tick_count(0), notice_ticks(0),
	  dead_bmp(NULL), life_bmp(NULL), blue_bmp(NULL), white_bmp(NULL),
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
	if (!dead_bmp || !life_bmp || !blue_bmp || !white_bmp || !mark_bmp[0] ||
	    !mark_bmp[1] || !team_mark_bmp[0] || !team_mark_bmp[1])
		return false;

	/* Sound is optional; a missing device just means silence. */
	snd_dot = load_sample("./sounds/eatdot.wav");
	snd_ghost = load_sample("./sounds/eatghost.wav");
	snd_attack = load_sample("./sounds/attack.wav");
	snd_dead = load_sample("./sounds/dead.wav");
	dot_voice = allocate_voice(snd_dot);
	return true;
}

bool MovingWall::Blocks(int c, int r) const
{
	for (size_t k = 0; k < cols.size(); k++) {
		if ((!moved || slide > 0) && c == cols[k] && r == rows[k]) return true;
		if ((moved || slide > 0) && c == cols[k] + dx && r == rows[k] + dy) return true;
	}
	return false;
}

/* init/moving_walls.txt: "col row width height dx dy ms_at_home ms_moved"
   per block. The block must be a free-standing piece of wall, and the
   cells it moves into must be corridor. */
bool Game::LoadWalls(const std::string& path)
{
	FILE* fp = fopen(path.c_str(), "r");
	if (!fp) return false;
	char line[160];
	while (fgets(line, sizeof(line), fp)) {
		int col, row, bw, bh, dx, dy, home_ms, moved_ms;
		if (line[0] == '#' ||
		    sscanf(line, "%d %d %d %d %d %d %d %d", &col, &row, &bw, &bh, &dx, &dy, &home_ms, &moved_ms) != 8 ||
		    bw < 1 || bh < 1 || (dx == 0 && dy == 0))
			continue;
		MovingWall w;
		bool ok = true;
		for (int r = row; r < row + bh; r++)
			for (int c = col; c < col + bw; c++) {
				if (!level.IsWall(c, r)) ok = false;
				w.cols.push_back(c);
				w.rows.push_back(r);
				w.tiles.push_back(level.IsWall(c, r) ? level.At(c, r).wall_tile : 0);
			}
		for (size_t k = 0; k < w.cols.size() && ok; k++) {
			int c = w.cols[k] + dx, r = w.rows[k] + dy;
			bool own = c >= col && c < col + bw && r >= row && r < row + bh;
			if (!own && (level.IsWall(c, r) || level.IsDoor(c, r) || level.InHouse(c, r))) ok = false;
		}
		if (!ok) {
			fprintf(stderr, "%s: ignoring block at %d,%d (not free-standing or no room to move)\n", path.c_str(), col, row);
			continue;
		}
		for (size_t k = 0; k < w.cols.size(); k++) level.ClearWall(w.cols[k], w.rows[k]);
		w.dx = dx;
		w.dy = dy;
		w.moved = false;
		w.slide = 0;
		w.home_ticks = home_ms * TICKS_PER_SEC / 1000;
		w.moved_ticks = moved_ms * TICKS_PER_SEC / 1000;
		/* Stagger the blocks so they don't all move together. */
		w.wait = w.home_ticks / 2 + (int)walls.size() * TICKS_PER_SEC * 7 / 10;
		walls.push_back(w);
	}
	fclose(fp);
	return true;
}

bool Game::Init(int _difficulty, int _speed, int _players, int _my_id, Net* _net, int _options)
{
	difficulty = _difficulty;
	speed = _speed;
	num_players = _players;
	my_id = _my_id;
	net = _net;
	options = _options;
	pac_terrain = my_id;

	/* 2 players split the maze in two, 3-4 players in four. */
	int grid = num_players <= 1 ? 1 : num_players == 2 ? 2 : 4;
	char path[64];
	snprintf(path, sizeof(path), "./init/Grid/Grid_%d.txt", grid);
	if (!level.Load(path) || !LoadAssets())
		return false;
	if (Opt(OPT_MOVING_WALLS)) LoadWalls("./init/moving_walls.txt");

	for (int i = 1; i <= MAX_PLAYERS; i++) {
		players[i].present = i <= num_players;
		players[i].state = PS_PLAYING;
		players[i].score = 0;
		players[i].lives = LIVES;
	}
	/* In multiplayer everyone starts paused and presses P to join,
	   like the original. */
	if (Multiplayer())
		for (int i = 1; i <= num_players; i++)
			players[i].state = PS_PAUSED;

	for (int c = 0; c < 4; c++)
		AddGhost(c, my_id, GHOST_START[c][0], GHOST_START[c][1], GHOST_NORMAL);
	ResetActors();
	dot_cooldown.assign(level.MarkCount(), 0);

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
	ReturnHome();
	pac.Place(PAC_START_COL, PAC_START_ROW);
	pac.dir = DIR_LEFT;
	wanted = DIR_NONE;
	pac_anim = 0;
	for (size_t i = 0; i < ghosts.size(); i++) {
		Ghost& g = ghosts[i];
		/* Ghosts that arrived from elsewhere restart in the house. */
		if (level.IsWall(g.start_col, g.start_row) || level.InTunnel(g.start_col, g.start_row)) {
			g.start_col = level.DoorCol();
			g.start_row = level.HomeRow();
		}
		g.Place(g.start_col, g.start_row);
		g.mode = level.InHouse(g.start_col, g.start_row) ? GHOST_LEAVING : GHOST_NORMAL;
	}
	fright_ticks = 0;
	if (attack_playing) { stop_sample(snd_attack); attack_playing = false; }
	schedule_index = 0;
	schedule_ticks = SCHEDULE[0] * TICKS_PER_SEC;
	scatter = true;
	phase = PHASE_READY;
	phase_ticks = READY_TICKS;
}

void Game::Notify(const std::string& text)
{
	notice = text;
	notice_ticks = 3 * TICKS_PER_SEC;
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

bool Game::WallAt(const std::vector<MovingWall>& ws, int col, int row) const
{
	col = level.WrapCol(col);
	for (size_t i = 0; i < ws.size(); i++)
		if (ws[i].Blocks(col, row)) return true;
	return false;
}

bool Game::PacCanEnter(int col, int row) const
{
	if (level.IsWall(col, row) || level.IsDoor(col, row)) return false;
	/* In someone else's maze, their moving walls are the ones in the way. */
	return !WallAt(Away() ? remote_walls : walls, col, row);
}

bool Game::GhostCanEnter(const Ghost& g, int col, int row) const
{
	if (level.IsWall(col, row) || WallAt(walls, col, row)) return false;
	if (level.IsDoor(col, row)) return g.mode == GHOST_EYES || g.mode == GHOST_LEAVING;
	return true;
}

/* Moves one pixel. Returns +1/-1 when the actor wrapped out of the
   right/left side of the maze, 0 otherwise. */
static int Advance(Actor& a, int width)
{
	a.x += DIR_DX[a.dir];
	a.y += DIR_DY[a.dir];
	if (a.x <= -TILE) { a.x += width * TILE; return -1; }
	if (a.x >= width * TILE) { a.x -= width * TILE; return 1; }
	return 0;
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
		int side = Advance(pac, level.Width());
		/* The side tunnel leads into the neighbouring player's maze. */
		if (side && Multiplayer() && Opt(OPT_TERRAINS)) {
			int to = NeighbourTerrain(pac_terrain, side);
			if (to && to != pac_terrain) EnterTerrain(to);
		}
	}
	if (pac.moving) pac_anim++;
}

/* The closest Pac-Man in my maze: mine if it is home, or a visitor's. */
bool Game::NearestPacman(const Ghost& g, int& col, int& row, Dir& dir) const
{
	int best = -1;
	if (!Away()) {
		col = pac.Col(); row = pac.Row(); dir = pac.dir;
		best = (col - g.Col()) * (col - g.Col()) + (row - g.Row()) * (row - g.Row());
	}
	for (std::map<int, RemotePac>::const_iterator it = remote_pacs.begin(); it != remote_pacs.end(); ++it) {
		const RemotePac& r = it->second;
		if (r.terrain != my_id || r.age > REMOTE_STALE) continue;
		int c = FloorDiv(r.x + TILE / 2, TILE), rr = FloorDiv(r.y + TILE / 2, TILE);
		int d = (c - g.Col()) * (c - g.Col()) + (rr - g.Row()) * (rr - g.Row());
		if (best < 0 || d < best) { best = d; col = c; row = rr; dir = r.dir; }
	}
	return best >= 0;
}

void Game::GhostTarget(const Ghost& g, int& tc, int& tr) const
{
	int w = level.Width(), h = level.Height();
	int pc = 0, pr = 0;
	Dir pd = DIR_NONE;
	bool hunting = NearestPacman(g, pc, pr, pd);

	if (scatter || !hunting) {
		/* Each colour retreats to its own corner. */
		switch (g.color) {
		case 0: tc = w - 2; tr = -3; return;
		case 1: tc = 1; tr = -3; return;
		case 2: tc = w - 1; tr = h + 1; return;
		default: tc = 0; tr = h + 1; return;
		}
	}
	int pdx = pd == DIR_NONE ? 0 : DIR_DX[pd];
	int pdy = pd == DIR_NONE ? 0 : DIR_DY[pd];
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
	Dir options_[4];
	int n = 0;
	/* Classic tie-break order: up, left, down, right. */
	static const Dir ORDER[4] = { DIR_UP, DIR_LEFT, DIR_DOWN, DIR_RIGHT };
	for (int i = 0; i < 4; i++) {
		Dir d = ORDER[i];
		if (g.dir != DIR_NONE && d == Opposite(g.dir)) continue;
		if (GhostCanEnter(g, c + DIR_DX[d], r + DIR_DY[d])) options_[n++] = d;
	}
	if (n == 0) {
		/* Dead end, or boxed in by a moving wall: turn back if possible. */
		Dir back = Opposite(g.dir);
		if (back != DIR_NONE && GhostCanEnter(g, c + DIR_DX[back], r + DIR_DY[back])) return back;
		return DIR_NONE;
	}

	if (g.mode == GHOST_FRIGHTENED ||
	    (g.mode == GHOST_NORMAL && (float)rand() / RAND_MAX < RANDOM_TURN[difficulty]))
		return options_[rand() % n];

	int tc, tr;
	if (g.mode == GHOST_EYES) { tc = level.DoorCol(); tr = level.HomeRow(); }
	else if (g.mode == GHOST_LEAVING) { tc = level.DoorCol(); tr = level.ExitRow(); }
	else GhostTarget(g, tc, tr);

	Dir best = options_[0];
	int best_dist = -1;
	for (int i = 0; i < n; i++) {
		int nc = c + DIR_DX[options_[i]], nr = r + DIR_DY[options_[i]];
		int dist = (nc - tc) * (nc - tc) + (nr - tr) * (nr - tr);
		if (best_dist < 0 || dist < best_dist) { best_dist = dist; best = options_[i]; }
	}
	return best;
}

void Game::MoveGhost(size_t i)
{
	Ghost& g = ghosts[i];
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
			/* Is the next cell blocked by a wall that slid in? Wait. */
			if (!GhostCanEnter(g, c + DIR_DX[g.dir], r + DIR_DY[g.dir])) { g.acc = 0; break; }
		}
		g.moving = true;
		int side = Advance(g, level.Width());
		/* A hunting ghost that leaves through the tunnel wanders into the
		   neighbouring maze (always keeping one at home). */
		if (side && Multiplayer() && Opt(OPT_TERRAINS) && g.mode == GHOST_NORMAL && ghosts.size() > 1) {
			int to = NeighbourTerrain(my_id, side);
			if (to && to != my_id) {
				SendGhost(i, to, g.Col(), g.Row(), g.dir);
				return;
			}
		}
	}
}

/* Is any actor in my maze in this cell? */
bool Game::CellOccupied(int col, int row) const
{
	int w = level.Width();
	if (!Away() && BoxCovers(pac.x, pac.y, col, row, w)) return true;
	for (size_t i = 0; i < ghosts.size(); i++)
		if (BoxCovers(ghosts[i].x, ghosts[i].y, col, row, w)) return true;
	for (std::map<int, RemotePac>::const_iterator it = remote_pacs.begin(); it != remote_pacs.end(); ++it)
		if (it->second.terrain == my_id && it->second.age <= REMOTE_STALE &&
		    BoxCovers(it->second.x, it->second.y, col, row, w))
			return true;
	return false;
}

/* Blocks slide away and back on a timer. A block only moves when the
   cells it is about to take are empty; otherwise it waits (it never
   crushes anyone). */
void Game::MoveWalls()
{
	for (size_t i = 0; i < walls.size(); i++) {
		MovingWall& wl = walls[i];
		if (wl.slide > 0) { wl.slide--; continue; }
		if (wl.wait > 0) { wl.wait--; continue; }
		/* Cells it will take: the moved position when leaving, home when returning. */
		int sx = wl.moved ? 0 : wl.dx, sy = wl.moved ? 0 : wl.dy;
		bool blocked = false;
		for (size_t k = 0; k < wl.cols.size() && !blocked; k++)
			if (CellOccupied(wl.cols[k] + sx, wl.rows[k] + sy)) blocked = true;
		if (blocked) {
			wl.wait = TICKS_PER_SEC / 4;  /* someone is in the way: try again shortly */
			continue;
		}
		wl.moved = !wl.moved;
		wl.slide = WALL_SLIDE_TICKS;
		wl.wait = wl.moved ? wl.moved_ticks : wl.home_ticks;
	}
}

/* My dots run away from my Pac-Man when it comes close: each hops to the
   neighbouring corridor cell that is furthest from it. Power dots push
   past ordinary dots (swapping places); ordinary dots only move into
   cleared cells, so they get slippery once the maze thins out. Only the
   owner moves its dots, and tells everyone with [MOVE_DOT]. */
void Game::FleeDots()
{
	if (!Opt(OPT_MOVING_DOTS)) return;
	int pc = level.WrapCol(pac.Col()), pr = pac.Row();
	int w = level.Width();
	for (int id = 0; id < level.MarkCount(); id++) {
		if (dot_cooldown[id] > 0) { dot_cooldown[id]--; continue; }
		Mark& m = level.GetMark(id);
		if (m.eaten || m.owner != my_id) continue;
		int dc = abs(m.col - pc), dist = (dc < w - dc ? dc : w - dc) + abs(m.row - pr);
		if (dist == 0 || dist > FLEE_RADIUS[difficulty]) continue;

		/* Neighbouring corridor cells strictly further from Pac-Man. */
		int cand[4][3], n = 0, best_dist = dist;
		for (int d = 0; d < 4; d++) {
			int c = level.WrapCol(m.col + DIR_DX[d]), r = m.row + DIR_DY[d];
			if (!level.IsTrack(c, r) || (c == pc && r == pr) || WallAt(Away() ? remote_walls : walls, c, r)) continue;
			int other = level.MarkAt(c, r);
			if (other >= 0 && (m.type != CELL_POWER_MARK || level.GetMark(other).type != CELL_SIMPLE_MARK))
				continue;
			int cc = abs(c - pc), nd = (cc < w - cc ? cc : w - cc) + abs(r - pr);
			if (nd <= dist || nd < best_dist) continue;
			if (nd > best_dist) { best_dist = nd; n = 0; }
			cand[n][0] = c; cand[n][1] = r; cand[n][2] = other; n++;
		}
		int pick = n > 0 ? rand() % n : -1;
		int best_col = pick >= 0 ? cand[pick][0] : -1, best_row = pick >= 0 ? cand[pick][1] : -1;
		int best_other = pick >= 0 ? cand[pick][2] : -1;
		if (best_col < 0) continue;  /* cornered: catch it! */

		if (best_other >= 0) {
			Mark& o = level.GetMark(best_other);
			o.col = m.col;
			o.row = m.row;
			Send("[MOVE_DOT %d %d %d]", best_other, o.col, o.row);
		}
		m.col = best_col;
		m.row = best_row;
		Send("[MOVE_DOT %d %d %d]", id, m.col, m.row);
		float cell_ticks = TILE / PacSpeed();
		dot_cooldown[id] = (int)(cell_ticks * FLEE_SLOWNESS[difficulty] * (m.type == CELL_POWER_MARK ? 1 : 2));
	}
}

/* ---- rules --------------------------------------------------------- */

void Game::EatAt(int col, int row)
{
	int id = level.MarkAt(col, row);
	if (id < 0) return;
	Mark& m = level.GetMark(id);
	if (m.owner != my_id) return;
	m.eaten = true;
	if (m.type == CELL_SIMPLE_MARK) {
		players[my_id].score += SIMPLE_MARK_POINTS;
		if (dot_voice >= 0 && voice_get_position(dot_voice) < 0) voice_start(dot_voice);
		Send("[EAT_SM %d %d]", id, my_id);
	} else {
		players[my_id].score += POWER_MARK_POINTS;
		/* It frightens the ghosts of the maze I am in. */
		if (!Away()) StartFright();
		Send("[EAT_PM %d %d %d]", id, my_id, pac_terrain);
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

static int GhostPoints(int combo) { return 200 << (combo < 3 ? combo : 3); }

/* My Pac-Man against my own ghosts (only while it is at home). */
void Game::CheckCollisions()
{
	if (Away()) return;
	for (size_t i = 0; i < ghosts.size(); i++) {
		Ghost& g = ghosts[i];
		if (abs(g.x - pac.x) >= TILE / 2 || abs(g.y - pac.y) >= TILE / 2) continue;
		if (g.mode == GHOST_FRIGHTENED) {
			g.mode = GHOST_EYES;
			players[my_id].score += GhostPoints(ghost_combo++);
			if (snd_ghost) play_sample(snd_ghost, 255, 128, 1000, 0);
			Send("[SCORE %d %d]", my_id, players[my_id].score);
		} else if (g.mode == GHOST_NORMAL || g.mode == GHOST_LEAVING) {
			Die();
			return;
		}
	}
}

/* Host side: visiting Pac-Men against my ghosts. The visitor learns the
   outcome from [PAC_HIT] or [GHOST_EATEN]. */
void Game::CheckVisitorCollisions()
{
	for (std::map<int, RemotePac>::iterator it = remote_pacs.begin(); it != remote_pacs.end(); ++it) {
		const RemotePac& r = it->second;
		int id = it->first;
		if (r.terrain != my_id || r.age > REMOTE_STALE || hit_cooldown[id] > 0) continue;
		for (size_t i = 0; i < ghosts.size(); i++) {
			Ghost& g = ghosts[i];
			if (abs(g.x - r.x) >= TILE / 2 || abs(g.y - r.y) >= TILE / 2) continue;
			if (g.mode == GHOST_FRIGHTENED) {
				g.mode = GHOST_EYES;
				Send("[GHOST_EATEN %d %d]", id, GhostPoints(ghost_combo++));
			} else if (g.mode == GHOST_NORMAL || g.mode == GHOST_LEAVING) {
				Send("[PAC_HIT %d]", id);
				hit_cooldown[id] = HIT_COOLDOWN;
				break;
			}
		}
	}
}

/* Ghosts closing in from every way out: send the nearest one to the
   next player. */
void Game::CheckCornered()
{
	if (!Multiplayer() || !Opt(OPT_AUTO_SEND) || Away() || auto_send_cooldown > 0) return;
	if (ghosts.size() <= 1) return;
	int to = NextPlayer();
	if (!to) return;
	int c = pac.Col(), r = pac.Row();
	int open = 0, threatened = 0, nearest = -1, nearest_dist = 0;
	for (int d = 0; d < 4; d++) {
		if (!PacCanEnter(c + DIR_DX[d], r + DIR_DY[d])) continue;
		open++;
		bool hit = false;
		for (int k = 1; k <= CORNER_SCAN && !hit; k++) {
			int cc = level.WrapCol(c + k * DIR_DX[d]), rr = r + k * DIR_DY[d];
			if (k > 1 && !PacCanEnter(cc, rr)) break;
			for (size_t i = 0; i < ghosts.size(); i++) {
				const Ghost& g = ghosts[i];
				if ((g.mode != GHOST_NORMAL && g.mode != GHOST_LEAVING) ||
				    level.WrapCol(g.Col()) != cc || g.Row() != rr) continue;
				hit = true;
				if (nearest < 0 || k < nearest_dist) { nearest = (int)i; nearest_dist = k; }
			}
		}
		if (hit) threatened++;
	}
	if (open == 0 || threatened < open || nearest < 0) return;
	SendGhost(nearest, to, level.DoorCol(), level.HomeRow(), DIR_NONE);
	auto_send_cooldown = AUTO_SEND_COOLDOWN;
	char text[64];
	snprintf(text, sizeof(text), "Cornered! A ghost went to player %d", to);
	Notify(text);
}

/* Starts the death animation. The life is taken exactly once, when the
   animation ends (the original lost every life on a single hit). */
void Game::Die()
{
	phase = PHASE_DYING;
	phase_ticks = DEATH_TICKS;
	if (!Away()) fright_ticks = 0;
	if (attack_playing) { stop_sample(snd_attack); attack_playing = false; }
	if (snd_dead) play_sample(snd_dead, 255, 128, 1000, 0);
}

void Game::CheckCompleted()
{
	/* Single player: clear your maze. Multiplayer: the team clears
	   every territory together. */
	int left = Multiplayer() ? level.RemainingMarks(0) : level.RemainingMarks(my_id);
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
		if (!Multiplayer()) {
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

	if (Multiplayer() && phase != PHASE_GAMEOVER) {
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

	tick_count++;
	if (notice_ticks > 0) notice_ticks--;
	if (auto_send_cooldown > 0) auto_send_cooldown--;
	for (std::map<int, RemotePac>::iterator it = remote_pacs.begin(); it != remote_pacs.end(); ++it)
		it->second.age++;
	for (std::map<int, int>::iterator it = hit_cooldown.begin(); it != hit_cooldown.end(); ++it)
		if (it->second > 0) it->second--;
	/* The maze I am visiting has emptied: go home. */
	if (Away() && !Active(pac_terrain)) {
		ReturnHome();
		pac.Place(PAC_START_COL, PAC_START_ROW);
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
		if (Opt(OPT_MOVING_WALLS)) MoveWalls();
		MovePacman(in);
		CheckCollisions();
		if (phase != PHASE_PLAYING) break;
		for (size_t i = ghosts.size(); i-- > 0; ) MoveGhost(i);
		CheckCollisions();
		CheckVisitorCollisions();
		if (phase != PHASE_PLAYING) break;
		if (pac.AtCenter()) CheckCornered();
		UpdateFright();
		UpdateSchedule();
		FleeDots();
		break;

	case PHASE_DYING:
		if (--phase_ticks > 0) break;
		players[my_id].lives--;
		Send("[LIVES %d %d]", my_id, players[my_id].lives);
		if (players[my_id].lives <= 0) {
			ReturnHome();
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

	if (Multiplayer() && Opt(OPT_TERRAINS)) {
		if (Away()) SendPosition();
		/* My maze keeps running while I'm away; visitors still need to see it. */
		if (tick_count % 2 == 0) SendSnapshot();
	}
}

/* ---- multiplayer --------------------------------------------------- */

void Game::Send(const char* fmt, ...)
{
	if (!net) return;
	char buf[2048];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	net->Send(buf);
}

bool Game::Active(int id) const
{
	return id >= 1 && id <= MAX_PLAYERS && players[id].present &&
	       (players[id].state == PS_PLAYING || players[id].state == PS_PAUSED);
}

/* The next player still in the game, after me, in id order. */
int Game::NextPlayer() const
{
	int to = NeighbourTerrain(my_id, 1);
	return to == my_id ? 0 : to;
}

/* Players sit in a ring by id; step +1 is to the right, -1 to the left.
   Returns 0 when nobody else is in the game. */
int Game::NeighbourTerrain(int from, int step) const
{
	for (int k = 1; k <= MAX_PLAYERS; k++) {
		int id = ((from - 1 + step * k) % MAX_PLAYERS + MAX_PLAYERS) % MAX_PLAYERS + 1;
		if (id == from) break;
		if (Active(id)) return id;
	}
	return 0;
}

void Game::SendGhost(size_t i, int to, int col, int row, Dir dir)
{
	const Ghost& g = ghosts[i];
	Send("[GHOST %d %d %d %d %d %d]", to, g.color, g.label, level.WrapCol(col), row, (int)dir);
	ghosts.erase(ghosts.begin() + i);
}

void Game::SendGhostTo(int to)
{
	/* Keep at least one ghost; never send one that is only a pair of eyes. */
	if (ghosts.size() <= 1) return;
	for (size_t i = ghosts.size(); i-- > 0; ) {
		if (ghosts[i].mode == GHOST_EYES) continue;
		SendGhost(i, to, level.DoorCol(), level.HomeRow(), DIR_NONE);
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

void Game::MoveMark(int id, int owner)
{
	if (level.ValidMark(id)) level.GetMark(id).owner = owner;
}

/* Server only: hand one uneaten mark of type `type` from `from` to `to`. */
void Game::TransferMark(CellType type, int from, int to)
{
	std::vector<int> candidates;
	for (int i = 0; i < level.MarkCount(); i++) {
		const Mark& m = level.GetMark(i);
		if (m.type == type && !m.eaten && m.owner == from) candidates.push_back(i);
	}
	if (candidates.empty()) return;
	int id = candidates[rand() % candidates.size()];
	MoveMark(id, to);
	Send("[MOVE_MARK %d %d]", id, to);
	CheckCompleted();
}

/* Server only: share out a player's remaining marks among those still playing. */
void Game::Redistribute(int from)
{
	std::vector<int> heirs;
	for (int i = 1; i <= MAX_PLAYERS; i++)
		if (i != from && Active(i)) heirs.push_back(i);
	if (heirs.empty()) return;
	size_t next = 0;
	for (int i = 0; i < level.MarkCount(); i++) {
		const Mark& m = level.GetMark(i);
		if (m.eaten || m.owner != from) continue;
		int to = heirs[next++ % heirs.size()];
		MoveMark(i, to);
		Send("[MOVE_MARK %d %d]", i, to);
	}
}

void Game::EnterTerrain(int terrain)
{
	pac_terrain = terrain;
	remote_ghosts.clear();
	remote_walls.clear();
	remote_fright = 0;
	Send("[PAC_TERRAIN %d %d]", my_id, terrain);
	char text[64];
	if (terrain == my_id) snprintf(text, sizeof(text), "Back in your own maze");
	else snprintf(text, sizeof(text), "You are in player %d's maze", terrain);
	Notify(text);
}

void Game::ReturnHome()
{
	if (!Away()) return;
	pac_terrain = my_id;
	remote_ghosts.clear();
	remote_walls.clear();
	Send("[PAC_TERRAIN %d %d]", my_id, my_id);
}

/* Visitor: tell the host where my Pac-Man is. */
void Game::SendPosition()
{
	Send("[PAC_POS %d %d %d %d %d %d]", my_id, pac_terrain, pac.x, pac.y, (int)pac.dir, pac_anim);
}

/* Host: show visitors my ghosts, walls and Pac-Man. */
void Game::SendSnapshot()
{
	bool visited = false;
	for (std::map<int, RemotePac>::const_iterator it = remote_pacs.begin(); it != remote_pacs.end(); ++it)
		if (it->second.terrain == my_id && it->second.age <= REMOTE_STALE) visited = true;
	if (!visited) return;

	std::ostringstream s;
	s << "[SNAP " << my_id << " " << fright_ticks << " " << walls.size();
	for (size_t i = 0; i < walls.size(); i++) {
		s << " " << (int)walls[i].moved << " " << walls[i].slide;
	}
	s << " " << ghosts.size();
	for (size_t i = 0; i < ghosts.size(); i++) {
		const Ghost& g = ghosts[i];
		s << " " << g.x << " " << g.y << " " << (int)g.dir << " " << (int)g.mode << " " << g.color << " " << g.label;
	}
	s << "]";
	Send("%s", s.str().c_str());
	if (!Away() && phase != PHASE_DYING) Send("[PAC_POS %d %d %d %d %d %d]", my_id, my_id, pac.x, pac.y, (int)pac.dir, pac_anim);
}

void Game::ReadSnapshot(const std::vector<int>& a)
{
	/* a: terrain fright nwalls {moved slide}* nghosts {x y dir mode color label}*
	   Every maze has the same blocks (init/moving_walls.txt); only their
	   positions are sent. */
	size_t k = 1;
	if (a.size() < 3) return;
	remote_fright = a[k++];
	int nw = a[k++];
	remote_walls = walls;
	for (int i = 0; i < nw && k + 2 <= a.size(); i++, k += 2) {
		if (i >= (int)remote_walls.size()) continue;
		remote_walls[i].moved = a[k] != 0;
		remote_walls[i].slide = a[k + 1];
		remote_walls[i].wait = 0;
	}
	if (k >= a.size()) return;
	int ng = a[k++];
	remote_ghosts.clear();
	for (int i = 0; i < ng && k + 6 <= a.size(); i++) {
		Ghost g;
		memset(&g, 0, sizeof(g));
		g.x = a[k++]; g.y = a[k++]; g.dir = (Dir)a[k++]; g.mode = (GhostMode)a[k++];
		g.color = a[k++]; g.label = a[k++];
		remote_ghosts.push_back(g);
	}
}

void Game::HandleMessage(const std::string& msg)
{
	/* "[CMD n n n ...]" */
	std::string body = msg;
	if (!body.empty() && body[0] == '[') body.erase(0, 1);
	if (!body.empty() && body[body.size() - 1] == ']') body.erase(body.size() - 1);
	std::istringstream in(body);
	std::string m;
	in >> m;
	std::vector<int> v;
	int x;
	while (in >> x) v.push_back(x);
	while (v.size() < 7) v.push_back(0);
	int a = v[0], b = v[1], c = v[2];
	bool server = net && net->IsServer();

	if (m == "EAT_SM" || m == "EAT_PM") {
		if (level.ValidMark(a)) level.GetMark(a).eaten = true;
		/* A visitor ate a power dot in my maze: my ghosts run. */
		if (m == "EAT_PM" && c == my_id && b != my_id) StartFright();
		CheckCompleted();
	} else if (m == "MOVE_MARK") {
		MoveMark(a, b);
		CheckCompleted();
	} else if (m == "MOVE_DOT") {
		if (level.ValidMark(a)) { level.GetMark(a).col = b; level.GetMark(a).row = c; }
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
		/* [GHOST to color label col row dir] */
		if (a == my_id) {
			int col = v[3], row = v[4];
			Dir dir = (Dir)v[5];
			bool house = level.InHouse(col, row);
			AddGhost(b, c, col, row, house ? GHOST_LEAVING : GHOST_NORMAL);
			Ghost& g = ghosts.back();
			g.dir = house ? DIR_NONE : dir;
			char text[64];
			snprintf(text, sizeof(text), house ? "Player %d sent you a ghost" : "A ghost wandered in from player %d", c);
			Notify(text);
		}
	} else if (m == "GET_GHOST") {
		if (b == my_id) SendGhostTo(a);
	} else if (server && (m == "SEND_SM" || m == "SEND_PM")) {
		TransferMark(m == "SEND_SM" ? CELL_SIMPLE_MARK : CELL_POWER_MARK, a, b);
	} else if (server && (m == "GET_SM" || m == "GET_PM")) {
		TransferMark(m == "GET_SM" ? CELL_SIMPLE_MARK : CELL_POWER_MARK, b, a);
	} else if (m == "PAC_TERRAIN") {
		if (a != my_id) {
			remote_pacs[a].terrain = b;
			if (b == my_id) {
				remote_pacs[a].age = REMOTE_STALE + 1;  /* hidden until its first position */
				char text[64];
				snprintf(text, sizeof(text), "Player %d entered your maze", a);
				Notify(text);
			}
		}
	} else if (m == "PAC_POS") {
		if (a != my_id) {
			RemotePac& r = remote_pacs[a];
			r.terrain = b; r.x = c; r.y = v[3]; r.dir = (Dir)v[4]; r.anim = v[5]; r.age = 0;
		}
	} else if (m == "SNAP") {
		if (Away() && a == pac_terrain) ReadSnapshot(v);
	} else if (m == "PAC_HIT") {
		if (a == my_id && Away() && phase == PHASE_PLAYING) Die();
	} else if (m == "GHOST_EATEN") {
		if (a == my_id) {
			players[my_id].score += b;
			if (snd_ghost) play_sample(snd_ghost, 255, 128, 1000, 0);
			Send("[SCORE %d %d]", my_id, players[my_id].score);
		}
	} else if (m == "LEFT") {
		PlayerLeft(a);
	}
}

void Game::PlayerLeft(int id)
{
	if (id < 1 || id > MAX_PLAYERS || !players[id].present) return;
	players[id].state = PS_LEFT;
	remote_pacs.erase(id);
	if (net && net->IsServer()) {
		Send("[LEFT %d]", id);
		Redistribute(id);
	}
}

/* Client only: the server is gone, so finish the game alone with every
   remaining mark. */
void Game::ServerLost()
{
	if (Away()) { ReturnHome(); pac.Place(PAC_START_COL, PAC_START_ROW); }
	for (int i = 1; i <= MAX_PLAYERS; i++)
		if (i != my_id && players[i].present) players[i].state = PS_LEFT;
	if (players[my_id].state == PS_PAUSED) players[my_id].state = PS_PLAYING;
	for (int i = 0; i < level.MarkCount(); i++) level.GetMark(i).owner = my_id;
	remote_pacs.clear();
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

void Game::DrawPac(BITMAP* buf, int x, int y, Dir dir, int anim)
{
	static const int CYCLE[4] = { 0, 1, 2, 1 };
	int d = dir == DIR_NONE ? DIR_LEFT : dir;
	DrawSprite(buf, pac_bmp[d], 0, CYCLE[(anim / 4) % 4] * TILE, x, y);
}

void Game::DrawGhosts(BITMAP* buf, const std::vector<Ghost>& gs, int fright)
{
	for (size_t i = 0; i < gs.size(); i++) {
		const Ghost& g = gs[i];
		int d = g.dir == DIR_NONE ? DIR_UP : g.dir;
		if (g.mode == GHOST_EYES) {
			DrawSprite(buf, eyes_bmp[d], 0, 0, g.x, g.y);
			continue;
		}
		if (g.mode == GHOST_FRIGHTENED) {
			bool flash = fright < 2 * TICKS_PER_SEC && (fright / 12) % 2 == 0;
			DrawSprite(buf, flash ? white_bmp : blue_bmp, 0, 0, g.x, g.y);
		} else {
			DrawSprite(buf, ghost_bmp[g.color & 3][d], 0, 0, g.x, g.y);
		}
		textprintf_ex(buf, font, ORIGIN_X + g.x + 8, ORIGIN_Y + g.y + 12, makecol(255, 255, 255), -1, "%d", g.label);
	}
}

void Game::DrawWalls(BITMAP* buf, const std::vector<MovingWall>& ws)
{
	for (size_t i = 0; i < ws.size(); i++) {
		const MovingWall& w = ws[i];
		/* How far it has slid towards its moved position, in sixteenths. */
		int out = w.moved ? 16 - 16 * w.slide / WALL_SLIDE_TICKS : 16 * w.slide / WALL_SLIDE_TICKS;
		int ox = w.dx * TILE * out / 16, oy = w.dy * TILE * out / 16;
		for (size_t k = 0; k < w.cols.size(); k++) {
			int t = w.tiles[k];
			if (t > 0 && t < 15)
				blit(wall_bmp[t], buf, 0, 0, ORIGIN_X + w.cols[k] * TILE + ox, ORIGIN_Y + w.rows[k] * TILE + oy, TILE, TILE);
		}
	}
}

void Game::Draw(BITMAP* buf, int fps)
{
	clear_to_color(buf, makecol(0, 0, 0));

	for (int r = 0; r < level.Height(); r++)
		for (int c = 0; c < level.Width(); c++) {
			const Cell& cell = level.At(c, r);
			/* Tile 0 is the inside of a wall block; leave it black. */
			if (cell.type == CELL_WALL && cell.wall_tile > 0 && cell.wall_tile < 15)
				blit(wall_bmp[cell.wall_tile], buf, 0, 0, ORIGIN_X + c * TILE, ORIGIN_Y + r * TILE, TILE, TILE);
		}
	const std::vector<MovingWall>& view_walls = Away() ? remote_walls : walls;
	for (int i = 0; i < level.MarkCount(); i++) {
		const Mark& m = level.GetMark(i);
		if (m.eaten || WallAt(view_walls, m.col, m.row)) continue;  /* hidden under a block */
		int kind = m.type == CELL_POWER_MARK ? 1 : 0;
		BITMAP* b = m.owner == my_id ? mark_bmp[kind] : team_mark_bmp[kind];
		masked_blit(b, buf, 0, 0, ORIGIN_X + m.col * TILE + (TILE - b->w) / 2,
		            ORIGIN_Y + m.row * TILE + (TILE - b->h) / 2, b->w, b->h);
	}

	/* The maze my Pac-Man is in: my own, or the one it is visiting. */
	DrawWalls(buf, view_walls);
	if (phase != PHASE_DYING) DrawGhosts(buf, Away() ? remote_ghosts : ghosts, Away() ? remote_fright : fright_ticks);

	for (std::map<int, RemotePac>::const_iterator it = remote_pacs.begin(); it != remote_pacs.end(); ++it) {
		const RemotePac& r = it->second;
		if (r.terrain != pac_terrain || r.age > REMOTE_STALE) continue;
		DrawPac(buf, r.x, r.y, r.dir, r.anim);
		textprintf_ex(buf, font, ORIGIN_X + r.x + 8, ORIGIN_Y + r.y - 9, makecol(255, 255, 0), -1, "%d", it->first);
	}

	if (phase == PHASE_DYING) {
		int frame = (DEATH_TICKS - phase_ticks) / DEATH_FRAME_TICKS;
		if (frame < 6) DrawSprite(buf, dead_bmp, 0, frame * TILE, pac.x, pac.y);
	} else {
		DrawPac(buf, pac.x, pac.y, pac.dir, pac_anim);
	}

	/* HUD */
	DrawText(buf, 0, 0, "Player %d", my_id);
	DrawText(buf, 0, 10, "Score %d", players[my_id].score);
	DrawText(buf, 0, 104, "FPS: %d", fps);
	if (Away()) DrawText(buf, 0, 124, "In player %d's", pac_terrain), DrawText(buf, 0, 134, "maze");
	for (int i = 0; i < players[my_id].lives - 1; i++)
		masked_blit(life_bmp, buf, 0, 0, 30 * i, 459, life_bmp->w, life_bmp->h);

	if (Multiplayer()) {
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
		DrawCentered(buf, 546, LocallyPaused() && Multiplayer() ? "Game Paused - press P to rejoin" : "Game Paused");
	else if (phase == PHASE_READY)
		DrawCentered(buf, 546, phase_ticks > TICKS_PER_SEC ? "Ready?" : "Go!!!");
	else if (notice_ticks > 0)
		DrawCentered(buf, 546, notice.c_str());

	DrawText(buf, 440, 590, ESCAPE_HINT);
}
