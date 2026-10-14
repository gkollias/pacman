#include "gameInitialization.h"
#include "game.h"

GameInitialization* GameInitialization::gi;

GameInitialization& GameInitialization::Singleton()
	{
		if(gi == NULL)
			gi = new GameInitialization();
		return *gi;
	}

GameInitialization::GameInitialization()
{}

void GameInitialization::BoxesInit(std::string path , enum bitmap_option option)
{
	Rectangle rect;
	int x=0,y=0,w=0,h=0;
	FILE *fp;
	fp = fopen(path.c_str(),"r");
	if( fp == NULL)
		return;
	while(!feof(fp))
	{
		fscanf( fp , "%d %d %d %d", &x,&y,&w,&h );
		rect.x = x;
		rect.y = y;
		rect.w = w;
		rect.h = h;
		if( option == PACMAN)
			pac_boxes.push_back(rect);
		if( option == PACMAN_DEAD)
			pac_dead_boxes.push_back(rect);
		if( option == GHOST)
			ghost_boxes.push_back(rect);
		if( option == SIMPLE_MARK)
			simple_boxes.push_back(rect);
		if( option == POWER_MARK)
			power_boxes.push_back(rect);
	}
	fclose(fp);
}

void GameInitialization::ObstaclesInit(std::string path)
{
	Rectangle rect;
	int x=0,y=0,w=0,h=0;
	char dum[40];
	BITMAP* bmp;
	FILE *fp;
	fp = fopen(path.c_str(),"r");
	if( fp == NULL)
		return;
	while(!feof(fp))
	{
		fscanf( fp , "%d %d %d %d %s", &x,&y,&w,&h,dum );
		Obstacle* ob = new Obstacle(x,y,w,h);
		bmp = load_bitmap( dum , NULL);
		ob->SetBitmap(bmp);
		oh_obstacles.push_back(ob);
		/*Obstacle Boxes Init*/
		rect.x = 0;
		rect.y = 0;
		rect.w = w;
		rect.h = h;
		obstacle_boxes.push_back(rect);
	}
	fclose(fp);
}

void GameInitialization::BitmapNamesInit(std::string path)
{
	std::string name;
	char* dum;
	dum = (char *)malloc(80);
	FILE *fp;

	fp = fopen(path.c_str(),"r");
	if( fp == NULL)
	{return;}
	while(!feof(fp))
	{
		
		fscanf( fp , "%s",dum);
		name = "";
		name.append(dum);
		bitmap_names.push_back(name);
	}
	fclose(fp);
	free(dum);
}

void GameInitialization::SimpleMarksInit(std::string path)
{
	int x=0,y=0,id_ctr = 1;
	char dum[100];
	FILE *fp;
	fp = fopen(path.c_str(),"r");
	if( fp == NULL)
		return;
	while(!feof(fp))
	{
		fscanf( fp , "%d %d %s", &x,&y,dum );
		SimpleMark* sm = new SimpleMark(x,y);
		sm->SetId(id_ctr++);
		SimpleMarkHolder::Singleton().AddSimpleMark(sm);
	}
	fclose(fp);
}
void GameInitialization::PowerMarksInit(std::string path)
{
	int x=0,y=0,id_ctr = 1;
	char dum[40];
	FILE *fp;
	fp = fopen(path.c_str(),"r");
	if( fp == NULL)
		return;
	while(!feof(fp))
	{
		fscanf( fp , "%d %d %s", &x,&y,dum );
		PowerMark* pm = new PowerMark(x,y);
		pm->SetId(id_ctr++);
		PowerMarkHolder::Singleton().AddPowerMark(pm);
	}
	fclose(fp);
}

std::vector<Rectangle> GameInitialization::GetBoxes(enum bitmap_option option)
{	
	if( option == PACMAN)
		return pac_boxes;
	if( option == PACMAN_DEAD)
		return pac_dead_boxes;
	if( option == GHOST)
		return ghost_boxes;
	if( option == SIMPLE_MARK)
		return simple_boxes;
	if( option == POWER_MARK)
		return power_boxes;
	if( option == OBSTACLE)
		return power_boxes;
	else 
		assert(0);
	return pac_boxes;
}

