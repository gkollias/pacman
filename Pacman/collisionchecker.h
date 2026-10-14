#ifndef COLLISIONCHECKER_H_
#define COLLISIONCHECKER_H_
#include <list>
#include <iostream>
#include <algorithm>
#include <utility>
#include "gameInitialization.h"

#pragma warning(disable:4541)
#pragma warning(disable:4786)

class CollisionChecker{
typedef std::pair<Sprite*, Sprite*> Pair;
    std::list<Pair> pairs;

    class CheckFunctor : public std::unary_function<Pair, void> {
    public:
       void operator()(const Pair& p) { p.first->CollisionCheck(p.second); }
    };
	static CollisionChecker *cc;
	bool lost_life;
	CollisionChecker (void);
	~CollisionChecker (void);
public:


static CollisionChecker& Singleton (void);
void Register (Sprite* sprite1, Sprite* sprite2) ;
void Cancel (Sprite* sprite1, Sprite* sprite2);
void Cancel (Sprite* sprite);
void Check (void);
void Action (Sprite* sprite1, Sprite* sprite2);

void SetLostLife(bool _lost_life);
bool GetLostLife();
};

#endif