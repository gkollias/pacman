#include "rendering.h"
#include "game.h"

Rendering* Rendering::render;

Rendering::Rendering()
{	
	buffer = create_bitmap(SCREEN_W,SCREEN_H);
	score_b = create_bitmap(100,10);
	lives_b = create_bitmap(120,20);
	pause_b = create_bitmap(90,10);
	game_status_b = create_bitmap(140,10);
	ready_b = create_bitmap(90,10);
}
Rendering::~Rendering(){}

Rendering& Rendering::Singleton()
{
	if( render == NULL)
		render = new Rendering();
	return *render;
}
void Rendering::DrawSprite(Sprite* sprite)
{
	Point at;
	at.x = sprite->GetX();
	at.y = sprite->GetY();
	AnimationFilmHolder::Singleton().GetFilm(sprite->GetAnimationFilm()->GetId()).DisplayFrame(buffer,at,sprite->GetFrame());
}
void Rendering::EraseSprite(Sprite* sprite)
{
	rectfill( buffer, sprite->GetOldX(), sprite->GetOldY(), 
		sprite->GetOldX() + sprite->GetW(), sprite->GetOldY() + sprite->GetH(), makecol ( 0, 0, 0));
}
void Rendering::DrawAll()
{
	std::list<Sprite *>::iterator i;
	for( i = sprites.begin();i!=sprites.end();i++)
		DrawSprite(*i);
}

void Rendering::EraseAll()
{
	std::list<Sprite *>::iterator i;
	for( i = sprites.begin();i!=sprites.end();i++)
		EraseSprite(*i);
}
void Rendering::Register(Sprite* sprite){	sprites.push_back(sprite);}
void Rendering::Cancel(Sprite* sprite){	sprites.remove(sprite);	}

void Rendering::DrawSimpleMarks()
{
	Point at;
	std::list<SimpleMark *>::iterator i;
	std::list<SimpleMark *> sml = SimpleMarkHolder::Singleton().GetSMarks();
	for(i = sml.begin();i!=sml.end();i++)
	{
		at.x = (*i)->GetX();
		at.y = (*i)->GetY();
		AnimationFilmHolder::Singleton().GetFilm((*i)->GetAnimationFilm()->GetId()).DisplayFrame(buffer,at,0);
	}
}


void Rendering::DrawPowerMarks()
{
	Point at;
	std::list<PowerMark *>::iterator i;
	std::list<PowerMark *> pml = PowerMarkHolder::Singleton().GetPMarks();
	for(i = pml.begin();i!=pml.end();i++)
		{
		at.x = (*i)->GetX();
		at.y = (*i)->GetY();
		AnimationFilmHolder::Singleton().GetFilm((*i)->GetAnimationFilm()->GetId()).DisplayFrame(buffer,at,0);
	}
}


void Rendering::SetBuffer(BITMAP* _buffer){	buffer = _buffer;	}
BITMAP* Rendering::GetBuffer(){	return buffer;	}
void Rendering::Clear(){clear_to_color(buffer,makecol(0,0,0));}


void Rendering::DrawScore()
{
	clear_to_color(score_b,makecol(0,0,0));
	textprintf_ex(score_b, font, 0, 0, makecol(255, 255, 255), -1, "Score %d",Game::Singleton().GetScore());
	blit(score_b,buffer,0,0,0,10,100,10);
}

void Rendering::DrawPause()
{
	clear_to_color(pause_b,makecol(0,0,0));
	textprintf_ex(pause_b, font, 0, 0, makecol(255, 255, 255), -1, "Game Paused");
	blit(pause_b,buffer,0,0,270,250,90,10);
}

void Rendering::ErasePause()
{
	clear_to_color(pause_b,makecol(0,0,0));
	blit(pause_b,buffer,0,0,270,250,90,10);
}

void Rendering::DrawReadyScreen()
{
	clear_to_color(ready_b,makecol(0,0,0));
	textprintf_ex(ready_b, font, 0, 0, makecol(255, 255, 255), -1, "Ready?");
	blit(ready_b,screen,0,0,300,250,90,10);
	struct timeb current,start;
	ftime(&start);
	ftime(&current);

	while(current.time - start.time < 2)
		ftime(&current);

	ftime(&start);
	clear_to_color(ready_b,makecol(0,0,0));
	textprintf_ex(ready_b, font, 0, 0, makecol(255, 255, 255), -1, "Go!!!");
	blit(ready_b,screen,0,0,300,250,90,10);

	while(current.time - start.time < 2)
		ftime(&current);

	clear_to_color(ready_b,makecol(0,0,0));
	blit(ready_b,buffer,0,0,270,250,90,10);
}

