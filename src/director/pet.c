/* The PET's entries the run gives a use of its own (docs/PET.md). */
#include "pet.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "buttons.h"
#include "director.h"
#include "emu.h"
#include "gfx.h"
#include "guardians.h"
#include "meta.h"
#include "net.h"
#include "platform.h"
#include "powers.h"
#include "rivals.h"
#include "rom.h"
#include "run.h"
#include "save.h"

/* The SciLab link: pages, the dive and the battle records; a Save under
 * way (the PET closing, then the checkpoint). */
static struct { bool open; int page; bool swallow; int saving; } L;
static int page_count(void);

void pet_link_open(void) { L.open = true; L.page = 0; }
bool pet_link_is_open(void) { return L.open; }

void pet_install(void) {
	/* A on a disabled Comm or Save (flag 0x1706, which a run keeps set):
	 * the cursor into the menu's spare byte, not the buzzer (strb r4,
	 * [r5,#0xF]; b over it, to where the buzzer's branch went) */
	static const uint8_t buzz[4] = { 0x69, 0x20, 0xDF, 0xF6 }, take[4] = { 0xEC, 0x73, 0x09, 0xE0 };
	uint32_t a = BN6_PET_A_DISABLED - 0x08000000u, g = BN6_PET_GREY - 0x08000000u;
	if (!memcmp(R.data + a, buzz, sizeof buzz)) emu_write(BN6_PET_A_DISABLED, take, sizeof take);
	/* and neither greyed: the beq over the grey, always */
	if (R.data[g] == 0x04 && R.data[g + 1] == 0xD0) emu_write8(BN6_PET_GREY + 1, 0xE0);
}

void pet_update(void) {
	int chosen = emu_read8(BN6_PET_MENU + 0xF);
	if (chosen) {
		emu_write8(BN6_PET_MENU + 0xF, 0);
		if (chosen == 6) pet_link_open();
		else if (chosen == 7) L.saving = 1;
	}
	/* a Save: the PET closed, then the checkpoint on the map, as a
	 * layer's arrival takes one (it waits for MegaMan free to move) */
	if (L.saving && !(emu_read8(BN6_PET_MENU + 5) & 1)) {
		L.saving = 0;
		director_save_here();
	}
}

uint32_t pet_keys(uint32_t keys) {
	/* (the key that closed the link held down: not to the game, which
	 * would take it for a press of its own and close the PET too) */
	if (L.swallow) {
		if (keys & (KEY_A | KEY_B)) keys &= ~(uint32_t)(KEY_A | KEY_B);
		else L.swallow = false;
	}
	if (L.saving) {
		/* (B, pressed and let go, till the PET has closed) */
		++L.saving;
		return (L.saving / 6) % 2 ? KEY_B : 0;
	}
	if (!L.open) return keys;
	if (btn_pressed(BTN_B) || btn_pressed(BTN_A)) { L.open = false; L.swallow = true; }
	else if (btn_pressed(BTN_RIGHT)) L.page = (L.page + 1) % page_count();
	else if (btn_pressed(BTN_LEFT)) L.page = (L.page + page_count() - 1) % page_count();
	return 0;
}

static const SDL_Color GOLD = { 255, 230, 90, 255 }, SKY = { 170, 200, 255, 255 }, DIM = { 120, 140, 170, 255 },
	ORANGE = { 255, 170, 40, 255 };

static void row(int x0, int y, const char *label, const char *value, SDL_Color c) {
	text_draw(x0 + 20, y, label, SKY, TEXT_LEFT);
	text_draw(x0 + 88, y, value, c, TEXT_LEFT);
}

/* The dive as the lab sees it: where, what waits, what MegaMan brought
 * (twelve pixels a row, nine rows at most in the panel). */
