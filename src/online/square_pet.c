/* square_pet.h. */
#include "square_pet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "cinema.h"
#include "debug.h"
#include "devtools.h"
#include "emu.h"
#include "jukebox.h"
#include "library.h"
#include "netpass_screen.h"
#include "online_square.h"
#include "online_visit.h"
#include "pet.h"
#include "pet_carry.h"
#include "platform.h"
#include "run.h"
#include "save_blob.h"
#include "save_queue.h"
#include "trade_central.h"

#define RECORD "run.pet"   /* beside the checkpoint, the slot's own (save_path) */
/* BN6's state flags (EVENT_17xx: the player can move, the PET is off, no encounters, a request held), the map's
 * and the moment's, which the Square sets as it needs them (online_square.c): never carried */
/* frames on the map before a word is said after the PET: a chat box opened straight out of it drew its letters
 * as noise (director_board.c's bug_watch found six enough) */
#define CALM 8
#define SAVED_NOTE 150   /* the frames Save's note shows, as a run's */
#define STATE_FLAGS     0x1700
#define STATE_FLAGS_END 0x1800

/* the record and the regions read twice: on the heap from a visit's start
 * (or a CONTINUE's look for a record) until home has it */
typedef struct {
	uint8_t was[PET_CARRY_BYTES], is[PET_CARRY_BYTES];
	PetCarry carry;
} Work;

static struct {
	bool open;      /* the PET up, its regions read as it opened */
	int calm;       /* frames on the Square's map since it closed */
	bool saved;     /* Save chosen: the note due */
	int note;       /* ... showing, its frames so far (0 none) */
	bool known;     /* the record in memory is this visit's (else the file's, at a CONTINUE) */
	Work *w;
} S;

static bool work(void) {
	if (!S.w) S.w = calloc(1, sizeof *S.w);
	if (!S.w) fprintf(stderr, "square pet: no memory for the PET's changes; none is carried\n");
	return S.w != NULL;
}

static void work_free(void) {
	free(S.w);
	S.w = NULL;
	S.known = false;
}

static void regions_read(uint8_t *out) {
	for (int r = 0; r < PET_CARRY_REGIONS; ++r)
		for (uint32_t k = 0; k < pet_carry_regions[r].bytes; ++k) *out++ = emu_read8(pet_carry_regions[r].at + k);
}

/* the bytes that differ written back, a region after another */
static void regions_write(const uint8_t *from, const uint8_t *was) {
	for (int r = 0; r < PET_CARRY_REGIONS; ++r)
		for (uint32_t k = 0; k < pet_carry_regions[r].bytes; ++k, ++from, ++was)
			if (*from != *was) emu_write8(pet_carry_regions[r].at + k, *from);
}

/* the record's bytes that are event flags: up to the chips' data, where the
 * game's memory has it (0 where it cannot be read: nothing is carried) */
static uint32_t flag_bytes(void) {
	uint32_t chips = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	return chips > BN6_EVENT_FLAGS && chips < BN6_EVENT_FLAGS + PET_CARRY_MAIN ? chips - BN6_EVENT_FLAGS : 0;
}

static void record_remove(void) {
	char path[600];
	save_queue_wait();   /* (one still queued would come back after it) */
	save_path(path, sizeof path, RECORD);
	remove(path);
}

/* (debug: where a session's changes lie, the first few) */
static void changes_print(void) {
	unsigned at = 0, shown = 0;
	for (int r = 0; r < PET_CARRY_REGIONS; ++r)
		for (uint32_t k = 0; k < pet_carry_regions[r].bytes; ++k, ++at)
			if (S.w->was[at] != S.w->is[at] && shown++ < 24)
				fprintf(stderr, "square pet: %08x %02x to %02x\n", (unsigned)(pet_carry_regions[r].at + k), S.w->was[at], S.w->is[at]);
}

void square_pet_begin(void) {
	S.open = S.saved = false;
	S.calm = CALM;
	S.note = 0;
	record_remove();
	if (!work()) return;
	S.known = true;
	pet_carry_clear(&S.w->carry, run.seed, run.depth, flag_bytes());
}

bool square_pet_open(void) {
	int mode = emu_read8(emu_read32(BN6_TOOLKIT));
	return mode == BN6_MODE_SUBMENU || mode == BN6_MODE_MAIL || (mode == BN6_MODE_GAME && emu_read8(BN6_GAMESTATE) == BN6_SUB_PET);
}

/* the PET's first menu, taking input */
static bool on_menu(void) {
	return emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_GAME && emu_read8(BN6_GAMESTATE) == BN6_SUB_PET &&
		(emu_read8(BN6_PET_MENU_OPEN) & 1) && !emu_read8(BN6_PET_HOLD);
}

void square_pet_noting(void) {
	if (S.known && S.w) regions_read(S.w->was);
}

