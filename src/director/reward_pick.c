/* reward_pick.h. The offers a layer's scripts made, the flags their
 * scripts set to open one, its keys and the script its choice runs. */
#include "reward_pick.h"

#include <string.h>

#include "director.h"
#include "bn6.h"
#include "director_state.h"
#include "emu.h"
#include "flags.h"
#include "reward_pick_state.h"
#include "talk.h"

PickState RP;

static PickOffer offers[PICK_KINDS];
static bool offered[PICK_KINDS];
static int due = -1;   /* a choice's script to start, once the game takes it */

void reward_pick_offer(int kind, const PickOffer *o) {
	if (kind < 0 || kind >= PICK_KINDS || !o) return;
	offers[kind] = *o;
	if (offers[kind].n > PICK_MOST) offers[kind].n = PICK_MOST;
	offered[kind] = true;
}

int reward_pick_rows(void) { return RP.o.n + (RP.o.frags > 0); }

static void open_kind(int kind) {
	memset(&RP, 0, sizeof RP);
	RP.up = true;
	RP.kind = kind;
	RP.o = offers[kind];
}

void director_pick_watch(void) {
	if (!D.active) return;
	if (due >= 0 && talk_script(D.objs.archive, due)) due = -1;
	/* (the chat box closed first: its last box showed again under the
	 * game as the screen left) */
	if (RP.up || !flag_get(REWARD_PICK_FLAG) || emu_read8(BN6_CHATBOX)) return;
	int kind = 0;
	for (int b = 0; b < REWARD_KIND_BITS; ++b) {
		if (flag_get(REWARD_KIND_FLAG + b)) kind |= 1 << b;
		flag_clear(REWARD_KIND_FLAG + b);
	}
	flag_clear(REWARD_PICK_FLAG);
	if (kind < PICK_KINDS && offered[kind] && offers[kind].n > 0) open_kind(kind);
}

static void choose(int script) {
	RP.up = false;
	due = script;
}

void director_pick_keys(uint32_t keys) {
	if (!RP.up) return;
	++RP.t;
	uint32_t now = keys & ~RP.was;
	RP.was = keys;
	int rested = RP.rest;
	RP.rest = keys & KEY_A ? 0 : RP.rest + 1;
	if (RP.t < PICK_LOCK) return;
	int rows = reward_pick_rows();
	if (now & KEY_UP) RP.cursor = (RP.cursor + rows - 1) % rows;
	else if (now & KEY_DOWN) RP.cursor = (RP.cursor + 1) % rows;
	else if ((now & KEY_A) && rested >= PICK_REST) {
		/* (the choice's script, or the skip's from its row) */
		choose(RP.cursor < RP.o.n ? RP.o.c[RP.cursor].script : RP.o.skip);
	} else if (now & KEY_B) {
		/* (a skip with a row: B goes to it, A takes it; a vault's leaves) */
		if (RP.o.frags > 0) RP.cursor = RP.o.n;
		else if (RP.o.skip == PICK_B_LEAVES) choose(-1);
		else if (RP.o.skip >= 0) choose(RP.o.skip);
	}
}

bool director_pick_shown(void) { return RP.up; }

void director_pick_shut(void) {
	RP.up = false;
	due = -1;
}
