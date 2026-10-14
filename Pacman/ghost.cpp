#include "ghost.h"

Ghost::Ghost(Dim _x , Dim _y)
{
	s_class = GHOST_S;
	x = _x;
	y = _y;
	eat_status = true;
	is_alive = true;
}

void Ghost::Move (signed char dx, signed char dy) 
{
	if(x + dx < 0 || y + dy < 0 || x + dx >= SCREEN_W || y + dy >= SCREEN_H)
			return;

	if( x <= 89 &&  y<= 227 && y >= 197 && flag==false)
	{
		x = 532;y=199;
		flag = true;
		can_move = true;
		return;
	}
	if( x >= 532 &&  y<= 227 && y >= 197 && flag==false)
	{
		x = 89;y=199;
		flag = true;
		can_move = true;
		return;
	}
	if(!ObstacleHolder::Singleton().CanMove((Sprite *)this, dx, dy))
	{
		old_x = x;
		old_y = y;
		x += dx; y += dy;
		flag = false;
		can_move = true;
	}
	else
		can_move = false;
}


void Ghost::SetDirection( direction new_direction){	move_direction = new_direction;}
direction Ghost::GetDirection(){	return move_direction;	}
bool Ghost::CanMove(direction _direction)
{	
	if( _direction == UP )
		return can_move = !ObstacleHolder::Singleton().CanMove((Sprite *)this, 0, -1 );
	if( _direction == DOWN )
		return can_move = !ObstacleHolder::Singleton().CanMove((Sprite *)this, 0, 1 );
	if( _direction == RIGHT )
		return can_move = !ObstacleHolder::Singleton().CanMove((Sprite *)this, 1, 0 );
	if( _direction == LEFT )
		return can_move = !ObstacleHolder::Singleton().CanMove((Sprite *)this, -1, 0 );
	return false;
}



bool Ghost::CanEat(){	return eat_status;	}
void Ghost::SetCanEat( bool status){	eat_status = status;	}

bool Ghost::IsAlive(){	return is_alive;	}
void Ghost::SetIsAlive( bool _is_alive){	is_alive = _is_alive;	}

void Ghost::SetName( int _name){	name = _name;	}
int Ghost::GetName(){	return name;	}