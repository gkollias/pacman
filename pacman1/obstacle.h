#ifndef OBSTACLE_H_
#define OBSTACLE_H_
#include "moveTry.h"



class Obstacle : public Sprite{
unsigned int x,y,w,h;
unsigned int init_x,init_y;/*Added to control the movement of the obstacles*/
direction move_direction;/*Added to check the direction that the obstacle moves*/
BITMAP* bmp;
bool is_alive,can_eat;
public:
Obstacle(unsigned int _x,unsigned int _y,unsigned int _w,unsigned int _h);

bool AllowsMove (  Sprite& s, int dx, int dy) const;

void SetBitmap( BITMAP* _bmp );
BITMAP* GetBitmap();

unsigned int GetX( void );
unsigned int GetY( void );
unsigned int GetW( void );
unsigned int GetH( void );

unsigned int GetInitX( void );
unsigned int GetInitY( void );

struct DenyMovePred : public std::binary_function<Obstacle *,MoveTry, bool> {
bool operator()(const Obstacle *o, const MoveTry& m) const{ return !o->AllowsMove(m.sprite(),m.dx(),m.dy()); }
};


/*Added as a sprite*/
void Move (signed char dx, signed char dy);
bool CanEat();
void SetCanEat( bool status);
bool IsAlive();
void SetIsAlive( bool _is_alive);

/*Added for the movement*/
void SetDirection(direction _move_direction);
direction GetDirection();
};

#endif