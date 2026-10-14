#include "common.h"
#include <string.h>

/* Only these keys are polled; querying key[] pumps window events. */
static const int TRACKED[] = {
	KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_ENTER, KEY_ENTER_PAD, KEY_ESC,
	KEY_P, KEY_G, KEY_H, KEY_D, KEY_F, KEY_C, KEY_V, KEY_BACKSPACE, KEY_STOP,
	KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
};

Input::Input()
{
	memset(prev, 0, sizeof(prev));
	memset(cur, 0, sizeof(cur));
}

void Input::Update()
{
	for (size_t i = 0; i < sizeof(TRACKED) / sizeof(TRACKED[0]); i++) {
		int k = TRACKED[i];
		prev[k] = cur[k];
		cur[k] = key[k];
	}
}

bool Input::Pressed(int k) const { return cur[k] && !prev[k]; }
