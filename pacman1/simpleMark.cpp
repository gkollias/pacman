#include "simpleMark.h"


SimpleMark::SimpleMark (Dim _x, Dim _y)
{
	s_class = SIMPLE_MARK_S;
	x = _x;
	y = _y;
	eat_status = true;
	is_alive = true;
	old_x = x;
	old_y = y;
}

void SimpleMark::Move(offset_t dx, offset_t dy)
{
	x += dx;
	y += dy;

}

bool SimpleMark::CanEat(){	return eat_status;	}
void SimpleMark::SetCanEat( bool status){	eat_status = status;	}

bool SimpleMark::IsAlive(){	return is_alive;	}
void SimpleMark::SetIsAlive( bool _is_alive){	is_alive = _is_alive;	}

void SimpleMark::SetId(unsigned short _id){ id = _id; }
unsigned short SimpleMark::GetId(void){ return id;}