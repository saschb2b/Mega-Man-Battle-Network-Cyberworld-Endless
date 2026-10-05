/* second_read.h. Each panel's facts from BN6's memory and the run's, into
 * S2 (docs/ROM_DATA.md, the second screen's rows): read after the game's
 * frame, where the draw never reads the game. */
#include "second_read.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "data.h"
#include "director.h"
#include "emu.h"
#include "flags.h"
#include "guardians.h"
#include "navicust.h"
#include "powers.h"
#include "run.h"
#include "second_state.h"
#include "second_text.h"
#include "town.h"

/* The Pack's chips (chip | code << 9) and their copies, up to `most`:
 * outside the folder editor BN6's counts hold the folder's copies too,
 * which the editor takes out while it is open (watched: the folder's four
 * Cannons read 2 and 2 on the map, 0 and 0 in the editor) */
static int pack_now(uint16_t *entry, uint8_t *count, int most, bool editing) {
	static uint16_t all[400];
	static uint8_t copies[400];
	uint16_t folder[BN6_FOLDER_ENTRIES];
	int n = director_pack_now(all, copies, 400), k = 0;
	if (!editing) director_folder_now(folder);
	for (int i = 0; i < n && k < most; ++i) {
		int c = copies[i];
		for (int f = 0; !editing && f < BN6_FOLDER_ENTRIES; ++f) c -= folder[f] == all[i];
		if (c <= 0) continue;
		entry[k] = all[i];
		count[k++] = (uint8_t)c;
	}
	return k;
}

/* an entry of the folder's (0xFF none) as an index, -1 for none */
static int entry_of(int v) { return v < BN6_FOLDER_ENTRIES ? v : -1; }

/* The folder, its limits and its marks; in the editor its cursor, and the
 * pack (read every quarter second: 314 chips' counts) */
void second_read_folder(void) {
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
	if (S2.since % 15 == 0) S2.npack = pack_now(S2.pack, S2.pack_count, (int)(sizeof S2.pack / sizeof *S2.pack), editing);
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
void second_read_navicust(void) {
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

/* The picks sent: on OK BN6 marks their entries in the deck used
 * (0xFFFF) while the screen slides away, 60 frames (watched: its mode
 * 0x08, then 0x14), where the deck no longer holds what was picked */
static bool picks_sent(void) {
	for (int k = 0; k < emu_read8(BN6_CUSTOM_PICKED) && k < 5; ++k)
		if (emu_read16(BN6_BATTLE_DECK + 2u * emu_read8(BN6_CUSTOM_PICKS + (uint32_t)k)) == 0xFFFF) return true;
	return false;
}

/* The Custom screen: its hand, the card it shows (the chip under the
 * cursor, CROSSSELECT's Cross, Beast Out on its emblem, the picks on OK)
 * and the picks in order; once sent, as they were (a garbled chip, "*
 * 65132", stood in for them as the screen slid away) */
static void read_custom(void) {
	if (picks_sent()) return;
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
void second_read_battle(void) {
	S2.custom = emu_read8(BN6_BATTLE_PHASE) == BN6_PHASE_CUSTOM;
	read_fight();
	if (S2.custom) read_custom();
}

/* The PET's home: the run's next step (in the town where R jacks MegaMan
 * in, on a layer its guardian or its exit) and its setup */
void second_read_home(void) {
	if (S2.town) second_home_next(town_info()->landmark_at, S2.home_next, sizeof S2.home_next);
	else second_layer_next(director_guardian_waiting(), S2.home_next, sizeof S2.home_next);
	second_home_setup(S2.home_setup, sizeof S2.home_setup);
}

/* MegaMan's status: his max HP and its base (HPMemory counts into it,
 * programs on top), and the programs on the board, by name */
void second_read_status(void) {
	S2.st_max_hp = emu_read16(BN6_NAVI_MAX_HP);
	S2.st_base_hp = emu_read16(BN6_NAVI_BASE_MAX_HP);
	char *p = S2.st_programs;
	size_t left = sizeof S2.st_programs;
	*p = 0;
	for (int e = 0; e < BN6_NAVICUST_PLACED_MAX; ++e) {
		int v = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)e * 8);
		char name[16];
		if (!v) break;
		program_name(v / 4, name, sizeof name);
		int k = snprintf(p, left, "%s%s", *S2.st_programs ? ", " : "", name);
		if (k < 0 || (size_t)k >= left) break;
		p += k;
		left -= (size_t)k;
	}
}

/* a key item's count, as the PET holds it */
static int key_count(int id) {
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	return items >= BN6_EWRAM && items < BN6_EWRAM_END ? emu_read8(items + (uint32_t)id) : 0;
}

/* the Pack's chips, and the copies of `entry` among them (0xFFFF none):
 * on BN6's trade screen its counts are the Pack's alone, as in the folder
 * editor (watched: 12 given, the folder's two Cannon A out) */
static int pack_of(uint16_t entry, int *copies) {
	static uint16_t pack[400];
	static uint8_t count[400];
	bool trading = emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_TRADER;
	int n = pack_now(pack, count, 400, trading), all = 0;
	*copies = 0;
	for (int i = 0; i < n; ++i) {
		all += count[i];
		if (pack[i] == entry) *copies = count[i];
	}
	return all;
}

/* A shop's chip: its copies in the folder and the pack (read every
 * quarter second: the pack's 314 chips) */
static void shop_chip(uint16_t entry) {
	uint16_t folder[BN6_FOLDER_ENTRIES];
	director_folder_now(folder);
	S2.sh_folder = 0;
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i) S2.sh_folder += folder[i] == entry;
	if (S2.since % 15 == 0 || S2.since < 2) S2.pack_chips = pack_of(entry, &S2.sh_pack);
}

