/* second.h. The screen the player is on, from BN6's state once a frame
 * (docs/ROM_DATA.md, the second screen's contexts), and the panel drawn
 * for it in the PET's frame. Until a screen has a panel of its own (issue
 * #72's stories), a layer's screens show the layer's map, as Operate
 * Shooting Star keeps its map up through a battle, and the town and the
 * title are dark. */
#include "second.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "data.h"
#include "director.h"
#include "emu.h"
#include "flags.h"
#include "guardians.h"
#include "guest.h"
#include "navicust.h"
#include "platform.h"
#include "powers.h"
#include "run.h"
#include "second_battle.h"
#include "second_folder.h"
#include "second_frame.h"
#include "second_navicust.h"
#include "second_state.h"

SecondState S2;

/* What the second screen draws: a panel, which several screens may share */
typedef enum { PANEL_DARK, PANEL_MAP, PANEL_FOLDER, PANEL_BATTLE, PANEL_NAVICUST } Panel;
static Panel panel;
static int panel_since;

/* A PET screen by the first byte of its state */
static SecondContext pet_screen(int screen) {
	switch (screen) {
	case BN6_SUBMENU_FOLDERS: return SECOND_FOLDERS;
	case BN6_SUBMENU_EDIT: return SECOND_EDIT;
	case BN6_SUBMENU_NAVICUST: return SECOND_NAVICUST;
	case BN6_SUBMENU_STATUS: return SECOND_STATUS;
	case BN6_SUBMENU_LIBRARY: return SECOND_LIBRARY;
	case BN6_SUBMENU_MAIL: return SECOND_MAIL;
	case BN6_SUBMENU_KEYITEM: return SECOND_KEYITEM;
	case BN6_SUBMENU_SUBCHIP: return SECOND_SUBCHIP;
	case BN6_SUBMENU_COMM: return SECOND_COMM;
	case BN6_SUBMENU_SAVE: return SECOND_SAVE;
	default: return SECOND_OTHER;
	}
}

static SecondContext context_now(void) {
	if (!director_on_layer() && !director_in_town()) return SECOND_DARK;
	if (guest_active()) return SECOND_BATTLE;
	switch (emu_read8(emu_read32(BN6_TOOLKIT))) {
	case BN6_MODE_SUBMENU: return pet_screen(emu_read8(emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU)));
	case BN6_MODE_SHOP: return SECOND_SHOP;
	case BN6_MODE_TRADER: return SECOND_TRADER;
	case BN6_MODE_MAIL: return SECOND_MAIL;
	case BN6_MODE_GAME: break;
	default: return SECOND_OTHER;
	}
	int sub = emu_read8(BN6_GAMESTATE);
	if (sub == BN6_SUB_PET) return SECOND_PET;
	if (sub == BN6_SUB_BATTLE || sub == BN6_SUB_BATTLE_INIT) return SECOND_BATTLE;
	return director_in_town() ? SECOND_TOWN : SECOND_NET;
}

/* The panel for a screen: the folder's in the folders' screens; else the
 * map on a layer, dark in the town */
static Panel panel_for(SecondContext c) {
	if (c == SECOND_FOLDERS || c == SECOND_EDIT) return PANEL_FOLDER;
	if (c == SECOND_NAVICUST) return PANEL_NAVICUST;
	/* (an older net's battle runs in BN5's memory: the map then) */
	if (c == SECOND_BATTLE && !guest_active()) return PANEL_BATTLE;
	return c == SECOND_DARK || S2.town ? PANEL_DARK : PANEL_MAP;
}

/* an entry of the folder's (0xFF none) as an index, -1 for none */
static int entry_of(int v) { return v < BN6_FOLDER_ENTRIES ? v : -1; }

/* The folder, its limits and its marks; in the editor its cursor, and the
 * pack (read every quarter second: 314 chips' counts) */
