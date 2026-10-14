#include "animationFilm.h"


    byte				AnimationFilm::GetTotalFrames (void) 	const { return boxes.size(); }
    BITMAP*				AnimationFilm::GetBitmap (void) const{ return bitmap; }
    const std::string 	AnimationFilm::GetId (void) const { return id; }
    const Rectangle			AnimationFilm::GetFrameBox (byte frameNo) const
		    { assert(boxes.size()>frameNo); return boxes[frameNo]; }

	void AnimationFilm::DisplayFrame (BITMAP *dest, const Point& at, byte frameNo) const 
	{
		masked_blit(
			bitmap, dest, 
			GetFrameBox(frameNo).x, 
			GetFrameBox(frameNo).y, 
			at.x, 
			at.y,
			GetFrameBox(frameNo).w, 
			GetFrameBox(frameNo).h);
	}
AnimationFilm::AnimationFilm (BITMAP *_bitmap, const std::vector<Rectangle> _boxes, const std::string& _id)
{
	bitmap = _bitmap;
	boxes = _boxes;
	id = _id;
}
