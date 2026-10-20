/* The Arena's rollback window on this system (issue #128,
 * docs/ONLINE_ARENA.md, Rollback): what a frame of the pair costs here,
 * as the last battle (or --dev rollbench) measured it, kept in the data
 * folder (arena.ini, never in a .cwsave: it is the machine's, not the
 * player's), for the delay the next challenge or Yes asks for; the
 * battle's own window comes from its way in, timed as it runs
 * (arena_window_for, arena_sync.h). */
#ifndef CW_ARENA_WINDOW_H
#define CW_ARENA_WINDOW_H

/* a frame of the pair's mean cost here, in ms, kept */
void arena_window_keep(double frame_ms);
/* the window the kept cost affords; ARENA_WINDOW_FIRST where none is kept */
unsigned arena_window_kept(void);

#endif
