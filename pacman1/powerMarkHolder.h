#ifndef POWERMARKHOLDER_H_
#define POWERMARKHOLDER_H_
#include "powerMark.h"


class PowerMarkHolder{
	std::list<PowerMark *> pmarks;
	static PowerMarkHolder *pmh;
	PowerMarkHolder();
	~PowerMarkHolder();
public:
	static PowerMarkHolder& Singleton();
	void AddPowerMark(PowerMark* pm);
	void RemovePowerMark(PowerMark* pm);
	std::list<PowerMark *> GetPMarks();
};

#endif