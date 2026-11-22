/* pet_forms.h. The five marked are the run's (run.cross_active), kept
 * with its checkpoint: a CONTINUE lists them again, a new run has none. */
#include "pet_forms.h"

#include "bn6.h"
#include "buttons.h"
#include "crossselect.h"
#include "emu.h"
#include "forms_screen.h"
#include "forms_text.h"
#include "game.h"
#include "pet_look.h"
#include "platform.h"
#include "powers.h"
#include "run.h"
#include "souls.h"

static uint16_t held(void) {
	uint16_t h = 0;
	for (int i = 0; i < FORMS_CROSSES; ++i)
		if (powers_cross_owned(i + 1)) h |= (uint16_t)(1u << i);
	return h;
}

/* the five CROSSSELECT lists, bit i Cross i (CROSSSELECT's order: navi i + 1, powers.h) */
static uint16_t five_now(void) { return forms_five(held(), run.cross_active); }

static void choose(uint16_t five) {
	run_cross_active_set(five);
	crossselect_active(five_now());
}

void pet_forms_update(void) { crossselect_active(five_now()); }

/* BN6's PET menu taking input */
static bool on_menu(void) {
	return emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_GAME && emu_read8(BN6_GAMESTATE) == BN6_SUB_PET &&
		(emu_read8(BN6_PET_MENU_OPEN) & 1) && !emu_read8(BN6_PET_HOLD);
}

uint32_t pet_forms_keys(uint32_t keys) {
	if (forms_shown() || !on_menu() || !btn_pressed(BTN_R)) return keys;
	pet_forms_open();
	return keys & ~(uint32_t)KEY_R;
}

bool pet_forms_holds(void) { return forms_update(); }

bool pet_forms_shown(void) { return forms_shown(); }

void pet_forms_draw(void) {
	/* (R, which BN6's menu leaves unused, named at its band's end as the Square's R:NETPASS) */
	if (!forms_shown() && on_menu()) look_hint(P.core_x + CORE_W - 20 - look_hint_width("R", forms_word(FT_KEY)), P.core_y + 4, "R", forms_word(FT_KEY));
	forms_draw();
}

void pet_forms_leave(void) { forms_shut(); }

/* the run's Souls, BN5's then BN4's, each in its game's numbers */
static int souls_of(FormsSoul *out) {
	int n = 0;
	unsigned bn5 = souls_held_bn5(), bn4 = souls_held_bn4();
	for (int k = 0; k < FORMS_GAME_SOULS; ++k)
		if (bn5 >> k & 1) out[n++] = (FormsSoul){ forms_bn5_soul_names[k], forms_bn5_soul_kinds[k] };
	for (int k = 0; k < FORMS_GAME_SOULS; ++k)
		if (bn4 >> k & 1) out[n++] = (FormsSoul){ forms_bn4_soul_names[k], forms_bn4_soul_kinds[k] };
	return n;
}

void pet_forms_open(void) {
	static FormsRun r;
	r = (FormsRun){ .choose = choose };
	for (int i = 0; i < FORMS_CROSSES; ++i) {
		int navi = i + 1;
		if (!powers_cross_owned(navi)) continue;
		r.crosses[r.n_crosses++] = (FormsCross){ i, powers_cross_name(navi), powers_cross_weakness(navi), powers_cross_strength(navi),
			crossselect_row_art(i, false), crossselect_row_art(i, true), crossselect_row_pal(i, false), crossselect_row_pal(i, true) };
	}
	r.active = five_now();
	r.n_souls = souls_of(r.souls);
	forms_open(&r);
}

/* (Crosses by name, apart by commas, from a mask) */
static void names(FILE *f, const char *key, uint16_t mask) {
	fprintf(f, "%s", key);
	int n = 0;
	for (int i = 0; i < FORMS_CROSSES; ++i)
		if (mask >> i & 1) fprintf(f, "%s%s", n++ ? "," : " ", powers_cross_name(i + 1));
	fprintf(f, "%s\n", n ? "" : " none");
}

void pet_forms_describe(FILE *f) {
	names(f, "crossesfive", five_now());
	/* (CROSSSELECT open in a battle: its rows, as it listed them) */
	if (emu_read8(BN6_GAMESTATE) == BN6_SUB_BATTLE && emu_read8(BN6_CUSTOM_MODE) == BN6_MODE_CROSS) {
		fprintf(f, "crosslist");
		int rows = emu_read8(BN6_CUSTOM_SCREEN + BN6_CROSS_SCREEN_COUNT);
		for (int r = 0; r < rows && r < BN6_CROSS_ROWS; ++r) {
			int cross = emu_read8(BN6_CUSTOM_SCREEN + BN6_CROSS_SCREEN_LIST + (uint32_t)r);
			fprintf(f, "%s%s", r ? "," : " ", cross < BN6_CROSSES ? powers_cross_name(cross + 1) : "?");
		}
		fprintf(f, "%s\n", rows ? "" : " none");
	}
	forms_describe(f);
}
