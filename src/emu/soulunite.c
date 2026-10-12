/* UNITE on BN6's Custom screen (soulunite.h). */
#include "soulunite.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "chips.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soulchips.h"
#include "soulcolonel.h"
#include "soulcode.h"
#include "soulkinds.h"
#include "soullife.h"
#include "soullook.h"
#include "soulmood.h"
#include "soulpassive.h"
#include "soulpower.h"
#include "soulproxy.h"
#include "thumb_reloc.h"
#include "xvirus_recipe.h"

#define BUS 0x08000000u
/* The Souls' free space (free_space.h's FREE_SOULS to FREE_PANELS; docs/EMULATION.md), written once a core */
#define SOULS_AT      FREE_SOULS
#define AT_STUBS      (SOULS_AT + 0x000u)   /* soullife.c's two stubs, 8 bytes */
#define AT_UNITE_A    (SOULS_AT + 0x012u)   /* BN5's A on UNITE, moved (its distance from a word boundary BN5's: 2) */
#define AT_VENEERS    (SOULS_AT + 0x040u)   /* ... its calls out: one veneer, on to BN6's PlaySoundEffect */
#define AT_ART        (SOULS_AT + 0x100u)   /* UNITE's art for the chip's window, BN5_UNITE_ART_SIZE bytes */
#define AT_ART_PALS   (SOULS_AT + 0x700u)   /* ... its palettes, a Soul's and Chaos Unison's */
#define AT_PICTURES   (SOULS_AT + 0x800u)   /* the Souls' pictures, PICTURES of PICTURE_SIZE by BN5's number */
#define AT_PICTURE_PAL (SOULS_AT + 0xF00u)  /* ... their palette */
#define ART_PALS      2
#define PICTURES      14    /* by BN5's number: 0 none, 1-12 the Souls, 0xD Chaos Unison (mode 0x24 picks it) */
#define PICTURE_SIZE  0x80
#define PALETTE_SIZE  0x20
#define SOULS_BN5     12    /* BN5_SOUL_KINDS' and BN5_SOUL_FLAGS' entries: Souls 1-12 */
#define TEAM_SOULS    6     /* the run's Souls (souls.h): bit k BN5's Soul 7 + k, its flag BN5_FLAG_SOUL + k */
#define STATE_LIT     0     /* BN6_ELEMENT_STATE */
#define STATE_GREY    1
#define STATE_CHOSEN  2
#define TURNS_SOUL    3     /* a Soul's turns, before its NaviStats' +0x32 (BN5's OK, BN5_UNITE_COMMIT) */
#define TURNS_MAX     9
#define SOULS_USED    0x1000u   /* the Souls' place in use so far, from SOULS_AT: the rest the track's later milestones' */
_Static_assert(AT_UNITE_A + BN5_UNITE_A_SIZE <= AT_VENEERS && AT_VENEERS + THUMB_VENEER_SIZE <= AT_ART &&
	AT_ART + BN5_UNITE_ART_SIZE <= AT_ART_PALS && AT_ART_PALS + ART_PALS * PALETTE_SIZE <= AT_PICTURES &&
	AT_PICTURES + PICTURES * PICTURE_SIZE <= AT_PICTURE_PAL && AT_PICTURE_PAL + PALETTE_SIZE <= SOULS_AT + SOULS_USED &&
	SOULS_AT + SOULS_USED <= FREE_PANELS, "the Souls' places past their room (free_space.h)");

static int breaks;
static unsigned held;   /* the run's Souls (soulunite_held), set between frames */
static uint8_t kinds5[SOULS_BN5], flags5[SOULS_BN5];   /* BN5_SOUL_KINDS, BN5_SOUL_FLAGS, read from BN5's ROM */
static uint32_t emblem_tiles, emblem_pals;   /* the emblem's art, as Gregar's words name it */

void soulunite_held(unsigned souls) { held = souls & ((1u << TEAM_SOULS) - 1); }

/* ---- what UNITE reads ---- */

