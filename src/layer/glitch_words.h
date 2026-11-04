/* What is said about the run's Glitch layer (glitch_words.c, docs/game/glitch-layer.md):
 * its warp's talk, the copy's, and MegaMan's and Lan's words as the layer
 * comes apart. */
#ifndef CW_GLITCH_WORDS_H
#define CW_GLITCH_WORDS_H

/* The director's words on a Glitch layer (talk_start's boxes) as stage
 * `stage` comes (2 to 4; NULL for one that says nothing); its arrival's
 * are story_words.c's, the warp's talk and the copy's declared in
 * scripts.h */
const char *glitch_stage_words(int stage);

#endif
