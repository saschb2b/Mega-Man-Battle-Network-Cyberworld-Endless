/* How a profile earns MegaMan's palettes (issue #854, docs/game/looks.md,
 * Earning): six by milestones the profile already records, three at the
 * home shop for the run's zenny, two as a rare find, one a run at most.
 * Each ends in looks_grant (looks.h). Looks only: nothing here changes how
 * a run plays, and the week's net earns none (docs/game/meta/weekly-net.md). */
#ifndef CW_LOOKS_ROUTES_H
#define CW_LOOKS_ROUTES_H

#include <stdbool.h>

/* ---- the milestones ---- */

/* Every milestone's palette whose record the profile holds, granted:
 * Undernet (a run reached the endless net's Undernet act: the best dive
 * past layer 12), Gregar (the Cybeast's fall at the endless Nest), ProtoMan
 * (Chaud's second clearance: his netbattle won), Nebula (one of BN5's
 * Darkloids deleted, rivals.sav), Hazard (a win on the top threat rung,
 * the title's disc). The tournament has no record before this; its final
 * won grants Tournament at the moment (looks_milestone). Cheap, and
 * called every frame on the map: a record made is a palette at once, and a
 * profile from before palettes gets its own at its next start. The
 * palettes new here, a bit each (1 << LookPalette). */
unsigned looks_milestones(void);
/* A milestone's palette at its moment (the Undernet first reached, the
 * tournament's final won): whether it was new. */
bool looks_milestone(int palette);

/* ---- the home shop (docs/world/home/home-shop.md) ---- */

/* A palette's price at AsterLand's counter, in zenny: about a third act's
 * earnings, three quarters of a Mega chip's order (docs/game/looks.md,
 * Earning, the price measured) */
#define LOOKS_SHOP_PRICE 6000
#define LOOKS_SHOP_COUNT 3   /* Central Town, Seaside, Sky */
/* The home's palettes the profile lacks, in their order, into `out`; how
 * many (none in the week's net). */
int looks_shop_offer(int out[LOOKS_SHOP_COUNT]);

/* ---- the rare find (the Spins' rule, docs/game/meta/spins.md) ---- */

/* The palette this run can find: Glitch where the run's seed gives it a
 * Glitch layer and the profile lacks it, else Mystery where it lacks that;
 * the one it picked up already, for a run that did (so a checkpoint from
 * before the find gives it again, never a second); -1 none. */
int looks_find_palette(void);
/* The normal layer whose best blue Mystery Data holds a Mystery find: an
 * act's first or last of 4-7 (never a dark layer), never the Spin's. */
int looks_find_depth(void);
/* The run's find picked up: the profile keeps it, and the run it was. */
void looks_find_taken(int palette);

/* ---- what MegaMan has said ---- */

/* The palettes owned that MegaMan has not yet said he got (Classic never),
 * a bit each; and those said. */
unsigned looks_untold(void);
void looks_told(unsigned palettes);

#endif
