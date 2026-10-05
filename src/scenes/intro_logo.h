/* The start's boot screen (scene_intro.c): the name Saschb2b in letters
 * of our own, flying in from all round to their places, a hop running
 * through the word with a band of light sweeping it, a glow and sparkles
 * after the light; plucks as the letters settle and a chord with the
 * light, synthesized (audio_tone). */
#ifndef CW_INTRO_LOGO_H
#define CW_INTRO_LOGO_H

#include <stdint.h>

/* The frames it lasts, the first INTRO_LOGO_IN in from black and the last
 * INTRO_LOGO_OUT out to it. */
#define INTRO_LOGO_END 128
#define INTRO_LOGO_IN 8
#define INTRO_LOGO_OUT 10

/* Its picture at frame t (from 1), into px: ARGB, 240 x 160. */
void intro_logo_render(uint32_t *px, int t);
/* Its sounds due at frame t, from the scene's update. */
void intro_logo_sound(int t);

#endif
