#include "collisionchecker.h"
#include "game.h"

CollisionChecker* CollisionChecker::cc;


CollisionChecker::CollisionChecker (void) 
{
	lost_life = false;
}

CollisionChecker::~CollisionChecker (void) {}

CollisionChecker& CollisionChecker::Singleton (void)
{
	if(cc == NULL)
		cc = new CollisionChecker();
	return *cc;
}


void CollisionChecker::Cancel (Sprite* sprite) 
{
	for (std::list<Pair>::iterator i = pairs.begin(); i != pairs.end(); ++i)
	{
		if (sprite == i->first || sprite == i->second)
			pairs.erase(i--);
	}
}


void CollisionChecker::Cancel (Sprite* sprite1,Sprite* sprite2) 
{
	for (std::list<Pair>::iterator i = pairs.begin(); i != pairs.end(); ++i)
	{
		
		if ((sprite1 == i->first && sprite2 == i->second) || (sprite2 == i->first && sprite1 == i->second))
			pairs.erase(i--);
	}
}


void CollisionChecker::Check (void)
{ 
	if(pairs.size()==0)
		return;
	//std::for_each(pairs.begin(), pairs.end(),  CheckFunctor()); 
	for (std::list<Pair>::iterator i = pairs.begin(); i != pairs.end(); ++i)
	{
		if((*i).first->CollisionCheck((*i).second) == true)
			Action((*i).first,(*i).second);
	
	}
}

void  CollisionChecker::Register (Sprite* sprite1, Sprite* sprite2)
{
	int c=0;
	std::list<Pair>::iterator i = pairs.begin(); 
	while (i != pairs.end())
	{
		if ((sprite1 == i->first && sprite2 == i->second) || (sprite2 == i->first && sprite1 == i->second))
			return;
		i++;
	}
	pairs.push_back(Pair(sprite1, sprite2));
}




void CollisionChecker::Action (Sprite* sprite1, Sprite* sprite2)
{
	if( sprite1->GetClass() == PACMAN_S && sprite2->GetClass() == SIMPLE_MARK_S )
	{
		sprite2->Destroy();
		SimpleMark* sm = (SimpleMark*)sprite2; 
		SimpleMarkHolder::Singleton().RemoveSimpleMark(sm);
		Rendering::Singleton().EraseSprite(sm);
		Game::Singleton().IncScore(10);
		Game::Singleton().PlayEatDot();
		Game::Singleton().DecMarks();
	}
	if( sprite1->GetClass() == PACMAN_S && sprite2->GetClass() == POWER_MARK_S )
	{
		sprite2->Destroy();
		PowerMark* pm = (PowerMark*)sprite2; 
		PowerMarkHolder::Singleton().RemovePowerMark(pm);
		Rendering::Singleton().EraseSprite(pm);
		Game::Singleton().IncScore(50);
		Game::Singleton().DecMarks();
		sprite1->SetCanEat(true);
		Game::Singleton().ChangeGhostStatus(false);
		Game::Singleton().StartBonusTimer();
	}
	if( sprite1->GetClass() == PACMAN_S && sprite2->GetClass() == GHOST_S )
	{
		if(sprite1->CanEat() && sprite2->IsAlive())
		{
			sprite2->SetIsAlive(false);
			if( Game::Singleton().GetGhostEat() == 0 )
			{
				Game::Singleton().GhostEatInc();
				Game::Singleton().IncScore(200);
			}
			else if( Game::Singleton().GetGhostEat() == 1 )
			{
				Game::Singleton().GhostEatInc();
				Game::Singleton().IncScore(400);
			}
			else if( Game::Singleton().GetGhostEat() == 2 )
			{
				Game::Singleton().GhostEatInc();
				Game::Singleton().IncScore(800);
			}
			else if( Game::Singleton().GetGhostEat() == 3 )
			{
				Game::Singleton().GhostEatInc();
				Game::Singleton().IncScore(1600);
			}
			Game::Singleton().PlayEatGhost();
		}
		else if(!sprite1->CanEat()
			&& sprite2->CanEat())/**POSSIBLE BUG don't know why yet**/
		{
			sprite1->SetIsAlive(false);
			Game::Singleton().InitPlaces();
			Game::Singleton().DecLives();
			lost_life = true;
		}
		
	}
	if( sprite1->GetClass() == GHOST_S && sprite2->GetClass() == PACMAN_S )
	{
		if(sprite2->CanEat() && sprite1->IsAlive())
		{
			sprite1->SetIsAlive(false);
			if( Game::Singleton().GetGhostEat() == 0 )
			{
				Game::Singleton().GhostEatInc();
				Game::Singleton().IncScore(200);
			}
			else if( Game::Singleton().GetGhostEat() == 1 )
			{
				Game::Singleton().GhostEatInc();
				Game::Singleton().IncScore(400);
			}
			else if( Game::Singleton().GetGhostEat() == 2 )
			{
				Game::Singleton().GhostEatInc();
				Game::Singleton().IncScore(800);
			}
			else if( Game::Singleton().GetGhostEat() == 3 )
			{
				Game::Singleton().GhostEatInc();
				Game::Singleton().IncScore(1600);
			}
			Game::Singleton().PlayEatGhost();
		}
		else if(!sprite2->CanEat() && sprite1->CanEat())
		{
			sprite2->SetIsAlive(false);
			Game::Singleton().InitPlaces();
			Game::Singleton().DecLives();
			lost_life = true;	
		}	
	}
}

void CollisionChecker::SetLostLife(bool _lost_life){	lost_life = _lost_life;	}
bool CollisionChecker::GetLostLife(){	return lost_life;	}