static int side_now(void) { return hook_read8(BN6_BATTLE_SIDE) & 1; }
static uint8_t navi8(int side, uint32_t field) { return hook_read8(BN6_BATTLE_NAVI + (uint32_t)side * BN6_BATTLE_NAVI_SIZE + field); }
static uint32_t player(int side) { return hook_read32(BN6_BATTLE_PLAYERS + (uint32_t)side * BN6_BATTLE_PLAYER_STEP); }
static uint8_t slot_of(int side) { return hook_read8(soul_field(side, BN6_SOUL_SLOT)); }

/* BN5's mood as its emotion getter reads it for UNITE (0x08012738): a form
 * (4) or anger (3) and he is neither worried (1: mood 1-BN5_MOOD_WORRIED)
 * nor dark (5: mood 0). BN5's mood in BN6's battles is BN5's own, kept by
 * BN5's rules in the Souls' block (soulmood.c), opened as worried as his
 * HP says. */
static bool calm_enough(int side) {
	uint32_t obj = player(side), ai = obj ? hook_read32(obj + BN6_T1_AI_DATA) : 0;
	return navi8(side, BN6_NAVI_FORM) || (ai && hook_read16(ai + BN6_AI_ANGER)) || !soulmood_worried(side);
}

/* UNITE could stand (BN5_UNITE_STAND): a Soul held (Double Soul learned
 * with it), MegaMan himself on this side, no link, not beasted out (one form
 * at a time), BN5's mood */
static bool unite_stands(int side) {
	uint32_t settings = hook_read32(BN6_BATTLE_SETTINGS);
	bool link = settings >= BUS && hook_read8(settings + BN6_SETTINGS_MODE) == BN6_BATTLE_LINK;
	return held && !hook_read8(BN6_CUSTOM_NAVI) && !link && navi8(side, BN6_NAVI_FORM) < BN6_FORM_BEAST && calm_enough(side);
}

/* What element 0xB is, and the chip's window's art for it: UNITE's (BN5's,
 * copied) or the emblem's (the words its art routine 0x0802871C loads) */
static void slot_be(int side, int slot) {
	hook_write8(soul_field(side, BN6_SOUL_SLOT), (uint8_t)slot);
	uint32_t tiles = slot == BN6_SLOT_UNITE ? AT_ART : emblem_tiles, pals = slot == BN6_SLOT_UNITE ? AT_ART_PALS : emblem_pals;
	if (hook_read32(BN6_EMBLEM_ART_TILES) != tiles) hook_write32(BN6_EMBLEM_ART_TILES, tiles);
	if (hook_read32(BN6_EMBLEM_ART_PALS) != pals) hook_write32(BN6_EMBLEM_ART_PALS, pals);
}

/* BN5's check of the last chip picked (BN5_UNITE_CHECK): the Soul (1-12) it
 * unites, 0 none; *chaos 1 for a DarkChip of its kind. Its kind as BN5 has
 * it (soulkinds.h), the first Soul of that kind this version's and held,
 * not united this battle (its Chaos Unison apart). */
static int lights(int side, int *chaos) {
	int n = hook_read8(BN6_CUSTOM_PICKED);
	if (!n) return 0;
	uint32_t el = BN6_CUSTOM_ELEMENTS + (uint32_t)hook_read8(BN6_CUSTOM_PICKS + n - 1) * BN6_ELEMENT_SIZE;
	if (hook_read8(el + BN6_ELEMENT_TYPE) || (hook_read8(el + BN6_ELEMENT_SLOT) & 1) || hook_read8(el + BN6_ELEMENT_SOUL)) return 0;
	/* (the chip as the deck has it: BN6 shows a Mega or Giga as "?" in link battles alone, 0x0802A54A, where UNITE stands not) */
	int chip = hook_read16(hook_read32(el + BN6_ELEMENT_DECK)) & 0x1FF, kind = soulkinds_of(chip);
	bool dark = soulkinds_dark(chip);
	for (int s = 1; s <= SOULS_BN5; ++s) {
		if (kinds5[s - 1] + ((breaks & SOULUNITE_BREAK_KIND) ? 1 : 0) != kind) continue;
		int k = flags5[s - 1] - BN5_FLAG_SOUL;
		uint32_t bit = (1u << s) << (dark ? 16 : 0);
		if (k < 0 || k >= TEAM_SOULS || !(held >> k & 1) || (hook_read32(soul_field(side, BN6_SOUL_USED)) & bit)) return 0;
		*chaos = dark;
		return s;
	}
	return 0;
}