bool square_pet_noted(bool trade) {
	if (!S.known || !S.w || !S.w->carry.bits) return false;
	Work *w = S.w;
	regions_read(w->is);
	/* (BN6's own flags of the moment are no change to carry: EnterMap clears the no-encounters bit as the PET closes) */
	memcpy(w->is + STATE_FLAGS / 8, w->was + STATE_FLAGS / 8, (STATE_FLAGS_END - STATE_FLAGS) / 8);
	unsigned n = pet_carry_note(&w->carry, w->was, w->is);
	if (trade) w->carry.traded = 1;
	if (!n && !trade) return false;
	printf("square pet: %s changed %u bytes, %u to carry home\n", trade ? "the trade" : "the PET", n, (unsigned)w->carry.count);
	if (emu_debug_on()) changes_print();
	if (!save_write_blob(RECORD, PET_CARRY_MAGIC, &w->carry, sizeof w->carry)) fprintf(stderr, "square pet: the record was not written\n");
	return n > 0;
}

bool square_pet_frame(void) {
	pet_update();
	bool changed = false;
	if (!S.open && square_pet_open()) {
		S.open = true;
		square_pet_noting();
	} else if (S.open && online_square_arrived()) {
		/* (back on the Square's map: what this PET session changed, noted and kept beside the checkpoint) */
		S.open = false;
		S.calm = 0;
		changed = square_pet_noted(false);
		/* (MegaMan as carried read again, the next NetBattle's and his card's; a chip up for trade that went into
		 * a folder is no spare: online_visit.h, trade_central.h) */
		if (changed) {
			online_visit_carried_again();
			trade_central_folders_changed();
		}
	} else if (!S.open && S.calm < CALM) ++S.calm;
	/* (Save: the game's own note, as a run's "Run saved"; no character speaks of a save, docs/voice/rules.md) */
	if (S.note && ++S.note > SAVED_NOTE) S.note = 0;
	if (S.saved && square_pet_calm()) {
		S.saved = false;
		S.note = 1;
	}
	return changed;
}

uint32_t square_pet_keys(uint32_t keys) {
	keys = pet_keys(keys);
	/* (R on the PET's menu, which BN6 leaves unused there: the NetPass, its key kept from the game) */
	if (on_menu() && !netpass_screen_shown() && btn_pressed(BTN_R)) {
		netpass_screen_open();
		return 0;
	}
	return keys;
}

void square_pet_draw(void) {
	/* (the menu's one key a player would not guess, named at its band's end) */
	if (S.open && on_menu() && !netpass_screen_shown()) netpass_screen_key_hint();
	if (S.note && !S.open) cinema_note_box(P.core_x, P.core_y, "Run saved at the gate", S.note, SAVED_NOTE);
	netpass_screen_draw();
	if (!S.open) return;
	jukebox_draw();
	library_draw();
}

bool square_pet_calm(void) { return !S.open && S.calm >= CALM && online_square_arrived(); }

void square_pet_saved(void) { S.saved = true; }

void square_pet_leave(void) {
	S.open = S.saved = false;
	S.note = 0;
	pet_leave();
}

bool square_pet_home(bool *traded) {
	uint32_t seed = run.seed;
	int32_t depth = run.depth;
	*traded = false;
	bool known = S.known;
	/* (a game closed in the Square, now continued: the record from its file, where there is one) */
	if (!known) {
		char path[600];
		save_queue_wait();
		save_path(path, sizeof path, RECORD);
		FILE *f = fopen(path, "rb");
		if (!f) return false;
		fclose(f);
		if (!work()) return false;
		if (!save_read_blob(RECORD, PET_CARRY_MAGIC, &S.w->carry, sizeof S.w->carry) || !pet_carry_sound(&S.w->carry)) S.w->carry.count = 0;
	}
	record_remove();
	bool made = false;
	Work *w = S.w;
	unsigned count = w ? w->carry.count : 0;
	if (!count) {
	} else if (dev.no_carry) {
		printf("square pet: nocarry, %u bytes left behind\n", count);
	} else {
		regions_read(w->was);
		if (w->carry.seed != seed || w->carry.depth != depth || w->carry.bits != flag_bytes() || !pet_carry_fits(&w->carry, w->was)) {
			fprintf(stderr, "square pet: the checkpoint is not the one the PET's changes were made on; %u bytes left behind\n", count);
		} else {
			memcpy(w->is, w->was, sizeof w->is);
			pet_carry_apply(&w->carry, w->is);
			regions_write(w->is, w->was);
			printf("square pet: %u bytes of the PET's changes carried home%s\n", count, w->carry.traded ? ", the trade's among them" : "");
			*traded = w->carry.traded != 0;
			made = true;
		}
	}
	work_free();
	return made;
}
