/* What the drawing reads of the game each frame (director_see.c), for the director's other parts. */
#ifndef CW_DIRECTOR_SEE_H
#define CW_DIRECTOR_SEE_H

#include <stdbool.h>
#include <stdint.h>

/* What the drawing reads of the game, taken after each frame as the update
 * runs: on a 3DS the next frame runs on another core while this one is
 * drawn, and a read of the game then waits for it (the bottom screen's map
 * took 14 ms so, a frame lost six times a second; the duel's clock every
 * frame of its battle). */
typedef struct {
	int px, py;
	bool on_map, battle, custom;
	uint32_t timer;
	int tent_x, tent_y;
	int md_taken[3], md_known[3];   /* the layer's Mystery Data MegaMan knows of, by colour (green, blue, purple): taken, known */
	int counts_a;                   /* L's overlay of them: its alpha, 0 hidden */
	uint16_t md_marked;             /* the ones the map marks, a bit each: untaken, on a panel it shows, or purple with an Unlocker held */
	uint8_t locks;                  /* the layer's locks still shut (layer.block[k], a bit each): the map marks them */
	char bug_note[160];             /* the NaviCust's bug as its RUN leaves it, over the PET a few seconds */
	int bug_note_t;
} Seen;
extern Seen seen;

bool md_on_map(int k);
bool md_known(int k, bool taken);
bool counts_any(void);
bool lock_shut(int k);
void l_taken(void);

/* The NaviCust's RUN with a bug, the PET still open: a note over it naming
 * the bug's cause for five seconds. BN6's RUN lists errors only, and says
 * "RUN complete!" whatever the colours: a playtester read it as clean, and
 * MegaMan named the bug only once the PET had closed (sessions 60 and 61).
 * The game counts the bugs as it runs the board, in passes over frames:
 * counts changed in the PET and calm BUG_CALM frames are its RUN's. */
#define PET_MODE 0x28   /* the main mode of the PET's pages */

#endif
