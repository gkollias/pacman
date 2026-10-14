#ifndef POWERMARK_H_
#define POWERMARK_H_
#include "sprite.h"


class PowerMark : public Sprite{
public:
unsigned short id;
PowerMark (Dim _x, Dim _y);
void Move(offset_t dx, offset_t dy);
bool CanEat();
void SetCanEat( bool status);
bool IsAlive();
void SetIsAlive( bool _is_alive);
void SetId(unsigned short _id);
unsigned short GetId(void);
};

#endif