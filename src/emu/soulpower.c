/* A Soul's abilities in BN6's engine (soulpower.h). */
#include "soulpower.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "soulchaos.h"
#include "soulcode.h"
#include "soullife.h"
#include "xsong.h"

#define BUS 0x08000000u
#define SOULS 6
#define SOUL_FIRST 7
#define NONE 0xFF
/* a loadout row's bytes (BN6_FORM_LOADOUTS', BN5's 0x0801CA18): AI data +4, +5 APwrAtk, +6, +7 BPwrAtk, +8 BLeftAbility, +0x11 */
enum { ROW_4, ROW_A, ROW_6, ROW_B, ROW_LEFT, ROW_11 };
#define BN5_POWER_NORMAL 0x01   /* BN5's plain charged shot, BN6's too (the same charge times), which BN5's loadout gives the Souls whose A
                                   charges no chip (0x0801CA18; its chip charge's test, 0x08010902) */

/* The Souls' power attacks whose BN5 routine is an empty one (their charge times alone serve: the A button's charge of a
 * chip, the Souls track's S5): BN5's id, BN6's id after the moved ones */
static const uint8_t empty_powers[] = { 0x12, 0x18, 0x1B };
/* ... and the Souls' Chaos Unison's power attacks (their loadouts' sixth, AIData +0x11: the Souls track's S6), BN5's ids, their
 * routines Chaos Unison's Thumb (soulchaos.c), after the empty ones */
static const uint8_t chaos_powers[] = { 0x3A, 0x31, 0x32, 0x33, 0x38, 0x34 };

static int breaks;
static int nmoved;   /* the recipe's power attacks */
static int nchaos;   /* the chaos power attacks in the table (none where Chaos Unison is not installed) */

static int side_of(uint32_t obj) { return hook_read8(obj + BN6_T1_ALLIANCE) & 1; }

static int soul_on(int side) {
	int soul = hook_read8(soul_field(side, BN6_SOUL_ID));
	return soul >= SOUL_FIRST && soul < SOUL_FIRST + SOULS ? soul : 0;
}

/* BN6's id of BN5's power attack `id5` in the grown table, NONE where it is none of the Souls' */
static int power6(int id5) {
	int p = soulcode_power(id5);
	if (p >= 0) return p;
	for (size_t i = 0; i < sizeof empty_powers; ++i)
		if (empty_powers[i] == id5) return SOUL_POWER_FIRST + nmoved + (int)i;
	for (int i = 0; i < nchaos; ++i)
		if (chaos_powers[i] == id5) return SOUL_POWER_FIRST + nmoved + (int)sizeof empty_powers + i;
	return NONE;
}

/* BN5's id and the routine of the grown table's Souls' entry i (the moved, the empty, the chaos ones), 0 where none */
static uint32_t routine_of(int i, int *id5) {
	const SoulCode *c = &soulcode_bn5;
	int empty = (int)sizeof empty_powers;
	if (i < nmoved) {
		*id5 = c->powers[i].id;
		return soulcode_moved(c->powers[i].entry);
	}
	if (i < nmoved + empty) {
		*id5 = empty_powers[i - nmoved];
		return BN6_POWER_EMPTY | 1;
	}
	*id5 = chaos_powers[i - nmoved - empty];
	return soulchaos_power(*id5);
}

/* ---- the hooks: MegaMan's loadout, element, weakness and buster bonus in a Soul ---- */

/* (the loadout's read, r0 its row by the form, r5 MegaMan) the Soul's row */
static HookAct loadout(HookRegs *r, void *user) {
	(void)user;
	int soul = soul_on(side_of(r->r[5]));
	if (!soul || r->r[0] != BN6_FORM_LOADOUTS + BN6_FORM_LOADOUT_SIZE * BN6_FORM_PROXY) return HOOK_CONTINUE;
	r->r[0] = SOULS_LOADOUTS + (uint32_t)(BN6_FORM_LOADOUT_SIZE * (soul - SOUL_FIRST));
	return HOOK_CONTINUE;
}

/* (the element's read, r0 the form, r5 him) the Soul's (BN5's 0x0800E62C), on to the routine's next step */
static HookAct element(HookRegs *r, void *user) {
	int soul = soul_on(side_of(r->r[5]));
	if (!soul || r->r[0] != BN6_FORM_PROXY) return HOOK_CONTINUE;
	r->r[0] = (breaks & SOULPOWER_BREAK_ELEMENT) ? 0 : XR[XROM_BN5_COLONEL_US].data[BN5_SOUL_ELEMENTS - BUS + (uint32_t)soul];
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* a read by the form a Soul answers 0 to: on to `next`, the value in register `reg` */
typedef struct { uint32_t next; int reg; } Zero;
static Zero weakness = { BN6_FORM_WEAKNESS_NEXT, 0 }, bonus = { BN6_FORM_BONUS_NEXT, 1 };

/* (the second wheel's weakness read, the buster bonus read: r0 the form, r5 him) none for a Soul */
static HookAct none_for_soul(HookRegs *r, void *user) {
	const Zero *z = user;
	if (!soul_on(side_of(r->r[5])) || r->r[0] != BN6_FORM_PROXY) return HOOK_CONTINUE;
	r->r[z->reg] = 0;
	r->r[12] = z->next | 1;
	return HOOK_JUMP;
}

/* ---- the tables ---- */

static uint32_t rom5(uint32_t a) { return get32(XR[XROM_BN5_COLONEL_US].data + (a - BUS)); }

/* BN6's power attacks and charge times grown with the Souls', their words pointed at the copies; the Souls' rows */
static bool tables(void) {
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	const SoulCode *c = &soulcode_bn5;
	nmoved = c->npowers;
	nchaos = soulchaos_power(chaos_powers[0]) ? (int)sizeof chaos_powers : 0;
	int n = BN6_POWER_ATTACK_COUNT + nmoved + (int)sizeof empty_powers + nchaos;
	if (SOUL_POWER_FIRST != BN6_POWER_ATTACK_COUNT || SOULS_POWERS + 4u * (uint32_t)n > SOULS_POWERS_END ||
		SOULS_CHARGES + BN6_CHARGE_ROW * (uint32_t)n > SOULS_CHARGES_END || n > NONE)
		return false;
	uint8_t powers[4 * 0x100], charges[BN6_CHARGE_ROW * 0x100];
	memcpy(powers, R.data + (BN6_POWER_ATTACKS - BUS), 4 * BN6_POWER_ATTACK_COUNT);
	memcpy(charges, R.data + (BN6_CHARGE_TIMES - BUS), BN6_CHARGE_ROW * BN6_POWER_ATTACK_COUNT);
	for (int i = 0; i < n - BN6_POWER_ATTACK_COUNT; ++i) {
		int id5 = 0;
		uint32_t routine = routine_of(i, &id5);
		if (!routine) return false;
		put32(powers + 4 * (BN6_POWER_ATTACK_COUNT + i), routine);
		uint8_t *row = charges + BN6_CHARGE_ROW * (BN6_POWER_ATTACK_COUNT + i);
		memcpy(row, b5 + (BN5_CHARGE_TIMES - BUS) + BN6_CHARGE_ROW * (uint32_t)id5, BN6_CHARGE_ROW);
		for (int k = 0; (breaks & SOULPOWER_BREAK_CHARGE) && k < BN6_CHARGE_ROW / 2; ++k) put16(row + 2 * k, get16(row + 2 * k) + 10);
	}
	/* (each Soul's loadout: BN5's row in BN6's ids, BN5's plain shot BN6's own 1, the same charge times) */
	uint8_t rows[BN6_FORM_LOADOUT_SIZE * SOULS];
	for (int k = 0; k < SOULS; ++k) {
		const uint8_t *r5 = b5 + (BN5_SOUL_LOADOUTS - BUS) + BN6_FORM_LOADOUT_SIZE * (uint32_t)(SOUL_FIRST + k);
		uint8_t *r6 = rows + BN6_FORM_LOADOUT_SIZE * k;
		r6[ROW_4] = r5[ROW_4];
		r6[ROW_A] = r5[ROW_A] == BN5_POWER_NORMAL ? BN5_POWER_NORMAL : (uint8_t)power6(r5[ROW_A]);
		r6[ROW_6] = r5[ROW_6];
		r6[ROW_B] = (uint8_t)power6(r5[ROW_B]);
		r6[ROW_LEFT] = r5[ROW_LEFT] == NONE ? NONE : (uint8_t)power6(r5[ROW_LEFT]);
		r6[ROW_11] = nchaos ? (uint8_t)power6(r5[ROW_11]) : NONE;   /* (Chaos Unison's charged attack: the Souls track's S6, soulchaos.c) */
		if (r6[ROW_B] == NONE || (r5[ROW_LEFT] != NONE && r6[ROW_LEFT] == NONE)) return false;
		if (breaks & SOULPOWER_BREAK_FAMILY) r6[ROW_B] = (uint8_t)(SOUL_POWER_FIRST + (r6[ROW_B] - SOUL_POWER_FIRST + 1) % nmoved);
	}
	emu_write(SOULS_POWERS, powers, 4u * (uint32_t)n);
	emu_write(SOULS_CHARGES, charges, BN6_CHARGE_ROW * (uint32_t)n);
	emu_write(SOULS_LOADOUTS, rows, sizeof rows);
	emu_write32(BN6_POWER_ATTACKS_REF, SOULS_POWERS);
	emu_write32(BN6_CHARGE_TIMES_REF, SOULS_CHARGES);
	return true;
}

/* BN5's songs the moved code plays that Gregar has otherwise, each into the slot the tracks' allocations give that sound (the
 * recipe's imms number them so): copied there, or found there where another track's install copied it first (xsong.h) */
static bool sounds(void) {
	for (int i = 0; i < soulcode_bn5.nsongs; ++i) {
		const SoulSong *s = &soulcode_bn5.songs[i];
		if (xsong_install_at(XROM_BN5_COLONEL_US, s->from, s->to) != s->to) return false;
	}
	return true;
}

/* every byte the abilities stand on, as Gregar and BN5 have them */
static bool expected(void) {
	static const struct { uint32_t at; uint16_t op; } code[] = {
		{ BN6_FORM_LOADOUT_READ, 0x7801 }, { BN6_FORM_ELEMENT_READ, 0x5C20 }, { BN6_FORM_WEAKNESS_READ, 0x5C20 },
		{ BN6_FORM_BONUS_READ, 0x5C09 }, { BN6_POWER_EMPTY, 0x46F7 },
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i)
		if (rom_u16(code[i].at - BUS) != code[i].op) {
			fprintf(stderr, "soulpower: %08x is not the instruction expected\n", (unsigned)code[i].at);
			return false;
		}
	return emu_read32(BN6_POWER_ATTACKS_REF) == BN6_POWER_ATTACKS && emu_read32(BN6_CHARGE_TIMES_REF) == BN6_CHARGE_TIMES &&
		rom5(BN5_CHARGE_TIMES_REF) == BN5_CHARGE_TIMES && rom5(BN5_SOUL_LOADOUTS_REF) == BN5_SOUL_LOADOUTS;
}

bool soulpower_chip_charge(int id) {
	return nmoved && id >= SOUL_POWER_FIRST + nmoved && id < SOUL_POWER_FIRST + nmoved + (int)sizeof empty_powers;
}

bool soulpower_install(int brk) {
	static bool done;
	if (done) return true;
	if (!R.data || !emu_ready() || !XR[XROM_BN5_COLONEL_US].data || !expected()) return false;
	breaks = brk;
	claims_begin(SOULPOWER_CLAIMS);
	bool ok = tables() && sounds() && emu_hook(BN6_FORM_LOADOUT_READ, loadout, NULL) &&
		emu_hook(BN6_FORM_ELEMENT_READ, element, (void *)(uintptr_t)BN6_FORM_ELEMENT_NEXT) &&
		emu_hook(BN6_FORM_WEAKNESS_READ, none_for_soul, (void *)&weakness) && emu_hook(BN6_FORM_BONUS_READ, none_for_soul, (void *)&bonus);
	claims_end();
	done = ok;
	printf("soulpower: the Souls' abilities%s%s\n", ok ? ": their power attacks, charge times, loadouts, elements and sounds" : " NOT placed whole",
		brk ? " (soulbreak: parts of it broken, the differential's own test)" : "");
	return ok;
}
