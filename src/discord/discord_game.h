/* What Discord shows, read from the game (discord_game.c): the scene, the
 * run's layer and area, the battle on, home between acts, the Net Square,
 * the profile's best. Reads only: nothing of the game changes for it. */
#ifndef CW_DISCORD_GAME_H
#define CW_DISCORD_GAME_H

/* A frame of the game's loop (main.c): twice a second the game read and
 * handed to discord_show, and the pipe tended (discord_frame) */
void discord_game_frame(void);
/* The game's end: what Discord shows cleared (discord_quit) */
void discord_game_quit(void);

#endif
