/* What a guest battle's results screen gives, as the run gets it
 * (guest_reward.c; docs/MULTIROM.md, Guest battles): the rows its rewards
 * are drawn from, written before a battle so the screen shows what the
 * run will get, and its reward words read as the run's. */
#ifndef CW_GUEST_REWARD_H
#define CW_GUEST_REWARD_H

#include <stdbool.h>
#include <stdint.h>

#include "guest.h"

/* How a battle's rows are written */
typedef struct {
	bool star;          /* the All * helper: every chip in * (GuestMegaMan.star) */
	uint8_t codes[3];   /* the folder's codes, half the enemies' chip rewards in one (GuestMegaMan.codes) */
	int boss_id;        /* a guardian's enemy id, 0 none, */
	int boss_zenny;     /* ... and the zenny his rows' chips pay */
} GuestRowsFit;

/* The reward rows of enemies `ids` (n of them, its game's ids) as the run
 * gets them, in the guest's ROM copy: a chip BN6 has none of as zenny, the
 * others in * with All *, else half of them in the folder's codes; a
 * guardian's chips as his zenny. */
void guest_rows_fit(const int *ids, int n, const GuestRowsFit *how);
/* The battlefield Mystery Data's finds (BN5_FIND_ROWS) likewise: a chip BN6
 * has none of as zenny, in * with All *; else as its ROM has them. */
void guest_finds_fit(const GuestRowsFit *how);
/* A results screen's reward word (BN5_REWARD, BN5_REWARD_FIND) as the run
 * gets it: a chip as BN6's of its name, in a code BN6's has; zenny, BugFrags,
 * HP restored; nothing for 0 or 0xFFFF. */
GuestReward guest_reward_of(uint16_t word);

#endif
