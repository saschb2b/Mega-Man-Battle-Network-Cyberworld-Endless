/* soulproxy.h. The engine reads MegaMan's form (BN6_BATTLE_FORM) through
 * tables of BN6_FORMS entries and in three places by code (docs/ROM_DATA.md,
 * MegaMan's forms: every read of the form in Gregar followed with the forms
 * 1 and 8 side by side, and tools/xproxy.py's battles). Form 8 is Falzar's
 * TenguCross: Gregar keeps its entries (its overlay, element, Airshoe,
 * charged shot, palette, face) and its chip bonus, and no CROSSSELECT of
 * Gregar's lists it. Its entries are written over with HeatCross's here,
 * the three code readers that tell 8 from 1 answered as 1 by hooks (the
 * chip bonus, the arm flash's look, the summon), and its NameID's object id
 * (BN6_FORM_NAME_BASE plus the form) given HeatCross's AI index. What stays its own: the form byte and
 * its copies, the NameID (0x1B3 against 0x1AC), and what Beast Out makes of
 * it (form 0x14, Falzar's TenguBeast, as the Custom screen's emblem and its
 * commit add to the form: not a HeatCross's; the Souls grey Beast Out while
 * one is united). */
#include "soulproxy.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soullook.h"
#include "soulpassive.h"
#include "xvirus_grow.h"

#define BUS 0x08000000u

/* A table by form: its address, its entry's bytes, and entries 1 and 8 as
 * Gregar holds them (checked before anything is written) */
typedef struct { uint32_t at, size; uint8_t heat[BN6_FORM_LOADOUT_SIZE], tengu[BN6_FORM_LOADOUT_SIZE]; } FormTable;

static const FormTable tables[] = {
	{ BN6_FORM_SPRITES, 1, { 0x00 }, { 0x00 } },
	{ BN6_FORM_OVERLAYS, 4, { 0xE1, 0x12, 0x01, 0x08 }, { 0x1D, 0x13, 0x01, 0x08 } },
	{ BN6_FORM_OVERLAYS_END, 4, { 0xFD, 0x13, 0x01, 0x08 }, { 0xFD, 0x13, 0x01, 0x08 } },
	{ BN6_FORM_ELEMENTS, 1, { 0x01 }, { 0x00 } },
	{ BN6_FORM_WEAKNESSES, 1, { 0x00 }, { 0x80 } },
	{ BN6_FORM_PASSIVES, 4, { 0xB9, 0x45, 0x01, 0x08 }, { 0xD5, 0x45, 0x01, 0x08 } },
	{ BN6_FORM_PASSIVES_COLL, 4, { 0x1F, 0x47, 0x01, 0x08 }, { 0x3D, 0x47, 0x01, 0x08 } },
	{ BN6_FORM_LOADOUTS, BN6_FORM_LOADOUT_SIZE, { 0xFF, 0xFF, 0x00, 0x06, 0xFF, 0xFF }, { 0xFF, 0xFF, 0x00, 0x0F, 0x10, 0xFF } },
	{ BN6_FORM_PALETTES, 1, { 0x02 }, { 0x0B } },
	{ BN6_FORM_BUSTER_BONUS, 1, { 0x01 }, { 0x00 } },
	{ BN6_FORM_MUGSHOTS, 1, { 0x05 }, { 0x07 } },
};

/* the object id of a form's NameID: (version, actor type, AI index), HeatCross's 0x1AC and form 8's 0x1B3 */
#define OBJECT_ID_SIZE 3
static const uint8_t heat_id[OBJECT_ID_SIZE] = { 0x00, 0x02, 0x19 }, tengu_id[OBJECT_ID_SIZE] = { 0x00, 0x02, 0x20 };

/* the instructions the hooks stand on, as Gregar has them */
#define CMP_R0_1  0x2801u   /* BN6_CHIP_BONUS_HEAT */
#define CMP_R0_0  0x2800u   /* BN6_ARM_FLASH_FORM */
#define MOVS_R3_20 0x2314u  /* BN6_CROSS_SUMMON */
#define MOVS_R1_0 0x2100u   /* BN6_CROSS_COMMIT */

static bool rom_is(uint32_t at, const uint8_t *want, uint32_t n) {
	return at >= BUS && at - BUS + n <= ROM_SIZE && !memcmp(R.data + (at - BUS), want, n);
}

/* (r0 MegaMan's form, r4 the chip, r5 him: the proxy's chip bonus is HeatCross's; a Soul's BN5's, soulpassive.c) */
static HookAct heat_bonus(HookRegs *r, void *user) {
	(void)user;
	int bonus, look;
	if (r->r[0] == BN6_FORM_PROXY && soulpassive_bonus(hook_read8(r->r[5] + BN6_T1_ALLIANCE) & 1, (int)r->r[4], &bonus, &look)) {
		r->r[6] += (uint32_t)bonus;
		r->r[7] = (uint32_t)look;
		r->r[12] = BN6_CHIP_BONUS_DONE | 1;
		return HOOK_JUMP;
	}
	if (r->r[0] == BN6_FORM_PROXY) r->r[0] = BN6_FORM_HEAT_CROSS;
	return HOOK_CONTINUE;
}

/* (r0 MegaMan's form, r5 him: the proxy's arm flash is HeatCross's; a Soul's BN5's, its kind in r4 and its frame the
 * Soul, soullook.c) */
