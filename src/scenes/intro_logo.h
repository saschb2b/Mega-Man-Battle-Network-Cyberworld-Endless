/* The start's boot screen (scene_intro.c): the name Saschb2b in pixel
 * letters of our own, near-black on white, coming up in two pieces,
 * "Sasch" and "b2b", on the two tones of the start's chime (audio_chime). */
#ifndef CW_INTRO_LOGO_H
#define CW_INTRO_LOGO_H

#include <stdint.h>

/* The frames it lasts, the first INTRO_LOGO_IN in from black and the last
 * INTRO_LOGO_OUT out to white. */
#define INTRO_LOGO_END 104
#define INTRO_LOGO_IN 12
#define INTRO_LOGO_OUT 12

/* Its picture at frame t (from 1), into px: ARGB, 240 x 160. */
void intro_logo_render(uint32_t *px, int t);
/* Its sounds due at frame t, from the scene's update. */
void intro_logo_sound(int t);

#endif
