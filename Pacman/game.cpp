#include "game.h"

Game* Game::game;

void Game::PacmanMove()
{
	if(key[KEY_UP])
	{
		if(!ObstacleHolder::Singleton().CanMove((Sprite *)pac_sprite, 0, PACMAN_STEP_UP))
		{
			move_up = true;
			move_down = false;
			move_left = false;
			move_right = false;
		}
	}
	if(key[KEY_DOWN])
	{
		if(!ObstacleHolder::Singleton().CanMove((Sprite *)pac_sprite, 0, PACMAN_STEP_DOWN))
		{
			move_up = false;
			move_down = true;
			move_left = false;
			move_right = false;
		}
	}
	if(key[KEY_LEFT])
	{
		if(!ObstacleHolder::Singleton().CanMove((Sprite *)pac_sprite, PACMAN_STEP_LEFT, 0))
		{
			move_up = false;
			move_down = false;
			move_left = true;
			move_right = false;
		}
	}
	if(key[KEY_RIGHT])
	{
		if(!ObstacleHolder::Singleton().CanMove((Sprite *)pac_sprite, PACMAN_STEP_RIGHT, 0))
		{
			move_up = false;
			move_down = false;
			move_left = false;
			move_right = true;
		}
	}


	if(move_up == true)
	{
		struct timeb offset;
		/*Time Shift for frame range animator*/
		offset = currtime;
		if(offset.millitm >= PACMAN_FRAME_RANGE_DELAY)
			offset.millitm-=PACMAN_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - PACMAN_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		pacFRAnimatorUp->SetLastTime(offset);
		/*Time Shift for moving animator*/
		offset = currtime;
		if(offset.millitm >= PACMAN_MOVE_DELAY)
			offset.millitm-=PACMAN_MOVE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - PACMAN_MOVE_DELAY;
			offset.time -= 1;
		}
		pacMovAnimatorUp->SetLastTime(offset);

		pac_sprite->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/pacman/pacman_up.bmp")));

		/*Frame range Animators*/
		AnimatorHolder::MarkAsRunning(pacFRAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorRight);
		/*Moving Animators*/
		AnimatorHolder::MarkAsRunning(pacMovAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorRight);
	}
	else if(move_down == true)
	{
		struct timeb offset;
		/*Time Shift for frame range animator*/
		offset = currtime;
		if(offset.millitm >= PACMAN_FRAME_RANGE_DELAY)
			offset.millitm-=PACMAN_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - PACMAN_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		pacFRAnimatorDown->SetLastTime(offset);
		/*Time Shift for moving animator*/
		offset = currtime;
		if(offset.millitm >= PACMAN_MOVE_DELAY)
			offset.millitm-=PACMAN_MOVE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - PACMAN_MOVE_DELAY;
			offset.time -= 1;
		}
		pacMovAnimatorDown->SetLastTime(offset);
		pac_sprite->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/pacman/pacman_down.bmp")));

		/*Frame range Animators*/
		AnimatorHolder::MarkAsRunning(pacFRAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorRight);
		/*Moving Animators*/
		AnimatorHolder::MarkAsRunning(pacMovAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorRight);
		
	}
	else if(move_right == true)
	{
		struct timeb offset;
		/*Time Shift for frame range animator*/
		offset = currtime;
		if(offset.millitm >= PACMAN_FRAME_RANGE_DELAY)
			offset.millitm-=PACMAN_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - PACMAN_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		pacFRAnimatorRight->SetLastTime(offset);
		/*Time Shift for moving animator*/
		offset = currtime;
		if(offset.millitm >= PACMAN_MOVE_DELAY)
			offset.millitm-=PACMAN_MOVE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - PACMAN_MOVE_DELAY;
			offset.time -= 1;
		}
		pacMovAnimatorRight->SetLastTime(offset);
		pac_sprite->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/pacman/pacman_right.bmp")));

		/***************************************************************************/
		//obstacleMovAnimatorDown->SetLastTime(offset);
		//AnimatorHolder::MarkAsRunning(obstacleMovAnimatorDown);
		//AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorUp);
		/***************************************************************************/

		/*Frame range Animators*/
		AnimatorHolder::MarkAsRunning(pacFRAnimatorRight);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorLeft);	
		/*Moving Animators*/
		AnimatorHolder::MarkAsRunning(pacMovAnimatorRight);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorLeft);	
	}
	else if(move_left == true)
	{
		struct timeb offset;
		/*Time Shift for frame range animator*/
		offset = currtime;
		if(offset.millitm >= PACMAN_FRAME_RANGE_DELAY)
			offset.millitm-=PACMAN_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - PACMAN_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		pacFRAnimatorLeft->SetLastTime(offset);
		/*Time Shift for moving animator*/
		offset = currtime;
		if(offset.millitm >= PACMAN_MOVE_DELAY)
			offset.millitm-=PACMAN_MOVE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - PACMAN_MOVE_DELAY;
			offset.time -= 1;
		}
		pacMovAnimatorLeft->SetLastTime(offset);
		pac_sprite->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/pacman/pacman_left.bmp")));
		
		/***************************************************************************/
		//obstacleMovAnimatorUp->SetLastTime(offset);
		//AnimatorHolder::MarkAsRunning(obstacleMovAnimatorUp);
		//AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorDown);
		/***************************************************************************/
		/*Frame range Animators*/
		AnimatorHolder::MarkAsRunning(pacFRAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorRight);
		/*Moving Animators*/
		AnimatorHolder::MarkAsRunning(pacMovAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pacMovAnimatorRight);
	}
	if(!pac_sprite->IsAlive())
	{
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= 6*PACMAN_FRAME_RANGE_DELAY)
			offset.millitm-=6*PACMAN_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - 6*PACMAN_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		pacFRAnimatorDead->SetLastTime(offset);
		pac_sprite->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/pacman/dead_pacman.bmp")));

		AnimatorHolder::MarkAsRunning(pacFRAnimatorDead);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorRight);
		AnimatorHolder::MarkAsSuspended(pacFRAnimatorLeft);
		move_up = false;
		move_down = false;
		move_left = true;
		move_right = false;
	}
	clear_keybuf();	
}

