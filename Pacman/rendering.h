#ifndef RENDERING_H_
#define RENDERING_H_
#include "animationFilmHolder.h"
#include "simpleMarkHolder.h"
#include "powerMarkHolder.h"
#include "obstacleHolder.h"


class Rendering{
	BITMAP* buffer;
	BITMAP* score_b;
	BITMAP* lives_b;
	BITMAP* pause_b;
	BITMAP* game_status_b;
	BITMAP* ready_b;
	BITMAP* fps_b;
	static Rendering* render;
	std::list<Sprite *> sprites;
	Rendering();
	~Rendering();
public:
	static Rendering& Singleton();
	void DrawSprite(Sprite *sprite);
	void EraseSprite(Sprite *sprite);
	void DrawAll();
	void EraseAll();
	void DrawSimpleMarks();
	void DrawPowerMarks();
	void DrawObstacles();
	void DrawLives();
	void DrawFPS(int );
	void DrawScore();
	void DrawPause();
	void ErasePause();
	void DrawReadyScreen();
	void DrawWelcomeScreen();
	void DrawGameOver();
	void DrawGameCompleted();
	void SetBuffer(BITMAP* _buffer);
	BITMAP* GetBuffer();
	void Clear();
	void Register(Sprite* sprite);
	void Cancel(Sprite* sprite);
};

#endif