static void draw_dive(int x0, int y0) {
	char v[64];
	int y = y0 + 27;
	#define ROW(label, value, colour) (row(x0, y, label, value, colour), y += 12)
	/* where: the layer (of the short net's ten), the act, the area */
	if (run.mode == RUN_SHORT) snprintf(v, sizeof v, "%d of %d", run.depth, SHORT_LAYERS + (run.threat >= 10));
	else if (run.depth > CYCLE_LAYERS) snprintf(v, sizeof v, "%d, net V%d", run.depth, (run.depth - 1) / CYCLE_LAYERS + 1);
	else snprintf(v, sizeof v, "%d", run.depth);
	ROW("Layer", v, WHITE);
	const char *area = guardian_area_name(run.side_kind == LAYER_UNDERNET ? BIOME_UNDERNET : run.side_kind == LAYER_SECRET ? BIOME_SECRET : run.biome);
	if (run.side_kind == LAYER_NORMAL && run.biome != BIOME_NEST) snprintf(v, sizeof v, "Act %d, %s", ((run.depth - 1) % CYCLE_LAYERS) / 3 + 1, area);
	else snprintf(v, sizeof v, "%s", area);
	ROW("Area", v, WHITE);
	/* what waits at its end, as MegaMan knows him: by battle data, by word
	 * on the net, or a signal he doesn't know (docs/META.md) */
	int navi = run.side_kind == LAYER_NORMAL ? run.boss_order[run.biome] : 0;
	if (navi) {
		if (guardian_known(navi)) snprintf(v, sizeof v, "%s", guardian(navi)->name);
		else if (director_guardian_heard()) snprintf(v, sizeof v, "%s?", guardian(navi)->name);
		else snprintf(v, sizeof v, "???");
		ROW("Guardian", v, guardian_known(navi) ? WHITE : ORANGE);
	}
	snprintf(v, sizeof v, "%d of 3%s", run.fragments > 3 ? 3 : run.fragments, run.secret_cleared ? ", gate open" : "");
	ROW("ScrtData", v, WHITE);
	/* what was brought */
	ROW("Folder", meta_folder(run.folder)->name, WHITE);
	ROW("Cross", run.cross ? powers_cross_name(run.cross) : "None", run.cross ? ORANGE : WHITE);
	const char *weak = run.cross ? powers_cross_weakness(run.cross) : NULL;
	if (weak) { snprintf(v, sizeof v, "%s, 2x", weak); ROW("Weak to", v, ORANGE); }
	snprintf(v, sizeof v, "%d", run.threat);
	ROW("Threat", v, run.threat ? ORANGE : WHITE);
	static const char *const helpers[3] = { "HP+", "Heals", "Gentle" };
	int k = 0;
	v[0] = 0;
	for (int h = 0; h < 3; ++h)
		if (run.helpers >> h & 1) k += snprintf(v + k, k < (int)sizeof v ? sizeof v - (size_t)k : 0, "%s%s", k ? " " : "", helpers[h]);
	if (k) ROW("Help", v, WHITE);
	#undef ROW
}

/* Every guardian met, in any run, with how their battles went: eight a
 * page, in the game's font (two columns of names and records ran into
 * each other). */
static const uint8_t navis[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18 };
#define RECORDS_A_PAGE 8

static int met_count(void) {
	int met = 0;
	for (unsigned i = 0; i < sizeof navis; ++i) {
		const Rival *r = rival(navis[i]);
		met += r->met || r->megaman_won || r->navi_won;
	}
	return met;
}

static int page_count(void) { return 1 + (met_count() + RECORDS_A_PAGE - 1) / RECORDS_A_PAGE + !met_count(); }

static void draw_records(int x0, int y0, int page) {
	int n = 0, met = met_count();
	for (unsigned i = 0; i < sizeof navis; ++i) {
		const Rival *r = rival(navis[i]);
		if (!r->met && !r->megaman_won && !r->navi_won) continue;
		int at = n++ - page * RECORDS_A_PAGE;
		if (at < 0 || at >= RECORDS_A_PAGE) continue;
		int y = y0 + 27 + at * 12;
		char v[32];
		snprintf(v, sizeof v, "won %d, lost %d", r->megaman_won, r->navi_won);
		text_draw(x0 + 20, y, guardian(navis[i])->name, WHITE, TEXT_LEFT);
		text_draw(x0 + 108, y, v, r->megaman_won >= r->navi_won ? SKY : ORANGE, TEXT_LEFT);
	}
	char v[48];
	if (!met) snprintf(v, sizeof v, "No guardian met yet");
	else snprintf(v, sizeof v, "%d of %d guardians met", met, (int)sizeof navis);
	text_draw(x0 + CORE_W / 2, y0 + 126, v, DIM, TEXT_CENTER);
	if (profile.best_depth) snprintf(v, sizeof v, "Best: layer %d, Nest won %d", profile.best_depth, profile.nest_clears);
	else snprintf(v, sizeof v, "The first dive");
	text_draw(x0 + CORE_W / 2, y0 + 138, v, DIM, TEXT_CENTER);
}

void pet_link_draw(void) {
	if (!L.open) return;
	int x0 = P.core_x, y0 = P.core_y;
	fill_rect(x0 + 8, y0 + 6, CORE_W - 16, CORE_H - 12, rgba(66, 198, 231, 255));
	fill_rect(x0 + 10, y0 + 8, CORE_W - 20, CORE_H - 16, rgba(16, 60, 90, 245));
	text_draw(x0 + CORE_W / 2, y0 + 11, L.page ? "SCILAB LINK: BATTLE RECORDS" : "SCILAB LINK: THE DIVE", GOLD, TEXT_CENTER);
	if (L.page) draw_records(x0, y0, L.page - 1);
	else draw_dive(x0, y0);
	/* (the pages: the PET's orange arrows at the sides) */
	for (int i = 0; i < 4; ++i) {
		fill_rect(x0 + 14 + i, y0 + 76 - i, 1, 2 * i + 1, ORANGE);
		fill_rect(x0 + CORE_W - 15 - i, y0 + 76 - i, 1, 2 * i + 1, ORANGE);
	}
}
