#ifndef SPRITE_H_
#define SPRITE_H_
#include <iostream>
#include <cstdlib>
#include <string>
#include <math.h>
#include "animationFilm.h"
#include "latelyDestroyable.h"

#define max(a, b) (((a) > (b)) ? (a): (b))
#define min(a, b) (((a) < (b)) ? (a): (b))

enum sprite_class {
	PACMAN_S	= 0,
	GHOST_S	= 1,
	SIMPLE_MARK_S	= 2,
	POWER_MARK_S	= 3
};

enum direction {
	UP	= 0,
	DOWN	= 1,
	RIGHT = 2,
	LEFT = 3,
	NO_DIR = 4
};
	
class Sprite : public LatelyDestroyable {

protected:
    byte		frameNo;
	Rectangle		frameBox;
    Dim		x,y,old_x,old_y;
    bool		isVisible;
	bool eat_status;
	bool is_alive;
    AnimationFilm*	currFilm;
	sprite_class s_class;
	typedef signed char	offset_t;

    public:
    void SetFrame (int i);
    byte GetFrame (void) const;

	void SetAnimationFilm (AnimationFilm* anim);
	AnimationFilm* GetAnimationFilm ();
    void SetVisibility (bool v);
    bool IsVisible (void) const;
    bool CollisionCheck (Sprite* s);
	virtual void Move(offset_t dx, offset_t dy) = 0;
	virtual bool CanEat() = 0;
	virtual void SetCanEat( bool status) = 0;
	virtual bool IsAlive() = 0;
	virtual void SetIsAlive( bool _is_alive) = 0;
	void SetX(Dim _x);
	void SetY(Dim _y);
	Dim GetX( void );
	Dim GetY( void );
	Dim GetOldX();
	Dim GetOldY();
	Dim GetW( void );
	Dim GetH( void );
	sprite_class GetClass();
    Sprite (Dim _x, Dim _y, AnimationFilm* film);
	Sprite();
};

#endif
