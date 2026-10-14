#include "obstacle.h"


bool Obstacle::AllowsMove (  Sprite& s , int dx , int dy ) const
{
	return  ( s.GetX() + dx + s.GetW() < (int)x )     || 
			( s.GetX() + dx            > (int)(x+w) ) ||
			( s.GetY() + dy + s.GetH() < (int)y )     || 
			( s.GetY() + dy            > (int)(y+h) )  ;
}

unsigned int Obstacle::GetX( void ){	return x;	}
unsigned int Obstacle::GetY( void ){	return y;	}
unsigned int Obstacle::GetW( void ){	return w;	}
unsigned int Obstacle::GetH( void ){	return h;	}
unsigned int Obstacle::GetInitX( void ){	return init_x;	}
unsigned int Obstacle::GetInitY( void ){	return init_y;	}

void Obstacle::SetBitmap( BITMAP* _bmp ){	bmp = _bmp;	}
BITMAP* Obstacle::GetBitmap(){	return bmp;	}

Obstacle::Obstacle(unsigned int _x,unsigned int _y,unsigned int _w,unsigned int _h)
{
	x = _x;
	y = _y;
	w = _w;
	h = _h;
	old_x  = _x;
	old_y  = _y;
	init_x = _x;
	init_y = _y;
	move_direction = DOWN;
	eat_status = false;
	is_alive = true;
}


bool Obstacle::CanEat(){	return eat_status;	}
void Obstacle::SetCanEat( bool status){	eat_status = status;	}

bool Obstacle::IsAlive(){	return is_alive;	}
void Obstacle::SetIsAlive( bool _is_alive){	is_alive = _is_alive;	}

void Obstacle::Move (signed char dx, signed char dy) 
{
	if(x + dx < 0 || y + dy < 0 || (signed int)x + dx >= SCREEN_W || (signed int)y + dy >= SCREEN_H)
			return;

//	if(!ObstacleHolder::Singleton().CanMove((Sprite *)this, dx, dy))
//	{
		old_x = x;
		old_y = y;
		x += dx; 
		y += dy;
//	}
}

void Obstacle::SetDirection(direction _move_direction){ move_direction = _move_direction;}
direction Obstacle::GetDirection(){return move_direction;}