#ifndef UTILITIES_H_
#define UTILITIES_H_

#include "allegro.h"
#include <iostream>

namespace utilities
{



void Seconds_Tick(void);
void Display_Tick(void);
std::string I2S(int iA);
void TimerInit();
void GetFps();
void DisplayFps();
void DisplayTeq();
void FpsInc();
void CpsInc();
unsigned char GetDtick();
unsigned char GetDisplayTick();
//Utilities();
//~Utilities();
}

#endif