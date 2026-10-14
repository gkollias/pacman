#include "simpleMarkHolder.h"


SimpleMarkHolder* SimpleMarkHolder::smh;
SimpleMarkHolder::SimpleMarkHolder(){}
SimpleMarkHolder::~SimpleMarkHolder(){}
SimpleMarkHolder& SimpleMarkHolder::Singleton()
{
	if(smh == NULL)
		smh = new SimpleMarkHolder();
	return *smh;
}
void SimpleMarkHolder::AddSimpleMark(SimpleMark *sm)
{	smarks.push_back(sm);	}

void SimpleMarkHolder::RemoveSimpleMark(SimpleMark *sm)
{	
	std::list<SimpleMark *>::iterator i = smarks.begin();
	while(i != smarks.end()) 
	{
		if((*i)->GetX() == sm->GetX() && (*i)->GetY() == sm->GetY())
		{smarks.erase(i);return;}
		i++;
	}
}


std::list<SimpleMark *> SimpleMarkHolder::GetSMarks()
{	return smarks;	}