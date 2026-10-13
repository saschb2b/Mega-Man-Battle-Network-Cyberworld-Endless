/* DarkChips on the run (docs/META.md): the flame of darkness on each act's
 * middle layer, BN5's or BN6's own, the chip taken, its price in max HP
 * after a battle a dark power ran in, either net's, and the chips in the
 * Pack. */
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
#include "soulunite.h"
#include "save.h"
#include "talk.h"
#include "xnavi.h"

static bool dark_price_told;    /* MegaMan has said all of a DarkChip's price (a new run or a CONTINUE forgets) */

/* BN5's own DarkChips play in BN6's battles: its mood and worried rule are
 * in them (soulmood.h, with the Souls where BN5's Navis are) */
static bool dark_bn5_plays(void) { return soulunite_ready(); }

/* Kind `k` played by BN6's rule (a BugFrag a use, its base chip without
 * one): BN6's four, but one whose record BN5's code plays where BN5's ROM
 * is read (darkbn6_bn5_code) */
static bool dark_bn6_rule(int k) { return dark_bn6_id(k) && !darkbn6_bn5_code(dark_bn6_id(k)); }

/* Kind `k` plays in BN6's battles: by BN6's rule, or by BN5's where its
 * mood is in (BN5's own seven, and BN6's record of a name BN5's code
 * plays: DarkPlus only so, darkchips.h) */
static bool dark_kind_plays(int k) {
	int id = dark_chip_id(k);
	if (dark_bn6_rule(k)) return true;
	return id && (k != DARK_PLUS_KIND || darkbn6_bn5_code(id)) && dark_bn5_plays();
}

/* A flame holding kind `k`, one BN6's battles play by BN6's rule (docs/META.md,
 * BN6's own DarkChips): its words naming what its dark power does, the
 * BugFrag it burns and its base chip, from the ROM; `flame` its list-7
 * sprite (BN6's blue flame turned purple, or BN5's on its acts). */
static void dark6_flame_setup(int k, int flame) {
	static char chip[20], base[20], does[100];
	static ScriptsDark6 words;
	int id = dark_bn6_id(k), from = darkbn6_base(id);
	layer_objs_dark_flame = id && from ? flame : -1;
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
 * DarkChips): a DarkChip the run lacks, from the run's seed and the act, in
 * its last bystander's place (layer_objs_dark_flame): BN5's flame on the
 * acts its areas dress, where its DarkChips play, offering its kinds (BN6's
 * four among them); BN6's own elsewhere, offering its four; none where the
 * run holds every one. Its words follow the rule the chip plays by. A
 * CONTINUE on its layer finds the flame it was made with (dark_flame_of). */
void dark_flame_setup(void) {
	layer_objs_dark_flame = -1;
	layer_objs_dark6 = NULL;
	layer_objs_dark_plus = 0;
	D.dark_kind = -1;
	D.dark_given = false;
	if (run.side_kind != LAYER_NORMAL || layer_in_act(run.depth) != 1) return;
	bool five = D.layer_tiles >= NET_AREAS && dark_bn5_plays();
	/* (DarkPlus by BN5's flame alone, where BN5's code plays it: the
	 * owner's call of 13 October 2026; BN6's US DarkPlus does nothing) */
	D.dark_kind = dark_flame_of(run.seed, run.depth, run.side_kind, !five, dark_kinds_offered(five, five && dark_kind_plays(DARK_PLUS_KIND)));
	if (D.dark_kind < 0) return;
	int flame = five ? xnavi_object(XOBJ_DARK_FLAME) : xnavi_bn6_flame();
	if (dark_bn6_rule(D.dark_kind)) { dark6_flame_setup(D.dark_kind, flame); return; }
	static char chip[20];
	ChipInfo ci;
	int id = dark_chip_id(D.dark_kind);
	chip_info(id, &ci);
	if (!ci.name[0]) return;
	layer_objs_dark_flame = flame;
	snprintf(chip, sizeof chip, "%s", ci.name);
	layer_objs_dark_chip = chip;
	layer_objs_dark_first = !profile.dark_taught;
	/* (what DarkPlus does, by its record's power: BN5's +50 to the next
	 * attack chip) */
	layer_objs_dark_plus = D.dark_kind == DARK_PLUS_KIND ? ci.power : 0;
	if (emu_debug_on()) fprintf(stderr, "dark: BN5's flame of darkness with %s, list 7 %d\n", chip, layer_objs_dark_flame);
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

/* A battle a DarkChip's dark power ran in, either net's chip: one
 * HPMemory of max HP, for the rest of the run (the owner's price,
 * docs/META.md), from the base the game counts HPMemory into, so the
 * NaviCust's next RUN keeps it; MegaMan says so, the whole of it the first
 * time in a session (dark_price_told), and after a profile's first such
 * battle of BN6's own (`bn6`) what BN6's darkness took besides: the
 * BugFrags it burnt, the bug gone with the battle. A DarkChip given up for
 * a Chaos Unison is none used: no price (BN5's rule). */
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

/* After a battle (docs/META.md, DarkChips): where a DarkChip's dark power
 * ran, either net's (darkbn6.c's hook on every chip MegaMan runs: BN6's
 * where a BugFrag was paid, BN5's by its rule; one given up for a Chaos
 * Unison never runs there), its price, once a battle; else, where one of
 * BN6's ran as its base chip for want of a BugFrag, MegaMan says so, once
 * a session (a Sword's 80 where the card had read DrkSword's 400 from the
 * battle's start, or since its last BugFrag). */
static bool dark_base_told;
void dark_after_battle(void) {
	if (D.in_battle) return;
	unsigned ran = darkbn6_ran_take(), base = darkbn6_base_take();
	if (ran) { dark_price((ran & DARK_RAN_BN6) != 0); return; }
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
	uint32_t pack = pack_entry(id), data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	if (!pack || data < BN6_EWRAM || data >= BN6_EWRAM_END) return -1;
	int n = 0;
	for (uint32_t k = 0; k < 4; ++k) n += emu_read8(pack + k);
	for (uint32_t i = 0; i < BN6_FOLDER_ENTRIES; ++i) n += (emu_read16(data + 2 * i) & 0x1FF) == (uint32_t)id;
	return n;
}

/* The chips of the run's DarkChips (docs/META.md, DarkChips): one a flame
 * gave, a dev run's, or one a run held from before its kind played in
 * BN6's battles, given to the Pack where neither it nor the folder holds
 * it; once on the map, with no chat open. A kind that sits out of BN6's
 * battles (DarkPlus; BN5's own without its mood) waits. */
void dark_pack(void) {
	if (!D.dark_pack_due || talk_busy() || emu_read8(BN6_CHATBOX) || cinema_busy()) return;
	D.dark_pack_due = false;
	for (int k = 0; k < DARK_KINDS; ++k) {
		int id = dark_chip_id(k);
		if (!id || !dark_kind_plays(k) || !dark_count(k) || chip_held(id) != 0) continue;
		uint32_t out[2];
		game_call_ret(BN6_GIVE_CHIPS, (uint32_t)id, CHIP_CODE_STAR, 1, out);
		if (emu_debug_on()) fprintf(stderr, "dark: DarkChip %d (kind %d) to the Pack\n", id, k);
	}
}

/* What MegaMan has said of DarkChips this session, forgotten by a run
 * begun or continued (with the hooks' events, drop_events) */
void dark_forget(void) {
	dark_price_told = false;
	dark_base_told = false;
}