void Rendering::DrawLives()
{
	int p = 30;
	clear_to_color(lives_b,makecol(0,0,0));
	BITMAP* life = load_bitmap("./images/pacman/life.bmp",NULL);
	for(int i = 0;i < Game::Singleton().GetLives()-1;i++)
		masked_blit(life,lives_b,0,0,p*i,0,20,20);

	blit(lives_b,buffer,0,0,100,450,120,20);
}

void Rendering::DrawFPS(int FPS){

	fps_b  = create_bitmap(100,10);/*Bitmap that displays the fps of the game*/
	clear_to_color(fps_b,makecol(0,0,0));
	textprintf_ex(fps_b, font, 0, 0, makecol(255, 255, 255), -1, "FPS: %d",FPS);
	blit(fps_b,buffer,0,0,0,100,100,10);
}


void Rendering::DrawGameOver()
{
	clear_to_color(game_status_b,makecol(0,0,0));
	textprintf_ex(game_status_b, font, 0, 0, makecol(255, 255, 255), -1, "Game Over");
	blit(game_status_b,buffer,0,0,270,250,140,10);
}
void Rendering::DrawGameCompleted()
{
	clear_to_color(game_status_b,makecol(0,0,0));
	textprintf_ex(game_status_b, font, 0, 0, makecol(255, 255, 255), -1, "Game Completed");
	blit(game_status_b,buffer,0,0,270,250,140,10);
}


void Rendering::DrawWelcomeScreen()
{
	BITMAP* intro = create_bitmap(640,480);
	clear_to_color(intro,makecol(0,0,0));
	textprintf_ex(intro, font, 200, 100, makecol(255, 255, 255), -1, "University of Crete"); 
	textprintf_ex(intro, font, 160, 110, makecol(255, 255, 255), -1, "Department of Computer Science");
	textprintf_ex(intro, font, 250, 130, makecol(255, 255, 255), -1, "CSD454");
	textprintf_ex(intro, font,   0, 140, makecol(255, 255, 255), -1, "Software Engineering of Intelligent, Multimedia and Mobile User Interfaces");
	textprintf_ex(intro, font, 150, 150, makecol(255, 255, 255), -1, "Term Project, Fall Semester 2006.");

	textprintf_ex(intro, font, 220, 250, makecol(255, 255, 255), -1, "Project Team");
	textprintf_ex(intro, font, 180, 260, makecol(255, 255, 255), -1, "Name : Giwrgos Kollias");
	textprintf_ex(intro, font, 230, 270, makecol(255, 255, 255), -1, "AM : 1555");
	textprintf_ex(intro, font, 350, 450, makecol(255, 255, 255), -1, "Contact at : gkollias@csd.uoc.gr");
	textprintf_ex(intro, font, 190, 350, makecol(255, 255, 255), -1, "Press space to proceed");

	blit(intro,screen,0,0,0,0,640,480);

	while(!key[KEY_SPACE]){}
	
	BITMAP* ws = load_bitmap("./images/screen_characters.bmp",NULL);
	blit(ws,buffer,0,0,0,0,SCREEN_W,SCREEN_H);
	textprintf_ex(buffer, font, 210, 450, makecol(255, 255, 255), -1, "Press enter to play");
	blit(buffer,screen,0,0,0,0,SCREEN_W,SCREEN_H);
	

	while(!key[KEY_ENTER]){}
	clear_to_color(buffer,makecol(0,0,0));
	destroy_bitmap(intro);
	destroy_bitmap(ws);
}


void Rendering::DrawObstacles()
{
	std::vector<Obstacle *> ob = ObstacleHolder::Singleton().GetObstacles();
	for(std::vector<Obstacle *>::iterator i = ob.begin(); i!=ob.end(); i++)
	{
		int x = (*i)->GetX();
		int y = (*i)->GetY();
		int w = (*i)->GetW();
		int h = (*i)->GetH();
		//line( screen , x , y , x+w , y , makecol(0, 0, 255) );
		//line( screen , x+w , y , x+w , y+h , makecol(0, 0, 255) );
		//line( screen , x+w , y+h , x , y+h , makecol(0, 0, 255) );
		//line( screen , x , y+h , x , y , makecol(0, 0, 255) );
		blit((*i)->GetBitmap(),buffer,0,0,x,y,w,h);
	}
//	BITMAP* b = load_bitmap("images/obstacles/blue_ob.bmp",NULL);
//	blit(b,screen,0,0,307,15,2,2);
textprintf_ex(buffer, font, 0, 0, makecol(255, 255, 255), -1, "Player 1");
textprintf_ex(buffer, font, 470, 470, makecol(255, 255, 255), -1, "Press Escape to quit.");
}