#ifndef MOVINGANIMATOR_H_
#define MOVINGANIMATOR_H_
#include "animator.h"
#include "movingAnimation.h"
#include "sprite.h"
#include <cstdlib>

class MovingAnimator : public Animator {
	Sprite*		sprite;
	MovingAnimation*	anim;

	public:
	void Progress (struct timeb currTime);
	void Start (Sprite* s, MovingAnimation* a, struct timeb t);
	//accessors
	void SetSprite (Sprite* _sprite);
	void SetMovingAnimation (MovingAnimation* _anim);
	Sprite* GetSprite ( void );
	MovingAnimation* GetMovingAnimation ( void );
	void SetLastTime(struct timeb t);

	MovingAnimator (void);
	~MovingAnimator();
};

#endif
