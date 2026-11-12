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
#include "lab_folder.h"
#include "lab_navis.h"
#include "lab_stage.h"
#include "lab_state.h"
#include "analytics.h"
#include "rom.h"
#include "run.h"
#include "runlog.h"
#include "save.h"
#include "xpanels.h"

LabState LS;

/* (the hole first after the plain ones a panel mostly is: plain, cracked, then what changes the fight) */
const uint8_t lab_panel_types[] = {
	BN6_PANEL_PLAIN, BN6_PANEL_CRACKED, BN6_PANEL_HOLE, BN6_PANEL_POISON, BN6_PANEL_HOLY, BN6_PANEL_GRASS, BN6_PANEL_ICE, BN6_PANEL_VOLCANO,
	BN6_PANEL_CONVEYOR_UP, BN6_PANEL_CONVEYOR_DOWN, BN6_PANEL_CONVEYOR_LEFT, BN6_PANEL_CONVEYOR_RIGHT,
	/* (the other games' in BN6's engine, each where its ROM is read: BN5's lava, sea and magnet panel, BN4's Hole) */
	XPANEL_LAVA, XPANEL_SEA, XPANEL_MAGNET, XPANEL_HOLE,
};
const int lab_panel_type_count = (int)(sizeof lab_panel_types);

/* whether a panel type can be set now: BN6's own, or another game's in BN6's engine with its ROM read (xpanels.h) */
static bool lab_panel_offered(int type) { return (type >= BN6_PANEL_HOLE && type < BN6_PANEL_TYPES) || xpanels_has(type); }

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
	/* (the Navis after them: those MegaMan has beaten, open or not yet) */
	for (int n = 1; n <= LAB_NAVI_MOST && LS.nroster < LAB_ROSTER; ++n) {
		int wins = lab_navi_known(n) ? lab_navi_wins(n) : 0;
		if (!lab_navi_known(n) || (!LS.all && !wins)) continue;
		LabRow *r = &LS.roster[LS.nroster++];
		*r = (LabRow){ (uint8_t)n, 1, (uint8_t)lab_navi_versions(n, LS.all), 0, (uint8_t)(wins > 255 ? 255 : wins) };
		while (r->versions && !(r->versions >> r->version & 1)) ++r->version;
	}
}

bool lab_set_navi(void) { return LS.set.nfoes && (LS.set.foes[0].version & LAB_NAVI); }

int lab_rows(void) {
	return LS.page == LAB_PAGE_FOES ? LS.nroster : LS.page == LAB_PAGE_FIELD ? 1 : LS.page == LAB_PAGE_STAGE ? LAB_STAGE_ROWS : LAB_SETS;
}

int lab_foe_at(int col, int line) {
	for (int i = 0; i < LS.set.nfoes; ++i)
		if (LS.set.foes[i].col == col && LS.set.foes[i].row == line) return i;
	return -1;
}

/* ---- the set ---- */

