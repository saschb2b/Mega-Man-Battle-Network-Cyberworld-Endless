/* A party on one field (the shared dive, issue #589; docs/online/shared-dive.md):
 * up to four MegaMen on side 0 of one battle, against its enemies, on one
 * core. Each partner is a MegaMan of BN6's spawned beside the player (the
 * battle's list grown at BN6_SPAWN_ENTITIES), his keys written into his AI
 * data at the battle loop's key copy (BN6_KEY_COPY) as BN6 writes a link
 * player's.
 *
 * The party's core (party_core.c) sets a field up on its own core, each
 * seat's keys and HP going in given; --dev party=N (2-4) is the spike on the
 * run's own core: every battle of the run gets N MegaMen, the partners' keys
 * a script, and each MegaMan's HP, panel and the enemies' HP logged as they
 * change ("party: ..."). */
#ifndef CW_PARTY_FIELD_H
#define CW_PARTY_FIELD_H

#include <stdbool.h>
#include <stdint.h>

#include "hook.h"

#define PARTY_MOST 4

typedef struct {
	/* set before the battle */
	int n;                          /* its MegaMen (1 alone) */
	bool scripted;                  /* the partners' keys the spike's script, else keys[] */
	bool quiet;                     /* no log lines */
	uint16_t hp[PARTY_MOST], max_hp[PARTY_MOST];   /* each MegaMan's HP going in (0: as spawned) */
	int foe_from, foe_to;           /* the enemy spawning with foe_from HP given foe_to (the guardian's for the party) */
	int hp_set, end;                /* the spike's: the partners' HP (partyhp), the enemies' HP set to 1 at an iteration (partyend) */
	bool spread;                    /* the enemies' aim passed over the party in turn (party_field_aim) */
	/* set before each frame */
	uint16_t keys[PARTY_MOST];      /* each partner's keys (keys[0] the pad's, unused here) */
	/* the battle's, which a rollback puts back with the core */
	int standing;                   /* the battle under way's MegaMen spawned (0 none) */
	uint32_t tick;                  /* its key copies of side 0 so far */
	uint16_t seen_hp[2][PARTY_MOST];   /* each side's actors' HP as last seen */
	uint32_t seen_obj[2][PARTY_MOST];  /* ... and their objects */
	unsigned swapped;               /* side 0's slot traded into its first while an enemy's routine runs (0 none) */
} PartyField;

/* `f`'s hooks on `core`'s table (hook_core's): false where one was not
 * taken */
bool party_field_hooks(HookCore *core, PartyField *f);

/* ... and the enemies' aim spread over the party (f->spread): BN6's take
 * side 0's first slot of BN6_BATTLE_PLAYERS for their target as they
 * update (docs/re/ledger/experiments/2026-11-15-the-enemies-aim-at-side-0s-first-slot.json),
 * so while an object of side 1's routine runs that slot holds the MegaMan
 * the party's turn names, every AIM_TURN iterations the next standing one,
 * and the lead's object is back in it as the routine returns */
bool party_field_aim(HookCore *core, PartyField *f);

/* The hooks on the game's core, once (game_hooks_after_boot), for --dev
 * party=n: every battle of the run with n MegaMen (2 to PARTY_MOST); hp > 0
 * the partners' HP as each battle begins (partyhp), end > 0 the enemies' HP
 * set to 1 at the battle loop's iteration end (partyend). */
void party_field_install(int n, int hp, int end);

#endif
