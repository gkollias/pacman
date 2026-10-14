#ifndef AI_H_
#define AI_H_
#include <iostream>
#include <sys/timeb.h>
#include "gameInitialization.h"

class Ai{
	static Ai *ai;
	PacMan* start_point;
	Ai();
	~Ai();
public:
	static Ai& Singleton();
	void GhostMove(Ghost* ghost,PacMan* pacman);
	bool MovePriorityX(Ghost* ghost,PacMan* pacman);
	bool MovePriorityY(Ghost* ghost,PacMan* pacman);
	bool MoveAwayPriorityX(Ghost* ghost,PacMan* pacman);
	bool MoveAwayPriorityY(Ghost* ghost,PacMan* pacman);
};

#endif