void GameInitialization::MovingObstaclesInit(std::string path){
	int x=0;
	FILE *fp;
	fp = fopen(path.c_str(),"r");
	if( fp == NULL)
		return;
	while(!feof(fp))
	{
		fscanf( fp , "%d\n", &x );
		moving_obstacles_indexes.push_back(x);
	}
	fclose(fp);
}
std::vector<int > GameInitialization::GetMovingObstacles(){	return moving_obstacles_indexes;}

std::vector<Obstacle *> GameInitialization::GetObstacles(){	return oh_obstacles;	}

std::list<std::string> GameInitialization::GetBitmapNames(){	return bitmap_names;}


void GameInitialization::PacmanInit()
{
	pac_sprite = new PacMan(308 , 325);
	pac_sprite->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/pacman/pacman_left.bmp")));

	/*Frame Range Animators*/
	pacFRAnimatorUp = new FrameRangeAnimator();
	pacFRAnimatorDown = new FrameRangeAnimator();
	pacFRAnimatorRight = new FrameRangeAnimator();
	pacFRAnimatorLeft = new FrameRangeAnimator();
	pacFRAnimatorDead = new FrameRangeAnimator();

	/*Moving Animators*/
	pacMovAnimatorUp    = new MovingAnimator();
	pacMovAnimatorDown  = new MovingAnimator();
	pacMovAnimatorRight = new MovingAnimator();
	pacMovAnimatorLeft  = new MovingAnimator();

	FrameRangeAnimation* pacFRAnimationUp    = new FrameRangeAnimation(0,2, 0,0,PACMAN_FRAME_RANGE_DELAY,false,0);
	FrameRangeAnimation* pacFRAnimationDown  = new FrameRangeAnimation(0,2, 0,0,PACMAN_FRAME_RANGE_DELAY,false,1);
	FrameRangeAnimation* pacFRAnimationRight = new FrameRangeAnimation(0,2, 0,0,PACMAN_FRAME_RANGE_DELAY,false,2);
	FrameRangeAnimation* pacFRAnimationLeft  = new FrameRangeAnimation(0,2, 0,0,PACMAN_FRAME_RANGE_DELAY,false,3);
	FrameRangeAnimation* pacFRAnimationDead  = new FrameRangeAnimation(0,5, 0,0,20,false,8);

	MovingAnimation* pacMovAnimationUp    = new MovingAnimation(0,PACMAN_STEP_UP,PACMAN_MOVE_DELAY,false,0);
	MovingAnimation* pacMovAnimationDown  = new MovingAnimation(0,PACMAN_STEP_DOWN,PACMAN_MOVE_DELAY,false,1);
	MovingAnimation* pacMovAnimationRight = new MovingAnimation(PACMAN_STEP_RIGHT, 0,PACMAN_MOVE_DELAY,false,2);
	MovingAnimation* pacMovAnimationLeft  = new MovingAnimation(PACMAN_STEP_LEFT, 0,PACMAN_MOVE_DELAY,false,3);

	struct timeb lasttime;
	ftime(&lasttime);
	pacFRAnimatorUp->Start(pac_sprite , pacFRAnimationUp , lasttime );
	pacFRAnimatorDown->Start(pac_sprite , pacFRAnimationDown , lasttime );
	pacFRAnimatorRight->Start(pac_sprite , pacFRAnimationRight , lasttime );
	pacFRAnimatorLeft->Start(pac_sprite , pacFRAnimationLeft , lasttime );
	pacFRAnimatorDead->Start(pac_sprite , pacFRAnimationDead , lasttime );
	pacMovAnimatorUp->Start(pac_sprite , pacMovAnimationUp , lasttime );
	pacMovAnimatorDown->Start(pac_sprite , pacMovAnimationDown , lasttime );
	pacMovAnimatorRight->Start(pac_sprite , pacMovAnimationRight , lasttime );
	pacMovAnimatorLeft->Start(pac_sprite , pacMovAnimationLeft , lasttime );

	AnimatorHolder::Register(pacFRAnimatorUp);
	AnimatorHolder::Register(pacFRAnimatorDown);
	AnimatorHolder::Register(pacFRAnimatorRight);
	AnimatorHolder::Register(pacFRAnimatorLeft);
	AnimatorHolder::Register(pacFRAnimatorDead);
	AnimatorHolder::Register(pacMovAnimatorUp);
	AnimatorHolder::Register(pacMovAnimatorDown);
	AnimatorHolder::Register(pacMovAnimatorRight);
	AnimatorHolder::Register(pacMovAnimatorLeft);
	
	AnimatorHolder::MarkAsSuspended(pacFRAnimatorUp);
	AnimatorHolder::MarkAsSuspended(pacFRAnimatorDown);
	AnimatorHolder::MarkAsSuspended(pacFRAnimatorRight);
	AnimatorHolder::MarkAsSuspended(pacFRAnimatorLeft);
	AnimatorHolder::MarkAsSuspended(pacFRAnimatorDead);
	AnimatorHolder::MarkAsSuspended(pacMovAnimatorUp);
	AnimatorHolder::MarkAsSuspended(pacMovAnimatorDown);
	AnimatorHolder::MarkAsSuspended(pacMovAnimatorRight);
	AnimatorHolder::MarkAsSuspended(pacMovAnimatorLeft);


	/*Obstacle *moving_obstacle = oh_obstacles.front();
	moving_obstacle->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/obstacles/1.bmp")));

	obstacleMovAnimatorUp    = new MovingAnimator();
	MovingAnimation* obstacleMovAnimationUp    = new MovingAnimation(0,-10,PACMAN_MOVE_DELAY,false,0);
	obstacleMovAnimatorUp->Start(moving_obstacle , obstacleMovAnimationUp , lasttime );
	AnimatorHolder::Register(obstacleMovAnimatorUp);
	AnimatorHolder::MarkAsRunning(obstacleMovAnimatorUp);

	obstacleMovAnimatorDown    = new MovingAnimator();
	MovingAnimation* obstacleMovAnimationDown  = new MovingAnimation(0,10,PACMAN_MOVE_DELAY,false,0);
	obstacleMovAnimatorDown->Start(moving_obstacle , obstacleMovAnimationDown , lasttime );
	AnimatorHolder::Register(obstacleMovAnimatorDown);
	AnimatorHolder::MarkAsSuspended(obstacleMovAnimatorDown);*/
}




