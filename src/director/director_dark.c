/* DarkChips on the run (docs/META.md): the flame of darkness on each act's
 * middle layer, BN5's or BN6's own, the chip taken, its price in max HP
 * after a battle its dark power ran in, and BN6's chips in the Pack. */
#include "director_dark.h"

#include <stdio.h>

#include "cinema.h"
#include "dark_words.h"
#include "darkbn6.h"
#include "darkchips.h"
#include "data.h"
#include "debug.h"
#include "director_folder.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "gamecall.h"
#include "guest.h"
#include "save.h"
#include "talk.h"
#include "xnavi.h"

static bool dark_price_told;    /* MegaMan has said all of a DarkChip's price (a new run or a CONTINUE forgets) */

/* A flame of BN6's own (docs/META.md, BN6's own DarkChips): BN6's blue
 * flame turned purple, holding kind `k`, one of the four BN6's battles
 * play, its words naming what its dark power does, the BugFrag it burns and
 * its base chip, from the ROM. */
static void dark6_flame_setup(int k) {
	static char chip[20], base[20], does[100];
	static ScriptsDark6 words;
	int id = dark_bn6_id(k), from = darkbn6_base(id);
	layer_objs_dark_flame = id && from ? xnavi_bn6_flame() : -1;
	if (layer_objs_dark_flame < 0) return;
	ChipInfo ci;
	chip_info(id, &ci);
	snprintf(chip, sizeof chip, "%s", ci.name);
	chip_info(from, &ci);
	snprintf(base, sizeof base, "%s", ci.name);
	darkbn6_does(id, does, sizeof does);
	words = (ScriptsDark6){ chip, base, does, !(profile.dark6_taught & DARK6_FLAME_TAUGHT) };
	layer_objs_dark6 = &words;
	layer_objs_dark_chip = chip;
	if (emu_debug_on()) fprintf(stderr, "dark: BN6's flame of darkness with %s (%s without a BugFrag), list 7 %d\n", chip, base, layer_objs_dark_flame);
}

/* A flame of darkness on the middle layer of each act (docs/META.md,
 * DarkChips in BN5 territory and BN6's own): a DarkChip the run lacks, from
 * the run's seed and the act, in its last bystander's place
 * (layer_objs_dark_flame): BN5's flame where the act's battles are BN5's,
 * BN6's own where they are BN6's; none where the run holds every one. A
 * CONTINUE on its layer finds the flame it was made with (dark_flame_of). */
void dark_flame_setup(void) {
	layer_objs_dark_flame = -1;
	layer_objs_dark6 = NULL;
	D.dark_kind = -1;
	D.dark_given = false;
	if (run.side_kind != LAYER_NORMAL || layer_in_act(run.depth) != 1) return;
	D.dark_kind = dark_flame_of(run.seed, run.depth, run.side_kind, !encounter_guest);
	if (D.dark_kind < 0) return;
	if (!encounter_guest) { dark6_flame_setup(D.dark_kind); return; }
	if (!*guest_dark_name(D.dark_kind)) return;
	layer_objs_dark_flame = xnavi_object(XOBJ_DARK_FLAME);
	static char chip[16];
	snprintf(chip, sizeof chip, "%s", guest_dark_name(D.dark_kind));
	layer_objs_dark_chip = chip;
	layer_objs_dark_first = !profile.dark_taught;
	layer_objs_dark_ours = dark_bn6_id(D.dark_kind) != 0;
	if (emu_debug_on()) fprintf(stderr, "dark: a flame of darkness with %s, list 7 %d\n", chip, layer_objs_dark_flame);
}

/* ... and taken: the run holds it (its flag, from the flame's Yes), and
 * BN6's chip of its kind goes to the Pack (dark_pack: a BN6 flame's words
 * say where, and how it goes in the folder); one held already was taken
 * before a CONTINUE on its layer */
void dark_flame_watch(void) {
	if (D.dark_given || D.dark_kind < 0 || layer_objs_dark_flame < 0 || !flag_get(LAYER_DARK_TAKEN_FLAG)) return;
	D.dark_given = true;
	if (dark_count(D.dark_kind)) return;
	dark_give(D.dark_kind);
	D.dark_pack_due = true;
	if (layer_objs_dark6) profile.dark6_taught |= DARK6_FLAME_TAUGHT;
	else profile.dark_taught = 1;
	profile_save();
}

