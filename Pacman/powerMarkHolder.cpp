#include "powerMarkHolder.h"


PowerMarkHolder* PowerMarkHolder::pmh;
PowerMarkHolder::PowerMarkHolder(){}
PowerMarkHolder::~PowerMarkHolder(){}
PowerMarkHolder& PowerMarkHolder::Singleton()
{
	if(pmh == NULL)
		pmh = new PowerMarkHolder();
	return *pmh;
}
void PowerMarkHolder::AddPowerMark(PowerMark* pm)
{	pmarks.push_back(pm);	}

void PowerMarkHolder::RemovePowerMark(PowerMark* pm)
{	
	std::list<PowerMark *>::iterator i = pmarks.begin();
	while(i != pmarks.end()) 
	{
		if((*i)->GetX() == pm->GetX() && (*i)->GetY() == pm->GetY())
		{pmarks.erase(i);return;}
		i++;
	}	
}


std::list<PowerMark *> PowerMarkHolder::GetPMarks()
{	return pmarks;	}