void GameInitialization::GhostsInit()
{
	//INKY
	inky = new Ghost(472 , 73);
	inky->SetName(INKY);
	inky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/inky_up.bmp")));
	FrameRangeAnimation* ghostAnimationUp =    new FrameRangeAnimation(0,0, 0, GHOST_STEP_UP,GHOST_FRAME_RANGE_DELAY,false,4);
	FrameRangeAnimation* ghostAnimationDown =  new FrameRangeAnimation(0,0, 0, GHOST_STEP_DOWN,GHOST_FRAME_RANGE_DELAY,false,5);
	FrameRangeAnimation* ghostAnimationRight = new FrameRangeAnimation(0,0, GHOST_STEP_RIGHT, 0,GHOST_FRAME_RANGE_DELAY,false,6);
	FrameRangeAnimation* ghostAnimationLeft =  new FrameRangeAnimation(0,0, GHOST_STEP_LEFT, 0,GHOST_FRAME_RANGE_DELAY,false,7);
	inkyAnimatorUp = new FrameRangeAnimator();
	inkyAnimatorDown = new FrameRangeAnimator();
	inkyAnimatorRight = new FrameRangeAnimator();
	inkyAnimatorLeft = new FrameRangeAnimator();
	struct timeb lasttime;
	ftime(&lasttime);
	inkyAnimatorUp->Start(inky , ghostAnimationUp , lasttime );
	inkyAnimatorDown->Start(inky , ghostAnimationDown , lasttime );
	inkyAnimatorRight->Start(inky , ghostAnimationRight , lasttime );
	inkyAnimatorLeft->Start(inky , ghostAnimationLeft , lasttime );

	AnimatorHolder::Register(inkyAnimatorUp);
	AnimatorHolder::Register(inkyAnimatorDown);
	AnimatorHolder::Register(inkyAnimatorRight);
	AnimatorHolder::Register(inkyAnimatorLeft);
	
	AnimatorHolder::MarkAsSuspended(inkyAnimatorUp);
	AnimatorHolder::MarkAsSuspended(inkyAnimatorDown);
	AnimatorHolder::MarkAsSuspended(inkyAnimatorRight);
	AnimatorHolder::MarkAsSuspended(inkyAnimatorLeft);

	//PINKY
	pinky = new Ghost(308 , 157);
	pinky->SetName(PINKY);
	pinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/pinky_up.bmp")));
	pinkyAnimatorUp = new FrameRangeAnimator();
	pinkyAnimatorDown = new FrameRangeAnimator();
	pinkyAnimatorRight = new FrameRangeAnimator();
	pinkyAnimatorLeft = new FrameRangeAnimator();

	pinkyAnimatorUp->Start(pinky , ghostAnimationUp , lasttime );
	pinkyAnimatorDown->Start(pinky , ghostAnimationDown , lasttime );
	pinkyAnimatorRight->Start(pinky , ghostAnimationRight , lasttime );
	pinkyAnimatorLeft->Start(pinky , ghostAnimationLeft , lasttime );

	AnimatorHolder::Register(pinkyAnimatorUp);
	AnimatorHolder::Register(pinkyAnimatorDown);
	AnimatorHolder::Register(pinkyAnimatorRight);
	AnimatorHolder::Register(pinkyAnimatorLeft);
	
	AnimatorHolder::MarkAsSuspended(pinkyAnimatorUp);
	AnimatorHolder::MarkAsSuspended(pinkyAnimatorDown);
	AnimatorHolder::MarkAsSuspended(pinkyAnimatorRight);
	AnimatorHolder::MarkAsSuspended(pinkyAnimatorLeft);

	//BLINKY
	blinky = new Ghost(181 , 73);
	blinky->SetName(BLINKY);
	blinky->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/blinky_up.bmp")));
	blinkyAnimatorUp = new FrameRangeAnimator();
	blinkyAnimatorDown = new FrameRangeAnimator();
	blinkyAnimatorRight = new FrameRangeAnimator();
	blinkyAnimatorLeft = new FrameRangeAnimator();

	blinkyAnimatorUp->Start(blinky , ghostAnimationUp , lasttime );
	blinkyAnimatorDown->Start(blinky , ghostAnimationDown , lasttime );
	blinkyAnimatorRight->Start(blinky , ghostAnimationRight , lasttime );
	blinkyAnimatorLeft->Start(blinky , ghostAnimationLeft , lasttime );

	AnimatorHolder::Register(blinkyAnimatorUp);
	AnimatorHolder::Register(blinkyAnimatorDown);
	AnimatorHolder::Register(blinkyAnimatorRight);
	AnimatorHolder::Register(blinkyAnimatorLeft);
	
	AnimatorHolder::MarkAsSuspended(blinkyAnimatorUp);
	AnimatorHolder::MarkAsSuspended(blinkyAnimatorDown);
	AnimatorHolder::MarkAsSuspended(blinkyAnimatorRight);
	AnimatorHolder::MarkAsSuspended(blinkyAnimatorLeft);

	//CLYDE
	clyde = new Ghost(305 , 73);
	clyde->SetName(CLYDE);
	clyde->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/ghosts/clyde_up.bmp")));
	clydeAnimatorUp = new FrameRangeAnimator();
	clydeAnimatorDown = new FrameRangeAnimator();
	clydeAnimatorRight = new FrameRangeAnimator();
	clydeAnimatorLeft = new FrameRangeAnimator();

	clydeAnimatorUp->Start(clyde , ghostAnimationUp , lasttime );
	clydeAnimatorDown->Start(clyde , ghostAnimationDown , lasttime );
	clydeAnimatorRight->Start(clyde , ghostAnimationRight , lasttime );
	clydeAnimatorLeft->Start(clyde , ghostAnimationLeft , lasttime );

	AnimatorHolder::Register(clydeAnimatorUp);
	AnimatorHolder::Register(clydeAnimatorDown);
	AnimatorHolder::Register(clydeAnimatorRight);
	AnimatorHolder::Register(clydeAnimatorLeft);
	
	AnimatorHolder::MarkAsSuspended(clydeAnimatorUp);
	AnimatorHolder::MarkAsSuspended(clydeAnimatorDown);
	AnimatorHolder::MarkAsSuspended(clydeAnimatorRight);
	AnimatorHolder::MarkAsSuspended(clydeAnimatorLeft);
}


