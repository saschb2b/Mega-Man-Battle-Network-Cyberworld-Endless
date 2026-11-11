/* reward_pick.h. The offers a layer's scripts made, the flags their
 * scripts set to open one, its keys and the script its choice runs. */
#include "reward_pick.h"

#include <string.h>

#include "director.h"
#include "bn6.h"
#include "chip_entry.h"
#include "data.h"
#include "director_folder.h"
#include "director_state.h"
#include "emu.h"
#include "flags.h"
#include "navicust.h"
#include "reward_pick_state.h"
#include "run.h"
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

/* A choice's variant `mark` (a code, a colour), its count `held` more:
 * listed once, in the order they come */
static void add_held(int k, int mark, int held) {
	int i = 0;
	while (i < RP.nvar[k] && RP.var[k][i] != mark) ++i;
	if (i == RP.nvar[k]) {
		if (i >= PICK_VARIANTS) return;
		RP.var[k][i] = (uint8_t)mark;
		++RP.nvar[k];
	}
	RP.var_held[k][i] = (uint8_t)(RP.var_held[k][i] + held > 99 ? 99 : RP.var_held[k][i] + held);
}

/* A chip's codes, the one offered among them, and its copies in each in
 * the Folder and the Pack */
static void chip_held(int k, const uint16_t *folder, const uint16_t *pack, const uint8_t *copies, int np) {
	const PickChoice *c = &RP.o.c[k];
	ChipInfo ci;
	chip_info(c->id, &ci);
	for (int i = 0; i < ci.ncodes && i < PICK_VARIANTS; ++i) add_held(k, ci.codes[i] == '*' ? 26 : ci.codes[i] - 'A', 0);
	add_held(k, c->code, 0);
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i)
		if (folder[i] && chip_entry_id(folder[i]) == c->id) add_held(k, (int)chip_entry_code(folder[i]), 1);
	for (int i = 0; i < np; ++i)
		if (chip_entry_id(pack[i]) == c->id) add_held(k, (int)chip_entry_code(pack[i]), copies[i]);
}

/* A program's colours, and its copies of each on MegaMan's board (as the
 * draft counts them, run.programs) */
static void program_held(int k) {
	int p = RP.o.c[k].id, part, color;
	/* (a variant of no colour of the six, BN6's records' blank ones, not
	 * listed) */
	for (int v = 0; v < 4; ++v)
		if (navicust_part(p * 4 + v, &part, &color) && color >= 1 && color <= 6) add_held(k, color, 0);
	for (int i = 0; i < (int)sizeof run.programs && run.programs[i]; ++i)
		if (run.programs[i] / 4 == p && navicust_part(run.programs[i], &part, &color) && color >= 1 && color <= 6) add_held(k, color, 1);
}

#define PICK_PACK_MOST 1024   /* the Pack's entries read at most, a chip in a code each (order_sort.c's) */
static void count_held(void) {
	static uint16_t folder[BN6_FOLDER_ENTRIES], pack[PICK_PACK_MOST];
	static uint8_t copies[PICK_PACK_MOST];
	director_folder_now(folder);
	int np = director_pack_now(pack, copies, PICK_PACK_MOST), hp;
	director_megaman_hp(&hp, &RP.max_hp);
	for (int k = 0; k < RP.o.n; ++k) {
		if (RP.o.c[k].what == PICK_CHIP) chip_held(k, folder, pack, copies, np);
		else if (RP.o.c[k].what == PICK_PROGRAM) program_held(k);
	}
}

static void open_kind(int kind) {
	memset(&RP, 0, sizeof RP);
	RP.up = true;
	RP.kind = kind;
	RP.o = offers[kind];
	count_held();
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
	int rows = reward_pick_rows(), was = RP.cursor;
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
	if (RP.cursor != was) RP.moved = RP.t;
}

bool director_pick_shown(void) { return RP.up; }

void director_pick_shut(void) {
	RP.up = false;
	due = -1;
}
