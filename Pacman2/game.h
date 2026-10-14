#ifndef GAME_H_
#define GAME_H_

#include <map>
#include <string>
#include <vector>
#include "common.h"
#include "level.h"

class Net;

#define MAX_PLAYERS 4

/* Optional features (the original's Read me lists them; all on by default). */
enum Option {
	OPT_MOVING_WALLS = 1,  /* blocks of the maze slide back and forth */
	OPT_MOVING_DOTS  = 2,  /* dots run away from Pac-Man */
	OPT_TERRAINS     = 4,  /* LAN: the side tunnel leads to a neighbour's maze */
	OPT_AUTO_SEND    = 8,  /* LAN: a cornered Pac-Man sends a ghost away */
	OPT_ALL          = 15
};

/* A 24x24 sprite moving on the grid. x/y are pixels relative to the
   maze origin (top-left of the sprite); it is on a cell centre when
   both are multiples of TILE. */
struct Actor {
	int x, y;
	Dir dir;
	float acc;      /* sub-pixel movement accumulator */
	bool moving;

	int Col() const;
	int Row() const;
	bool AtCenter() const;
	bool Overlaps(int col, int row) const;
	void Place(int col, int row) { x = col * TILE; y = row * TILE; dir = DIR_NONE; acc = 0; moving = false; }
};

enum GhostMode { GHOST_NORMAL, GHOST_FRIGHTENED, GHOST_EYES, GHOST_LEAVING };

struct Ghost : Actor {
	int color;      /* 0 blinky, 1 pinky, 2 inky, 3 clyde */
	int label;      /* player whose ghost this originally was */
	GhostMode mode;
	int start_col, start_row;
};

/* A free-standing block of the maze that slides as a whole by (dx, dy)
   cells and back. While moved, its home cells are open floor. While
   sliding it blocks both its home cells and the cells it moves into. */
struct MovingWall {
	std::vector<int> cols, rows, tiles;  /* its cells at home, and their wall tiles */
	int dx, dy;
	bool moved;
	int slide;           /* ticks of sliding left, 0 when still */
	int home_ticks, moved_ticks, wait;

	bool Blocks(int c, int r) const;
};

/* Another player's Pac-Man, as last reported. */
struct RemotePac {
	int terrain;
	int x, y;
	Dir dir;
	int anim;
	int age;  /* ticks since the last report */
};

enum PlayerState { PS_PLAYING, PS_PAUSED, PS_GAMEOVER, PS_LEFT };

struct PlayerInfo {
	bool present;
	PlayerState state;
	int score;
	int lives;
};

enum Phase { PHASE_READY, PHASE_PLAYING, PHASE_DYING, PHASE_GAMEOVER, PHASE_COMPLETED };

class Game {
	friend struct GameTest;  /* tests/logic_test.cpp */
public:
	Game();
	~Game();
	/* players: 1 for single player. my_id is 1-based. net may be NULL. */
	bool Init(int difficulty, int speed, int players, int my_id, Net* net, int options = OPT_ALL);
	void Tick(const Input& in);   /* one 1/60 s step */
	void Draw(BITMAP* buf, int fps);
	bool Finished() const { return phase == PHASE_GAMEOVER || phase == PHASE_COMPLETED; }

	/* Multiplayer (see net.h for the protocol). */
	void HandleMessage(const std::string& msg);
	void PlayerLeft(int id);
	void ServerLost();

private:
	Level level;
	Net* net;
	int difficulty, speed, num_players, my_id, options;
	PlayerInfo players[MAX_PLAYERS + 1];  /* index 1..MAX_PLAYERS */

	Actor pac;
	Dir wanted;
	int pac_anim;
	int pac_terrain;                 /* whose maze my Pac-Man is in */
	std::vector<Ghost> ghosts;       /* the ghosts of my own maze */
	std::vector<MovingWall> walls;   /* the moving walls of my own maze */

	/* Seen from another player's maze while my Pac-Man visits it. */
	std::vector<Ghost> remote_ghosts;
	std::vector<MovingWall> remote_walls;
	int remote_fright;
	std::map<int, RemotePac> remote_pacs;
	std::map<int, int> hit_cooldown; /* host: visitor id -> ticks */

	Phase phase;
	int phase_ticks;
	int fright_ticks;
	int ghost_combo;
	int schedule_index, schedule_ticks;
	bool scatter;
	std::vector<int> dot_cooldown;   /* per mark: ticks until it may hop again */
	int auto_send_cooldown;
	int tick_count;
	std::string notice;
	int notice_ticks;

	/* bitmaps */
	BITMAP* wall_bmp[15];
	BITMAP* pac_bmp[4];
	BITMAP* dead_bmp;
	BITMAP* life_bmp;
	BITMAP* ghost_bmp[4][4];   /* [color][dir] */
	BITMAP* eyes_bmp[4];
	BITMAP* blue_bmp;
	BITMAP* white_bmp;
	BITMAP* mark_bmp[2];       /* simple, power: own */
	BITMAP* team_mark_bmp[2];  /* simple, power: teammates' */

	SAMPLE *snd_dot, *snd_ghost, *snd_attack, *snd_dead;
	int dot_voice;
	bool attack_playing;

	bool LoadAssets();
	bool LoadWalls(const std::string& path);
	void ResetActors();
	void AddGhost(int color, int label, int col, int row, GhostMode mode);

	bool Multiplayer() const { return num_players > 1; }
	bool Away() const { return pac_terrain != my_id; }
	bool Opt(int o) const { return (options & o) != 0; }

	float PacSpeed() const;
	float GhostSpeed(const Ghost& g) const;
	bool WallAt(const std::vector<MovingWall>& ws, int col, int row) const;
	bool PacCanEnter(int col, int row) const;
	bool GhostCanEnter(const Ghost& g, int col, int row) const;
	void MovePacman(const Input& in);
	void MoveGhost(size_t i);
	Dir ChooseGhostDir(Ghost& g);
	bool NearestPacman(const Ghost& g, int& col, int& row, Dir& dir) const;
	void GhostTarget(const Ghost& g, int& tc, int& tr) const;
	void MoveWalls();
	bool CellOccupied(int col, int row) const;
	void FleeDots();

	void EatAt(int col, int row);
	void StartFright();
	void UpdateFright();
	void UpdateSchedule();
	void CheckCollisions();
	void CheckVisitorCollisions();
	void CheckCornered();
	void Die();
	void CheckCompleted();
	bool LocallyPaused() const;
	bool AnyonePaused() const;
	void Notify(const std::string& text);

	/* multiplayer */
	void Send(const char* fmt, ...);
	bool Active(int id) const;
	int NextPlayer() const;
	int NeighbourTerrain(int from, int step) const;
	void SendGhost(size_t i, int to, int col, int row, Dir dir);
	void SendGhostTo(int to);
	void RequestMark(const char* what, int from, int to);
	void MoveMark(int id, int owner);
	void TransferMark(CellType type, int from, int to);
	void Redistribute(int from);
	void EnterTerrain(int terrain);
	void ReturnHome();
	void SendPosition();
	void SendSnapshot();
	void ReadSnapshot(const std::vector<int>& a);

	void DrawText(BITMAP* buf, int x, int y, const char* fmt, ...);
	void DrawCentered(BITMAP* buf, int y, const char* text);
	void DrawPac(BITMAP* buf, int x, int y, Dir dir, int anim);
	void DrawGhosts(BITMAP* buf, const std::vector<Ghost>& gs, int fright);
	void DrawWalls(BITMAP* buf, const std::vector<MovingWall>& ws);
};

#endif
