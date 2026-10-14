#ifndef ANIMATIONFILMHOLDER_H_
#define ANIMATIONFILMHOLDER_H_
#include <map>
#include <iostream>
#include "gameInitialization.h"

#pragma warning(disable:4786)

class AnimationFilmHolder {
    typedef std::map<std::string, AnimationFilm*> FilmMap;
    FilmMap		filmMap;
    //AnimationFilm*	filmMem;
	static AnimationFilmHolder *afh;

    public:
			AnimationFilmHolder (); // Decoder constructor.
    ~AnimationFilmHolder();
    AnimationFilm& GetFilm (const std::string id) const;
	static AnimationFilmHolder& Singleton();

};

#endif