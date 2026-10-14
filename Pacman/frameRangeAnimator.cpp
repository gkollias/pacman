#include "frameRangeAnimator.h"

void FrameRangeAnimator::Start (Sprite* s, FrameRangeAnimation* a, struct timeb t) {
		sprite = s;
		anim = a;
		lastTime = t;
		flag = 0;
		state = ANIMATOR_RUNNING;
		sprite->SetFrame(currFrame = anim->GetStartFrame());
	}

void FrameRangeAnimator::SetLastTime(struct timeb t)
{lastTime = t;}


void FrameRangeAnimator::SetSprite (Sprite* _sprite){	sprite = _sprite;	}

void FrameRangeAnimator::SetFrameRangeAnimation (FrameRangeAnimation* _anim){	anim = _anim;	}
Sprite* FrameRangeAnimator::GetSprite ( void ){	return sprite;	}
FrameRangeAnimation* FrameRangeAnimator::GetFrameRangeAnimation ( void ){	return anim;}


FrameRangeAnimator::FrameRangeAnimator (void): 
	 	sprite((Sprite*) 0), 
		anim((FrameRangeAnimation*) 0), currFrame(0xFF){}
FrameRangeAnimator::FrameRangeAnimator (int _id): 
	 	sprite((Sprite*) 0), 
			anim((FrameRangeAnimation*) 0), currFrame(0xFF){ id = _id;}

FrameRangeAnimator::~FrameRangeAnimator(){}
	
void FrameRangeAnimator::Progress (struct timeb currTime) 
{
	long int curr,last;
	curr = currTime.time*1000 + currTime.millitm;
	last = lastTime.time*1000 + lastTime.millitm;
	if(curr - last >= anim->GetDelay()) 
	{

		if (currFrame == anim->GetEndFrame())
			currFrame = anim->GetStartFrame();
		else
			++currFrame;

		sprite->Move(anim->GetDx(), anim->GetDy());

		sprite->SetFrame(currFrame);
		int x = lastTime.millitm;
		int y = anim->GetDelay();
		if( x + y >= 1000)
		{
			lastTime.time +=1;
			lastTime.millitm = (lastTime.millitm + anim->GetDelay()) - 1000;
		}
		else
			lastTime.millitm += anim->GetDelay();

		if (currFrame == anim->GetEndFrame() && !anim->GetContinuous()) 
		{
			state = ANIMATOR_FINISHED;
			NotifyStopped();
			//SetLastTime(currTime);
			return;
		}
	}
	//SetLastTime(currTime);
}
