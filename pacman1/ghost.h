#ifndef GHOST_H_
#define GHOST_H_
#include "obstacleHolder.h"

#define INKY 0
#define PINKY 1
#define BLINKY 2
#define CLYDE 3

class Ghost : public Sprite {
	bool flag;
	direction move_direction;
	bool can_move;
	int name;
public:
Ghost(Dim _x , Dim _y);
void SetDirection( direction new_direction);
direction GetDirection();
void Move (signed char dx, signed char dy);
bool CanMove(direction _direction);
bool CanEat();
void SetCanEat( bool status);
bool IsAlive();
void SetIsAlive( bool _is_alive);
void SetName( int _name);
int GetName();
};

#endif