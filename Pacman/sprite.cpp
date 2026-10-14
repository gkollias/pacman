#include "sprite.h"

	void Sprite::SetFrame (int i) {	
	if (i != frameNo) {
	    assert(i < currFilm->GetTotalFrames());
	    frameBox = currFilm->GetFrameBox(frameNo = i);
	}
    }
    byte Sprite::GetFrame (void) const { return frameNo; }
	void Sprite::SetAnimationFilm (AnimationFilm* anim){	currFilm = anim;	}
	AnimationFilm* Sprite::GetAnimationFilm (){	return currFilm;}
    void Sprite::SetVisibility (bool v) { isVisible = v; }
    bool Sprite::IsVisible (void) const { return isVisible; }
    

	Sprite::Sprite(){}
    Sprite::Sprite (Dim _x, Dim _y, AnimationFilm* film): 
        x(_x), y(_y), currFilm(film), isVisible(true)
        { frameNo = currFilm->GetTotalFrames(); SetFrame(0);old_x=x;old_y=y; }
		
	void Sprite::SetX(Dim _x){	x = _x;	}
	void Sprite::SetY(Dim _y){	y = _y;	}
	Dim Sprite::GetX( void ){	return x;	}
	Dim Sprite::GetY( void ){	return y;	}
	Dim Sprite::GetOldX( void ){	return old_x;	}
	Dim Sprite::GetOldY( void ){	return old_y;	}

	Dim Sprite::GetW( void )
	{	
		if( s_class == SIMPLE_MARK_S)
			return 4;
		if( s_class == POWER_MARK_S)
			return 12;
		return frameBox.w;
	}
	Dim Sprite::GetH( void )
	{	
		if( s_class == SIMPLE_MARK_S)
			return 4;
		if( s_class == POWER_MARK_S)
			return 12;
		return frameBox.h;
	}
	sprite_class Sprite::GetClass(){	return s_class;	}



	//pixel perfect collision detection
	bool Sprite::CollisionCheck (Sprite* s)
	 {
		int right_side1 = this->GetX() + this->GetW();
		int down_side1 = this->GetY() + this->GetH();
		int right_side2 = s->GetX() + s->GetW();
		int down_side2 = s->GetY() + s->GetH();
		int min_x = max(this->GetX(), s->GetX());
		int min_y = max(this->GetY(), s->GetY());
		int max_x = min(right_side1, right_side2);
		int max_y = min(down_side1, down_side2);

		if (max_x <= min_x || max_y <= min_y) //no collision
		{ return false; }

		//etrwga kapoia stigmi ena segmentation se anupopto xrono k to allaxa
		//k den einai pixel perfect to collision detection
		//afou epistrefw true.an vgei to return,einai pixel perfect
		//den epirreazei to paixnidi para se elaxisto vathmo. 
		return true;

		int mask1 = bitmap_mask_color(this->GetAnimationFilm()->GetBitmap() );
		int mask2 = bitmap_mask_color(s->GetAnimationFilm()->GetBitmap());
		for (int y = min_y; y < max_y; y++) 
		{
			for (int x = min_x; x < max_x; x++) 
			{
				 int x1 = x - this->GetX();
				 int y1 = y - this->GetY();
				 int x2 = x - s->GetX();
				 int y2 = y - s->GetY();
				 if(s == NULL)
					 return false;
				 if(s->GetAnimationFilm() == NULL)
					 return false;
				 if(s->GetAnimationFilm()->GetBitmap() == NULL)
					 return false;
				 int color1 = getpixel(this->GetAnimationFilm()->GetBitmap(), x1, y1);
				 int color2 = getpixel(s->GetAnimationFilm()->GetBitmap(), x2, y2);
				 if (color1 != mask1 && color2 != mask2) 
					return true;
			}
		 }
  return false;
}
