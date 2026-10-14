#include "ai.h"

Ai* Ai::ai;


Ai& Ai::Singleton()
{
	if(ai == NULL)
		ai = new Ai();
	return *ai;
}


void Ai::GhostMove(Ghost* ghost,PacMan* pacman)
{
	bool changeX = false ,changeY = false , priorityX = false , priorityY = false;
	int pacX = pacman->GetX();
	int pacY = pacman->GetY();
	int ghostX = ghost->GetX();
	int ghostY = ghost->GetY();

	if(!ghost->IsAlive())
	{
		pacX = start_point->GetX();
		pacY = start_point->GetY();
		if(	abs(pacX - ghostX) < abs(pacY - ghostY))
		{
				priorityY = true;
		}
		else
		{
				priorityX = true;
		}
		
		if(priorityX == true)
			changeX = MovePriorityX( ghost, start_point);
		if(priorityY == true)
			changeY = MovePriorityY( ghost, start_point);
		
		if(priorityX == true && changeX == false)
			changeY = MovePriorityY( ghost, start_point);
		
		if(priorityY == true && changeY == false)
			changeX = MovePriorityX( ghost, start_point);
		
		if(pacX == ghostX && pacY == ghostY)
		{	
			ghost->SetIsAlive(true);
			ghost->SetCanEat(true);
		}
		return;
	}

	if(!pacman->CanEat())
	{
		if(	abs(pacX - ghostX) < abs(pacY - ghostY))
		{
			if(	ghost->GetName() == BLINKY || ghost->GetName() == INKY)
				priorityX = true;
			else
				priorityY = true;
		}
		else
		{
			if(	ghost->GetName() == BLINKY || ghost->GetName() == INKY)
				priorityY = true;
			else
				priorityX = true;
		}
		
		if(priorityX == true)
			changeX = MovePriorityX( ghost, pacman);
		if(priorityY == true)
			changeY = MovePriorityY( ghost, pacman);
		
		if(priorityX == true && changeX == false)
			changeY = MovePriorityY( ghost, pacman);
		
		if(priorityY == true && changeY == false)
			changeX = MovePriorityX( ghost, pacman);
		
		

	}
	if(pacman->CanEat() && !ghost->CanEat())
	{
		if(	abs(pacX - ghostX) < abs(pacY - ghostY))
		{
			if(	ghost->GetName() == BLINKY || ghost->GetName() == INKY)
				priorityX = true;
			else
				priorityY = true;
		}
		else
		{
			if(	ghost->GetName() == BLINKY || ghost->GetName() == INKY)
				priorityY = true;
			else
				priorityX = true;
		}

		if(ghostX <= 95 &&  ghostY<= 227 && ghostY >= 197)
			return;
		if(ghostX >= 520 &&  ghostY<= 227 && ghostY >= 197)
			return;
		if(priorityX == true)
			changeX = MoveAwayPriorityX( ghost, pacman);
		if(priorityY == true)
			changeY = MoveAwayPriorityY( ghost, pacman);
		
		if(priorityX == true && changeX == false)
			changeY = MoveAwayPriorityY( ghost, pacman);
		
		if(priorityY == true && changeY == false)
			changeX = MoveAwayPriorityX( ghost, pacman);
	}

	return;
}



