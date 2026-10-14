#ifndef BITMAPLOADER_H_
#define BITMAPLOADER_H_
#include "allegro.h"
#include <iostream>
#include <map>
#include <string>
#include <assert.h>
#include "animatorHolder.h"


class BitmapLoader {
    typedef std::map<std::string, BITMAP *> BitmapMap;
    BitmapMap bitmaps;
	static BitmapLoader *bl;
	BitmapLoader (void);
    ~BitmapLoader();
    public:
    BITMAP *GetBitmap (std::string path) const;
    BITMAP *Load (std::string path);
	void Destroy(  std::string path );
	static BitmapLoader& Singleton();

};

#endif