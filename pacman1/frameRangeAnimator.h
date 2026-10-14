#ifndef FRAMERANGEANIMATOR_H_
#define FRAMERANGEANIMATOR_H_
#include "animator.h"
#include "frameRangeAnimation.h"
#include "sprite.h"


class FrameRangeAnimator : public Animator {
	Sprite*			sprite;
	FrameRangeAnimation*	anim;
	frame_t			currFrame;
	int id;
	int flag;
	public:
	void Progress (struct timeb currTime);
	void Start (Sprite* s, FrameRangeAnimation* a, struct timeb t);
	void SetLastTime(struct timeb t);
	void SetSprite (Sprite* _sprite);
	void SetFrameRangeAnimation (FrameRangeAnimation* _anim);
	Sprite* GetSprite ( void );
	FrameRangeAnimation* GetFrameRangeAnimation ( void );
	FrameRangeAnimator (int _id);
	FrameRangeAnimator (void);
	~FrameRangeAnimator();
};

#endif