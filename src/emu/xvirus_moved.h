/* What the installs moved where, for a trace's reader (battle_trace.c,
 * xdiff.c, xdiff_events.c): an address range of a family's game, or of one
 * of BN6's tables, and its copy. Kept only where a reader asked before the
 * installs (the differential, CYBERWORLD_BTRACE), on the heap and as many as
 * there are: a player's run keeps none and pays nothing for them. A reader
 * gets the whole list or none, never a part (docs/devtools/differential.md,
 * The trace's moved ranges). */
#ifndef CW_XVIRUS_MOVED_H
#define CW_XVIRUS_MOVED_H

#include <stdbool.h>
#include <stdint.h>

typedef struct { uint32_t from, to, len; } XvMoved;

/* From now on keep what the installs move (asked before them; the first
 * range added asks the environment too: CYBERWORLD_BTRACE's trace) */
void xvirus_moved_keep(void);
/* A range moved (one already held, by its from and to, again ignored);
 * one the heap cannot hold makes the list unwhole, said on stderr */
void xv_moved_add(uint32_t from, uint32_t to, uint32_t len);
/* The ranges moved, in the order added, their count in *n; NULL (and *n 0)
 * where the list is not whole: never kept, or a range lost to the heap,
 * said on stderr so a reader fails loudly rather than compares a part */
const XvMoved *xvirus_moved(int *n);
/* Everything forgotten and kept no more (the unit tests) */
void xvirus_moved_reset(void);

#endif
