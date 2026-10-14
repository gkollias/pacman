#include "pacman.h"

PacMan::PacMan(Dim _x , Dim _y)
{
	s_class = PACMAN_S;
	x = _x;
	y = _y;
	old_x = _x;
	old_y = _y;
	eat_status = false;
	is_alive = true;
}

void PacMan::Move (signed char dx, signed char dy) 
{
	if(x + dx < 0 || y + dy < 0 || x + dx >= SCREEN_W || y + dy >= SCREEN_H)
			return;

	if( x <= 89 &&  y<= 227 && y >= 197 && flag==0)
	{old_x = x;old_y = y;x = 532;y=199;flag = 1;return;}
	if( x >= 532 &&  y<= 227 && y >= 197 && flag==0)
	{old_x = x;old_y = y;x = 89;y=199;flag = 1;return;}
	if(!ObstacleHolder::Singleton().CanMove((Sprite *)this, dx, dy))
	{
		old_x = x;
		old_y = y;
		x += dx; y += dy;
		flag=0;
	}
}

bool PacMan::CanEat(){	return eat_status;	}
void PacMan::SetCanEat( bool status){	eat_status = status;	}

bool PacMan::IsAlive(){	return is_alive;	}
void PacMan::SetIsAlive( bool _is_alive){	is_alive = _is_alive;	}
