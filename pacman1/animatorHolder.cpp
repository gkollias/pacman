#include "animatorHolder.h"

std::list<Animator *> AnimatorHolder::running, AnimatorHolder::suspended;
void AnimatorHolder::Register (Animator* a) { suspended.push_back(a); }
void AnimatorHolder::Cancel (Animator* a) { suspended.remove(a); }
void AnimatorHolder::MarkAsRunning (Animator* a)
		{ suspended.remove(a); running.push_back(a); }
void AnimatorHolder::MarkAsSuspended (Animator* a)
		{ running.remove(a); suspended.push_back(a); }
void AnimatorHolder::MarkAsRunning (int id)
{
	std::list<Animator *>::iterator i;
	for( i = suspended.begin(); i != suspended.end(); i++ )
	{
		if((*i)->GetId() == id )
		{suspended.remove(*i); running.push_back(*i);}

	}
}
void AnimatorHolder::MarkAsSuspended (int id)
{
	std::list<Animator *>::iterator i;
	for( i = running.begin(); i != running.end(); i++ )
	{
		if((*i)->GetId() == id )
		{running.remove(*i); suspended.push_back(*i);}

	}
}
void AnimatorHolder::Progress (struct timeb currTime) {
	    std::for_each(
		running.begin(), running.end(), ProgressFunctor(currTime)
	    ); 
	}

