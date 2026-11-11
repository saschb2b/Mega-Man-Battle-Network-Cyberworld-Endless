/* The Battle Lab's builder's course (lab.h): the families it lists, its
 * keys, the set as a battle, and the battle's start and end. */
#include "lab.h"

#include <string.h>

#include "battle_data.h"
#include "bn6.h"
#include "bn6_panels.h"
#include "data.h"
#include "director.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "jukebox_text.h"
#include "lab_state.h"
#include "rom.h"
#include "run.h"

LabState LS;

/* (the hole first after the plain ones a panel mostly is: plain, cracked, then what changes the fight) */
const uint8_t lab_panel_types[] = {
	BN6_PANEL_PLAIN, BN6_PANEL_CRACKED, BN6_PANEL_HOLE, BN6_PANEL_POISON, BN6_PANEL_HOLY, BN6_PANEL_GRASS, BN6_PANEL_ICE, BN6_PANEL_VOLCANO,
	BN6_PANEL_CONVEYOR_UP, BN6_PANEL_CONVEYOR_DOWN, BN6_PANEL_CONVEYOR_LEFT, BN6_PANEL_CONVEYOR_RIGHT,
};
const int lab_panel_type_count = (int)(sizeof lab_panel_types);

/* ---- what it lists ---- */

/* the versions of a family the engine holds, a bit each */
static unsigned versions_held(int family) {
	unsigned m = 0;
	for (int v = 0; v < DATA_VERSIONS; ++v)
		if (enemy_id(0, family, v) >= 0) m |= 1u << v;
	return m;
}

static void roster_make(void) {
	LS.nroster = 0;
	for (int f = 1; f < DATA_FAMILIES && LS.nroster < LAB_ROSTER; ++f) {
		unsigned held = versions_held(f);
		int count = battle_data_count(f);
		if (!held || (!LS.all && !count)) continue;
		LabRow *r = &LS.roster[LS.nroster++];
		r->family = (uint8_t)f;
		r->count = (uint8_t)count;
		r->versions = (uint8_t)(LS.all ? held : battle_data_complete(f) ? held & battle_data_versions(f) : 0);
		r->version = 0;
		while (r->versions && !(r->versions >> r->version & 1)) ++r->version;
	}
}

static int lab_areas(void) { return NET_AREAS; }
static int lab_area(int i) { return i >= 0 && i < NET_AREAS ? i : 0; }

static int lab_songs(void) {
	int n = 1;   /* (the battle's own first) */
	for (int i = 0; i < jukebox_song_count; ++i) n += jukebox_songs[i].game == 0;
	return n;
}

static int lab_song(int i) {
	for (int k = 0; i > 0 && k < jukebox_song_count; ++k)
		if (jukebox_songs[k].game == 0 && --i == 0) return jukebox_songs[k].song;
	return 0;
}

int lab_rows(void) { return LS.page == LAB_PAGE_FOES ? LS.nroster : LS.page == LAB_PAGE_FIELD ? 1 : LAB_STAGE_ROWS; }

int lab_foe_at(int col, int line) {
	for (int i = 0; i < LS.set.nfoes; ++i)
		if (LS.set.foes[i].col == col && LS.set.foes[i].row == line) return i;
	return -1;
}

/* ---- the set ---- */

void lab_set_encounter(const LabSet *s, Encounter *e) {
	memset(e, 0, sizeof *e);
	e->biome = s->area;
	e->song = s->song;
	for (int i = 0; i < s->nfoes && i < LAB_FOES; ++i) {
		const LabFoe *f = &s->foes[i];
		int id = enemy_id(0, f->family, f->version);
		if (id < 0) continue;
		e->foes[e->nfoes++] = (Foe){ FOE_VIRUS, f->family, f->version, f->col, f->row, id };
	}
}

static void set_plain(LabSet *s) {
	memset(s, 0, sizeof *s);
	memset(s->panels, BN6_PANEL_PLAIN, sizeof s->panels);
	s->area = (uint8_t)(run.biome >= 0 && run.biome < NET_AREAS ? run.biome : 0);
}

