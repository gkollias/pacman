#ifndef PLAYERHOLDER_H_
#define PLAYERHOLDER_H_
#pragma once

#include "game.h"
#include "player.h"
class PlayerHolder
{
	PlayerHolder(void);
	~PlayerHolder(void);
	static PlayerHolder *player_holder;
	std::list<Player *> players;
public:
	static PlayerHolder& Singleton(void);
	void addPlayer(Player *_player);
	void removePlayer(Player *_player);
};

#endif