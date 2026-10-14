#ifndef COMMON_H_
#define COMMON_H_

#include "allegro.h"

#define GAME_W   800
#define GAME_H   600
#define TILE     24
#define ORIGIN_X 120   /* top-left of the maze on screen */
#define ORIGIN_Y 24
#define TICKS_PER_SEC 60

/* The browser build is single player only, and Escape returns to the
   menu instead of quitting (a web page has nothing to quit to). */
#ifdef __EMSCRIPTEN__
#define WEB_BUILD 1
#define ESCAPE_HINT "Press Escape for the menu."
#else
#define WEB_BUILD 0
#define ESCAPE_HINT "Press Escape to quit."
#endif

enum Dir { DIR_NONE = -1, DIR_UP = 0, DIR_DOWN = 1, DIR_LEFT = 2, DIR_RIGHT = 3 };

static const int DIR_DX[4] = { 0, 0, -1, 1 };
static const int DIR_DY[4] = { -1, 1, 0, 0 };
static const char* const DIR_NAME[4] = { "up", "down", "left", "right" };

inline Dir Opposite(Dir d)
{
	switch (d) {
	case DIR_UP: return DIR_DOWN;
	case DIR_DOWN: return DIR_UP;
	case DIR_LEFT: return DIR_RIGHT;
	case DIR_RIGHT: return DIR_LEFT;
	default: return DIR_NONE;
	}
}

enum Difficulty { EASY = 0, NORMAL = 1, HARD = 2 };
enum Speed { SLOW = 0, MEDIUM = 1, FAST = 2 };

/* Edge-triggered keyboard helper on top of Allegro's key[] array. */
class Input {
	bool prev[KEY_MAX];
public:
	Input();
	/* Call once per frame before Pressed(). */
	void Update();
	bool Down(int k) const { return cur[k]; }
	bool Pressed(int k) const;
private:
	bool cur[KEY_MAX];
};

#endif
