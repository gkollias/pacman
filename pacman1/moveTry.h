#ifndef MOVETRY_H_
#define MOVETRY_H_
#include "sprite.h"
#include <iostream>
#include <algorithm>
#include <utility>
#include <list>


struct MoveTry: public std::pair<  Sprite*, std::pair<int, int> > {
 Sprite& sprite (void) const { return *first; }
const int& dx (void) const { return second.first; }
const int& dy (void) const { return second.second; }
MoveTry( Sprite* s, int _dx, int _dy){
first = s; second.first = _dx; second.second = _dy;}
};

#endif