Game& Game::Singleton (void)
{
	if(game == NULL)
		game = new Game();
	return *game;
}
Game::Game()
{ 
	pac_sprite = GameInitialization::Singleton().GetPacman();
	inky = GameInitialization::Singleton().GetInky();
	pinky = GameInitialization::Singleton().GetPinky();
	blinky = GameInitialization::Singleton().GetBlinky();
	clyde = GameInitialization::Singleton().GetClyde();

	pacFRAnimatorUp     = GameInitialization::Singleton().GetPacmanFRAnimator("up");
	pacFRAnimatorDown   = GameInitialization::Singleton().GetPacmanFRAnimator("down");
	pacFRAnimatorRight  = GameInitialization::Singleton().GetPacmanFRAnimator("right");
	pacFRAnimatorLeft   = GameInitialization::Singleton().GetPacmanFRAnimator("left");
	pacFRAnimatorDead   = GameInitialization::Singleton().GetPacmanFRAnimator("dead");
	pacMovAnimatorUp    = GameInitialization::Singleton().GetPacmanMovAnimator("up");
	pacMovAnimatorDown  = GameInitialization::Singleton().GetPacmanMovAnimator("down");
	pacMovAnimatorRight = GameInitialization::Singleton().GetPacmanMovAnimator("right");
	pacMovAnimatorLeft  = GameInitialization::Singleton().GetPacmanMovAnimator("left");

	inkyAnimatorUp    = GameInitialization::Singleton().GetGhostAnimator("inky","up");
	inkyAnimatorDown  = GameInitialization::Singleton().GetGhostAnimator("inky","down");
	inkyAnimatorRight = GameInitialization::Singleton().GetGhostAnimator("inky","right");
	inkyAnimatorLeft  = GameInitialization::Singleton().GetGhostAnimator("inky","left");

	pinkyAnimatorUp    = GameInitialization::Singleton().GetGhostAnimator("pinky","up");
	pinkyAnimatorDown  = GameInitialization::Singleton().GetGhostAnimator("pinky","down");
	pinkyAnimatorRight = GameInitialization::Singleton().GetGhostAnimator("pinky","right");
	pinkyAnimatorLeft  = GameInitialization::Singleton().GetGhostAnimator("pinky","left");

	blinkyAnimatorUp    = GameInitialization::Singleton().GetGhostAnimator("blinky","up");
	blinkyAnimatorDown  = GameInitialization::Singleton().GetGhostAnimator("blinky","down");
	blinkyAnimatorRight = GameInitialization::Singleton().GetGhostAnimator("blinky","right");
	blinkyAnimatorLeft  = GameInitialization::Singleton().GetGhostAnimator("blinky","left");

	clydeAnimatorUp    = GameInitialization::Singleton().GetGhostAnimator("clyde","up");
	clydeAnimatorDown  = GameInitialization::Singleton().GetGhostAnimator("clyde","down");
	clydeAnimatorRight = GameInitialization::Singleton().GetGhostAnimator("clyde","right");
	clydeAnimatorLeft  = GameInitialization::Singleton().GetGhostAnimator("clyde","left");
	ftime(&currtime);
	
	players = 1;
	score = 0;
	ghosts_eaten = 0;
	lives = 4;
	marks = 247;
	game_over = false;
	game_completed = false;
	blue_flag = false;
	move_left = true;
	move_right = false;
	move_up = false;
	move_down = false;
	in_bonus = false;
	sound_played = false;
	Ai::Singleton();
	Rendering::Singleton();
	Rendering::Singleton().Register(pac_sprite);
	Rendering::Singleton().Register(inky);
	Rendering::Singleton().Register(pinky);
	Rendering::Singleton().Register(blinky);
	Rendering::Singleton().Register(clyde);

	CollisionChecker::Singleton().Register(pac_sprite , inky);
	CollisionChecker::Singleton().Register(pac_sprite , pinky);
	CollisionChecker::Singleton().Register(pac_sprite , blinky);
	CollisionChecker::Singleton().Register(pac_sprite , clyde);

	if ( install_sound( DIGI_AUTODETECT, MIDI_NONE, "" ) != 0 )
		allegro_message("Error with sound");

	eatdot = load_sample("./sounds/eatdot.wav");
	eatghost = load_sample("./sounds/eatghost.wav");
	attack = load_sample("./sounds/attack.wav");

	playsound_eatdot = allocate_voice(eatdot);
	playsound_eatghost = allocate_voice(eatghost);

	/***************************************************************************************/
	struct timeb lasttime;
	ftime(&lasttime);
	/*Initializing obstacle*/
	moving_obstacle = GameInitialization::Singleton().GetObstacles()[0];
	/*moving_obstacle->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/obstacles/1.bmp")));*/
	moving_obstacle->SetDirection(LEFT);
	moving_obstacle = GameInitialization::Singleton().GetObstacles()[1];
	moving_obstacle->SetDirection(RIGHT);
	/*Obstacle animation Up*/
	obstacleMovAnimatorUp    = new MovingAnimator();
	MovingAnimation* obstacleMovAnimationUp    = new MovingAnimation(0,-1,PACMAN_MOVE_DELAY,false,0);
	obstacleMovAnimatorUp->Start(moving_obstacle , obstacleMovAnimationUp , lasttime );
	/*AnimatorHolder::Register(obstacleMovAnimatorUp);
	AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorUp);*/
	/*Obstacle animation Down*/
	obstacleMovAnimatorDown    = new MovingAnimator();
	MovingAnimation* obstacleMovAnimationDown  = new MovingAnimation(0,1,PACMAN_MOVE_DELAY,false,0);
	obstacleMovAnimatorDown->Start(moving_obstacle , obstacleMovAnimationDown , lasttime );
	/*AnimatorHolder::Register(obstacleMovAnimatorDown);
	AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorDown);*/
	/*Obstacle animation Right*/
	obstacleMovAnimatorRight    = new MovingAnimator();
	MovingAnimation* obstacleMovAnimationRight  = new MovingAnimation(1,0,PACMAN_MOVE_DELAY,false,0);
	obstacleMovAnimatorRight->Start(moving_obstacle , obstacleMovAnimationRight , lasttime );
	/*AnimatorHolder::Register(obstacleMovAnimatorRight);
	AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorRight);*/
	/*Obstacle animation Left*/
	obstacleMovAnimatorLeft    = new MovingAnimator();
	MovingAnimation* obstacleMovAnimationLeft  = new MovingAnimation(-1,0,PACMAN_MOVE_DELAY,false,0);
	obstacleMovAnimatorLeft->Start(moving_obstacle , obstacleMovAnimationLeft , lasttime );
	/*AnimatorHolder::Register(obstacleMovAnimatorLeft);
	AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorLeft);*/
	/**************************************************************************************/
}
Game::~Game()
{
	stop_sample(attack);
	voice_stop(playsound_eatdot);
	voice_stop(playsound_eatghost);
	deallocate_voice(playsound_eatdot);
	deallocate_voice(playsound_eatghost);
}


