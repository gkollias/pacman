#ifndef PACMAN_H_
#define PACMAN_H_
#include "obstacleHolder.h"

class PacMan : public Sprite {
	int flag;
public:
PacMan(Dim _x , Dim _y);
void Move (signed char dx, signed char dy);
bool CanEat();
void SetCanEat( bool status);
bool IsAlive();
void SetIsAlive( bool _is_alive);
};

#endif