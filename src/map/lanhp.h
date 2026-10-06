/* Lan's HP taken over (lanhp.c, docs/HOME.md): MegaMan's homepage, where
 * every jack-in of a run arrives and the run's portals stand. */
#ifndef CW_LANHP_H
#define CW_LANHP_H

#include <stdbool.h>
#include <stdint.h>

#define LANHP_GROUP  0x88   /* bn6f HOMEPAGES */
#define LANHP_NUMBER 0x00   /* ... LAN_HP */
#define LANHP_PORTALS 5     /* its warp spots but the arrival pad */

/* Set while the town holds nothing for this visit: the courier stays away
 * (the director keeps it, docs/HOME.md piece 2). */
#define LANHP_COURIER_GONE_FLAG 0x146C

/* Lan's HP installed: BN6's own tiles, walls and decorations, no map
 * scripts or Mystery Data, its own warp list (every entry back to the
 * arrival until a portal is pointed) and its song; of people, the courier,
 * a Mr.Prog with BN6's "!!" over his head, who is there while
 * LANHP_COURIER_GONE_FLAG is clear, and says nothing until
 * lanhp_courier_say. False where the map's warp spots cannot be read. */
bool lanhp_install(void);
/* The courier's words: a text archive's bytes (ta_build, his talk its
 * script 0), written over his last; false where they do not fit. */
bool lanhp_courier_say(const uint8_t *archive, int n);
/* Where BN6's jack-in to Lan's HP sets MegaMan down (world units): on its
 * blue pad, the arrival. */
void lanhp_arrival(int *x, int *y);
/* Portal `k` (0 the pink pad, then the link squares on the floor) leads to
 * world (x, y) of map (group, number), MegaMan facing `facing`. */
void lanhp_portal(int k, int group, int number, int x, int y, int facing);
/* The portal a warp entry (BN6_WARP_INDEX) is; -1 the arrival or none. */
int lanhp_portal_of(int entry);
/* The portals in `lit` on (bit k portal k), the rest off (BN6's warp-off
 * flags, which entering a map clears: again after each entry). The blue
 * pad stays BN6's jack-out. */
void lanhp_lit(unsigned lit);
/* Portal `k`'s spot: the middle of its cells (world units). */
void lanhp_portal_spot(int k, int *x, int *y);
/* The portal whose spot holds world (x, y), or lies within `reach` world
 * units of it; -1 none. */
int lanhp_portal_near(int x, int y, int reach);

#endif
