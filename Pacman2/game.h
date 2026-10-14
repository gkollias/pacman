#ifndef GAME_H_
#define GAME_H_

#include <string>
#include <vector>
#include "common.h"
#include "level.h"

class Net;

#define MAX_PLAYERS 4

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
	void Place(int col, int row) { x = col * TILE; y = row * TILE; dir = DIR_NONE; acc = 0; moving = false; }
};

enum GhostMode { GHOST_NORMAL, GHOST_FRIGHTENED, GHOST_EYES, GHOST_LEAVING };

struct Ghost : Actor {
	int color;      /* 0 blinky, 1 pinky, 2 inky, 3 clyde */
	int label;      /* player whose ghost this originally was */
	GhostMode mode;
	int start_col, start_row;
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
	bool Init(int difficulty, int speed, int players, int my_id, Net* net);
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
	int difficulty, speed, num_players, my_id;
	PlayerInfo players[MAX_PLAYERS + 1];  /* index 1..MAX_PLAYERS */

	Actor pac;
	Dir wanted;
	int pac_anim;
	std::vector<Ghost> ghosts;

	Phase phase;
	int phase_ticks;
	int fright_ticks;
	int ghost_combo;
	int schedule_index, schedule_ticks;
	bool scatter;

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
	void ResetActors();
	void AddGhost(int color, int label, int col, int row, GhostMode mode);

	float PacSpeed() const;
	float GhostSpeed(const Ghost& g) const;
	bool PacCanEnter(int col, int row) const;
	bool GhostCanEnter(const Ghost& g, int col, int row) const;
	void MovePacman(const Input& in);
	void MoveGhost(Ghost& g);
	Dir ChooseGhostDir(Ghost& g);
	void GhostTarget(const Ghost& g, int& tc, int& tr) const;

	void EatAt(int col, int row);
	void StartFright();
	void UpdateFright();
	void UpdateSchedule();
	void CheckCollisions();
	void Die();
	void CheckCompleted();
	bool LocallyPaused() const;
	bool AnyonePaused() const;

	void Send(const char* fmt, ...);
	void SendGhostTo(int to);
	int NextPlayer() const;
	void RequestMark(const char* what, int from, int to);
	void MoveMark(int idx, int owner);
	void TransferMark(CellType type, int from, int to);
	void Redistribute(int from);

	void DrawText(BITMAP* buf, int x, int y, const char* fmt, ...);
	void DrawCentered(BITMAP* buf, int y, const char* text);
};

#endif
