/* A liberation's team picked on arrival (liberation_pick.h). */
#include "liberation_pick.h"

#include <string.h>

#include "emu.h"
#include "liberation_state.h"
#include "run.h"

LibPickMemory lib_pick_memory;

static struct {
	int places;               /* beside MegaMan */
	int n;                    /* the Navis held */
	uint8_t navi[LIB_HELD];
	bool on[LIB_HELD];        /* ... picked */
	int cursor;
} K;

bool lib_pick_needed(void) { return LB.b.team.nheld > LB.b.team.n - 1 && LB.b.team.n > 1; }

static int picked(void) {
	int n = 0;
	for (int i = 0; i < K.n; ++i) n += K.on[i];
	return n;
}

static void mark(int navi) {
	for (int i = 0; i < K.n; ++i)
		if (K.navi[i] == navi && !K.on[i] && picked() < K.places) K.on[i] = true;
}

void lib_pick_open(void) {
	memset(&K, 0, sizeof K);
	K.places = LB.b.team.n - 1;
	K.n = LB.b.team.nheld;
	memcpy(K.navi, LB.b.team.held, (size_t)K.n);
	/* (the last pick where its Navis are held, the board's own team after) */
	if (lib_pick_memory.run == run.seed)
		for (int i = 0; i < lib_pick_memory.n; ++i) mark(lib_pick_memory.navi[i]);
	for (int k = 1; k < LB.b.team.n; ++k) mark(LB.b.team.navi[k]);
}

bool lib_pick_keys(uint32_t pressed, uint8_t navi[LIB_TEAM], int *n) {
	if (pressed & KEY_DOWN) K.cursor = (K.cursor + 1) % K.n;
	if (pressed & KEY_UP) K.cursor = (K.cursor + K.n - 1) % K.n;
	if (pressed & KEY_A) {
		if (K.on[K.cursor]) K.on[K.cursor] = false;
		else if (picked() < K.places) K.on[K.cursor] = true;
	}
	if (!(pressed & KEY_START) || picked() < K.places) return false;
	*n = 0;
	for (int i = 0; i < K.n; ++i)
		if (K.on[i]) navi[(*n)++] = K.navi[i];
	lib_pick_memory.run = run.seed;
	lib_pick_memory.n = (uint8_t)*n;
	memcpy(lib_pick_memory.navi, navi, (size_t)*n);
	return true;
}

void lib_pick_draw(void) {}
