#include "PlayerHolder.h"

PlayerHolder* PlayerHolder::player_holder;

PlayerHolder::PlayerHolder(void)
{}

PlayerHolder& PlayerHolder::Singleton (void)
{
	if(player_holder == NULL)
		player_holder = new PlayerHolder();
	return *player_holder;
}


void PlayerHolder::addPlayer(Player *_player){players.push_back(_player);}
void PlayerHolder::removePlayer(Player *_player){players.remove(_player);}











PlayerHolder::~PlayerHolder(void){}