static HookAct heat_flash(HookRegs *r, void *user) {
	(void)user;
	int kind, frame;
	if (r->r[0] == BN6_FORM_PROXY && soullook_flash(hook_read8(r->r[5] + BN6_T1_ALLIANCE) & 1, &kind, &frame)) {
		r->r[0] = (uint32_t)frame;
		r->r[4] = (uint32_t)kind;
		return HOOK_CONTINUE;
	}
	if (r->r[0] == BN6_FORM_PROXY) r->r[0] = BN6_FORM_HEAT_CROSS;
	return HOOK_CONTINUE;
}

/* (r4 the form he crosses into: the proxy's summon is HeatMan's) */
static HookAct summon(HookRegs *r, void *user) {
	(void)user;
	if (r->r[4] == BN6_FORM_PROXY) r->r[4] = BN6_FORM_HEAT_CROSS;
	return HOOK_CONTINUE;
}

bool soulproxy_install(int breaks) {
	static bool done;
	if (done) return true;
	if (!R.data || !emu_ready()) return false;
	/* (everything checked first: nothing written where one byte is not Gregar's) */
	bool ok = rom_is(BN6_OBJECT_IDS + OBJECT_ID_SIZE * (BN6_FORM_NAME_BASE + BN6_FORM_HEAT_CROSS), heat_id, OBJECT_ID_SIZE) &&
		rom_is(BN6_OBJECT_IDS + OBJECT_ID_SIZE * (BN6_FORM_NAME_BASE + BN6_FORM_PROXY), tengu_id, OBJECT_ID_SIZE) &&
		rom_u16(BN6_CHIP_BONUS_HEAT - BUS) == CMP_R0_1 && rom_u16(BN6_ARM_FLASH_FORM - BUS) == CMP_R0_0 &&
		rom_u16(BN6_CROSS_SUMMON - BUS) == MOVS_R3_20;
	for (size_t i = 0; ok && i < sizeof tables / sizeof *tables; ++i) {
		const FormTable *t = &tables[i];
		ok = rom_is(t->at + t->size * BN6_FORM_HEAT_CROSS, t->heat, t->size) && rom_is(t->at + t->size * BN6_FORM_PROXY, t->tengu, t->size);
	}
	if (!ok) {
		fprintf(stderr, "soulproxy: the ROM's tables by form are not Gregar's as expected: form 8 left as it is\n");
		return false;
	}
	/* (its NameID's object id in the enemy ids' table the engine reads: the
	 * families' room's grown copy where BN5's ROM grew it, through the
	 * room's own writer, else BN6's own; written first, as it is checked
	 * there: nothing else is written where it is not TenguCross's) */
	if (!(breaks & SOULPROXY_BREAK_OBJECT_ID) && !xg_bn6_entry(XG_IDS, BN6_FORM_NAME_BASE + BN6_FORM_PROXY, tengu_id, heat_id)) {
		fprintf(stderr, "soulproxy: form 8's object id is not TenguCross's in the table the engine reads: form 8 left as it is\n");
		return false;
	}
	done = true;
	claims_begin(SOULS_CLAIMS);
	for (size_t i = 0; !(breaks & SOULPROXY_BREAK_TABLES) && i < sizeof tables / sizeof *tables; ++i)
		emu_write(tables[i].at + tables[i].size * BN6_FORM_PROXY, tables[i].heat, tables[i].size);
	bool hooked = ((breaks & SOULPROXY_BREAK_BONUS) || emu_hook(BN6_CHIP_BONUS_HEAT, heat_bonus, NULL)) &&
		((breaks & SOULPROXY_BREAK_FLASH) || emu_hook(BN6_ARM_FLASH_FORM, heat_flash, NULL)) &&
		((breaks & SOULPROXY_BREAK_SUMMON) || emu_hook(BN6_CROSS_SUMMON, summon, NULL));
	claims_end();
	printf("soulproxy: form %d made HeatCross's copy: %d tables and its object id written%s%s\n", BN6_FORM_PROXY, (int)(sizeof tables / sizeof *tables),
		hooked ? ", its three code readers hooked" : ", a hook NOT placed", breaks ? " (with parts left out: proxybreak)" : "");
	if (breaks) printf("soulproxy: proxybreak=%d\n", breaks);
	return hooked;
}

/* the form the next Cross chosen is committed as, 0 none (one Cross) */
static int next_form;

/* (r0 the form CROSSSELECT's commit writes: its row + 1) */
static HookAct commit(HookRegs *r, void *user) {
	(void)user;
	if (next_form) {
		printf("cross: form %u chosen on CROSSSELECT, committed as form %d\n", (unsigned)r->r[0], next_form);
		r->r[0] = (uint32_t)next_form;
		next_form = 0;
	}
	return HOOK_CONTINUE;
}

bool soulproxy_cross(int form) {
	static bool hooked;
	if (form < 1 || form >= BN6_FORMS || !R.data || rom_u16(BN6_CROSS_COMMIT - BUS) != MOVS_R1_0) {
		printf("cross: %d is no form (1-%d), or CROSSSELECT's commit is not Gregar's\n", form, BN6_FORMS - 1);
		return false;
	}
	if (!hooked) hooked = emu_hook(BN6_CROSS_COMMIT, commit, NULL);
	if (hooked) next_form = form;
	printf("cross: the next Cross chosen on CROSSSELECT %s form %d\n", hooked ? "commits" : "cannot commit (no hook)", form);
	return hooked;
}
