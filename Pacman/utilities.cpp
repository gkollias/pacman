#include "utilities.h"

BITMAP* buf;
unsigned char sticks;
unsigned char dticks;
unsigned char SecondTick;
unsigned char DisplayTick;
int FPS;
int iFPS;
int CPS;
int iCPS;

//Timer for Seconds
void Seconds_Tick(void)
{
    if (sticks==255) {sticks=0;SecondTick=0;}
    sticks++;
}
END_OF_FUNCTION(Seconds_Tick);

void Display_Tick(void)
{
    if (dticks==255) {dticks=0;DisplayTick=0;}
    dticks++;
}
END_OF_FUNCTION(Display_Tick);


//~Utilities(){}

std::string I2S(int iA)
{
	char buffer[10];
	sprintf(buffer,"%d",iA);
	return buffer;
}


void TimerInit()
{
sticks=0;
dticks=0;
SecondTick=0;
DisplayTick=0;
FPS=0;
iFPS=0;
CPS=0;
iCPS=0;

	install_timer();
    LOCK_VARIABLE(dticks);
    LOCK_FUNCTION(Display_Tick);
    LOCK_VARIABLE(sticks);
    LOCK_FUNCTION(Seconds_Tick);
    install_int_ex(&Display_Tick, MSEC_TO_TIMER(50));
    install_int_ex(&Seconds_Tick, SECS_TO_TIMER(1));
}


void GetFps()
{
if(sticks - SecondTick > 0)
        {//Game FPS Counter
            FPS=iFPS;
            iFPS=0;
            CPS=iCPS;
            iCPS=0;
            SecondTick=sticks;
        }

}

void DisplayFps()
{
clear_to_color(buf, makecol(255,255,255));
buf = create_bitmap(SCREEN_W , 30);
textprintf_ex(buf, font, 0, 10, makecol(255, 255, 255), -1, "FPS: %s",I2S(FPS).c_str());
textprintf_ex(buf, font, 0, 20, makecol(255, 255, 255), -1, "CPS: %s",I2S(CPS).c_str());
blit(buf, screen, 0, 0, 0, 0, buf->w, buf->h);
}

void DisplayTeq(){	DisplayTick=dticks;	}
void FpsInc(){	iFPS++;	}
void CpsInc(){	iFPS++;	}
unsigned char GetDtick(){	return dticks;	}
unsigned char GetDisplayTick(){	return DisplayTick;	}