/* A battle a DarkChip's dark power ran in, in either net: one HPMemory of
 * max HP, for the rest of the run (the owner's price, docs/META.md), from
 * the base the game counts HPMemory into, so the NaviCust's next RUN keeps
 * it; MegaMan says so, the whole of it the first time in a session
 * (dark_price_told), and after a profile's first such battle of BN6's own
 * (`bn6`) what BN6's darkness took besides: the BugFrags it burnt, the bug
 * gone with the battle. */
void dark_price(bool bn6) {
	int base = emu_read16(BN6_NAVI_BASE_MAX_HP), max = emu_read16(BN6_NAVI_MAX_HP), hp = emu_read16(BN6_NAVI_HP), v[3];
	dark_price_hp(base, max, hp, v);
	uint16_t w[3] = { (uint16_t)v[0], (uint16_t)v[1], (uint16_t)v[2] };
	emu_write(BN6_NAVI_BASE_MAX_HP, &w[0], 2);
	emu_write(BN6_NAVI_HP, &w[2], 2);
	emu_write(BN6_NAVI_MAX_HP, &w[1], 2);
	if (emu_debug_on()) fprintf(stderr, "dark: a DarkChip used: max HP %d -> %d (base %d -> %d), HP %d\n", max, v[1], base, v[0], v[2]);
	int kind = bn6 && !(profile.dark6_taught & DARK6_PRICE_TAUGHT) ? DARK_PRICE_BN6 : !dark_price_told ? DARK_PRICE_FIRST : DARK_PRICE_MORE;
	if (kind == DARK_PRICE_BN6) {
		profile.dark6_taught |= DARK6_PRICE_TAUGHT;
		profile_save();
	}
	snprintf(D.dark_words, sizeof D.dark_words, "%s", dark_price_words(kind, base - v[0]));
	dark_price_told = true;
}

/* After a BN6 battle (docs/META.md, BN6's own DarkChips): where a
 * DarkChip's dark power ran, its price; else, where one ran as its base
 * chip for want of a BugFrag, MegaMan says so, once a session (a Sword's
 * 80 where the card had read DrkSword's 400 from the battle's start, or
 * since its last BugFrag). */
static bool dark_base_told;
void dark6_after_battle(void) {
	unsigned ran = darkbn6_ran_take(), base = darkbn6_base_take();
	if (ran) { dark_price(true); return; }
	if (!base || dark_base_told) return;
	int d = 0;
	while (d < DARK_BN6_COUNT - 1 && !(base >> d & 1)) ++d;
	ChipInfo dc, bc;
	chip_info(DARK_BN6_FIRST + d, &dc);
	chip_info(darkbn6_base(DARK_BN6_FIRST + d), &bc);
	dark_base_words(D.dark_words, sizeof D.dark_words, dc.name, bc.name);
	dark_base_told = true;
}

/* A chip's copies in the Pack and the folder, -1 where they cannot be read */
static int chip_held(int id) {
	uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK), data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	if (pack < BN6_EWRAM || pack + 12u * PACK_CHIPS >= BN6_EWRAM_END || data < BN6_EWRAM || data >= BN6_EWRAM_END) return -1;
	int n = 0;
	for (uint32_t k = 0; k < 4; ++k) n += emu_read8(pack + 12u * (uint32_t)id + k);
	for (uint32_t i = 0; i < BN6_FOLDER_ENTRIES; ++i) n += (emu_read16(data + 2 * i) & 0x1FF) == (uint32_t)id;
	return n;
}

/* BN6's chips of the run's DarkChips (docs/META.md, BN6's own DarkChips):
 * one a flame of either net gave, a dev run's, or one a run held from
 * before BN6's own came in, given to the Pack where neither it nor the
 * folder holds it; once on the map, with no chat open. DarkPlus stays out
 * of BN6's battles. */
void dark_pack(void) {
	if (!D.dark_pack_due || talk_busy() || emu_read8(BN6_CHATBOX) || cinema_busy()) return;
	D.dark_pack_due = false;
	for (int k = 0; k < DARK_KINDS; ++k) {
		int id = dark_bn6_id(k);
		if (!id || !dark_count(k) || chip_held(id) != 0) continue;
		uint32_t out[2];
		game_call_ret(BN6_GIVE_CHIPS, (uint32_t)id, CHIP_CODE_STAR, 1, out);
		if (emu_debug_on()) fprintf(stderr, "dark: BN6's DarkChip %d to the Pack\n", id);
	}
}

/* What MegaMan has said of DarkChips this session, forgotten by a run
 * begun or continued (with the hooks' events, drop_events) */
void dark_forget(void) {
	dark_price_told = false;
	dark_base_told = false;
}
