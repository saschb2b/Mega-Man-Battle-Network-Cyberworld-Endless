/* A phone's part of the platform (platform_phone.c): Android's and the
 * iPhone's hints before SDL starts, and their window's flags. Nothing
 * elsewhere. */
#ifndef CW_PLATFORM_PHONE_H
#define CW_PLATFORM_PHONE_H

#include <SDL.h>

/* The hints, before SDL starts */
void platform_phone_hints(void);
/* The window's flags beside the game's own */
Uint32 platform_phone_window_flags(void);

#endif
