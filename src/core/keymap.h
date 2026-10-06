/* The keyboard's map (keymap.c): which keys are which of the GBA's buttons,
 * the defaults and keys.ini's, for platform.c's input; the controls screen
 * sets it through platform.h's platform_keys_*. */
#ifndef CW_KEYMAP_H
#define CW_KEYMAP_H

#include <SDL.h>
#include <stdint.h>

/* The defaults in play (the window's start, before keys.ini is read) */
void keymap_default(void);
/* The GBA buttons (BTN_*) on a key */
uint32_t keymap_button(SDL_Scancode sc);

#endif
