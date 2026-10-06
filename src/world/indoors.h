/* BN6's own indoor maps of the real world taken over as home's places
 * (indoors.c, docs/HOME.md): Lan's house and room, AsterLand, the Cyber
 * Academy. */
#ifndef CW_INDOORS_H
#define CW_INDOORS_H

#include <stdbool.h>
#include <stdint.h>


/* BN6's own warp list of map (group, number), as it was before the run's
 * first install took the map over (0 none): the first call for a map
 * comes before its takeover. */
uint32_t indoors_bn6_warps(int group, int number);
/* Where warp entry `entry` of `list` sets Lan down. */
void indoors_dest(uint32_t list, int entry, int *x, int *y, int *face);
/* Map (group, number) taken over: its people (place_lines.c), BN6's own
 * objects kept, no map scripts, its own warp list (every entry back to
 * world (x, y), where BN6 sets Lan down entering it) with BN6's doors in
 * `keep` copied (bit e: its warp entry e), and its song in home's music
 * slot `song_k`. The list's bus address, 0 none. */
uint32_t indoors_take_over(int group, int number, int x, int y, unsigned keep, int song_k, int song);
/* Kept door `entry` of `list` leading instead to world (x, y) of its map:
 * a door out to the planned town, which moves BN6's places. */
void indoors_door_to(uint32_t list, int entry, int x, int y);
/* The middle of map (group, number)'s trigger cells of `value`, and their
 * bounds (x0, y0, x1, y1, or NULL); false none. */
bool indoors_spot(int group, int number, int value, int *x, int *y, int *box);
/* Each frame in such a map: every warp trigger value but its doors in
 * `keep` (bit e: warp entry e) kept off, as BN6 clears the warp-off flags
 * entering a map. Its story's own (BN6's map scripts kept them off, and
 * some lie across the floor: the Academy foyer's 6-8, class 6-1's 3-6)
 * would take Lan back to where he came in. */
void indoors_shut(unsigned keep);

#endif
