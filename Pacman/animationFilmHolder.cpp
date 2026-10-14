#include "animationFilmHolder.h"

AnimationFilmHolder* AnimationFilmHolder::afh;

    AnimationFilm& AnimationFilmHolder::GetFilm (const std::string id) const {
			
		/*textprintf_ex(screen, font, 0, 0, makecol(255, 255, 255), -1, "3.EEEE->>> %s",id.c_str());		
		while(!key[KEY_ESC]) { }*/

	FilmMap::const_iterator i = filmMap.find(id);
	assert(i!=filmMap.end());
	return *i->second;
    }

	AnimationFilmHolder& AnimationFilmHolder::Singleton() 
	{
		if(AnimationFilmHolder::afh == NULL)
			afh = new AnimationFilmHolder();
        return *afh;
	}

	AnimationFilmHolder::AnimationFilmHolder ()
	{
		BITMAP *bmp;
		std::string path="";
		std::list<std::string> bitmap_names;
	
		bitmap_names = GameInitialization::Singleton().GetBitmapNames();
		for(std::list<std::string>::iterator i=bitmap_names.begin(); i!=bitmap_names.end();i++)
		{
			path = *i;
			bmp = BitmapLoader::Singleton().Load(path);
			if(path[9] == 'g')
			filmMap[*i] = new AnimationFilm(bmp,GameInitialization::Singleton().GetBoxes(GHOST),*i);
			if(path[16] == 'p')
			filmMap[*i] = new AnimationFilm(bmp,GameInitialization::Singleton().GetBoxes(PACMAN),*i);
			if(path[16] == 'd')
			filmMap[*i] = new AnimationFilm(bmp,GameInitialization::Singleton().GetBoxes(PACMAN_DEAD),*i);
			if(path[9] == 'm' && ( path[15] == 'p' || path[24] == 'p'))
				filmMap[*i] = new AnimationFilm(bmp,GameInitialization::Singleton().GetBoxes(POWER_MARK),*i);
			if(path[9] == 'm' && ( path[15] == 's' || path[24] == 's'))
				filmMap[*i] = new AnimationFilm(bmp,GameInitialization::Singleton().GetBoxes(SIMPLE_MARK),*i);
			if(path[9] == 'o')
				filmMap[*i] = new AnimationFilm(bmp,GameInitialization::Singleton().GetBoxes(OBSTACLE),*i);
			path = "";
		}
	}// Decoder constructor.

    AnimationFilmHolder::~AnimationFilmHolder()
	{ 
		filmMap.clear();
		//delete[] filmMem;
	}
