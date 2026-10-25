/* The feel on BN6 (feel.h's rules): after each frame of the run's game,
 * BN6's state read (MegaMan's HP and buster, his mood and form, the
 * enemies with HP, the Custom gauge, the real world or the net) and its
 * change since the last frame raised as beats; the triggers asked for
 * (the gauge full in the fight, fast forward where it can run). Nothing
 * raised while fast forward runs. */
#ifndef CW_FEEL_GAME_H
#define CW_FEEL_GAME_H

/* After a frame of the run's own game (scene_emu.c) */
void feel_game_frame(void);

#endif
