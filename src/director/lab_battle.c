/* A test battle inside a snapshot of the game (lab_battle.h;
 * docs/game/battle-lab.md, The battle). */
#include "lab_battle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bn6_forms.h"
#include "debug.h"
#include "director_layer.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "events.h"
#include "game.h"
#include "run.h"

static struct {
	bool due;            /* asked for: begins with the next update */
	Encounter e;
	int ask_backdrop;
	bool show;           /* the builder's: begun to be held with its field up (lab_battle_show) */
	bool post;           /* begun between frames, the director's update of its first frame already run: none is owed as it ends */
	bool held;           /* ... and held there now: the scene runs no frame of it */
	bool bare;           /* ... a field with no foe on it: held before the one its record names comes in */
	int in_battle;       /* frames since BN6 left its map for it */
	bool fought;         /* BN6 has left its map for the battle */
	void *state;         /* the core's state as the battle began, its flash save in it */
	size_t size;
	DirectorState d;     /* ... the director's, */
	Run run;             /* ... the run's, */
	int backdrop;        /* ... and the layer's battle background */
	int hp;
	uint32_t rng;        /* ... and the engine's own dice, so that a set shown again rolls its background the same */
	bool painted;        /* its field is ours: BN6's last field's bytes in the core's copy, */
	bool ours;           /* ... written there now */
	uint8_t field[BN6_FIELD_SIZE], was[BN6_FIELD_SIZE];   /* ... ours and BN6's */
	LabResult result, last;   /* the battle under way's, and the last one fought to its end */
} L;

#define LAB_FIELD (BN6_FIELD_COUNT - 1)   /* the field of BN6's a painted one takes the place of while its battle is on */
#define LAB_FIELD_AT (BN6_FIELDS + LAB_FIELD * BN6_FIELD_SIZE)

/* the game's work RAM, hashed (FNV-1a): the proof that it came back */
static uint32_t ram_hash(void) {
	uint32_t h = 2166136261u;
	for (uint32_t at = BN6_EWRAM; at < BN6_EWRAM_END; at += 4) h = (h ^ emu_read32(at)) * 16777619u;
	return h;
}

void lab_battle_ask(const Encounter *e, int backdrop) {
	if (D.lab) return;
	L.e = *e;
	L.e.held = false;
	L.e.leave = true;
	L.ask_backdrop = backdrop;
	L.show = L.post = false;
	L.due = true;
}

void lab_battle_panels(const uint8_t *panels) {
	L.painted = panels != NULL;
	if (!panels) return;
	/* (a field's bytes: a u32 a row, a nibble a column from the left) */
	for (int row = 0; row < 3; ++row) {
		uint32_t w = 0;
		for (int col = 0; col < 6; ++col) w |= (uint32_t)(panels[row * 6 + col] & 15) << (4 * col);
		for (int k = 0; k < 4; ++k) L.field[row * 4 + k] = (uint8_t)(w >> (8 * k));
	}
}

bool lab_battle_on(void) { return L.due || D.lab; }
bool lab_battle_held(void) { return D.lab && L.held; }
bool lab_battle_loading(void) { return L.show && !L.held && (L.due || D.lab) && !L.result.end; }

const LabResult *lab_battle_result(void) { return &L.last; }

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
	L.rng = rng_state();
	L.fought = L.held = false;
	L.in_battle = 0;
	L.result = (LabResult){ .before = emu_debug_on() ? ram_hash() : 0 };
	D.lab = true;
	encounter_backdrop = L.ask_backdrop;
	if (L.painted) {
		for (uint32_t k = 0; k < BN6_FIELD_SIZE; ++k) L.was[k] = emu_read8(LAB_FIELD_AT + k);
		emu_write(LAB_FIELD_AT, L.field, sizeof L.field);
		L.e.field = LAB_FIELD;
		L.ours = true;
	}
	emu_battle_watch(0, true);
	emu_battle_unpaid();
	set_encounter(&L.e, true);
	return true;
}

/* The game, the run and the director as they were before the battle, BN6's
 * field its own again: whether each came back */
static bool put_back(bool *run_same, bool *field_back) {
	emu_battle_release();
	bool back = emu_snapshot_load(L.state, L.size);
	free(L.state);
	L.state = NULL;
	*run_same = !memcmp(&run, &L.run, sizeof run);
	run = L.run;
	D = L.d;
	encounter_backdrop = L.backdrop;
	*field_back = true;
	if (L.ours) {
		emu_write(LAB_FIELD_AT, L.was, sizeof L.was);
		for (uint32_t k = 0; k < BN6_FIELD_SIZE; ++k) *field_back &= emu_read8(LAB_FIELD_AT + k) == L.was[k];
	}
	L.ours = L.held = false;
	/* (the battle the map was owed is the engine's record again: the test's took its place) */
	if (D.active && !D.town) set_encounter(&D.next, false);
	/* (the dice as they lay, after that record's own rolls: a set shown again rolls its background the same) */
	rng_seed(L.rng);
	return back;
}