static void read_folder(void) {
	director_folder_now(S2.folder);
	S2.mega_level = emu_read8(BN6_NAVI_MEGA_LEVEL);
	S2.giga_level = emu_read8(BN6_NAVI_GIGA_LEVEL);
	S2.reg = entry_of(emu_read8(BN6_NAVI_FOLDER1_REG));
	S2.tag[0] = entry_of(emu_read8(BN6_NAVI_FOLDER1_TAG));
	S2.tag[1] = entry_of(emu_read8(BN6_NAVI_FOLDER1_TAG2));
	uint32_t edit = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU);
	bool editing = S2.context == SECOND_EDIT;
	S2.pack_side = editing && emu_read8(edit + BN6_EDIT_SIDE) == BN6_EDIT_PACK;
	S2.entry = editing && !S2.pack_side ? entry_of(emu_read8(edit + BN6_EDIT_SCROLL) + emu_read8(edit + BN6_EDIT_ROW)) : -1;
	S2.pack_entry = S2.pack_side ? emu_read8(edit + BN6_EDIT_PACK_SCROLL) + emu_read8(edit + BN6_EDIT_PACK_ROW) : -1;
	if (S2.since % 15 == 0) S2.npack = director_pack_now(S2.pack, S2.pack_count, (int)(sizeof S2.pack / sizeof *S2.pack));
}

/* What the NaviCustomizer's cursor is on in `mode`, its state at `m`: a
 * program under the board's cursor, or the list's entry */
static void navicust_cursor(uint32_t m, int mode) {
	S2.nc_on = NC_NONE;
	S2.nc_variant = S2.nc_copies = 0;
	if (mode == BN6_NCMENU_BOARD || mode == BN6_NCMENU_PLACED) {
		int x = emu_read16(m + BN6_NCMENU_X), y = emu_read16(m + BN6_NCMENU_Y);
		int cell = x < NAVICUST_GRID && y < NAVICUST_GRID ? emu_read8(BN6_NAVICUST_GRID + (uint32_t)(y * NAVICUST_GRID + x)) : 0;
		int v = cell ? emu_read16(BN6_NAVICUST_PLACED + (uint32_t)(cell - 1) * 8) : 0;
		if (v > 0 && v < 47 * 4) {
			S2.nc_on = NC_PLACED;
			S2.nc_variant = v;
		}
	} else if (mode == BN6_NCMENU_LIST) {
		uint32_t e = BN6_NCMENU_ENTRIES + 4u * (uint32_t)(emu_read16(m + BN6_NCMENU_SCROLL) + emu_read16(m + BN6_NCMENU_ROW));
		int item = emu_read16(e);
		if (item == BN6_NCMENU_RUN) S2.nc_on = NC_RUN;
		else if (item > BN6_PROGRAM_ITEMS && item < BN6_PROGRAM_ITEMS + 47 * 4) {
			S2.nc_on = NC_LIST;
			S2.nc_variant = item - BN6_PROGRAM_ITEMS;
			S2.nc_copies = emu_read16(e + 2);
		}
	}
}

/* The NaviCustomizer: the program its cursor is on (in the list, on the
 * board, or held: the one taken up), its name and its shape as the PET
 * has it, whether it fits and turns; and why RUN would bug as the board
 * stands (read twice a second) */
