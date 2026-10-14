#ifndef ANIMATOR_H_
#define ANIMATOR_H_
#include <sys/timeb.h>
typedef unsigned long timestamp_t;
enum animatorstate_t {
	ANIMATOR_FINISHED	= 0,
	ANIMATOR_RUNNING	= 1,
	ANIMATOR_STOPPED	= 2
};
class Animator {
public:
	typedef void (*FinishCallback)(Animator*,void*);
protected:
	struct timeb		lastTime;
	animatorstate_t		state;
	FinishCallback		onFinish;
	void*			finishClosure;
	void NotifyStopped (void);
	int id;
public:
	void Stop (void);
	bool HasFinished (void) const;
	virtual void TimeShift (struct timeb offset);
	virtual void Progress (struct timeb currTime)=0;
	void SetOnFinish (FinishCallback f, void* c=(void*) 0);
	int GetId();
	Animator (void); 
	Animator (int _id); 
	~Animator();
};

#endif
