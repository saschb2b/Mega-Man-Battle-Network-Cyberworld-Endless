/* looks_due.h. A palette got is said in BN6's chat box at the first calm
 * moment on the map: right after the battle or the find that earned it,
 * or at a run's start for one earned as the last ended (the summary named
 * it) or held from before palettes (looks_routes.h). */
#include "looks_due.h"

#include "bn6.h"
#include "boss.h"
#include "cinema.h"
#include "director_hold.h"
#include "director_state.h"
#include "emu.h"
#include "flags.h"
#include "looks.h"
#include "looks_routes.h"
#include "looks_words.h"
#include "net.h"
#include "order_looks.h"
#include "run.h"
#include "save.h"
#include "scripts.h"
#include "talk.h"

/* (frames of calm on the map before MegaMan speaks: a map entered or a
 * chat box closed settles first) */
#define CALM_FRAMES 30

static struct {
	int calm;
	unsigned found;   /* palettes BN6's own box named as ColrData */
} L;

/* AsterLand's sales (order_looks.h): the clerk's script set the flag as it
 * took the zenny and said "MegaMan got:", so the palette is told there */
static void shop_watch(void) {
	for (int k = 0; k < LOOKS_SHOP_COUNT; ++k)
		if (flag_get(LOOKS_SOLD_FLAG + k) && looks_grant(LOOK_CENTRAL + k)) looks_told(1u << (LOOK_CENTRAL + k));
}

/* the milestones the profile keeps no record of: the Undernet as a layer
 * of it begins (the endless net's fifth act, or the dark warp's), the
 * tournament as its final falls */
static void moments(void) {
	bool undernet = run.side_kind == LAYER_UNDERNET || (run.side_kind == LAYER_NORMAL && run.biome == BIOME_UNDERNET);
	if (D.active && !D.town && undernet) looks_milestone(LOOK_UNDERNET);
	if (boss_bracket_won()) looks_milestone(LOOK_TOURNAMENT);
}

void looks_due_frame(void) {
	looks_milestones();
	moments();
	shop_watch();
	unsigned untold = looks_untold();
	bool calm = on_map() && hold_save_ok() && !emu_read8(BN6_CHATBOX) && cinema_input_mode() == CINEMA_FREE;
	if (!untold || !calm) {
		L.calm = 0;
		return;
	}
	if (++L.calm < CALM_FRAMES) return;
	if (talk_start(looks_got_words(untold, L.found), FACE_MEGAMAN)) {
		looks_told(untold);
		L.found = 0;
	}
}

void looks_item_given(int item, int palette) {
	if (item != SCRIPTS_COLOR_DATA || palette < 0) return;
	looks_find_taken(palette);
	L.found |= 1u << palette;
	/* (ColrData is the find's carrier, not kept: KeyItem lists none) */
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	if (items >= BN6_EWRAM && items < BN6_EWRAM_END) emu_write8(items + SCRIPTS_COLOR_DATA, 0);
}

void looks_describe(FILE *f) {
	unsigned owned = 0;
	for (int p = 0; p < LOOK_PALETTES; ++p)
		if (looks_owned(p)) owned |= 1u << p;
	fprintf(f, "looksowned %u\nlookstold %u\nlookfind %d\nlookdepth %d\n", owned, (unsigned)profile.looks_told, looks_find_palette(), looks_find_depth());
}
