#include "bitmapLoader.h"

BitmapLoader* BitmapLoader::bl;
    BITMAP *BitmapLoader::GetBitmap (std::string path) const 
	{
	BitmapMap::const_iterator i = bitmaps.find(path);
	return i != bitmaps.end() ? i->second : (BITMAP *) 0;
    }

    BITMAP *BitmapLoader::Load (std::string path) {
					
	BITMAP *b = GetBitmap(path);
	if (!b) {
	    bitmaps[path] = (b = load_bitmap(path.c_str(),NULL));
	    assert(b);
	}
	return b;
    }
	BitmapLoader& BitmapLoader::Singleton() 
	{
		if(bl == NULL)
			bl = new BitmapLoader();
        return *bl;
	}

	BitmapLoader::BitmapLoader (void){}

    BitmapLoader::~BitmapLoader() { 
	// Destroy all bitmaps in the map.
	bitmaps.clear();
    }

void BitmapLoader::Destroy( std::string path )
{	
	BitmapMap::iterator i = bitmaps.find(path);
	if (i != bitmaps.end()) 
	{
		destroy_bitmap(i->second);
		bitmaps.erase(i);
	}
}