/* BN6's field `field` as the set's panels: a panel of no type of BN6's plain */
static void preset_take(int field) {
	for (int row = 0; row < LAB_ROWS; ++row) {
		uint32_t w = emu_read32(BN6_FIELDS + (uint32_t)field * BN6_FIELD_SIZE + 4u * (uint32_t)row);
		for (int col = 0; col < LAB_COLS; ++col) {
			int t = (int)(w >> (4 * col) & 15);
			LS.set.panels[row * LAB_COLS + col] = (uint8_t)(t >= BN6_PANEL_HOLE && t < BN6_PANEL_TYPES ? t : BN6_PANEL_PLAIN);
		}
	}
	/* (MegaMan stands on column 2, row 2: no hole under him) */
	if (LS.set.panels[LAB_COLS + 1] == BN6_PANEL_HOLE) LS.set.panels[LAB_COLS + 1] = BN6_PANEL_PLAIN;
}

void director_lab_open(bool all) {
	LabSet keep = LS.set;
	bool had = LS.nroster > 0;
	memset(&LS, 0, sizeof LS);
	LS.up = true;
	LS.all = all;
	/* (the last set fought, this session's; its foes those still listed) */
	if (had) LS.set = keep;
	else set_plain(&LS.set);
	roster_make();
}

/* ---- its keys ---- */

static void turn(int *v, int n, int by) { *v = (*v + n + by) % n; }

/* the row's version, on to the next it can be set in */
static void version_turn(LabRow *r, int by) {
	if (!r->versions) return;
	do r->version = (uint8_t)((r->version + DATA_VERSIONS + by) % DATA_VERSIONS);
	while (!(r->versions >> r->version & 1));
}

static int index_of(int (*at)(int), int n, int value) {
	for (int i = 0; i < n; ++i)
		if (at(i) == value) return i;
	return 0;
}

/* Left and Right on a row: a family's version, the field to start from, the background, the song */
static void row_turn(int by) {
	int row = LS.row[LS.page];
	if (LS.page == LAB_PAGE_FOES) {
		if (row < LS.nroster) version_turn(&LS.roster[row], by);
	} else if (LS.page == LAB_PAGE_FIELD) {
		turn(&LS.preset, BN6_FIELD_COUNT - 1, by);
		preset_take(LS.preset);
	} else if (row == 0) {
		int i = index_of(lab_area, lab_areas(), LS.set.area);
		turn(&i, lab_areas(), by);
		LS.set.area = (uint8_t)lab_area(i);
	} else {
		int i = index_of(lab_song, lab_songs(), LS.set.song);
		turn(&i, lab_songs(), by);
		LS.set.song = (uint8_t)lab_song(i);
	}
}

/* A on a panel: FOES sets the family lifted there, or takes off the foe
 * that stands on it; FIELD turns the panel to its next type */
static void panel_take(void) {
	int at = lab_foe_at(LS.col, LS.line);
	if (LS.page == LAB_PAGE_FOES) {
		if (at >= 0) {
			LS.set.foes[at] = LS.set.foes[--LS.set.nfoes];
		} else if (LS.set.nfoes < LAB_FOES && LS.col >= LAB_COLS / 2) {
			const LabRow *r = &LS.roster[LS.row[LAB_PAGE_FOES]];
			LS.set.foes[LS.set.nfoes++] = (LabFoe){ r->family, r->version, (uint8_t)LS.col, (uint8_t)LS.line };
		}
		return;
	}
	uint8_t *p = &LS.set.panels[LS.line * LAB_COLS + LS.col];
	int i = 0;
	while (i < lab_panel_type_count && lab_panel_types[i] != *p) ++i;
	/* (no hole under MegaMan, who stands on column 2, row 2) */
	i = (i + 1) % lab_panel_type_count;
	if (lab_panel_types[i] == BN6_PANEL_HOLE && LS.col == 1 && LS.line == 1) i = (i + 1) % lab_panel_type_count;
	*p = lab_panel_types[i];
}

/* the cursor over the field's panels: FOES keeps to the foes' side */
static void field_move(uint32_t now) {
	int left = LS.page == LAB_PAGE_FOES ? LAB_COLS / 2 : 0;
	if ((now & KEY_LEFT) && LS.col > left) --LS.col;
	if ((now & KEY_RIGHT) && LS.col < LAB_COLS - 1) ++LS.col;
	if ((now & KEY_UP) && LS.line > 0) --LS.line;
	if (now & KEY_DOWN) {
		if (LS.line < LAB_ROWS - 1) ++LS.line;
		else if (LS.page == LAB_PAGE_FIELD) LS.at = LAB_AT_LIST;
	}
}