PacMan* GameInitialization::GetPacman(){	return pac_sprite;	}
Ghost* GameInitialization::GetInky(){	return inky;	}
Ghost* GameInitialization::GetPinky(){	return pinky;	}
Ghost* GameInitialization::GetBlinky(){	return blinky;	}
Ghost* GameInitialization::GetClyde(){	return clyde;	}

FrameRangeAnimator* GameInitialization::GetPacmanFRAnimator(std::string dir)
{
	if( dir == "up")
		return pacFRAnimatorUp;
	if( dir == "down")
		return pacFRAnimatorDown;
	if( dir == "left")
		return pacFRAnimatorLeft;
	if( dir == "right")
		return pacFRAnimatorRight;
	if( dir == "dead")
		return pacFRAnimatorDead;
	return NULL;
}


MovingAnimator* GameInitialization::GetPacmanMovAnimator(std::string dir)
{
	if( dir == "up")
		return pacMovAnimatorUp;
	if( dir == "down")
		return pacMovAnimatorDown;
	if( dir == "left")
		return pacMovAnimatorLeft;
	if( dir == "right")
		return pacMovAnimatorRight;
	return NULL;
}
FrameRangeAnimator* GameInitialization::GetGhostAnimator(std::string gh_name , std::string dir)
{
	if( gh_name == "inky" )
	{
		if( dir == "up")
			return inkyAnimatorUp;
		if( dir == "down")
			return inkyAnimatorDown;
		if( dir == "left")
			return inkyAnimatorLeft;
		if( dir == "right")
			return inkyAnimatorRight;
		return NULL;
	}
	if( gh_name == "pinky" )
	{
		if( dir == "up")
			return pinkyAnimatorUp;
		if( dir == "down")
			return pinkyAnimatorDown;
		if( dir == "left")
			return pinkyAnimatorLeft;
		if( dir == "right")
			return pinkyAnimatorRight;
		return NULL;
	}
	if( gh_name == "blinky" )
	{
		if( dir == "up")
			return blinkyAnimatorUp;
		if( dir == "down")
			return blinkyAnimatorDown;
		if( dir == "left")
			return blinkyAnimatorLeft;
		if( dir == "right")
			return blinkyAnimatorRight;
		return NULL;
	}
	if( gh_name == "clyde" )
	{
		if( dir == "up")
			return clydeAnimatorUp;
		if( dir == "down")
			return clydeAnimatorDown;
		if( dir == "left")
			return clydeAnimatorLeft;
		if( dir == "right")
			return clydeAnimatorRight;
		return NULL;
	}
return NULL;
}



