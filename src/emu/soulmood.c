/* BN5's mood and its DarkChip rule in BN6's battles (soulmood.h). */
#include "soulmood.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "bn6_attacks.h"
#include "bn6_chips.h"
#include "bn6_forms.h"
#include "chips.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soullife.h"
#include "xchips_install.h"
#include "xchips_registry.h"

#define BUS 0x08000000u
#define MOOD_CALM 0x80       /* BN5's calm (0x08011A8C), the opening's at full HP */
#define MOOD_SYNCHRO 0xFF    /* Full Synchro */
#define MOOD_TOP 0xFE        /* the most a gain raises it (0x08012810) */
#define WORRIED_HP 4         /* the opening's: worried at a quarter of his HP or less (guest.c's mood_of) */
#define MOOD_WORRIED_OPEN 0x30
#define LOW_HP 2             /* the worried rule's low HP: under a quarter (lsrs r1,r1,#2 at 0x08025168) */
#define SHELF 4              /* the most DarkChips the shelf keeps behind the padding (guest.c's shelf) */
#define BN5_DRKRECOV 194     /* BN5's DrkRecov, the worried rule's choice at low HP (cmp r0,#0xc2 at 0x08025196) */
#define LEAVE_FORM 0x40u     /* AI data +0x44's request to leave the form (BN6's 0x08015994 sets it) */
#define GATE_NOT_DARK 1      /* BN5_CHIP_DARK_GATE: usable only while he is not dark (any other but 0: only while he is) */
/* the emotion BN5's getter answers (0x08012738), in its order: a form, anger, dark, Full Synchro, calm, worried */
enum { EMO_CALM, EMO_WORRIED, EMO_SYNCHRO, EMO_ANGER, EMO_FORM, EMO_DARK };

static int breaks;
static bool done;
static uint8_t gates[XREG_IDS];   /* the registry's own ids: BN5's dark gate of each (BN5_CHIP_DARK_GATE), read from BN5's ROM */

static int side_of(uint32_t obj) { return hook_read8(obj + BN6_T1_ALLIANCE) & 1; }
static uint8_t navi8(int side, uint32_t field) { return hook_read8(BN6_BATTLE_NAVI + (uint32_t)side * BN6_BATTLE_NAVI_SIZE + field); }
static uint32_t player(int side) { return hook_read32(BN6_BATTLE_PLAYERS + (uint32_t)side * BN6_BATTLE_PLAYER_STEP); }
static uint32_t mood_at(int side) { return soul_field(side, BN6_SOUL_MOOD); }

/* side's mood as BN5 keeps it (0 too until the battle's first Custom screen opens it, which it does before any hit or
 * chip: BN6_SOUL_OPENED); whether a BN5 DarkChip made it dark for the battle */
static int soulmood_of(int side) { return hook_read8(mood_at(side)); }
static bool soulmood_dark(int side) { return done && hook_read8(soul_field(side, BN6_SOUL_DARK)); }

/* BN5's three writers: set (0x080127CE: none at dark), gain (0x080127FA: none at Full Synchro, 0xFE at most), loss
 * (0x08012818: never under 1, none at dark) */
static void set(int side, int v) {
	if (hook_read8(mood_at(side))) hook_write8(mood_at(side), (uint8_t)v);
}
static void gain(int side, int v) {
	int m = hook_read8(mood_at(side));
	if (m == MOOD_SYNCHRO || !m) return;
	hook_write8(mood_at(side), (uint8_t)(m + v > MOOD_TOP ? MOOD_TOP : m + v));
}
static void loss(int side, int v) {
	int m = hook_read8(mood_at(side));
	if (m) hook_write8(mood_at(side), (uint8_t)(m - v < 1 ? 1 : m - v));
}

/* BN5's emotion (0x08012738) from this mood */
static int emotion(int side) {
	uint32_t obj = player(side), ai = obj ? hook_read32(obj + BN6_T1_AI_DATA) : 0;
	int m = soulmood_of(side);
	if (navi8(side, BN6_NAVI_FORM)) return EMO_FORM;
	if (ai && hook_read16(ai + BN6_AI_ANGER)) return EMO_ANGER;
	return !m ? EMO_DARK : m == MOOD_SYNCHRO ? EMO_SYNCHRO : m > BN5_MOOD_WORRIED ? EMO_CALM : EMO_WORRIED;
}