bool Ai::MovePriorityX(Ghost* ghost,PacMan* pacman)
{
	int pacX = pacman->GetX();
	int pacY = pacman->GetY();
	int ghostX = ghost->GetX();
	int ghostY = ghost->GetY();

	if( pacX > ghostX )
	{
		if(	ghost->CanMove(RIGHT) )
		{
			ghost->SetDirection(RIGHT);
			return true;
		}
		else if(  ghost->CanMove(DOWN) && ghost->GetDirection() != UP)
		{
			ghost->SetDirection(DOWN);
			return true;
		}
		else if(ghost->CanMove(UP) && ghost->GetDirection() != DOWN)
		{
			ghost->SetDirection(UP);
			return true;
		}
		else if( ghost->CanMove(LEFT) )
		{
			ghost->SetDirection(LEFT);
			return true;
		}
		
	}	
	else if( pacX < ghostX )
	{
		if(	ghost->CanMove(LEFT) )
		{
			ghost->SetDirection(LEFT);
			return true;
		}
		else if(  ghost->CanMove(DOWN) && ghost->GetDirection() != UP)
		{
			ghost->SetDirection(DOWN);
			return true;
		}
		else if(ghost->CanMove(UP) && ghost->GetDirection() != DOWN)
		{
			ghost->SetDirection(UP);
			return true;
		}
			
		
		else if( ghost->CanMove(RIGHT) )
		{
			ghost->SetDirection(RIGHT);
			return true;
		}
		
	}
	else if( pacX == ghostX )
	{
		if(ghost->CanMove(DOWN) && ghost->GetDirection() != UP)
		{
			ghost->SetDirection(DOWN);
			return true;
		}
		else if(ghost->CanMove(UP))
		{
			ghost->SetDirection(UP);
			return true;
		}
		else if(ghost->CanMove(RIGHT) )
		{
			ghost->SetDirection(RIGHT);
			return true;
		}
		else if(ghost->CanMove(LEFT) )
		{
			ghost->SetDirection(LEFT);
			return true;
		}
	}
	ghost->SetDirection(NO_DIR);
	return false;
}

bool Ai::MovePriorityY(Ghost* ghost,PacMan* pacman)
{
	int pacX = pacman->GetX();
	int pacY = pacman->GetY();
	int ghostX = ghost->GetX();
	int ghostY = ghost->GetY();

	if( pacY > ghostY )
	{
		if(	ghost->CanMove(DOWN) )
		{
			ghost->SetDirection(DOWN);
			return true;
		}
		else if(  ghost->CanMove(RIGHT) && ghost->GetDirection() != LEFT)
		{
			ghost->SetDirection(RIGHT);
			return true;
		}
		else if( ghost->CanMove(LEFT) && ghost->GetDirection() != RIGHT)
		{
			ghost->SetDirection(LEFT);
			return true;
		}
		else if(ghost->CanMove(UP) )
		{
			ghost->SetDirection(UP);
			return true;
		}
	
	}	
	else if( pacY < ghostY )
	{
		if(	ghost->CanMove(UP) )
		{
			ghost->SetDirection(UP);
			return true;
		}
		else if( ghost->CanMove(RIGHT) && ghost->GetDirection() != LEFT )
		{
			ghost->SetDirection(RIGHT);
			return true;
		}
		else if( ghost->CanMove(LEFT) && ghost->GetDirection() != RIGHT)
		{
			ghost->SetDirection(LEFT);
			return true;
		}
		else if(ghost->CanMove(DOWN) )
		{
			ghost->SetDirection(DOWN);
			return true;
		}
	}
	else if( pacY == ghostY )
	{
		if(ghost->CanMove(RIGHT) && ghost->GetDirection() != LEFT)
		{
			ghost->SetDirection(RIGHT);
			return true;
		}
		else if( ghost->CanMove(LEFT))
		{
			ghost->SetDirection(LEFT);
			return true;
		}
		else if( ghost->CanMove(DOWN) )
		{
			ghost->SetDirection(DOWN);
			return true;
		}
		else if( ghost->CanMove(UP) )
		{
			ghost->SetDirection(UP);
			return true;
		}
		
	}
	ghost->SetDirection(NO_DIR);
	return false;
}