void Game::ObstacleMoveAll(){
	std::vector<int > mov_ob = GameInitialization::Singleton().GetMovingObstacles();
	std::vector<int >::iterator i;
	for(i = mov_ob.begin();i!=mov_ob.end();i++)
		ObstacleMove(*i);
}

void Game::ObstacleMove(unsigned short obstacle_id){

	unsigned int x,y,init_x,init_y;
	moving_obstacle = GameInitialization::Singleton().GetObstacles()[obstacle_id];
	obstacleMovAnimatorUp->SetSprite(moving_obstacle);
	obstacleMovAnimatorDown->SetSprite(moving_obstacle);
	obstacleMovAnimatorRight->SetSprite(moving_obstacle);
	obstacleMovAnimatorLeft->SetSprite(moving_obstacle);
	x = moving_obstacle->GetX();
	y = moving_obstacle->GetY();
	init_x = moving_obstacle->GetInitX();
	init_y = moving_obstacle->GetInitY();

	struct timeb offset;
	/*Time Shift for moving animator*/
	offset = currtime;
	if(offset.millitm >= PACMAN_MOVE_DELAY)
		offset.millitm-=PACMAN_MOVE_DELAY;
	else
	{
		offset.millitm=1000 + offset.millitm - PACMAN_MOVE_DELAY;
		offset.time -= 1;
	}

	if(moving_obstacle->GetDirection() == UP)
	{
		if(( y >= init_y - PASSAGE_WIDTH ) && ( y <= init_y + PASSAGE_WIDTH))
		{
			obstacleMovAnimatorUp->SetLastTime(offset);
			obstacleMovAnimatorUp->Progress(Game::Singleton().GetGameTime());
			/*Using the animator Holder*/
			/*AnimatorHolder::MarkAsRunning(obstacleMovAnimatorUp);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorDown);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorRight);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorLeft);*/
		}
		else
		{
			obstacleMovAnimatorDown->SetLastTime(offset);
			obstacleMovAnimatorDown->Progress(Game::Singleton().GetGameTime());
			/*AnimatorHolder::MarkAsRunning(obstacleMovAnimatorDown);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorUp);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorRight);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorLeft);*/
			moving_obstacle->SetDirection(DOWN);
		}
	}
	else if(moving_obstacle->GetDirection() == DOWN)
	{
		if((y <= init_y + PASSAGE_WIDTH ) && ( y >= init_y - PASSAGE_WIDTH))
		{
			obstacleMovAnimatorDown->SetLastTime(offset);
			obstacleMovAnimatorDown->Progress(Game::Singleton().GetGameTime());
			/*AnimatorHolder::MarkAsRunning(obstacleMovAnimatorDown);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorUp);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorRight);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorLeft);*/
		}
		else
		{
			obstacleMovAnimatorUp->SetLastTime(offset);
			obstacleMovAnimatorUp->Progress(Game::Singleton().GetGameTime());
			/*AnimatorHolder::MarkAsRunning(obstacleMovAnimatorUp);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorDown);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorRight);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorLeft);*/
			moving_obstacle->SetDirection(UP);
		}
	}
	else if(moving_obstacle->GetDirection() == RIGHT)
	{
		if((x <= init_x + PASSAGE_WIDTH ) && ( x >= init_x - PASSAGE_WIDTH))
		{
			obstacleMovAnimatorRight->SetLastTime(offset);
			obstacleMovAnimatorRight->Progress(Game::Singleton().GetGameTime());
			/*AnimatorHolder::MarkAsRunning(obstacleMovAnimatorRight);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorLeft);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorUp);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorDown);*/
		}
		else
		{
			obstacleMovAnimatorLeft->SetLastTime(offset);
			obstacleMovAnimatorLeft->Progress(Game::Singleton().GetGameTime());
			/*AnimatorHolder::MarkAsRunning(obstacleMovAnimatorLeft);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorRight);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorUp);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorDown);*/
			moving_obstacle->SetDirection(LEFT);
		}
	}
	else if(moving_obstacle->GetDirection() == LEFT)
	{
		if((x <= init_x + PASSAGE_WIDTH ) && ( x >= init_x - PASSAGE_WIDTH))
		{
			obstacleMovAnimatorLeft->SetLastTime(offset);
			obstacleMovAnimatorLeft->Progress(Game::Singleton().GetGameTime());
			/*AnimatorHolder::MarkAsRunning(obstacleMovAnimatorLeft);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorRight);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorUp);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorDown);*/
		}
		else
		{
			obstacleMovAnimatorRight->SetLastTime(offset);
			obstacleMovAnimatorRight->Progress(Game::Singleton().GetGameTime());
			/*AnimatorHolder::MarkAsRunning(obstacleMovAnimatorRight);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorLeft);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorUp);
			AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorDown);*/
			moving_obstacle->SetDirection(RIGHT);
		}
	}

}