static void read_navicust(void) {
	static int held;
	uint32_t m = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU);
	int mode = emu_read8(m + BN6_NCMENU_MODE);
	if (mode == BN6_NCMENU_TAKE || mode == BN6_NCMENU_HELD || mode == BN6_NCMENU_MOVE || mode == BN6_NCMENU_MOVED) {
		S2.nc_on = held ? NC_HELD : NC_NONE;
		S2.nc_variant = held;
	} else {
		navicust_cursor(m, mode);
		held = S2.nc_variant;
	}
	S2.nc_nboard = 0;
	for (int e = 0; S2.nc_on == NC_RUN && e < BN6_NAVICUST_PLACED_MAX && S2.nc_nboard < 8; ++e) {
		int p = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)e * 8);
		NaviShape s;
		if (!p) break;
		program_name(p / 4, S2.nc_board[S2.nc_nboard].name, sizeof S2.nc_board[0].name);
		S2.nc_board[S2.nc_nboard++].color = p < 47 * 4 && navicust_shape(p, &s) ? s.color : 0;
	}
	int v = S2.nc_variant;
	*S2.nc_name = 0;
	if (v) {
		program_name(v / 4, S2.nc_name, sizeof S2.nc_name);
		if (!navicust_shape_as(v, flag_get(BN6_FLAG_COMPRESSED + v), &S2.nc_shape)) memset(&S2.nc_shape, 0, sizeof S2.nc_shape);
		S2.nc_turns = S2.nc_shape.color && navicust_spins() >> (S2.nc_shape.color - 1) & 1;
		S2.nc_fits = S2.nc_on == NC_PLACED || director_board_fits(v);
	}
	if (S2.since % 30 == 0 || S2.since < 2) {
		/* (its first sentence: the rules' line says the rest) */
		const char *bug = director_board_bug();
		snprintf(S2.nc_bug, sizeof S2.nc_bug, "%s", bug ? bug : "");
		size_t end = strcspn(S2.nc_bug, ".!?");
		if (S2.nc_bug[end]) S2.nc_bug[end + 1] = 0;
	}
}

/* The Cross under CROSSSELECT's cursor at `row`: the Crosses MegaMan has,
 * in Gregar's order, less the one he is in (`form`); 0 for none */
static int cross_at(int row, int form) {
	for (int navi = 1; navi <= 5; ++navi) {
		if (!powers_cross_owned(navi) || navi == form) continue;
		if (row-- == 0) return navi;
	}
	return 0;
}

/* The Custom screen: its hand, the card it shows (the chip under the
 * cursor, CROSSSELECT's Cross, Beast Out on its emblem, the picks on OK)
 * and the picks in order */
static void read_custom(void) {
	S2.nhand = emu_read8(BN6_CUSTOM_HAND);
	if (S2.nhand > 10) S2.nhand = 10;
	for (int i = 0; i < S2.nhand; ++i) S2.hand[i] = emu_read16(BN6_BATTLE_DECK + 2u * (uint32_t)i);
	int mode = emu_read8(BN6_CUSTOM_MODE);
	bool crossing = mode == BN6_MODE_CROSS_OPEN || mode == BN6_MODE_CROSS || mode == BN6_MODE_CROSS_SHUT || mode == BN6_MODE_CROSS_TAKEN;
	S2.cross_under = crossing ? cross_at(emu_read8(BN6_CUSTOM_CROSS_ROW), S2.form) : 0;
	S2.cursor = emu_read8(BN6_CUSTOM_CURSOR);
	S2.card = S2.cross_under ? CARD_CROSS : S2.cursor == BN6_CUSTOM_EMBLEM ? CARD_BEAST : S2.cursor < S2.nhand ? CARD_CHIP : CARD_PICKS;
	S2.npicks = 0;
	for (int k = 0; k < emu_read8(BN6_CUSTOM_PICKED) && k < 5; ++k) {
		int slot = emu_read8(BN6_CUSTOM_PICKS + (uint32_t)k);
		if (slot < S2.nhand) S2.picks[S2.npicks++] = slot;
	}
}

/* The fight: MegaMan's Cross or Beast Out and his emotion, the chips he
 * holds after OK, the enemies in play, and the guardian's */