/* The battle over, by its end or by a deletion the sparing blow did not
 * meet: its result read, then all put back */
static void over(bool deleted) {
	bool won = !deleted && emu_read8(BN6_BATTLE_RESULT) == 1;
	int hp = deleted ? 0 : emu_read16(BN6_NAVI_HP);
	L.result.end = emu_battle_unwatch() || deleted ? LAB_DELETED : won ? LAB_WON : LAB_LEFT;
	L.result.frames = (int)emu_read32(BN6_BATTLE_TIMER);
	L.result.bust = won ? emu_read8(BN6_BATTLE_BUSTING) : 0;
	L.result.hp_lost = hp < L.hp ? L.hp - hp : 0;
	bool run_same, field_back, back = put_back(&run_same, &field_back);
	if (emu_debug_on()) L.result.after = ram_hash();
	if (emu_debug_on() || !back || !run_same || !field_back)
		fprintf(stderr, "lab: %s in %d frames, busting level %d, %d HP lost; %d frames of the game %s (RAM %08x, then %08x), the run %s, BN6's field %s\n",
			L.result.end == LAB_WON ? "won" : L.result.end == LAB_DELETED ? "deleted" : "left", L.result.frames, L.result.bust, L.result.hp_lost,
			L.result.took, back ? "put back" : "NOT put back", (unsigned)L.result.before, (unsigned)L.result.after, run_same ? "untouched" : "CHANGED, put back",
			field_back ? "as it was" : "NOT put back");
	L.last = L.result;
}

/* ---- the builder's: the battle shown, held with its field up ---- */

/* frames into BN6's battle at which its field is whole and no foe has come in yet (seen headless: the first materializes
 * about 20 frames on) */
#define LAB_BARE_FRAMES 104
/* ... and at most before it is held whatever its Custom screen says */
#define LAB_HOLD_MOST 600

void lab_battle_show(const Encounter *e, int backdrop, bool bare) {
	/* (one shown already: put back first, the game on the frame it began on) */
	if (D.lab) {
		bool a, b;
		emu_battle_unwatch();
		put_back(&a, &b);
	}
	L.e = *e;
	L.e.held = false;
	L.e.leave = true;
	L.ask_backdrop = backdrop;
	L.show = L.post = true;
	L.bare = bare;
	L.due = false;
	if (!begin()) L.show = false;
}

void lab_battle_go(void) {
	if (!lab_battle_held()) return;
	L.held = L.show = false;
	if (emu_debug_on()) fprintf(stderr, "lab: the battle held runs on, background %d, BN6's song %#x playing\n", encounter_backdrop, emu_read8(BN6_SONG_PLAYING));
}

void lab_battle_leave(void) {
	L.due = L.show = false;
	if (!D.lab) return;
	bool run_same, field_back;
	uint32_t before = L.result.before;
	emu_battle_unwatch();
	bool back = put_back(&run_same, &field_back);
	if (emu_debug_on() || !back || !run_same || !field_back)
		fprintf(stderr, "lab: left with no fight; the game %s (RAM %08x, then %08x), the run %s, BN6's field %s\n", back ? "put back" : "NOT put back",
			(unsigned)before, (unsigned)(emu_debug_on() ? ram_hash() : 0), run_same ? "untouched" : "CHANGED, put back", field_back ? "as it was" : "NOT put back");
}

/* whether the battle shown has its field up as the builder wants it: its foes all in and BN6's Custom screen about to
 * open (its state's first byte 4, a frame before its window slides in), or for a field with no foe the frame before
 * the first comes in */
static bool field_up(void) {
	if (L.in_battle >= LAB_HOLD_MOST) return true;
	return L.bare ? L.in_battle >= LAB_BARE_FRAMES : emu_read8(BN6_CUSTOM_SCREEN) == BN6_CUSTOM_OPENING;
}

bool lab_battle_frame(void) {
	if (L.due) {
		L.due = false;
		if (!D.lab && begin()) return true;
	}
	if (!D.lab) return false;
	if (L.held) return true;
	++L.result.took;
	/* (the hooks' events: the battle's start alone, for the Souls BN6's engine is handed) */
	HookEvent ev[32];
	int n = emu_hook_events(ev, 32);
	for (int i = 0; i < n; ++i)
		if (ev[i].kind == EV_BATTLE_START) emu_encounter_started(ev[i].r[0]);
	/* (ended: all put back, and the update this battle took the place of runs now, unless it ran before the battle began) */
	if (main_mode() == BN6_MODE_GAME_OVER) { over(true); return L.post; }
	int sub = emu_read8(BN6_GAMESTATE);
	if (sub == BN6_SUB_BATTLE_INIT || sub == BN6_SUB_BATTLE) {
		L.fought = true;
		++L.in_battle;
		emu_battle_release();
		if (L.show && sub == BN6_SUB_BATTLE && field_up()) L.held = true;
		return true;
	}
	if (L.fought && on_map()) { over(false); return L.post; }
	return true;
}
