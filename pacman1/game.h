#ifndef GAME_H_
#define GAME_H_
#include <iostream>
#include <sys/timeb.h>
#include "gameInitialization.h"
#include "ai.h"

#define BONUS_TIME 7

class Game{
	Game();
	~Game();
	static Game *game;
	PacMan* pac_sprite;
	Ghost* inky,*pinky,*blinky,*clyde;
	long int score;
	unsigned short players;
	unsigned short player_id;
	FrameRangeAnimator* pacFRAnimatorUp;
	FrameRangeAnimator* pacFRAnimatorDown;
	FrameRangeAnimator* pacFRAnimatorRight;
	FrameRangeAnimator* pacFRAnimatorLeft;
	FrameRangeAnimator* pacFRAnimatorDead;
	MovingAnimator* pacMovAnimatorUp;
	MovingAnimator* pacMovAnimatorDown;
	MovingAnimator* pacMovAnimatorRight;
	MovingAnimator* pacMovAnimatorLeft;
	FrameRangeAnimator* pinkyAnimatorUp;
	FrameRangeAnimator* pinkyAnimatorDown;
	FrameRangeAnimator* pinkyAnimatorRight;
	FrameRangeAnimator* pinkyAnimatorLeft;
	FrameRangeAnimator* blinkyAnimatorUp;
	FrameRangeAnimator* blinkyAnimatorDown;
	FrameRangeAnimator* blinkyAnimatorRight;
	FrameRangeAnimator* blinkyAnimatorLeft;
	FrameRangeAnimator* inkyAnimatorUp;
	FrameRangeAnimator* inkyAnimatorDown;
	FrameRangeAnimator* inkyAnimatorRight;
	FrameRangeAnimator* inkyAnimatorLeft;
	FrameRangeAnimator* clydeAnimatorUp;
	FrameRangeAnimator* clydeAnimatorDown;
	FrameRangeAnimator* clydeAnimatorRight;
	FrameRangeAnimator* clydeAnimatorLeft;
	struct timeb currtime;
	struct timeb bonus_time;
	struct timeb ghost_time;
	int ghosts_eaten;
	int lives;
	int marks;
	bool sound_played;
	bool game_over,game_completed;
	bool blue_flag,move_left,move_right,move_up,move_down,in_bonus;
	/*sound*/
	SAMPLE *eatdot;
	SAMPLE *attack;
	SAMPLE *eatghost;
	int playsound_eatdot;
	int playsound_eatghost;
	/**************************/
	Obstacle *moving_obstacle;
	MovingAnimator* obstacleMovAnimatorUp;
	MovingAnimator* obstacleMovAnimatorDown;
	MovingAnimator* obstacleMovAnimatorRight;
	MovingAnimator* obstacleMovAnimatorLeft;
	/*************************/
public:
	
static Game& Singleton (void);
void PacmanMove();
void GhostMove();
void ObstacleMoveAll();/*Added for obstacle movement*/
void ObstacleMove(unsigned short obstacle_id);/*Added for obstacle movement*/
void SetGameTime();
timeb GetGameTime();
void CheckMarksCollision();
void IncScore(int increment);
int GetScore();
void StartBonusTimer();
void CheckBonusTimer();
void CheckGameStatus();
bool IsGameOver();
bool IsGameCompleted();
void ChangeGhostStatus(bool status);
void GhostEatInc();
int GetGhostEat();
int GetLives();
void DecLives();
int GetMarks();
void DecMarks();
void InitPlaces();
void Pause();
void PlayEatDot();
void PlayEatGhost();
void PlayAttack(bool play);

void SetPlayers(unsigned short _players);
unsigned short GetPlayers(void);
void SetPlayerId(unsigned short _player_id);
unsigned short GetPlayerId(void);


};



#endif