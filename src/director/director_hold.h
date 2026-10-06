/* MegaMan held (issue #23): what of BN6 holds him on the map, the saves
 * that wait for him free, and the watch after a CONTINUE that frees him
 * where a state saved so holds him. */
#ifndef CW_DIRECTOR_HOLD_H
#define CW_DIRECTOR_HOLD_H

#include <stdbool.h>

/* What holds MegaMan (or Lan) on the map, as BN6 tests it before the pad
 * and START (bn6f sub_809D9E0, sub_8005AF4): 0 where he is free. */
enum {
	HELD_OFF_MAP = 1 << 0,   /* not on the map: a battle, a menu, a map being entered */
	HELD_FADE = 1 << 1,      /* the screen fading */
	HELD_CUTSCENE = 1 << 2,  /* a cutscene running */
	HELD_CHAT = 1 << 3,      /* a chat showing or under way */
	HELD_LOCK = 1 << 4,      /* a dialogue's lock, or the game's can-move flag clear */
	HELD_SCRIPT = 1 << 5,    /* a cutscene's pad walking him */
	HELD_CONVEYOR = 1 << 6,  /* BN6's conveyor carrying him */
	HELD_NPC = 1 << 7,       /* A at an NPC, its chat not yet ended */
	HELD_WARP = 1 << 8,      /* a warp under way */
	HELD_PET = 1 << 9,       /* the PET opening, or turned off */
	HELD_PAUSED = 1 << 10,   /* the map's objects paused */
	HELD_PLAYER = 1 << 11,   /* his object just spawned, or in a state that reads no pad */
	HELD_COUNT = 12
};
unsigned bn6_held(void);
/* The holds of a mask by name, apart by spaces ("fade conveyor"); "none" */
const char *held_names(unsigned held);
/* The mask a dev step names ("conveyor,script", "all", or a number) */
unsigned held_parse(const char *spec);

/* Whether a save may be written now: MegaMan free as BN6 has him, and no
 * talk of the director's under way (every checkpoint's, the arena door's,
 * home's and the quit's). */
bool hold_save_ok(void);

/* A CONTINUE's state loaded: its first steps watched */
void hold_watch_arm(void);

#endif
