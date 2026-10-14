#include "movingAnimator.h"

void MovingAnimator::Start (Sprite* s, MovingAnimation* a, struct timeb t) {
		sprite	= s;
		anim 	= a;
		lastTime 	= t;
		state 	= ANIMATOR_RUNNING;
	}

void MovingAnimator::Progress (struct timeb currTime) {

	long int curr,last;
	curr = currTime.time*1000 + currTime.millitm;
	last = lastTime.time*1000 + lastTime.millitm;
	if(curr - last >= anim->GetDelay()) 
	{
		sprite->Move(anim->GetDx(), anim->GetDy());

		int x = lastTime.millitm;
		int y = anim->GetDelay();
		if( x + y >= 1000)
		{
			lastTime.time +=1;
			lastTime.millitm = (lastTime.millitm + anim->GetDelay()) - 1000;
		}
		else
			lastTime.millitm += anim->GetDelay();

		if (!anim->GetContinuous()) 
		{
			state = ANIMATOR_FINISHED;
			NotifyStopped();
			//SetLastTime(currTime);
			return;
		}
	}
	//SetLastTime(currTime);
}

void MovingAnimator::SetSprite (Sprite* _sprite){	sprite = _sprite;	}
void MovingAnimator::SetMovingAnimation (MovingAnimation* _anim){	anim = _anim;	}
Sprite* MovingAnimator::GetSprite ( void ){	return sprite;	}
MovingAnimation* MovingAnimator::GetMovingAnimation ( void ){return anim;}

void MovingAnimator::SetLastTime(struct timeb t)
{lastTime = t;}

MovingAnimator::MovingAnimator (void): 
		sprite((Sprite*) 0), anim((MovingAnimation*) 0){}
MovingAnimator::~MovingAnimator(){}
