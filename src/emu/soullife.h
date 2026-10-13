/* A Soul's life on the proxy form (the Souls track's S2; docs/MULTIROM.md,
 * Transplanted Souls), by BN5's rule (docs/ROM_DATA.md, UNITE on BN6's
 * Custom screen, and a Soul's life on the proxy form): its state per side
 * in BN6_SOULS (zeroed with the battle's RAM as each battle is set up),
 * taken up as MegaMan enters form 8 with a Soul committed on the Custom
 * screen and cleared as he leaves it (the proxy's entries 8 of the overlay
 * tables, through two stubs of ours), its turns counted down as each
 * Custom screen opens (BN6's own countdown's two calls), its end as the
 * turn after its last starts (Beast Out's end's place, through BN6's
 * request to leave the form), and no weakness break while it lasts.
 * soulunite.c, which offers UNITE, installs it. */
#ifndef CW_SOULLIFE_H
#define CW_SOULLIFE_H

#include <stdbool.h>
#include <stdint.h>

/* side's field of the Souls' state (BN6_SOUL_*): its address */
uint32_t soul_field(int side, uint32_t field);
/* (from a hook) whether side is united with a Soul */
bool soul_united(int side);

/* The hooks of a Soul's life, and its two stubs written at `stubs` (8 bytes
 * of free space), the proxy's overlay entries 8 pointed at them (each on
 * to the routine the entry held, the proxy's own). Once a core, after
 * soulproxy_install; false (nothing written) where a byte it stands on is
 * not Gregar's as expected. `keep` (the differential's own test, soulunite.h's
 * SOULUNITE_BREAK_LEAVE) keeps a Soul as he leaves the form. */
bool soullife_install(uint32_t stubs, bool keep);
/* A Soul's way into the proxy form and out of it (the Souls track's S3,
 * soullook.c): `enter[n]` and `leave[n]` the routines BN5's Soul n runs
 * there in place of the proxy's overlay (BN6 Thumb addresses, its moved
 * code's; 0 none), BN5's 0-12. Before this, a Soul enters and leaves as
 * the proxy does (HeatCross's overlay). */
#define SOULS_BN5_FORMS 13
void soullife_routes(const uint32_t enter[SOULS_BN5_FORMS], const uint32_t leave[SOULS_BN5_FORMS]);

#endif