/* ---- the hooks ---- */

/* (r0 Beast Out's test, 0x08029FB0) element 0xB stands where Beast Out or
 * UNITE does; the emblem where Beast Out stands, else UNITE (grey until a
 * pick lights it: the lit test below answers so) */
static HookAct stand(HookRegs *r, void *user) {
	(void)user;
	if (!held) return HOOK_CONTINUE;
	int side = side_now();
	if (soul_united(side)) hook_write8(soul_field(side, BN6_SOUL_SCREEN), 1);   /* (its end may come as this turn starts: soullife.c) */
	bool beast = r->r[0] != 0, unite = unite_stands(side);
	hook_write8(soul_field(side, BN6_SOUL_STANDS), (uint8_t)((unite ? BN6_STANDS_UNITE : 0) | (beast ? BN6_STANDS_BEAST : 0)));
	slot_be(side, unite && !beast ? BN6_SLOT_UNITE : BN6_SLOT_EMBLEM);
	if (unite) r->r[0] = 1;
	return HOOK_CONTINUE;
}

/* (the emblem's lit test) grey for UNITE, and for the emblem while a Soul
 * is united: no Beast Out from the proxy (it would give form 0x14) */
static HookAct emblem_lit(HookRegs *r, void *user) {
	(void)user;
	if (!held) return HOOK_CONTINUE;
	int side = side_now();
	if (slot_of(side) != BN6_SLOT_UNITE && (!soul_united(side) || (breaks & SOULUNITE_BREAK_GREY))) return HOOK_CONTINUE;
	r->r[0] = 0;
	return HOOK_RETURN;
}

/* (after each pick) UNITE lit while the last pick lights a held Soul and no
 * Cross is chosen on this screen; else the emblem by BN6's own rule where
 * Beast Out stands, else UNITE grey (BN5's own writes: +4 the last pick,
 * +5 the Soul, +6 its chaos, +7) */
static HookAct unite_lit(HookRegs *r, void *user) {
	(void)r, (void)user;
	if (!held) return HOOK_CONTINUE;
	int side = side_now(), chaos = 0, picks = hook_read8(BN6_CUSTOM_PICKED);
	uint32_t el = BN6_CUSTOM_UNITE;
	uint8_t stands = hook_read8(soul_field(side, BN6_SOUL_STANDS));
	if (hook_read8(el + BN6_ELEMENT_TYPE) != BN6_ELEMENT_EMBLEM || hook_read8(el + BN6_ELEMENT_STATE) == STATE_CHOSEN || !(stands & BN6_STANDS_UNITE))
		return HOOK_CONTINUE;
	int soul = hook_read8(BN6_CUSTOM_CROSS_TAKEN) ? 0 : lights(side, &chaos);
	if (!soul && (stands & BN6_STANDS_BEAST)) {
		hook_write8(el + BN6_ELEMENT_SOUL, 0);
		hook_write8(el + BN6_ELEMENT_CHAOS, 0);
		slot_be(side, BN6_SLOT_EMBLEM);
		return HOOK_CONTINUE;
	}
	if (picks) hook_write8(el + BN6_ELEMENT_SLOT, hook_read8(BN6_CUSTOM_PICKS + picks - 1));
	hook_write8(el + BN6_ELEMENT_SOUL, (uint8_t)soul);
	hook_write8(el + BN6_ELEMENT_CHAOS, (uint8_t)chaos);
	hook_write8(el + BN6_ELEMENT_STATE, soul ? STATE_LIT : STATE_GREY);
	slot_be(side, BN6_SLOT_UNITE);
	return HOOK_RETURN;
}

/* (A on element 0xB, r4 it, r5 the screen) UNITE's: BN5's own code, moved
 * (lit: sound 0x81 and mode 0x24, which drops the last pick, puts UNITE
 * first among the picks and marks it chosen; grey: 0x69); CROSSSELECT barred
 * while it is chosen, as while the emblem is (BN6's mode 0x48) */
static HookAct emblem_a(HookRegs *r, void *user) {
	(void)user;
	if (!held || slot_of(side_now()) != BN6_SLOT_UNITE) return HOOK_CONTINUE;
	if (hook_read8(BN6_CUSTOM_UNITE + BN6_ELEMENT_STATE) == STATE_LIT) hook_write8(BN6_CUSTOM_EMBLEM_TAKEN, 1);
	r->r[12] = AT_UNITE_A | 1;
	return HOOK_JUMP;
}