void Game::SetPlayers(unsigned short _players){ players = _players; }
unsigned short Game::GetPlayers(void){ return players;}

void Game::SetPlayerId(unsigned short _player_id){ player_id = _player_id; }
unsigned short Game::GetPlayerId(void){ return player_id;}

bool Game::IsGameOver(){	return game_over;	}
bool Game::IsGameCompleted(){	return game_completed;	}

void Game::CheckGameStatus()
{
	if( marks == 0 )
		game_completed = true;
	if( lives == 0)
		game_over = true;
}



void Game::SetGameTime(){	ftime(&currtime);	}
timeb Game::GetGameTime(){	return currtime;	}


void Game::PlayEatDot()
{
	if(voice_get_position(playsound_eatdot)<0)
		voice_start(playsound_eatdot);
}

void Game::PlayEatGhost()
{
	if(voice_get_position(playsound_eatghost)<0)
		voice_start(playsound_eatghost);
}

void Game::PlayAttack(bool play)
{
	if (play)
		play_sample(attack, 255, 128, 1000, 1);
	else
		stop_sample(attack);
}

void Game::CheckMarksCollision()
{
	//add according to the position
	/*std::list<SimpleMark *>::iterator i;
	std::list<SimpleMark *> sml = SimpleMarkHolder::Singleton().GetSMarks();
	for(i = sml.begin();i!=sml.end();i++)
	{
		int x = abs((*i)->GetX() - pac_sprite->GetX());
		int y = abs((*i)->GetX() - pac_sprite->GetY());
		if( x > 0 && x < 30 && y > 0 && y < 30)
		{
			CollisionChecker::Singleton().Register(pac_sprite , *i);
		}
		else
				CollisionChecker::Singleton().Cancel(pac_sprite , *i);
	}
	std::list<PowerMark *>::iterator p;
	std::list<PowerMark *> pml = PowerMarkHolder::Singleton().GetPMarks();

	for(p = pml.begin();p!=pml.end();p++)
	{
		
		int x = abs((*p)->GetX() - pac_sprite->GetX());
		int y = abs((*p)->GetX() - pac_sprite->GetY());
		if( x > 0 && x < 20 && y > 0 && y < 20)
		{
			CollisionChecker::Singleton().Register(pac_sprite , *p);
		}
		else
			CollisionChecker::Singleton().Cancel(pac_sprite , *p);
	}*/
	//add all
	std::list<SimpleMark *>::iterator i;
	std::list<SimpleMark *> sml = SimpleMarkHolder::Singleton().GetSMarks();
	for(i = sml.begin();i!=sml.end();i++)
	{
		if((*i)->CanEat())
			CollisionChecker::Singleton().Register(pac_sprite , *i);
	}
	std::list<PowerMark *>::iterator p;
	std::list<PowerMark *> pml = PowerMarkHolder::Singleton().GetPMarks();
	for(p = pml.begin();p!=pml.end();p++)
	{
		if((*p)->CanEat())
			CollisionChecker::Singleton().Register(pac_sprite , *p);
	}
}


