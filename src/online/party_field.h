/* A party on one field (the shared dive, issue #589; docs/online/shared-dive.md):
 * up to four MegaMen on side 0 of one of the run's own battles, against its
 * enemies, on one core. Each partner is a MegaMan of BN6's spawned beside the
 * player (the battle's list grown at BN6_SPAWN_ENTITIES), his keys written
 * into his AI data at the battle loop's key copy (BN6_KEY_COPY) as BN6 writes
 * a link player's.
 *
 * --dev party=N (2-4) is the spike: every battle of the run gets N MegaMen,
 * the partners' keys a script, and each MegaMan's HP, panel and the enemies'
 * HP logged as they change ("party: ..."). */
#ifndef CW_PARTY_FIELD_H
#define CW_PARTY_FIELD_H

#include <stdbool.h>
#include <stdint.h>

#define PARTY_MOST 4

/* The hooks on the game's core, once (game_hooks_after_boot), for --dev
 * party=n: every battle of the run with n MegaMen (2 to PARTY_MOST); hp > 0
 * the partners' HP as each battle begins (partyhp), end > 0 the enemies' HP
 * set to 1 at the battle loop's iteration end (partyend). */
void party_field_install(int n, int hp, int end);

#endif