/* the battle's opening: side's mood as worried as his HP and max HP say, Full Synchro kept (the guest battles' mood_of) */
static void soulmood_open(int side, int hp, int max_hp) {
	if (!done) return;
	int mood = max_hp <= 0 ? MOOD_CALM : hp * WORRIED_HP <= max_hp ? MOOD_WORRIED_OPEN : 0x40 + 0x40 * hp / max_hp;
	if (soulmood_of(side) != MOOD_SYNCHRO) hook_write8(mood_at(side), (uint8_t)mood);
}

bool soulmood_worried(int side) {
	if (breaks & SOULMOOD_BREAK_WORRY) return false;
	int m = soulmood_of(side);
	return m <= BN5_MOOD_WORRIED;
}

void soulmood_unison(int side) {
	if (done) set(side, MOOD_CALM);
}

/* ---- a hit's ---- */

/* (the hit's mood step, past BN6's own Full Synchro: r4 the struck's collision data, r5 the struck) BN5's
 * (0x08016FE6-0x08017022): the attacker's raised by the hit's gain where it is no counter, a counter calming him to Full
 * Synchro where his side is in no form (in one, BN5 adds its getter's leftover word: 0xFE), the struck's lowered */
static HookAct hit(HookRegs *r, void *user) {
	(void)user;
	uint32_t coll = r->r[4];
	int side = side_of(r->r[5]), opp = side ^ 1;
	int v = hook_read16(coll + BN6_COLL_MOOD_GAIN);
	if (v & BN6_HIT_COUNTER) {
		if (!navi8(opp, BN6_NAVI_FORM)) set(opp, MOOD_SYNCHRO);
		else gain(opp, MOOD_TOP);
	} else if (v && !(breaks & SOULMOOD_BREAK_GAIN)) {
		gain(opp, v);
	}
	loss(side, hook_read16(coll + BN6_COLL_MOOD_LOSS));
	return HOOK_CONTINUE;
}

/* ---- BN6's own writers: their callers that are twins of BN5's mirrored, and BN5's dark kept ---- */

static uint32_t caller(const HookRegs *r) { return (r->lr & ~1u) - 4; }

/* (BN6's set, r0 the side, r1 the mood, at its first instruction) from the chip's Full Synchro, Anger's start and a
 * Custom screen's script (BN5's 0x0800FFAE, 0x08011A5A, 0x0802359C): BN5's set */
static HookAct mirror_set(HookRegs *r, void *user) {
	(void)user;
	uint32_t at = caller(r);
	if (at == BN6_MOOD_FROM_CHIP || at == BN6_MOOD_FROM_ANGER || at == BN6_MOOD_FROM_SCRIPT) set((int)(r->r[0] & 1), (int)(r->r[1] & 0xFF));
	return HOOK_CONTINUE;
}

/* (BN6's calm, r5 him, at its first instruction) from the chip's Anger and Anger's end (BN5's 0x0800FFB8, 0x08011A74):
 * BN5's calm */
static HookAct mirror_calm(HookRegs *r, void *user) {
	(void)user;
	uint32_t at = caller(r);
	if (at == BN6_CALM_FROM_CHIP || at == BN6_CALM_FROM_ANGER) set(side_of(r->r[5]), MOOD_CALM);
	return HOOK_CONTINUE;
}