void lab_set_encounter(const LabSet *s, Encounter *e) {
	memset(e, 0, sizeof *e);
	/* (a Navi's battle is his own record's, alone, on the set's stage) */
	if (s->nfoes && (s->foes[0].version & LAB_NAVI)) {
		const LabFoe *f = &s->foes[0];
		if (!lab_navi_encounter(f->family, f->version & ~LAB_NAVI, f->col, f->row, e)) memset(e, 0, sizeof *e);
		e->biome = lab_stage_biome(s->area);
		if (lab_stage_song_slot(s->song)) e->song = lab_stage_song_slot(s->song);
		return;
	}
	e->biome = lab_stage_biome(s->area);
	e->song = lab_stage_song_slot(s->song);
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
	s->area = (uint8_t)lab_stage_area_like(run.biome >= 0 && run.biome < NET_AREAS ? run.biome : 0);
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

/* ---- the sets kept ---- */

/* whether a win of `bust` and `frames` beats a set's best: a higher
 * busting level, or the same in less time */
static bool lab_better(const LabSaved *best, int bust, int frames) {
	return !best->bust || bust > best->bust || (bust == best->bust && frames < best->frames);
}

/* the set in hand into its place in the profile: a set changed since it
 * was kept has no best yet */
static void set_keep(void) {
	LabSaved *k = &profile.lab_sets[profile.lab_slot % LAB_SETS];
	if (!memcmp(&k->set, &LS.set, sizeof LS.set)) return;
	memset(k, 0, sizeof *k);
	k->set = LS.set;
	profile_save();
}

/* the set kept in place `slot` in hand: a place never used a plain field */
static void set_take(int slot) {
	profile.lab_slot = (uint8_t)(slot % LAB_SETS);
	const LabSaved *k = &profile.lab_sets[profile.lab_slot];
	static const LabSet none;
	if (memcmp(&k->set, &none, sizeof none)) LS.set = k->set;
	else set_plain(&LS.set);
	LS.preset = 0;
}

void director_lab_open(bool all) {
	memset(&LS, 0, sizeof LS);
	LS.up = LS.show_due = true;
	LS.all = all;
	lab_stage_make();
	set_take(profile.lab_slot);
	roster_make();
}

/* ---- its keys ---- */

static void turn(int *v, int n, int by) { *v = (*v + n + by) % n; }

/* the row's version, on to the next it can be set in */
static void version_turn(LabRow *r, int by) {
	if (!r->versions) return;
	do r->version = (uint8_t)((r->version + 8 + by) % 8);
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
	} else if (LS.page == LAB_PAGE_SETS) {
		return;
	} else if (row == 0) {
		int i = index_of(lab_stage_area, lab_stage_areas(), lab_stage_area_like(LS.set.area));
		turn(&i, lab_stage_areas(), by);
		LS.set.area = (uint8_t)lab_stage_area(i);
	} else if (row == LAB_ROW_FOLDER) {
		/* (MegaMan's own, or the Lab's: the profile's, not the set's) */
		profile.lab_folder_on = !profile.lab_folder_on;
		profile_save();
		LS.refold = true;
	} else {
		int i = index_of(lab_stage_song, lab_stage_songs(), LS.set.song);
		turn(&i, lab_stage_songs(), by);
		LS.set.song = (uint8_t)lab_stage_song(i);
	}
}

/* A on a panel: FOES sets the family lifted there, or takes off the foe
 * that stands on it; FIELD turns the panel to its next type */