/* (B, r0 the last pick's type, r4 its element) UNITE the last pick: the chip
 * given up back in its place, its icon redrawn (BN5_UNITE_TAKE_BACK), then
 * BN6's own end of it (UNITE lit again, the picks' update, sound 0x83) */
static HookAct emblem_b(HookRegs *r, void *user) {
	(void)user;
	int picks = hook_read8(BN6_CUSTOM_PICKED);
	if (!held || r->r[0] != BN6_ELEMENT_EMBLEM || r->r[4] != BN6_CUSTOM_UNITE || slot_of(side_now()) != BN6_SLOT_UNITE || !picks) return HOOK_CONTINUE;
	uint8_t given = hook_read8(BN6_CUSTOM_UNITE + BN6_ELEMENT_SLOT);
	hook_write8(BN6_CUSTOM_PICKS + picks - 1, given);
	uint32_t el = BN6_CUSTOM_ELEMENTS + (uint32_t)given * BN6_ELEMENT_SIZE;
	int chip = hook_read16(hook_read32(el + BN6_ELEMENT_DECK)) & 0x1FF;
	uint32_t icon = hook_read32(chips_table_bus() + (uint32_t)chip * BN6_CHIP_RECORD_SIZE + BN6_CHIP_ICON_PTR);   /* (the records the game reads: chips.h) */
	for (uint32_t i = 0; icon >= BUS && i < PICTURE_SIZE; i += 4)
		hook_write32(BN6_CUSTOM_PICK_ICONS + (uint32_t)(picks - 1) * PICTURE_SIZE + i, hook_read32(icon + i));
	hook_write8(BN6_CUSTOM_EMBLEM_TAKEN, 0);
	r->r[12] = BN6_EMBLEM_B_TAIL | 1;
	return HOOK_JUMP;
}

/* (the picks leaving the deck: r0 the battle's mode, the pick's element
 * pushed at sp) UNITE's pick names the chip it took (its +4): that chip
 * leaves the deck too, as in BN5's every battle (0x080250A2), where BN6
 * follows a type 2 pick's +4 in a link battle alone */
static HookAct emblem_given(HookRegs *r, void *user) {
	(void)user;
	if (!held || hook_read32(r->sp) != BN6_CUSTOM_UNITE || slot_of(side_now()) != BN6_SLOT_UNITE) return HOOK_CONTINUE;
	r->r[0] = BN6_BATTLE_LINK;
	return HOOK_CONTINUE;
}

/* (OK, a pick of type 2: r0 its element) UNITE chosen: BN5's commit
 * (BN5_UNITE_COMMIT), the record the proxy form's with the Soul's chaos
 * and turns, the Soul pending for soullife.c, its used bit; Beast Out's
 * commit otherwise */
static HookAct emblem_commit(HookRegs *r, void *user) {
	(void)user;
	uint32_t el = BN6_CUSTOM_UNITE;
	if (!held || r->r[0] != el) return HOOK_CONTINUE;
	int side = side_now();
	if (slot_of(side) != BN6_SLOT_UNITE || hook_read8(el + BN6_ELEMENT_STATE) != STATE_CHOSEN) return HOOK_CONTINUE;
	int soul = hook_read8(el + BN6_ELEMENT_SOUL), chaos = hook_read8(el + BN6_ELEMENT_CHAOS) != 0;
	int turns = chaos ? 1 : TURNS_SOUL + (int8_t)navi8(side, BN6_NAVI_TURN_EXTRA);
	turns = (turns > TURNS_MAX ? TURNS_MAX : turns < 1 ? 1 : turns) + ((breaks & SOULUNITE_BREAK_TURNS) ? 1 : 0);
	hook_write8(BN6_CROSS_RECORD, BN6_FORM_PROXY);
	hook_write8(BN6_CROSS_RECORD + BN6_RECORD_CHAOS, (uint8_t)chaos);
	hook_write8(BN6_CROSS_RECORD + BN6_RECORD_TURNS, (uint8_t)turns);
	hook_write8(soul_field(side, BN6_SOUL_NEXT), (uint8_t)soul);
	hook_write8(soul_field(side, BN6_SOUL_NEXT_CHAOS), (uint8_t)chaos);
	hook_write8(soul_field(side, BN6_SOUL_NEXT_TURNS), (uint8_t)turns);
	uint32_t used = soul_field(side, BN6_SOUL_USED);
	if (!(breaks & SOULUNITE_BREAK_USED)) hook_write32(used, hook_read32(used) | (1u << soul) << (chaos ? 16 : 0));
	r->r[12] = BN6_EMBLEM_COMMIT_NEXT | 1;
	return HOOK_JUMP;
}

