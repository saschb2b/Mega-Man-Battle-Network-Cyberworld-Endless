/* The Game Boy Advance's buttons as the engine reads them, a bit each. */
#ifndef CW_BUTTONS_H
#define CW_BUTTONS_H

enum {
	BTN_UP = 1 << 0,
	BTN_DOWN = 1 << 1,
	BTN_LEFT = 1 << 2,
	BTN_RIGHT = 1 << 3,
	BTN_A = 1 << 4,      /* chip / confirm */
	BTN_B = 1 << 5,      /* buster / back */
	BTN_L = 1 << 6,
	BTN_R = 1 << 7,
	BTN_START = 1 << 8,
	BTN_SELECT = 1 << 9,
};

#endif
