#ifndef ANIMATIONFILM_H_
#define ANIMATIONFILM_H_
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <assert.h>
#include "allegro.h"
#include "def.h"



class AnimationFilm {
    std::vector<Rectangle> boxes;
    BITMAP*		bitmap;
    std::string	id;
    public:
	byte		GetTotalFrames (void) 	const;
    BITMAP*		GetBitmap (void) const ;
    const std::string 	GetId (void) const;
    const Rectangle	GetFrameBox (byte frameNo) const;
    void		DisplayFrame (BITMAP *d, const Point& at, byte frameNo) const;
    AnimationFilm (BITMAP *_bitmap, const std::vector<Rectangle> _boxes, const std::string& _id);
	
};

#endif