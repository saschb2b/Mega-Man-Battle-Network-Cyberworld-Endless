/* A test battle inside a snapshot of the game (lab_battle.h;
 * docs/game/battle-lab.md, The battle). */
#include "lab_battle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "debug.h"
#include "director_layer.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "events.h"
#include "run.h"

static struct {
	bool due;            /* asked for: begins with the next update */
	Encounter e;
	int ask_backdrop;
	bool fought;         /* BN6 has left its map for the battle */
	void *state;         /* the core's state as the battle began, its flash save in it */
	size_t size;
	DirectorState d;     /* ... the director's, */
	Run run;             /* ... the run's, */
	int backdrop;        /* ... and the layer's battle background */
	int hp;
	LabResult result;
} L;

/* the game's work RAM, hashed (FNV-1a): the proof that it came back */
static uint32_t ram_hash(void) {
	uint32_t h = 2166136261u;
	for (uint32_t at = BN6_EWRAM; at < BN6_EWRAM_END; at += 4) h = (h ^ emu_read32(at)) * 16777619u;
	return h;
}

void lab_battle_ask(const Encounter *e, int backdrop) {
	if (D.lab) return;
	L.e = *e;
	L.ask_backdrop = backdrop;
	L.due = true;
}

/* The battle begun: the game and the director kept first */
static bool begin(void) {
	free(L.state);
	L.state = emu_snapshot(&L.size);
	if (!L.state) {
		fprintf(stderr, "lab: the game's state could not be kept, no test battle\n");
		return false;
	}
	L.d = D;
	L.run = run;
	L.backdrop = encounter_backdrop;
	L.hp = emu_read16(BN6_NAVI_HP);
	L.fought = false;
	L.result = (LabResult){ .before = emu_debug_on() ? ram_hash() : 0 };
	D.lab = true;
	if (L.ask_backdrop >= 0) encounter_backdrop = L.ask_backdrop;
	emu_battle_watch(0, true);
	set_encounter(&L.e, true);
	return true;
}

/* The battle over, by its end or by a deletion the sparing blow did not
 * meet: its result read, the game, the run and the director as they were */
static void over(bool deleted) {
	bool won = !deleted && emu_read8(BN6_BATTLE_RESULT) == 1;
	int hp = deleted ? 0 : emu_read16(BN6_NAVI_HP);
	L.result.end = emu_battle_unwatch() || deleted ? LAB_DELETED : won ? LAB_WON : LAB_LEFT;
	L.result.frames = (int)emu_read32(BN6_BATTLE_TIMER);
	L.result.bust = won ? emu_read8(BN6_BATTLE_BUSTING) : 0;
	L.result.hp_lost = hp < L.hp ? L.hp - hp : 0;
	emu_battle_release();
	bool back = emu_snapshot_load(L.state, L.size);
	free(L.state);
	L.state = NULL;
	bool run_same = !memcmp(&run, &L.run, sizeof run);
	run = L.run;
	D = L.d;
	encounter_backdrop = L.backdrop;
	/* (the battle the map was owed is the engine's record again: the test's took its place) */
	if (D.active && !D.town) set_encounter(&D.next, false);
	if (emu_debug_on()) L.result.after = ram_hash();
	if (emu_debug_on() || !back || !run_same)
		fprintf(stderr, "lab: %s in %d frames, busting level %d, %d HP lost; %d frames of the game %s (RAM %08x, then %08x), the run %s\n",
			L.result.end == LAB_WON ? "won" : L.result.end == LAB_DELETED ? "deleted" : "left", L.result.frames, L.result.bust, L.result.hp_lost,
			L.result.took, back ? "put back" : "NOT put back", (unsigned)L.result.before, (unsigned)L.result.after, run_same ? "untouched" : "CHANGED, put back");
}

bool lab_battle_frame(void) {
	if (L.due) {
		L.due = false;
		if (!D.lab && begin()) return true;
	}
	if (!D.lab) return false;
	++L.result.took;
	/* (the hooks' events: the battle's start alone, for the Souls BN6's engine is handed) */
	HookEvent ev[32];
	int n = emu_hook_events(ev, 32);
	for (int i = 0; i < n; ++i)
		if (ev[i].kind == EV_BATTLE_START) emu_encounter_started(ev[i].r[0]);
	if (main_mode() == BN6_MODE_GAME_OVER) { over(true); return false; }
	int sub = emu_read8(BN6_GAMESTATE);
	if (sub == BN6_SUB_BATTLE_INIT || sub == BN6_SUB_BATTLE) {
		L.fought = true;
		emu_battle_release();
		return true;
	}
	if (L.fought && on_map()) { over(false); return false; }
	return true;
}

const LabResult *lab_battle_result(void) { return &L.result; }