static void panel_take(void) {
	int at = lab_foe_at(LS.col, LS.line);
	if (LS.page == LAB_PAGE_FOES) {
		if (at >= 0) {
			LS.set.foes[at] = LS.set.foes[--LS.set.nfoes];
		} else if (LS.col >= LAB_COLS / 2) {
			/* (a Navi takes the field alone: the foes set make way for him, and none joins him) */
			const LabRow *r = &LS.roster[LS.row[LAB_PAGE_FOES]];
			if (r->navi) LS.set.nfoes = 0;
			else if (lab_set_navi() || LS.set.nfoes >= LAB_FOES) return;
			LS.set.foes[LS.set.nfoes++] = (LabFoe){ r->family, (uint8_t)(r->version | (r->navi ? LAB_NAVI : 0)), (uint8_t)LS.col, (uint8_t)LS.line };
		}
		return;
	}
	uint8_t *p = &LS.set.panels[LS.line * LAB_COLS + LS.col];
	int i = 0;
	while (i < lab_panel_type_count && lab_panel_types[i] != *p) ++i;
	/* (no hole under MegaMan, who stands on column 2, row 2; none of a game whose ROM is not read) */
	bool megaman = LS.col == 1 && LS.line == 1;
	do i = (i + 1) % lab_panel_type_count;
	while ((lab_panel_types[i] == BN6_PANEL_HOLE && megaman) || !lab_panel_offered(lab_panel_types[i]));
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

/* The set shown: BN6's own battle of it begun and held with its field up
 * (lab_battle.h), in place of the one shown before. A set with nobody on
 * it is its field alone: a battle needs a foe, so its record names a
 * Mettaur, and it is held before he comes in. */
static void show(void) {
	Encounter e;
	lab_set_encounter(&LS.set, &e);
	bool bare = !e.nfoes;
	if (bare) {
		memset(&e, 0, sizeof e);
		e.biome = lab_stage_biome(LS.set.area);
		e.song = lab_stage_song_slot(LS.set.song);
		e.foes[e.nfoes++] = (Foe){ FOE_VIRUS, 1, 0, LAB_COLS - 2, 1, enemy_id(0, 1, 0) };
	}
	/* (a panel of a game whose ROM is not read now is fought as plain, the set keeping it) */
	uint8_t panels[LAB_PANELS];
	for (int i = 0; i < LAB_PANELS; ++i) panels[i] = lab_panel_offered(LS.set.panels[i]) ? LS.set.panels[i] : BN6_PANEL_PLAIN;
	lab_battle_panels(panels);
	/* (the background the set names, not one of its area's two by the dice: the stage stays as it is while the set
	 * changes; one of a game whose ROM is not read now, BN6's of the area's place) */
	int backdrop = lab_stage_backdrop(LS.set.area);
	lab_battle_show(&e, backdrop >= 0 ? backdrop : biome_backdrop(lab_stage_biome(LS.set.area), 0), bare);
	LS.shown = LS.set;
	LS.refold = false;
}

/* BATTLE!: the battle held runs on into its fight */
static void battle(void) {
	if (!LS.set.nfoes || !lab_battle_held()) return;
	set_keep();
	runlog_choice(CHOICE_LAB);   /* (the run's choices: a test battle fought) */
	lab_battle_go();
	LS.up = false;
	LS.fighting = true;
}

/* The Lab's folder into BN6's own folder editor: the battle held put back first, the screen away until the editor is
 * left (director_lab_watch) */
static void folder_edit(void) {
	if (!profile.lab_folder_on) return;
	set_keep();
	lab_battle_leave();
	if (!lab_folder_edit()) { show(); return; }
	LS.up = false;
	LS.editing = true;
}

/* A and B where the cursor is */
static void take(uint32_t now) {
	if (now & KEY_B) {
		if (LS.lifted) { LS.lifted = false; LS.at = LAB_AT_LIST; }
		else if (LS.at == LAB_AT_GO) LS.at = lab_rows() ? LAB_AT_LIST : LAB_AT_FIELD;
		else { set_keep(); LS.up = false; }
		return;
	}
	if (!(now & KEY_A)) return;
	if (LS.at == LAB_AT_GO) battle();
	else if (LS.at == LAB_AT_FIELD) panel_take();
	else if (LS.page == LAB_PAGE_STAGE) {
		if (LS.row[LAB_PAGE_STAGE] == LAB_ROW_FOLDER) folder_edit();
	}
	else if (LS.page == LAB_PAGE_SETS) {
		/* (the set in hand kept first, then the one of the row taken) */
		set_keep();
		set_take(LS.row[LAB_PAGE_SETS]);
	}
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
	/* (opened inside the director's update: its set shown from here, between frames) */
	if (LS.show_due) { LS.show_due = false; show(); return; }
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
	/* (left: all put back; the set changed: BN6's field of it in the last one's place) */
	if (!LS.up && !LS.fighting && !LS.editing) lab_battle_leave();
	else if (LS.up && (LS.refold || memcmp(&LS.set, &LS.shown, sizeof LS.set))) show();
}

/* (while BN6 brings a set's field up, its frames run under the screen) */
bool director_lab_shown(void) { return LS.up && !lab_battle_loading(); }
bool director_lab_loading(void) { return (LS.up && lab_battle_loading()) || (LS.editing && lab_folder_opening()); }

/* The keys the game gets once the screen has left: none of those held
 * as it did, until each is let go (the START that began its battle
 * opened BN6's PET) */
uint32_t director_lab_after(uint32_t keys) {
	if (LS.up) return 0;   /* (its field coming up: no key is the game's) */
	held_after &= keys;
	return keys & ~held_after;
}

bool director_lab_editing(void) { return LS.editing && lab_folder_on(); }
uint32_t director_lab_edit_keys(uint32_t keys) { return lab_folder_keys(keys); }

void director_lab_watch(void) {
	/* (BN6's folder editor left, the game put back: the screen again, the battle held with the folder as it is now) */
	if (LS.editing && !lab_folder_on()) {
		LS.editing = false;
		LS.up = true;
		LS.t = 0;
		LS.was = ~0u;
		show();
		return;
	}
	if (!LS.fighting || lab_battle_on()) return;
	/* (its battle over, the game put back: the screen again, with how it ended) */
	LS.fighting = false;
	LS.up = true;
	LS.t = 0;
	LS.was = ~0u;
	LS.at = LAB_AT_GO;
	LS.last = *lab_battle_result();
	/* (a win that beats the set's best is its best) */
	LabSaved *k = &profile.lab_sets[profile.lab_slot % LAB_SETS];
	if (LS.last.end == LAB_WON && lab_better(k, LS.last.bust, LS.last.frames)) {
		k->bust = (uint8_t)LS.last.bust;
		k->frames = (uint16_t)(LS.last.frames > 0xFFFF ? 0xFFFF : LS.last.frames);
		k->hp_lost = (uint16_t)LS.last.hp_lost;
		profile_save();
	}
	show();
}
