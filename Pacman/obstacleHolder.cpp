#include "obstacleHolder.h"

ObstacleHolder* ObstacleHolder::obh;

bool ObstacleHolder::CanMove ( Sprite* s, int dx, int dy) 
{
return std::find_if(
	obstacles.begin(),
	obstacles.end(),
	std::bind2nd(
	Obstacle::DenyMovePred(),
	MoveTry(s, dx, dy)
	)
	) != obstacles.end();
}



ObstacleHolder& ObstacleHolder::Singleton()
{
if(obh == NULL)
	obh = new ObstacleHolder();
return *obh;

}

ObstacleHolder::ObstacleHolder(){}

void ObstacleHolder::SetObstacles( std::vector<Obstacle *> _obstacles){	obstacles = _obstacles; }

std::vector<Obstacle *> ObstacleHolder::GetObstacles(){	return obstacles;}