/* A shop's program: the variant of its colour, its shape and name, the
 * copies MegaMan has and those on the board */
static void shop_program(int program, int color) {
	S2.sh_variant = 0;
	memset(&S2.sh_shape, 0, sizeof S2.sh_shape);
	for (int v = program * 4; v < program * 4 + 4 && !S2.sh_variant; ++v)
		if (navicust_shape(v, &S2.sh_shape) && S2.sh_shape.color == color) S2.sh_variant = v;
	program_name(program, S2.sh_name, sizeof S2.sh_name);
	S2.sh_held = S2.sh_variant ? key_count(BN6_PROGRAM_ITEMS + S2.sh_variant) : 0;
	S2.sh_placed = 0;
	for (int e = 0; e < BN6_NAVICUST_PLACED_MAX; ++e) {
		int v = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)e * 8);
		if (!v) break;
		S2.sh_placed += v == S2.sh_variant;
	}
}

/* A shop: the entry under its cursor (its kind, id, code, the shop's
 * currency) and what the run holds of it */
void second_read_shop(void) {
	uint32_t desc = emu_read32(BN6_SHOP_DESC), data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SHOP_DATA);
	int at = emu_read16(BN6_SHOP_SCROLL) + emu_read16(BN6_SHOP_ROW);
	S2.sh_kind = 0;
	*S2.sh_name = 0;
	if (desc >> 24 != 0x08 || data < BN6_EWRAM || data >= BN6_EWRAM_END || at >= (int)emu_read32(desc + 12)) return;
	uint32_t e = data + emu_read32(desc + 8) + BN6_SHOP_ENTRY * (uint32_t)at;
	S2.sh_kind = emu_read8(e);
	S2.sh_id = emu_read16(e + 2);
	S2.sh_code = emu_read8(e + 4);
	S2.sh_currency = (int)emu_read32(desc);
	if (S2.sh_kind == BN6_SHOP_KIND_CHIP) shop_chip((uint16_t)(S2.sh_id | S2.sh_code << 9));
	else if (S2.sh_kind == BN6_SHOP_KIND_PROGRAM) shop_program(S2.sh_id / 4, S2.sh_code);
	else if (S2.sh_kind == BN6_SHOP_KIND_ITEM) {
		item_name(S2.sh_id, S2.sh_name, sizeof S2.sh_name);
		S2.sh_held = key_count(S2.sh_id);
	}
}

/* A trader: the layer's kind, and the chips in the pack it takes from */
void second_read_trader(void) {
	int none;
	S2.trader = director_trader_kind();
	if (S2.since % 15 == 0 || S2.since < 2) S2.pack_chips = pack_of(0xFFFF, &none);
}