/* ---- the install ---- */

static uint32_t rom6(uint32_t a) { return rom_u32(a - BUS); }
static uint32_t rom5(uint32_t a) { const uint8_t *p = XR[XROM_BN5_COLONEL_US].data + (a - BUS); return (uint32_t)p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static bool in5(uint32_t a, uint32_t n) { return a >= BUS && a - BUS + n <= 0x800000u; }

/* the calls out of BN5's A on UNITE: its PlaySoundEffect, BN6's through a veneer */
static uint32_t calls_out(uint32_t target, void *user) { return target == BN5_PLAY_SOUND ? thumb_veneer_get(user, BN6_PLAY_SOUND) : 0; }

/* BN5's A on UNITE moved, its calls aimed at BN6's (its bytes checked first, expected()); false where they cannot be */
static bool move_a(const uint8_t *b5) {
	uint8_t code[BN5_UNITE_A_SIZE], veneer[THUMB_VENEER_SIZE];
	uint32_t offs[4];
	memcpy(code, b5 + (BN5_UNITE_A - BUS), sizeof code);
	ThumbVeneers pool = { veneer, AT_VENEERS, 1, 0 };
	ThumbBlock block = { code, sizeof code, BN5_UNITE_A, AT_UNITE_A };
	int n = thumb_bl_scan(code, sizeof code, offs, 4);
	if (n != 2 || !thumb_block_rebase(&block, offs, n, calls_out, &pool) || pool.n != 1) return false;
	emu_write(AT_UNITE_A, code, sizeof code);
	emu_write(AT_VENEERS, veneer, sizeof veneer);
	return true;
}

/* BN5's art and pictures into the Souls' free space, mode 0x24 pointed at the pictures */
static void copy_art(const uint8_t *b5) {
	emu_write(AT_ART, b5 + (rom5(BN5_UNITE_ART_TILES) - BUS), BN5_UNITE_ART_SIZE);
	emu_write(AT_ART_PALS, b5 + (rom5(BN5_UNITE_ART_PALS) - BUS), ART_PALS * PALETTE_SIZE);
	emu_write(AT_PICTURES, b5 + (rom5(BN5_SOUL_PICTURES) - BUS), PICTURES * PICTURE_SIZE);
	emu_write(AT_PICTURE_PAL, b5 + (rom5(BN5_SOUL_PICTURE_PAL) - BUS), PALETTE_SIZE);
	emu_write32(BN6_UNITE_PICTURES, AT_PICTURES);
	emu_write32(BN6_UNITE_PICTURES_2, AT_PICTURES);
	emu_write32(BN6_UNITE_PICTURE_PAL, AT_PICTURE_PAL);
}

/* every byte the install stands on, as Gregar and BN5 have it */
static bool expected(const uint8_t *b5) {
	static const struct { uint32_t at; uint16_t op; } code[] = {
		{ BN6_UNITE_STAND, 0x4200 }, { BN6_EMBLEM_LIT, 0xB510 }, { BN6_UNITE_LIT, 0xB5D0 }, { BN6_EMBLEM_A, 0xB500 },
		{ BN6_EMBLEM_B, 0x2802 }, { BN6_EMBLEM_COMMIT, 0xF000 }, { BN6_EMBLEM_COMMIT_NEXT, 0xE02A }, { BN6_EMBLEM_B_TAIL, 0x2000 },
		{ BN6_EMBLEM_GIVEN, 0x2801 },
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i)
		if (rom_u16(code[i].at - BUS) != code[i].op) return false;
	bool words6 = rom6(BN6_UNITE_PICTURES) == rom6(BN6_UNITE_PICTURES_2) && rom6(BN6_EMBLEM_ART_TILES) >= BUS && rom6(BN6_EMBLEM_ART_PALS) >= BUS &&
		rom6(BN6_UNITE_PICTURE_PAL) >= BUS;
	bool words5 = xvirus_crc32(b5 + (BN5_UNITE_A - BUS), BN5_UNITE_A_SIZE) == BN5_UNITE_A_CRC &&
		in5(rom5(BN5_UNITE_ART_TILES), BN5_UNITE_ART_SIZE) && in5(rom5(BN5_UNITE_ART_PALS), ART_PALS * PALETTE_SIZE) &&
		in5(rom5(BN5_SOUL_PICTURES), PICTURES * PICTURE_SIZE) && in5(rom5(BN5_SOUL_PICTURE_PAL), PALETTE_SIZE);
	/* (Team Colonel's six Souls, BN5's 7-12, their flags BN5_FLAG_SOUL + 0-5) */
	for (int k = 0; words5 && k < TEAM_SOULS; ++k) words5 = b5[BN5_SOUL_FLAGS - BUS + (uint32_t)(SOULS_BN5 - TEAM_SOULS + k)] == BN5_FLAG_SOUL + k;
	return words6 && words5;
}

/* each Soul's own: its code moved, its look, abilities and passives (the Souls track's S3-S5), all or none of each */
static bool souls_own(int soulbreak) {
	return soulcode_install(&soulcode_bn5) && soullook_install(SOULBREAK_LOOK(soulbreak)) && soulpower_install(SOULBREAK_POWER(soulbreak)) &&
		soulpassive_install(SOULBREAK_PASSIVE(soulbreak)) && soulchips_install(SOULBREAK_CHIPS(soulbreak)) &&
		soulcolonel_install(SOULBREAK_PASSIVE(soulbreak)) && soulmood_install(SOULBREAK_MOOD(soulbreak));
}

static bool done;

bool soulunite_ready(void) { return done; }

bool soulunite_install(int brk, int soulbreak) {
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	if (done) return true;
	if (!R.data || !emu_ready() || !b5 || !soulkinds_make()) return false;
	if (!expected(b5)) {
		fprintf(stderr, "soulunite: the code or data UNITE stands on is not Gregar's and BN5's as expected: none of it placed\n");
		return false;
	}
	/* (a Soul's life first: it checks every byte it stands on before it writes one, so nothing is written where one is not as expected) */
	claims_begin(SOULS_CLAIMS);
	bool life = soullife_install(AT_STUBS, (brk & SOULUNITE_BREAK_LEAVE) != 0);
	claims_end();
	if (!life) return false;
	done = true;
	breaks = brk;
	memcpy(kinds5, b5 + (BN5_SOUL_KINDS - BUS), sizeof kinds5);
	memcpy(flags5, b5 + (BN5_SOUL_FLAGS - BUS), sizeof flags5);
	emblem_tiles = rom6(BN6_EMBLEM_ART_TILES);
	emblem_pals = rom6(BN6_EMBLEM_ART_PALS);
	claims_begin(SOULS_CLAIMS);
	bool moved = move_a(b5);
	if (moved) copy_art(b5);
	bool ok = moved && emu_hook(BN6_UNITE_STAND, stand, NULL) && emu_hook(BN6_EMBLEM_LIT, emblem_lit, NULL) &&
		emu_hook(BN6_UNITE_LIT, unite_lit, NULL) && emu_hook(BN6_EMBLEM_A, emblem_a, NULL) && emu_hook(BN6_EMBLEM_B, emblem_b, NULL) &&
		emu_hook(BN6_EMBLEM_COMMIT, emblem_commit, NULL) && emu_hook(BN6_EMBLEM_GIVEN, emblem_given, NULL);
	claims_end();
	printf("soulunite: UNITE on the Custom screen%s%s\n", ok ? ", the Souls' life on the proxy form" : ": NOT placed whole",
		brk ? " (with parts broken: unitebreak)" : "");
	if (ok && !souls_own(soulbreak)) fprintf(stderr, "soulunite: the Souls' own code, look and abilities NOT placed whole: they unite as the proxy\n");
	if (brk) printf("soulunite: unitebreak=%d\n", brk);
	return ok;
}
