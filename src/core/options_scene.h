/* Options' scene: on the Display page, BN6's battle staged for the line
 * under the cursor (showcase.h) beside its note, through the display's
 * settings as the game's own picture goes through them, so a change shows
 * as it is made (docs/engine/settings-screens.md). Where no scene can run
 * (the 3DS) or while it is staged, the paused game's own picture. */
#ifndef CW_OPTIONS_SCENE_H
#define CW_OPTIONS_SCENE_H

#include <SDL.h>

/* In Options' frame: the scene the cursor's line wants and its next frame
 * on the Display page; its core let go once Options closes */
void options_scene_update(void);
/* Before the panel is drawn: the game's picture under it kept (the scene's
 * stand-in) */
void options_scene_grab(void);
/* The scene's part round its action at its own pixels in `field` */
void options_scene_draw(SDL_Rect field);

#endif
