#include "animation.h"

Animation::Animation(animid_t _id)  
{
	id = _id;	
}

Animation::~Animation(void)  
{}


animid_t Animation::GetId(void) 
{ 
	return id; 
}