bool Ai::MoveAwayPriorityX(Ghost* ghost,PacMan* pacman)
{
	int pacX = pacman->GetX();
	int pacY = pacman->GetY();
	int ghostX = ghost->GetX();
	int ghostY = ghost->GetY();

	if( pacX > ghostX )
	{
		if(	ghost->CanMove(LEFT) )
		{
			ghost->SetDirection(LEFT);
			return true;
		}
		else
		{
			if( pacY > ghostY && ghost->CanMove(UP) )
			{
				ghost->SetDirection(UP);
				return true;
			}
			else if( pacY < ghostY && ghost->CanMove(DOWN) )
			{
				ghost->SetDirection(DOWN);
				return true;
			}
			else if( ghost->CanMove(DOWN) )
			{
				ghost->SetDirection(DOWN);
				return true;
			}
			else if( ghost->CanMove(UP) )
			{
				ghost->SetDirection(UP);
				return true;
			}
		}
	}	
	else if( pacX < ghostX )
	{
		if(	ghost->CanMove(RIGHT) )
		{
			ghost->SetDirection(RIGHT);
			return true;
		}
		else
		{
			if( pacY > ghostY && ghost->CanMove(UP) )
			{
				ghost->SetDirection(UP);
				return true;
			}
			else if( pacY < ghostY && ghost->CanMove(DOWN) )
			{
				ghost->SetDirection(DOWN);
				return true;
			}
			else if( ghost->CanMove(DOWN) )
			{
				ghost->SetDirection(DOWN);
				return true;
			}
			else if( ghost->CanMove(UP) )
			{
				ghost->SetDirection(UP);
				return true;
			}
		}
	}
	else if( pacX == ghostX )
	{
		if( pacY > ghostY && ghost->CanMove(UP) )
		{
			ghost->SetDirection(UP);
			return true;
		}
		else if( pacY < ghostY && ghost->CanMove(DOWN) )
		{
			ghost->SetDirection(DOWN);
			return true;
		}
		else if( ghost->CanMove(DOWN) )
		{
			ghost->SetDirection(DOWN);
			return true;
		}
		else if( ghost->CanMove(UP) )
		{
			ghost->SetDirection(UP);
			return true;
		}
	}
	ghost->SetDirection(NO_DIR);
	return false;
}

bool Ai::MoveAwayPriorityY(Ghost* ghost,PacMan* pacman)
{
	int pacX = pacman->GetX();
	int pacY = pacman->GetY();
	int ghostX = ghost->GetX();
	int ghostY = ghost->GetY();

	if( pacY > ghostY )
	{
		if(	ghost->CanMove(UP) )
		{
			ghost->SetDirection(UP);
			return true;
		}
		else
		{
			if( pacX > ghostX && ghost->CanMove(LEFT) )
			{
				ghost->SetDirection(LEFT);
				return true;
			}
			else if( pacX < ghostX && ghost->CanMove(RIGHT) )
			{
				ghost->SetDirection(RIGHT);
				return true;
			}
			else if( ghost->CanMove(LEFT) )
			{
				ghost->SetDirection(LEFT);
				return true;
			}
			else if( ghost->CanMove(RIGHT) )
			{
				ghost->SetDirection(RIGHT);
				return true;
			}
		}
	}	
	else if( pacY < ghostY )
	{
		if(	ghost->CanMove(DOWN) )
		{
			ghost->SetDirection(DOWN);
			return true;
		}
		else
		{
			if( pacX > ghostX && ghost->CanMove(LEFT) )
			{
				ghost->SetDirection(LEFT);
				return true;
			}
			else if( pacX < ghostX && ghost->CanMove(RIGHT) )
			{
				ghost->SetDirection(RIGHT);
				return true;
			}
			else if( ghost->CanMove(LEFT) )
			{
				ghost->SetDirection(LEFT);
				return true;
			}
			else if( ghost->CanMove(RIGHT) )
			{
				ghost->SetDirection(RIGHT);
				return true;
			}
		}
	}
	else if( pacY == ghostY )
	{
		if( pacX > ghostX && ghost->CanMove(LEFT) )
		{
			ghost->SetDirection(LEFT);
			return true;
		}
		else if( pacX < ghostX && ghost->CanMove(RIGHT) )
		{
			ghost->SetDirection(RIGHT);
			return true;
		}
		else if( ghost->CanMove(LEFT) )
		{
			ghost->SetDirection(LEFT);
			return true;
		}
		else if( ghost->CanMove(RIGHT) )
		{
			ghost->SetDirection(RIGHT);
			return true;
		}
	}
	ghost->SetDirection(NO_DIR);
	return false;
}



Ai::Ai(){	start_point = new PacMan(308 , 157);	}
Ai::~Ai(){}