void Game::IncScore(int increment){	score += increment;	}
int Game::GetScore(){	return score;	}

void Game::ChangeGhostStatus(bool status)
{
	if(inky->IsAlive())
		inky->SetCanEat(status);
	if(pinky->IsAlive())
		pinky->SetCanEat(status);
	if(blinky->IsAlive())
		blinky->SetCanEat(status);
	if(clyde->IsAlive())
		clyde->SetCanEat(status);
}



void Game::StartBonusTimer(){	ftime(&bonus_time);in_bonus = true;	}
void Game::CheckBonusTimer()
{
	struct timeb time_now;
	ftime(&time_now);
	if( (time_now.time - bonus_time.time == BONUS_TIME) && (time_now.millitm - bonus_time.millitm > 0))
	{
		pac_sprite->SetCanEat(false);
		ChangeGhostStatus(true);
		ghosts_eaten = 0;
		PlayAttack(false);
		sound_played = false;
	}
	else
	{
		if(sound_played == false)
		{
			PlayAttack(true);
			sound_played = true;
		}
	}

}


void Game::GhostEatInc(){	ghosts_eaten += 1;	}
int Game::GetGhostEat(){	return ghosts_eaten;	}

int Game::GetLives(){	return lives;	}
void Game::DecLives()
{
	lives--;
	if(lives == 0)
		game_over = true;
	else
		pac_sprite->SetIsAlive(true);
}

