#include "obstaclePainter.h"


ObstaclePainter::ObstaclePainter()
{}
void ObstaclePainter::Draw(std::list<Obstacle> ob)
{
	for(std::list<Obstacle>::iterator i = ob.begin(); i!=ob.end(); i++)
	{
		int x = i->GetX();
		int y = i->GetY();
		int w = i->GetW();
		int h = i->GetH();
		//line( screen , x , y , x+w , y , makecol(0, 0, 255) );
		//line( screen , x+w , y , x+w , y+h , makecol(0, 0, 255) );
		//line( screen , x+w , y+h , x , y+h , makecol(0, 0, 255) );
		//line( screen , x , y+h , x , y , makecol(0, 0, 255) );
		blit(i->GetBitmap(),screen,0,0,x,y,w,h);
	}
//	BITMAP* b = load_bitmap("images/obstacles/blue_ob.bmp",NULL);
//	blit(b,screen,0,0,307,15,2,2);
//	blit(b,screen,0,0,307,15,2,2);

}