static void list_move(uint32_t now) {
	int rows = lab_rows(), *row = &LS.row[LS.page];
	if (now & KEY_UP) {
		if (*row > 0) --*row;
		else if (LS.page == LAB_PAGE_FIELD) LS.at = LAB_AT_FIELD;
	}
	if (now & KEY_DOWN) {
		if (*row < rows - 1) ++*row;
		else LS.at = LAB_AT_GO;
	}
	if (now & KEY_LEFT) row_turn(-1);
	if (now & KEY_RIGHT) row_turn(1);
	if (LS.page == LAB_PAGE_FOES) {
		if (*row < LS.top) LS.top = *row;
		if (*row >= LS.top + LAB_ROWS_SHOWN) LS.top = *row - LAB_ROWS_SHOWN + 1;
	}
}

static void page_turn(int by) {
	turn(&LS.page, LAB_PAGES, by);
	LS.lifted = false;
	LS.at = LS.page == LAB_PAGE_FIELD ? LAB_AT_FIELD : lab_rows() ? LAB_AT_LIST : LAB_AT_GO;
	if (LS.page == LAB_PAGE_FIELD) LS.col = LS.line = 1;
}

static void battle(void) {
	if (!LS.set.nfoes) return;
	Encounter e;
	lab_set_encounter(&LS.set, &e);
	lab_battle_panels(LS.set.panels);
	lab_battle_ask(&e, -1);
	LS.up = false;
	LS.fighting = true;
}

/* A and B where the cursor is */
static void take(uint32_t now) {
	if (now & KEY_B) {
		if (LS.lifted) { LS.lifted = false; LS.at = LAB_AT_LIST; }
		else if (LS.at == LAB_AT_GO) LS.at = lab_rows() ? LAB_AT_LIST : LAB_AT_FIELD;
		else LS.up = false;
		return;
	}
	if (!(now & KEY_A)) return;
	if (LS.at == LAB_AT_GO) battle();
	else if (LS.at == LAB_AT_FIELD) panel_take();
	else if (LS.page == LAB_PAGE_FOES && LS.row[LAB_PAGE_FOES] < LS.nroster && LS.roster[LS.row[LAB_PAGE_FOES]].versions) {
		LS.lifted = true;
		LS.at = LAB_AT_FIELD;
		if (LS.col < LAB_COLS / 2) { LS.col = LAB_COLS - 2; LS.line = 1; }
	}
}

static uint32_t held_after;   /* the keys held as the screen left, each until let go */

void director_lab_keys(uint32_t keys) {
	if (!LS.up) return;
	held_after = ~0u;
	++LS.t;
	uint32_t now = keys & ~LS.was;
	LS.was = keys;
	if (LS.t < LAB_LOCK) return;
	if ((now & (KEY_L | KEY_R)) && !LS.lifted) { page_turn(now & KEY_R ? 1 : -1); return; }
	if (now & KEY_START) { battle(); return; }
	if (LS.at == LAB_AT_FIELD) field_move(now);
	else if (LS.at == LAB_AT_LIST) list_move(now);
	else if (now & KEY_UP) LS.at = lab_rows() ? LAB_AT_LIST : LAB_AT_FIELD;
	take(now);
}

bool director_lab_shown(void) { return LS.up; }

/* The keys the game gets once the screen has left: none of those held
 * as it did, until each is let go (the START that began its battle
 * opened BN6's PET) */
uint32_t director_lab_after(uint32_t keys) {
	held_after &= keys;
	return keys & ~held_after;
}

void director_lab_watch(void) {
	if (!LS.fighting || lab_battle_on()) return;
	/* (its battle over, the game put back: the screen again, with how it ended) */
	LS.fighting = false;
	LS.up = true;
	LS.t = 0;
	LS.was = ~0u;
	LS.at = LAB_AT_GO;
	LS.last = *lab_battle_result();
}
