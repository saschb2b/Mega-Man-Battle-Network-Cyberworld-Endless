/* What SDL 2.0.9, the PS4's (OpenOrbis's), lacks of what the game calls:
 * stand-ins, which the PS4's build reads before every file (the
 * Makefile's -include). Every other build's SDL has them all, and this
 * adds nothing there. */
#ifndef CW_SDL_OLD_H
#define CW_SDL_OLD_H

#include <SDL.h>

#if !SDL_VERSION_ATLEAST(2, 0, 12)
/* (a texture keeps the filtering SDL_HINT_RENDER_SCALE_QUALITY gave it
 * when it was made: nearest, the game's own) */
typedef enum { SDL_ScaleModeNearest, SDL_ScaleModeLinear, SDL_ScaleModeBest } SDL_ScaleMode;
static inline int SDL_SetTextureScaleMode(SDL_Texture *texture, SDL_ScaleMode mode) {
	(void)texture;
	(void)mode;
	return -1;
}
/* (a pad's kind unknown: pads.c names the console's own) */
typedef enum {
	SDL_CONTROLLER_TYPE_UNKNOWN, SDL_CONTROLLER_TYPE_XBOX360, SDL_CONTROLLER_TYPE_XBOXONE, SDL_CONTROLLER_TYPE_PS3,
	SDL_CONTROLLER_TYPE_PS4, SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO, SDL_CONTROLLER_TYPE_VIRTUAL, SDL_CONTROLLER_TYPE_PS5
} SDL_GameControllerType;
static inline SDL_GameControllerType SDL_GameControllerGetType(SDL_GameController *pad) {
	(void)pad;
	return SDL_CONTROLLER_TYPE_UNKNOWN;
}
#endif

#if !SDL_VERSION_ATLEAST(2, 0, 14)
/* (the button 2.0.14 added after the D-pad: none of the PS4's pads') */
#define SDL_CONTROLLER_BUTTON_MISC1 15
#endif

#endif