int Game::GetMarks(){	return marks;	}
void Game::DecMarks(){	marks -=1;	}

void Game::InitPlaces()
{
	pac_sprite->SetX(308);
	pac_sprite->SetY(325);
	inky->SetX(308);
	inky->SetY(240);
	pinky->SetX(472);
	pinky->SetY(73);
	blinky->SetX(181);
	blinky->SetY(73);
	clyde->SetX(305);
	clyde->SetY(73);

}


void Game::Pause()
{
	while(key[KEY_P]) { } 
	Rendering::Singleton().DrawPause();
	while(!key[KEY_P]) { } 
	Rendering::Singleton().ErasePause();
	while(key[KEY_P]) { } 
}
void Game::GhostMove()
{
	Ai::Singleton().GhostMove(inky,pac_sprite);
	Ai::Singleton().GhostMove(pinky,pac_sprite);
	Ai::Singleton().GhostMove(blinky,pac_sprite);
	Ai::Singleton().GhostMove(clyde,pac_sprite);
	
	if( in_bonus == true && (currtime.time - bonus_time.time > BONUS_TIME - 4) && (currtime.millitm - bonus_time.millitm > 0))
	{
		ftime(&ghost_time);
		in_bonus = false;
	}

	if( in_bonus == false)
	{			
		if((currtime.time > ghost_time.time) || (currtime.time == ghost_time.time && currtime.millitm > ghost_time.millitm))
		{
			if(blue_flag)
			{
				blue_flag = false;
				if(ghost_time.millitm > 500)
				{
					ghost_time.time += 1;
					ghost_time.millitm -= 500; 
				}
				else
					ghost_time.millitm += 500; 
				}
			else
			{
				blue_flag = true;
				if(ghost_time.millitm > 500)
				{
					ghost_time.time += 1;
					ghost_time.millitm -= 500; 
				}
				else
					ghost_time.millitm += 500; 
			}
		}
	}


	if(inky->GetDirection() == UP)
	{
		if(!inky->IsAlive())
			inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_up.bmp")));
		else if(inky->CanEat())
			inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/inky_up.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		inkyAnimatorUp->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(inkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorRight);
	}
	if(inky->GetDirection() == DOWN)
	{
		if(!inky->IsAlive())
			inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_down.bmp")));
		else if(inky->CanEat())
			inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/inky_down.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		inkyAnimatorDown->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(inkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorRight);
		
		
	}
	if(inky->GetDirection() == LEFT)
	{
		if(!inky->IsAlive())
			inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_left.bmp")));
		else if(inky->CanEat())
			inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/inky_left.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		inkyAnimatorLeft->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(inkyAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorRight);
	
	}
	if(inky->GetDirection() == RIGHT)
	{
		if(!inky->IsAlive())
			inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_right.bmp")));
		else if(inky->CanEat())
			inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/inky_right.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		inkyAnimatorRight->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(inkyAnimatorRight);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(inkyAnimatorLeft);
		
	}

	if(pinky->GetDirection() == UP)
	{
		if(!pinky->IsAlive())
			pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_up.bmp")));
		else if(pinky->CanEat())
			pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/pinky_up.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		pinkyAnimatorUp->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(pinkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorRight);
		
	}
	if(pinky->GetDirection() == DOWN)
	{
		if(!pinky->IsAlive())
			pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_down.bmp")));
		else if(pinky->CanEat())
			pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/pinky_down.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		pinkyAnimatorDown->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(pinkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorRight);
	}
	if(pinky->GetDirection() == LEFT)
	{
		if(!pinky->IsAlive())
			pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_left.bmp")));
		else if(pinky->CanEat())
			pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/pinky_left.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}

		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		pinkyAnimatorLeft->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(pinkyAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorRight);
	}
	if(pinky->GetDirection() == RIGHT)
	{
		if(!pinky->IsAlive())
			pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_right.bmp")));
		else if(pinky->CanEat())
			pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/pinky_right.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		pinkyAnimatorRight->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(pinkyAnimatorRight);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(pinkyAnimatorLeft);
		
	}

	if(blinky->GetDirection() == UP)
	{
		if(!blinky->IsAlive())
			blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_up.bmp")));
		else if(blinky->CanEat())
			blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blinky_up.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		blinkyAnimatorUp->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(blinkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorRight);
		
	}
	if(blinky->GetDirection() == DOWN)
	{
		if(!blinky->IsAlive())
			blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_down.bmp")));
		else if(blinky->CanEat())
			blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blinky_down.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		blinkyAnimatorDown->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(blinkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorRight);
		
	}
	if(blinky->GetDirection() == LEFT)
	{
		if(!blinky->IsAlive())
			blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_left.bmp")));
		else if(blinky->CanEat())
			blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blinky_left.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		blinkyAnimatorLeft->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(blinkyAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorRight);
	}
	if(blinky->GetDirection() == RIGHT)
	{
		if(!blinky->IsAlive())
			blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_right.bmp")));
		else if(blinky->CanEat())
			blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blinky_right.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		blinkyAnimatorRight->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(blinkyAnimatorRight);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorUp);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorDown);
		AnimatorHolder::MarkAsSuspended(blinkyAnimatorLeft);
	}

	if(clyde->GetDirection() == UP)
	{
		if(!clyde->IsAlive())
			clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_up.bmp")));
		else if(clyde->CanEat())
			clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/clyde_up.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		clydeAnimatorUp->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(clydeAnimatorUp);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorDown);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorRight);
	}
	if(clyde->GetDirection() == DOWN)
	{
		if(!clyde->IsAlive())
			clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_down.bmp")));
		else if(clyde->CanEat())
			clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/clyde_down.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		clydeAnimatorDown->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(clydeAnimatorDown);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorUp);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorRight);
	}
	if(clyde->GetDirection() == LEFT)
	{
		if(!clyde->IsAlive())
			clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_left.bmp")));
		else if(clyde->CanEat())
			clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/clyde_left.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		clydeAnimatorLeft->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(clydeAnimatorLeft);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorUp);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorDown);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorRight);
	}
	if(clyde->GetDirection() == RIGHT)
	{
		if(!clyde->IsAlive())
			clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/eyes_right.bmp")));
		else if(clyde->CanEat())
			clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/clyde_right.bmp")));
		else
		{
			if( in_bonus == false)
			{
				if(blue_flag)
					clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));	
				else
					clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/white_ghost.bmp")));
			}
			else
				clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blue_ghost.bmp")));
		}
		struct timeb offset;
		offset = currtime;
		if(offset.millitm >= GHOST_FRAME_RANGE_DELAY)
			offset.millitm-=GHOST_FRAME_RANGE_DELAY;
		else
		{
			offset.millitm=1000 + offset.millitm - GHOST_FRAME_RANGE_DELAY;
			offset.time -= 1;
		}
		clydeAnimatorRight->SetLastTime(offset);
		AnimatorHolder::MarkAsRunning(clydeAnimatorRight);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorUp);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorDown);
		AnimatorHolder::MarkAsSuspended(clydeAnimatorLeft);
	}

}



