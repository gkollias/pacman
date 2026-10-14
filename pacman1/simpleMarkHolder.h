#ifndef SIMPLEMARKHOLDER_H_
#define SIMPLEMARKHOLDER_H_
#include "simpleMark.h"
#include "gameInitialization.h"

class SimpleMarkHolder{
	std::list<SimpleMark *> smarks;
	static SimpleMarkHolder *smh;
	SimpleMarkHolder();
	~SimpleMarkHolder();
public:
	static SimpleMarkHolder& Singleton();
	void AddSimpleMark(SimpleMark *sm);
	void RemoveSimpleMark(SimpleMark *sm);
	std::list<SimpleMark *> GetSMarks();
};

#endif