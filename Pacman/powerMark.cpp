#include "powerMark.h"

PowerMark::PowerMark (Dim _x, Dim _y)
{
	s_class = POWER_MARK_S;
	x = _x;
	y = _y;
	eat_status = true;
	is_alive = true;
	old_x = x;
	old_y = y;
}

void PowerMark::Move(offset_t dx, offset_t dy)
{
	x += dx;
	y += dy;

}

bool PowerMark::CanEat(){	return eat_status;	}
void PowerMark::SetCanEat( bool status){	eat_status = status;	}

bool PowerMark::IsAlive(){	return is_alive;	}
void PowerMark::SetIsAlive( bool _is_alive){	is_alive = _is_alive;	}

void PowerMark::SetId(unsigned short _id){ id = _id; }
unsigned short PowerMark::GetId(void){ return id;}