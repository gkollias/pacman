#ifndef OBSTACLEHOLDER_H_
#define OBSTACLEHOLDER_H_
#include <iostream>
#include <vector>
#include "obstacle.h" 

class ObstacleHolder { // Singleton class.
std::vector<Obstacle *> obstacles;
static ObstacleHolder *obh;
public:
ObstacleHolder();
bool CanMove ( Sprite* s, int dx, int dy);
static ObstacleHolder& Singleton();
void SetObstacles( std::vector<Obstacle *> _obstacles);
std::vector<Obstacle *> GetObstacles();

};

#endif