/* (BN6's set's write, r6 the side; its calm's, r5 him) none while a BN5 DarkChip keeps him dark: dark for the battle */
static HookAct keep_dark(HookRegs *r, void *user) {
	int side = user == (void *)(uintptr_t)BN6_MOOD_SET_PAST ? (int)(r->r[6] & 1) : side_of(r->r[5]);
	if (!soulmood_dark(side)) return HOOK_CONTINUE;
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* ---- a BN5 DarkChip used ---- */

static bool bn5_dark_chip(int chip) {
	const XChipReg *row = chip > 0 ? xreg_at(chip) : NULL;
	return row && row->how == XREG_DARK;
}

void soulmood_chip_used(uint32_t me, int chip) {
	int side = side_of(me);
	if (!done || !bn5_dark_chip(chip) || navi8(side, BN6_NAVI_SIDE_NAVI) || (breaks & SOULMOOD_BREAK_DARK)) return;
	uint32_t ai = hook_read32(me + BN6_T1_AI_DATA);
	/* (a Soul united ends: BN5 asks it at once, 0x08010068; here BN6's request to leave the form, 0x08015994's) */
	if (soul_united(side)) hook_write32(ai + BN6_AI_FLAGS, hook_read32(ai + BN6_AI_FLAGS) | LEAVE_FORM);
	set(side, 0);
	hook_write8(soul_field(side, BN6_SOUL_DARK), 1);
	/* (dark is BN6's dark too: its own mood 0, his anger gone as BN5's calm after it clears it, 0x08011AA8) */
	hook_write8(BN6_BATTLE_NAVI + (uint32_t)side * BN6_BATTLE_NAVI_SIZE + BN6_NAVI_MOOD, 0);
	hook_write16(ai + BN6_AI_ANGER, 0);
}

/* BN5's dark gate (0x08010110, at its chip use 0x080100DE): a chip of its own whose BN5 record names one is usable only
 * while he is not dark, or only while he is, where his side names no Navi (+0x29); dark is the battle's here (a BN5
 * DarkChip used), where BN5 reads his dark meter (no meter comes into a run: docs/MULTIROM.md) */
bool soulmood_chip_refused(uint32_t me, uint32_t attack, int chip, uint32_t *record) {
	int side = side_of(me), gate = done && chip > 0 && chip < XREG_IDS ? gates[chip] : 0;
	if (!gate || navi8(side, BN6_NAVI_SIDE_NAVI) || (breaks & SOULMOOD_BREAK_GATE) || soulmood_dark(side) == (gate != GATE_NOT_DARK))
		return false;
	/* (BN5's dud, 0x080100E4-0x080100F0: the chip of nothing's spawner, its chip the chip of nothing, its family the
	 * twin's of 0x1A, read from that chip's record) */
	hook_write8(attack + BN6_ATTACK_SPAWNER, BN6_SPAWNER_NOTHING);
	hook_write32(attack + BN6_ATTACK_SPAWN_ARG, 0);
	hook_write16(attack + BN6_ATTACK_CHIP, BN6_CHIP_NOTHING);
	*record = chips_table_bus() + BN6_CHIP_NOTHING * BN6_CHIP_RECORD_SIZE;
	return true;
}

static void gates_make(void) {
	const uint8_t *rom5 = XR[XROM_BN5_COLONEL_US].data;
	memset(gates, 0, sizeof gates);
	for (int i = 0; rom5 && i < xreg_count; ++i)
		if (xreg_own(&xreg_rows[i]))
			gates[xreg_rows[i].bn6] = rom5[BN5_CHIP_RECORDS + (uint32_t)xreg_rows[i].bn5 * BN6_CHIP_RECORD_SIZE + BN5_CHIP_DARK_GATE];
}

/* ---- the hand ---- */

static uint32_t rng_secondary(void) {
	uint32_t x = hook_read32(BN6_RNG_SECONDARY);
	x = ((x << 1 | x >> 31) + 1) ^ BN5_RNG_XOR;
	hook_write32(BN6_RNG_SECONDARY, x);
	return x & 0x7FFFFFFFu;
}

/* BN5's worried rule (0x08025118), while he worries or is dark: one of BN5's DarkChips the deck holds (at low HP its
 * DrkRecov where it holds one; else each after the first by a coin), swapped into the deck's place at the hand's size
 * where no DarkChip stands before it */
static void worried_rule(uint16_t deck[BN6_DECK_SIZE], int side) {
	uint32_t obj = player(side);
	bool low = obj && hook_read16(obj + BN6_T1_HP) < hook_read16(obj + BN6_T1_MAX_HP) >> LOW_HP;
	const XChipReg *rec = xreg_of_bn5(BN5_DRKRECOV);
	int recov = rec && rec->how == XREG_DARK ? rec->bn6 : -1, chosen = -1, first = -1;
	for (int i = 0; i < BN6_DECK_SIZE; ++i) {
		int id = deck[i] & 0x1FF;
		if (!bn5_dark_chip(id)) continue;
		if (chosen < 0) {
			chosen = first = i;
			continue;
		}
		if (low && id == recov) {
			chosen = i;
			continue;
		}
		if (low && (deck[chosen] & 0x1FF) == recov) continue;
		if (rng_secondary() & 1) chosen = i;
	}
	int hand = hook_read8(BN6_CUSTOM_HAND);
	if (chosen < 0 || first < hand || hand >= BN6_DECK_SIZE) return;
	uint16_t t = deck[hand];
	deck[hand] = deck[chosen];
	deck[chosen] = t;
}

/* (the deck's compaction as a Custom screen opens, its first instruction; lr its caller) BN5's worried rule before it,
 * then the compaction with the guest battles' shelf (guest.c's): a BN5 DarkChip stays live only in the hand after the
 * battle's first screen, or at the hand's end while he worries or is dark; the others behind the padding, four at most */
static HookAct compact(HookRegs *r, void *user) {
	(void)user;
	int side = hook_read8(BN6_BATTLE_SIDE) & 1;
	/* (the battle's opening, at its first screen: his mood as worried as his HP says, the guest battles' rule) */
	uint32_t opened = soul_field(side, BN6_SOUL_OPENED), obj = player(side);
	if (!hook_read8(opened) && obj) {
		hook_write8(opened, 1);
		soulmood_open(side, hook_read16(obj + BN6_T1_HP), hook_read16(obj + BN6_T1_MAX_HP));
	}
	uint16_t deck[BN6_DECK_SIZE];
	bool any = false;
	for (int i = 0; i < BN6_DECK_SIZE; ++i) {
		deck[i] = hook_read16(BN6_BATTLE_DECK + 2u * (uint32_t)i);
		any = any || (deck[i] != 0xFFFF && bn5_dark_chip(deck[i] & 0x1FF));
	}
	if (!any) return HOOK_CONTINUE;
	int emo = emotion(side);
	bool rule = emo == EMO_WORRIED || emo == EMO_DARK;
	if (rule && caller(r) == BN6_COMPACT_CALL && !(breaks & SOULMOOD_BREAK_RULE)) worried_rule(deck, side);
	bool later = hook_read8(BN6_BATTLE_SCREENS) > 1;
	int hand = hook_read8(BN6_CUSTOM_HAND), w = 0, nshelf = 0;
	uint16_t out[BN6_DECK_SIZE], shelf[SHELF];
	for (int i = 0; i < BN6_DECK_SIZE; ++i) {
		if (deck[i] == 0xFFFF) continue;
		if (bn5_dark_chip(deck[i] & 0x1FF) && nshelf < SHELF && !(i < hand ? later : i == hand ? rule : false)) {
			shelf[nshelf++] = deck[i];
			continue;
		}
		out[w++] = deck[i];
	}
	while (w < BN6_DECK_SIZE - nshelf) out[w++] = 0xFFFF;
	for (int k = 0; k < nshelf; ++k) out[w++] = shelf[k];
	for (int i = 0; i < BN6_DECK_SIZE; ++i) hook_write16(BN6_BATTLE_DECK + 2u * (uint32_t)i, out[i]);
	return HOOK_RETURN;
}

/* ---- the install ---- */

static bool expected(void) {
	static const struct { uint32_t at; uint16_t op; } code[] = {
		{ BN6_MOOD_HIT, 0x218E }, { BN6_MOOD_SET, 0xB570 }, { BN6_MOOD_SET_WRITE, 0x7384 }, { BN6_MOOD_CALM, 0xB500 },
		{ BN6_MOOD_CALM_WRITE, 0x7381 }, { BN6_COMPACT, 0xB5D0 }, { BN6_MOOD_FROM_CHIP, 0xF003 }, { BN6_MOOD_FROM_ANGER, 0xF001 },
		{ BN6_MOOD_FROM_SCRIPT, 0xF7EE }, { BN6_CALM_FROM_CHIP, 0xF001 }, { BN6_CALM_FROM_ANGER, 0xF000 }, { BN6_COMPACT_CALL, 0xF002 },
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i)
		if (rom_u16(code[i].at - BUS) != code[i].op) {
			fprintf(stderr, "soulmood: %08x is not the instruction expected\n", (unsigned)code[i].at);
			return false;
		}
	return true;
}

bool soulmood_install(int brk) {
	if (done) return true;
	if (!R.data || !emu_ready() || !expected()) return false;
	breaks = brk;
	gates_make();
	claims_begin(SOULMOOD_CLAIMS);
	bool ok = emu_hook(BN6_MOOD_HIT, hit, NULL) && emu_hook(BN6_MOOD_SET, mirror_set, NULL) &&
		emu_hook(BN6_MOOD_CALM, mirror_calm, NULL) && emu_hook(BN6_MOOD_SET_WRITE, keep_dark, (void *)(uintptr_t)BN6_MOOD_SET_PAST) &&
		emu_hook(BN6_MOOD_CALM_WRITE, keep_dark, (void *)(uintptr_t)BN6_MOOD_CALM_PAST) && emu_hook(BN6_COMPACT, compact, NULL);
	claims_end();
	/* (BN5's DarkChips come into the deck from now on, shelved: the registry's sit-out of them lifted) */
	if (ok) xchips_dark_shelved(true);
	done = ok;
	printf("soulmood: BN5's mood in BN6's battles%s%s\n", ok ? ": its hits, its writers' twins, its dark, the worried rule, the shelf and the gate"
		: " NOT placed whole", brk ? " (soulbreak: parts of it broken, the differential's own test)" : "");
	return ok;
}