void GameInitialization::InitAll()
{
	BitmapLoader::Singleton();
	BoxesInit("./init/pacman_boxes.txt",PACMAN);
	BoxesInit("./init/ghosts_boxes.txt",GHOST);
	BoxesInit("./init/simple_mark_box.txt",SIMPLE_MARK);
	BoxesInit("./init/power_mark_box.txt",POWER_MARK);
	BitmapNamesInit("./init/bitmap_names.txt");
	ObstaclesInit("./init/obstacles.txt");
	MovingObstaclesInit("./init/moving_obstacles.txt");
	ObstacleHolder::Singleton();
	ObstacleHolder::Singleton().SetObstacles(GetObstacles());
	AnimationFilmHolder::Singleton();
	Rendering::Singleton();
	SimpleMarkHolder::Singleton();
	PowerMarkHolder::Singleton();
	CollisionChecker::Singleton();
	SimpleMarksInit("./init/simplemarks.txt");
	PowerMarksInit("./init/powermarks.txt");
	PacmanInit();
	GhostsInit();

	/*Simple and Power marks initialization*/
	SimpleMarksStateInit("./init/simple_marks_state.txt");
	PowerMarksStateInit("./init/power_marks_state.txt");
	std::list<SimpleMark *>::iterator i;
	std::list<SimpleMark *> sml = SimpleMarkHolder::Singleton().GetSMarks();
	for(i = sml.begin();i!=sml.end();i++)/*Setting the film for every simple mark*/
	{
		if( CheckId((*i)->GetId(),SIMPLE_MARK) )
			(*i)->SetCanEat(false);

		if( (*i)->CanEat())
			(*i)->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/marks/simple_mark.bmp")));
		else
			(*i)->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/marks/opponent_simple_mark.bmp")));
		/*CollisionChecker::Singleton().Register(pac_sprite , *i);*/
	}
	std::list<PowerMark *>::iterator p;
	std::list<PowerMark *> pml = PowerMarkHolder::Singleton().GetPMarks();
	for(p = pml.begin();p!=pml.end();p++)/*Setting the film for every power mark*/
	{
		if( CheckId((*p)->GetId(),POWER_MARK) )
			(*p)->SetCanEat(false);

		if( (*p)->CanEat())
			(*p)->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/marks/power_mark.bmp")));
		else
			(*p)->SetAnimationFilm(&(AnimationFilmHolder::Singleton().GetFilm("./images/marks/opponent_power_mark.bmp")));

		/*CollisionChecker::Singleton().Register(pac_sprite , *p);*/
	}
}



