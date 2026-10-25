/* A liberation's team picked on arrival (liberation_pick.h): a list of the
 * Navis held, GO! at its foot as NaviCust's RUN! at its list's (docs/UI.md,
 * Ours: the JACK-IN SETUP's JACK IN!). A picks or puts back the Navi under
 * the cursor; with one place, A on another Navi takes him instead; A on
 * GO!, or START, goes once every place is filled. B does nothing: the pick
 * is to be made. Its picture in liberation_pick_draw.c. */
#include "liberation_pick.h"

#include <string.h>

#include "emu.h"
#include "devtools.h"
#include "director_save.h"
#include "director_state.h"
#include "liberation.h"
#include "liberation_pick_state.h"
#include "liberation_plan.h"
#include "liberation_state.h"
#include "liberation_team.h"
#include "run.h"

LibPickMemory lib_pick_memory;
LibPickState LP;

bool lib_pick_needed(void) { return LB.b.team.nheld > LB.b.team.n - 1 && LB.b.team.n > 1; }

int lib_pick_count(void) {
	int n = 0;
	for (int i = 0; i < LP.n; ++i) n += LP.order[i] > 0;
	return n;
}

static void pick(int i) {
	int last = 0;
	for (int j = 0; j < LP.n; ++j) last = LP.order[j] > last ? LP.order[j] : last;
	LP.order[i] = (uint8_t)(last + 1);
}

/* Navi i put back: those picked after him one place up */
static void unpick(int i) {
	for (int j = 0; j < LP.n; ++j)
		if (LP.order[j] > LP.order[i]) --LP.order[j];
	LP.order[i] = 0;
}

static void mark(int navi) {
	for (int i = 0; i < LP.n; ++i)
		if (LP.navi[i] == navi && !LP.order[i] && lib_pick_count() < LP.places) pick(i);
}

void lib_pick_open(void) {
	memset(&LP, 0, sizeof LP);
	LP.places = LB.b.team.n - 1;
	LP.n = LB.b.team.nheld < LIB_HELD ? LB.b.team.nheld : LIB_HELD;
	memcpy(LP.navi, LB.b.team.held, (size_t)LP.n);
	/* (the last pick where its Navis are held, the board's own team after) */
	if (lib_pick_memory.run == run.seed)
		for (int i = 0; i < lib_pick_memory.n; ++i) mark(lib_pick_memory.navi[i]);
	for (int k = 1; k < LB.b.team.n; ++k) mark(LB.b.team.navi[k]);
	/* (the cursor on GO! where every place is filled, the last pick taken
	 * again with one press; else on the first Navi) */
	LP.cursor = lib_pick_count() == LP.places ? LP.n : 0;
}

/* A on the Navi under the cursor */
static void toggle(int i) {
	if (LP.order[i]) { unpick(i); return; }
	if (lib_pick_count() < LP.places) { pick(i); return; }
	/* (one place: the one picked gives it up) */
	if (LP.places == 1) {
		for (int j = 0; j < LP.n; ++j)
			if (LP.order[j]) unpick(j);
		pick(i);
	}
}

/* The pick made: the Navis in the order picked, the run's last pick */
static bool go(uint8_t navi[LIB_TEAM], int *n) {
	if (lib_pick_count() < LP.places) return false;
	*n = 0;
	for (int o = 1; o <= LP.places; ++o)
		for (int i = 0; i < LP.n; ++i)
			if (LP.order[i] == o && *n < LIB_TEAM - 1) navi[(*n)++] = LP.navi[i];
	lib_pick_memory.run = run.seed;
	lib_pick_memory.n = (uint8_t)*n;
	memcpy(lib_pick_memory.navi, navi, (size_t)*n);
	return true;
}

bool lib_pick_keys(uint32_t pressed, uint8_t navi[LIB_TEAM], int *n) {
	++LP.t;
	int rows = LP.n + 1;
	if (pressed & KEY_DOWN) LP.cursor = (LP.cursor + 1) % rows;
	if (pressed & KEY_UP) LP.cursor = (LP.cursor + rows - 1) % rows;
	/* (the list scrolled to keep a Navi's row under the cursor in view) */
	if (LP.cursor < LP.n) {
		if (LP.cursor < LP.first) LP.first = LP.cursor;
		if (LP.cursor >= LP.first + PICK_ROWS_SHOWN) LP.first = LP.cursor - PICK_ROWS_SHOWN + 1;
	}
	if ((pressed & KEY_A) && LP.cursor < LP.n) toggle(LP.cursor);
	if ((pressed & KEY_START) || ((pressed & KEY_A) && LP.cursor == LP.n)) return go(navi, n);
	return false;
}

/* The team picked: where it differs from the board's own, its Navis'
 * bodies on the board's objects, the map entered again for them, the par
 * the planner's for it; then the opening's words */
static void apply(const uint8_t *navi, int n) {
	bool same = n == LB.b.team.n - 1;
	for (int k = 1; same && k < LB.b.team.n && k < LIB_TEAM; ++k) same = LB.b.team.navi[k] == navi[k - 1];
	LB.step = LIB_STEP_WAIT;
	LB.tour = 3;
	if (same) return;
	lib_team_pick(navi, n);
	lib_plan_par(&LB.b);
	if (dev.lib_par > 0) LB.b.par = dev.lib_par;
	lib_team_dress(D.objs.lib_body);
	layer_reenter(true);
	lib_camera_mend();
}

void lib_pick_step(uint32_t pressed) {
	uint8_t navi[LIB_TEAM];
	int n = 0;
	if (lib_pick_keys(pressed, navi, &n)) apply(navi, n);
}