static void read_fight(void) {
	S2.form = emu_read8(BN6_BATTLE_FORM);
	S2.beast_turns = emu_read8(BN6_BATTLE_BEAST_TURNS);
	S2.beast = flag_get(BN6_FLAG_BEAST_OUT);
	S2.synchro = emu_read8(BN6_BATTLE_MOOD) == BN6_MOOD_SYNCHRO;
	S2.queue_at = emu_read8(BN6_BATTLE_HAND + BN6_HAND_AT);
	for (S2.nqueue = 0; S2.nqueue < BN6_HAND_MAX; ++S2.nqueue) {
		uint16_t id = emu_read16(BN6_BATTLE_HAND + BN6_HAND_CHIPS + 2u * (uint32_t)S2.nqueue);
		if (id == 0xFFFF) break;
		S2.queue[S2.nqueue] = id;
		S2.queue_power[S2.nqueue] = emu_read16(BN6_BATTLE_HAND + BN6_HAND_POWERS + 2u * (uint32_t)S2.nqueue);
		S2.queue_bonus[S2.nqueue] = emu_read16(BN6_BATTLE_HAND + BN6_HAND_BONUS + 2u * (uint32_t)S2.nqueue);
	}
	S2.nfoes = 0;
	for (uint32_t i = 0; i < BN6_T1_COUNT && S2.nfoes < (int)(sizeof S2.foe / sizeof *S2.foe); ++i) {
		uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		/* (an enemy with HP: a guardian's helpers have none, "TestVirs") */
		if (!(emu_read8(o + BN6_T1_IN_PLAY) & 1) || emu_read8(o + BN6_T1_ALLIANCE) != 1 || !emu_read16(o + BN6_T1_HP)) continue;
		S2.foe[S2.nfoes].name = emu_read16(o + BN6_T1_NAME_ID);
		S2.foe[S2.nfoes].hp = emu_read16(o + BN6_T1_HP);
		S2.foe[S2.nfoes].element = emu_read8(o + BN6_T1_ELEMENT);
		S2.foe[S2.nfoes++].max_hp = emu_read16(o + BN6_T1_MAX_HP);
	}
	/* (what MegaMan told at the arena, where he knows the guardian) */
	S2.guardian = director_guardian_battle();
	S2.tip = S2.guardian && guardian_known(S2.guardian) ? guardian_tip(S2.guardian) : NULL;
}

/* The battle: the Custom screen while it is open, and the fight */
static void read_battle(void) {
	S2.custom = emu_read8(BN6_BATTLE_PHASE) == BN6_PHASE_CUSTOM;
	read_fight();
	if (S2.custom) read_custom();
}

void second_update(void) {
	SecondContext c = context_now();
	if (c != S2.context) S2.since = 0;
	else if (S2.since < 1 << 20) ++S2.since;
	S2.context = c;
	S2.town = director_in_town();
	director_megaman_hp(&S2.hp, &S2.max_hp);
	S2.zenny = emu_read32(BN6_ZENNY);
	S2.bugfrags = emu_read32(BN6_BUGFRAGS);
	S2.depth = run.depth;
	snprintf(S2.area, sizeof S2.area, "%s", S2.town ? "" : guardian_area_in_text(run.biome, run.side_kind));
	Panel p = panel_for(c);
	if (p == PANEL_FOLDER) read_folder();
	if (p == PANEL_BATTLE) read_battle();
	if (p == PANEL_NAVICUST) read_navicust();
	if (p != panel) panel_since = 0;
	else if (panel_since < 1 << 20) ++panel_since;
	panel = p;
	/* (a new panel at once, its title sliding in over two draws) */
	if (panel_since < 2) platform_second_screen_soon();
}

/* what the last picture showed: the panel and the state it drew from */
static Panel drawn_panel;
static SecondState drawn;

bool second_changed(void) {
	if (panel == PANEL_MAP || panel != drawn_panel || panel_since < 2) return true;
	SecondState now = S2;
	now.since = drawn.since;
	return memcmp(&now, &drawn, sizeof now) != 0;
}

bool second_draw(int w, int h) {
	drawn_panel = panel;
	drawn = S2;
	if (panel == PANEL_DARK) return false;
	static const char *const title[] = { [PANEL_MAP] = "NET", [PANEL_FOLDER] = "FOLDER", [PANEL_BATTLE] = "BATTLE", [PANEL_NAVICUST] = "NAVICUST" };
	const char *t = S2.context == SECOND_EDIT ? "FOLDER EDIT" : title[panel];
	SDL_Rect body = second_frame(w, h, t, panel_since == 0 ? 24 : panel_since == 1 ? 8 : 0);
	if (panel == PANEL_FOLDER) second_folder_draw(body);
	else if (panel == PANEL_BATTLE) second_battle_draw(body);
	else if (panel == PANEL_NAVICUST) second_navicust_draw(body);
	else director_draw_layer_map(body.x, body.y, body.w, body.h);
	return true;
}