void GameInitialization::SetPlayers(unsigned short _players){ players = _players; }
unsigned short GameInitialization::GetPlayers(void){ return players;}

void GameInitialization::SimpleMarksStateInit(std::string path)
{
	int i = 0;
	FILE *fp;
	fp = fopen(path.c_str(),"r");
	if( fp == NULL)
		return;
	while(!feof(fp))
	{
		fscanf( fp , "%d\n", &sm_state[i]);
		i++;
	}
	assert(i <= MAX_SIMPLE_MARKS);
	sm_state[i] = END_FLAG;
	fclose(fp);
}

void GameInitialization::PowerMarksStateInit(std::string path)
{
	int i = 0;
	FILE *fp;
	fp = fopen(path.c_str(),"r");
	if( fp == NULL)
		return;
	while(!feof(fp))
	{
		fscanf( fp , "%d\n", &pm_state[i]);
		i++;
	}
	assert(i <= MAX_POWER_MARKS);
	pm_state[i] = END_FLAG;
	fclose(fp);
}
bool GameInitialization::CheckId(unsigned short id,enum bitmap_option option)
{
	if(option == SIMPLE_MARK)
	{
		for(int i = 0;i <=MAX_SIMPLE_MARKS || sm_state[i] == END_FLAG;i++)
			if(sm_state[i] == id)
				return true;
	}
	else if(option == POWER_MARK)
	{
		for(int i = 0;i <=MAX_POWER_MARKS || pm_state[i] == END_FLAG;i++)
			if(pm_state[i] == id)
				return true;
	}
	return false;
}













