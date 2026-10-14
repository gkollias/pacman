#include "animator.h"


bool Animator::HasFinished (void) const 
		{ return state != ANIMATOR_RUNNING; }
void Animator::SetOnFinish (FinishCallback f, void* c) 
		{ onFinish = f, finishClosure = c; }


void Animator::Stop (void) {
	if (!HasFinished()) {
		state = ANIMATOR_STOPPED;
		NotifyStopped();
	}
}
void Animator::NotifyStopped (void) {
	if (onFinish)
		(*onFinish)(this, finishClosure);
}
void Animator::TimeShift (struct timeb offset)
{
	lastTime.time += offset.time;
	lastTime.millitm += offset.millitm; 
}

Animator::Animator (int _id) :
     state(ANIMATOR_FINISHED), 
    onFinish((FinishCallback) 0), finishClosure((void*) 0)
	 {
		lastTime.time = 0;
		lastTime.millitm = 0;
		id = _id;
	 }

Animator::Animator (void) :
     state(ANIMATOR_FINISHED), 
    onFinish((FinishCallback) 0), finishClosure((void*) 0)
	 {
		lastTime.time = 0;
		lastTime.millitm = 0;
	 }
Animator::~Animator(){}

int Animator::GetId(){	return id;	}