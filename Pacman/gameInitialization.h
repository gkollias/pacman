#ifndef GAMEINITIALIZATION_H_
#define GAMEINITIALIZATION_H_
#include <iostream>
#include <vector>
#include <stdio.h>
#include "obstacle.h"
#include "bitmapLoader.h"
#include "simpleMarkHolder.h"
#include "powerMarkHolder.h"
#include "animationFilmHolder.h"
#include "movingAnimator.h"
#include "frameRangeAnimator.h"
#include "animatorHolder.h"
#include "rendering.h"
#include "collisionChecker.h"
#include "pacman.h"
#include "ghost.h"

#define MAX_SIMPLE_MARKS 243
#define MAX_POWER_MARKS	 4
#define END_FLAG     	 65000

#define PACMAN_STEP_UP    -1
#define PACMAN_STEP_DOWN   1
#define PACMAN_STEP_RIGHT  1
#define PACMAN_STEP_LEFT  -1

#define GHOST_STEP_UP     -1
#define GHOST_STEP_DOWN    1
#define GHOST_STEP_RIGHT   1
#define GHOST_STEP_LEFT   -1

#define PACMAN_FRAME_RANGE_DELAY  100
#define GHOST_FRAME_RANGE_DELAY   200
#define PACMAN_MOVE_DELAY         200
#define GHOST_MOVE_DELAY          200

#define PASSAGE_WIDTH 25

enum bitmap_option {
	PACMAN	    = 0,
	PACMAN_DEAD	= 1,
	GHOST	    = 2,
	SIMPLE_MARK = 3,
	POWER_MARK  = 4,
	OBSTACLE    = 5
};

class GameInitialization{
	static GameInitialization* gi;
	GameInitialization();
	~GameInitialization();
	std::vector<Rectangle> pac_boxes;
	std::vector<Rectangle> pac_dead_boxes;
	std::vector<Rectangle> ghost_boxes;
	std::vector<Rectangle> simple_boxes;
	std::vector<Rectangle> power_boxes;
	std::vector<Rectangle> obstacle_boxes;
	std::vector<Obstacle *> oh_obstacles;
	std::vector<int > moving_obstacles_indexes;
	std::list<std::string> bitmap_names;
	PacMan* pac_sprite;
	FrameRangeAnimator* pacFRAnimatorUp;
	FrameRangeAnimator* pacFRAnimatorDown;
	FrameRangeAnimator* pacFRAnimatorRight;
	FrameRangeAnimator* pacFRAnimatorLeft;
	FrameRangeAnimator* pacFRAnimatorDead;
	MovingAnimator* pacMovAnimatorUp;
	MovingAnimator* pacMovAnimatorDown;
	MovingAnimator* pacMovAnimatorRight;
	MovingAnimator* pacMovAnimatorLeft;

	Obstacle *moving_obstacle;
	MovingAnimator* obstacleMovAnimatorUp;
	MovingAnimator* obstacleMovAnimatorDown;

	Ghost* pinky;
	FrameRangeAnimator* pinkyAnimatorUp;
	FrameRangeAnimator* pinkyAnimatorDown;
	FrameRangeAnimator* pinkyAnimatorRight;
	FrameRangeAnimator* pinkyAnimatorLeft;
	Ghost* blinky;
	FrameRangeAnimator* blinkyAnimatorUp;
	FrameRangeAnimator* blinkyAnimatorDown;
	FrameRangeAnimator* blinkyAnimatorRight;
	FrameRangeAnimator* blinkyAnimatorLeft;
	Ghost* inky;
	FrameRangeAnimator* inkyAnimatorUp;
	FrameRangeAnimator* inkyAnimatorDown;
	FrameRangeAnimator* inkyAnimatorRight;
	FrameRangeAnimator* inkyAnimatorLeft;
	Ghost* clyde;
	FrameRangeAnimator* clydeAnimatorUp;
	FrameRangeAnimator* clydeAnimatorDown;
	FrameRangeAnimator* clydeAnimatorRight;
	FrameRangeAnimator* clydeAnimatorLeft;
	/*Additional vars*/
	unsigned short players;
	unsigned short sm_state[MAX_SIMPLE_MARKS];
	unsigned short pm_state[MAX_POWER_MARKS];

public:
	static GameInitialization& Singleton();
	void BoxesInit(std::string path , enum bitmap_option option);//apxeio me ta coordinates twn mm boxes
	void ObstaclesInit(std::string path);//apxeio me ta coordinates twn obstacles
	void BitmapNamesInit(std::string path);//arxeio me ta names twn bitmap
	void SimpleMarksInit(std::string path);//apxeio me ta coordinates twn  simple marks
	void PowerMarksInit(std::string path);//apxeio me ta coordinates twn  power marks
	void PacmanInit();
	void GhostsInit();
	std::vector<Rectangle> GetBoxes(enum bitmap_option option);
	std::vector<Obstacle *> GetObstacles();
	/*For the moving Obstacles*/
	void MovingObstaclesInit(std::string path);//apxeio me ta indexes twn kinoumenwn empodiwn
	std::vector<int > GetMovingObstacles();
	std::list<std::string> GetBitmapNames();
	PacMan* GetPacman();
	Ghost* GetInky();
	Ghost* GetPinky();
	Ghost* GetBlinky();
	Ghost* GetClyde();
	FrameRangeAnimator* GetPacmanFRAnimator(std::string dir);
	MovingAnimator* GetPacmanMovAnimator(std::string dir);
	FrameRangeAnimator* GetGhostAnimator(std::string gh_name , std::string dir);
	void InitAll();
	/*Additional funcs*/
	void SetPlayers(unsigned short _players);
	unsigned short GetPlayers(void);
	void SimpleMarksStateInit(std::string path);
	void PowerMarksStateInit(std::string path);
	bool CheckId(unsigned short id,enum bitmap_option option);
};

#endif