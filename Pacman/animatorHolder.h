#ifndef ANIMATORHOLDER_H_
#define ANIMATORHOLDER_H_
#include <iostream>
#include <list>
#include <algorithm>
#include "animator.h"


class AnimatorHolder {
	static std::list<Animator *> running, suspended;
	class ProgressFunctor : public std::unary_function<Animator*, void> {
	    struct timeb t;
	public: 
	    void operator()(Animator* a) const { a->Progress(t); }
	    ProgressFunctor (struct timeb _t) : t(_t){}
	};
public:
	static void Register (Animator* a);
	static void Cancel (Animator* a);
	static void MarkAsRunning (Animator* a);
	static void MarkAsSuspended (Animator* a);
	static void MarkAsRunning (int id);
	static void MarkAsSuspended (int id);
	static void Progress (struct timeb currTime);
};

#endif