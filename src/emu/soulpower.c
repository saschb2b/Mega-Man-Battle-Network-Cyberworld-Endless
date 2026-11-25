/* A Soul's abilities in BN6's engine (soulpower.h). */
#include "soulpower.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "bn6_attacks.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "soulbn4.h"
#include "soulchaos.h"
#include "soulcode.h"
#include "soullife.h"
#include "soulteam.h"
#include "thumb_reloc.h"
#include "xchips_registry.h"
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
/* Team ProtoMan's (soulteam.h), from SOUL_POWER_TEAM, a fixed place past room for BN4's twelve so that neither moves the
 * other's (the gap between stubs): their empty ones (ProtoSoul's, NapalmSoul's and MagnetSoul's A, their chip charge), their
 * chaos ones (stubs until their Chaos Unison is moved), then their moved ones in the recipe's order */
static const uint8_t team_empty_powers[] = { 0x05, 0x27, 0x16 };
static const uint8_t team_chaos_powers[] = { 0x06, 0x39, 0x30, 0x37, 0x35, 0x36 };
#define TEAM_AT (SOUL_POWER_TEAM - SOUL_POWER_FIRST)   /* their first entry past BN6's own */
#define TEAM_CHAOS_AT (TEAM_AT + (int)sizeof team_empty_powers)
#define TEAM_MOVED_AT (TEAM_CHAOS_AT + (int)sizeof team_chaos_powers)
/* BN4's Souls' own power attacks past their charged shots, at a fixed place past Team ProtoMan's: two by k, its chip charge where
 * no empty power of BN5's has its charge times (an empty one, its charge times BN4's: FireSoul's 50), then a plain shot or a
 * B-left of its own (none yet) */
#define BN4_MORE_AT (SOUL_POWER_BN4_MORE - SOUL_POWER_FIRST)
#define BN4_MORE_EACH 2
#define BN4_MORE_END (BN4_MORE_AT + BN4_MORE_EACH * SOULS_HELD_BN4_ALL)   /* (its second: his plain shot, MetlSoul's) */
enum { MORE_CHIP, MORE_OTHER };
static bool bn4_chip_own(const SoulBn4 *s);
/* ... their chaos ones' DarkChips (BN5's 0x0800F660, 0x0800FA86, 0x0800FA2C, 0x0800FA72, 0x0800FA5E, 0x0800FA68: DrkSword, DarkTorn,
 * DarkCirc, DarkMetr, DarkThnd, DrkRecov), each a stub of ours at SOULS_TEAM_CHAOS as Team Colonel's are Chaos Unison's Thumb */
static const uint16_t team_chaos_chips[] = { 0xBC, 0xC5, 0xBB, 0xC3, 0xC1, 0xC2 };
#define CHAOS_STUB 24        /* a stub's bytes: push {lr}; the chip; the writer; bl a bx r3; pop {pc}; its two words */
_Static_assert(SOULS_TEAM_CHAOS + CHAOS_STUB * sizeof team_chaos_chips / sizeof *team_chaos_chips <= SOULS_TEAM_CHAOS_END,
	"Team ProtoMan's chaos stubs past their room (soul_room.h)");
static unsigned team_chaos_on;   /* bit k: team_chaos_powers[k]'s stub written (its DarkChip registered, Chaos Unison installed) */

static int breaks;
static int nmoved;   /* the recipe's power attacks */
static int nchaos;   /* the chaos power attacks in the table (none where Chaos Unison is not installed) */
static SoulBn4 bn4s[SOULS_BN4];   /* BN4's Souls (soulbn4.h), their power attacks after the chaos ones, */
static int nbn4;                  /* ... how many are placed (none where BN4's ROM is not read or their code is not moved) */
static int nteam;                 /* Team ProtoMan's moved power attacks, after BN4's */
static unsigned team_on;          /* bit n - 1: Team ProtoMan's Soul n stands (its charged shot moved, its row written) */

bool soulpower_team(int soul) { return soulteam_is(soul) && (team_on >> (soul - SOUL_TEAM_FIRST) & 1); }

/* the Soul MegaMan `obj` is united with: his own (a side's partner's in his block, soul_id_of), BN5's or a placed BN4's */
static int soul_on(uint32_t obj) {
	int soul = soul_id_of(obj), k = soulbn4_k(soul);
	return (soul >= SOUL_FIRST && soul < SOUL_FIRST + SOULS) || (k >= 0 && k < nbn4) || soulpower_team(soul) ? soul : 0;
}

/* its row of the Souls' loadouts: BN5's six by number, then BN4's */
static int row_of(int soul) { return soulbn4_k(soul) >= 0 ? SOULS + soulbn4_k(soul) : soul - SOUL_FIRST; }

bool soulpower_bn4(int k) { return k >= 0 && k < nbn4; }

/* the grown table's Souls' entries in order: Team Colonel's moved, their empty and chaos ones, BN4's, Team ProtoMan's moved
 * and empty ones; where Team ProtoMan's begin */
static int team_first(void) { return nmoved + (int)sizeof empty_powers + nchaos + nbn4; }
_Static_assert(8 + 3 + 6 + SOULS_HELD_BN4_ALL == TEAM_AT, "Team ProtoMan's power attacks past room for Team Colonel's and BN4's (soul_room.h)");

/* BN6's id of BN5's power attack `id5` in the grown table, NONE where it is none of the Souls' */
static int power6(int id5) {
	int p = soulcode_power(id5);
	if (p >= 0) return p;
	for (size_t i = 0; i < sizeof empty_powers; ++i)
		if (empty_powers[i] == id5) return SOUL_POWER_FIRST + nmoved + (int)i;
	for (int i = 0; i < nchaos; ++i)
		if (chaos_powers[i] == id5) return SOUL_POWER_FIRST + nmoved + (int)sizeof empty_powers + i;
	const SoulCode *c = &soulcode_bn5;
	for (int i = 0; i < nteam; ++i)
		if (c->powers[c->npowers_colonel + i].id == id5) return SOUL_POWER_FIRST + TEAM_MOVED_AT + i;
	for (size_t i = 0; nteam && i < sizeof team_empty_powers; ++i)
		if (team_empty_powers[i] == id5) return SOUL_POWER_FIRST + TEAM_AT + (int)i;
	for (size_t i = 0; i < sizeof team_chaos_powers; ++i)
		if (team_chaos_powers[i] == id5 && (team_chaos_on >> i & 1)) return SOUL_POWER_FIRST + TEAM_CHAOS_AT + (int)i;
	return NONE;
}

/* BN5's id and the routine of the grown table's Souls' entry i (the moved, the empty, the chaos ones), 0 where none */
static uint32_t routine_of(int i, int *id5) {
	/* (BN5's id -1 for a BN4 Soul's) */
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
	if (i < nmoved + empty + nchaos) {
		*id5 = chaos_powers[i - nmoved - empty];
		return soulchaos_power(*id5);
	}
	if (i < team_first()) {
		*id5 = -1;   /* (BN4's: its charge times its own row, not BN5's) */
		return bn4s[i - nmoved - empty - nchaos].routine | 1;
	}
	if (i >= TEAM_CHAOS_AT && i < TEAM_MOVED_AT && (team_chaos_on >> (i - TEAM_CHAOS_AT) & 1)) {
		*id5 = team_chaos_powers[i - TEAM_CHAOS_AT];
		return (SOULS_TEAM_CHAOS + CHAOS_STUB * (uint32_t)(i - TEAM_CHAOS_AT)) | 1;
	}
	if (i < TEAM_AT || (i >= TEAM_CHAOS_AT && i < TEAM_MOVED_AT)) {
		*id5 = -2;   /* (a gap before Team ProtoMan's, or a chaos one of theirs not written: never named, its charge times none) */
		return BN6_POWER_EMPTY | 1;
	}
	if (i < TEAM_CHAOS_AT) {
		*id5 = team_empty_powers[i - TEAM_AT];
		return BN6_POWER_EMPTY | 1;
	}
	if (i >= BN4_MORE_AT && i < BN4_MORE_END) {
		/* (BN4's own: an empty one where its Soul's chip charge takes one, its charge times BN4's; his plain shot's copy) */
		int k = (i - BN4_MORE_AT) / BN4_MORE_EACH, role = (i - BN4_MORE_AT) % BN4_MORE_EACH;
		if (role == MORE_OTHER && k < nbn4 && (bn4s[k].plain_routine || bn4s[k].left_routine)) {
			*id5 = -4;
			return (bn4s[k].plain_routine ? bn4s[k].plain_routine : bn4s[k].left_routine) | 1;
		}
		*id5 = role == MORE_CHIP && k < nbn4 && bn4_chip_own(&bn4s[k]) ? -3 : -2;
		return BN6_POWER_EMPTY | 1;
	}
	if (i >= TEAM_MOVED_AT + nteam) {
		*id5 = -2;   /* (a gap past Team ProtoMan's) */
		return BN6_POWER_EMPTY | 1;
	}
	*id5 = c->powers[c->npowers_colonel + i - TEAM_MOVED_AT].id;
	return soulcode_moved(c->powers[c->npowers_colonel + i - TEAM_MOVED_AT].entry);
}

/* ---- the hooks: MegaMan's loadout, element, weakness and buster bonus in a Soul ---- */

/* (the loadout's read, r0 its row by the form, r5 MegaMan) the Soul's row */
static HookAct loadout(HookRegs *r, void *user) {
	(void)user;
	int soul = soul_on(r->r[5]);
	if (!soul || r->r[0] != BN6_FORM_LOADOUTS + BN6_FORM_LOADOUT_SIZE * BN6_FORM_PROXY) return HOOK_CONTINUE;
	r->r[0] = soulteam_is(soul) ? SOULS_TEAM_LOADOUTS + (uint32_t)(BN6_FORM_LOADOUT_SIZE * (soul - SOUL_TEAM_FIRST)) :
		SOULS_LOADOUTS + (uint32_t)(BN6_FORM_LOADOUT_SIZE * row_of(soul));
	return HOOK_CONTINUE;
}

/* (the element's read, r0 the form, r5 him) the Soul's (BN5's 0x0800E62C), on to the routine's next step */
static HookAct element(HookRegs *r, void *user) {
	int soul = soul_on(r->r[5]);
	if (!soul || r->r[0] != BN6_FORM_PROXY) return HOOK_CONTINUE;
	int k = soulbn4_k(soul);
	r->r[0] = (breaks & SOULPOWER_BREAK_ELEMENT) ? 0 : k >= 0 ? bn4s[k].element : XR[XROM_BN5_COLONEL_US].data[BN5_SOUL_ELEMENTS - BUS + (uint32_t)soul];
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* a read by the form a Soul answers 0 to: on to `next`, the value in register `reg` */
typedef struct { uint32_t next; int reg; } Zero;
static Zero weakness = { BN6_FORM_WEAKNESS_NEXT, 0 }, bonus = { BN6_FORM_BONUS_NEXT, 1 };

/* (the second wheel's weakness read, the buster bonus read: r0 the form, r5 him) none for a Soul */
static HookAct none_for_soul(HookRegs *r, void *user) {
	const Zero *z = user;
	if (!soul_on(r->r[5]) || r->r[0] != BN6_FORM_PROXY) return HOOK_CONTINUE;
	r->r[z->reg] = 0;
	r->r[12] = z->next | 1;
	return HOOK_JUMP;
}

/* ---- the tables ---- */

static uint32_t rom5(uint32_t a) { return get32(XR[XROM_BN5_COLONEL_US].data + (a - BUS)); }

/* Team ProtoMan's Chaos Unison's power attacks, a stub each at SOULS_TEAM_CHAOS as BN5's (0x0800FA72: push {lr}; movs r0,
 * #DarkChip; bl the chip writer's second entry; pop {pc}), the DarkChip in BN6's ids by the registry: where Team Colonel's are
 * (Chaos Unison installed) and its DarkChip is registered; none written otherwise (their rows' chaos none) */
static void team_chaos(void) {
	uint8_t out[CHAOS_STUB * sizeof team_chaos_chips / sizeof *team_chaos_chips];
	static const uint16_t code[] = {
		0xB500,           /* push {lr} */
		0x4803,           /* ldr r0,[pc,#12]: its DarkChip */
		0x4B03,           /* ldr r3,[pc,#12]: the chip writer's second entry */
		0, 0,             /* bl the bx r3 below */
		0xBD00,           /* pop {pc} */
		0x4718,           /* bx r3 */
		0x46C0,           /* nop */
	};
	team_chaos_on = 0;
	memset(out, 0xFF, sizeof out);
	for (size_t k = 0; nchaos && rom_u16(BN6_CHIP_WRITE_NOLOG - BUS) == 0xB5D0 /* push {r4,r6,r7,lr} */ && k < sizeof team_chaos_chips / sizeof *team_chaos_chips; ++k) {
		const XChipReg *row = xreg_of_bn5(team_chaos_chips[k]);
		uint8_t *p = out + CHAOS_STUB * k;
		uint32_t at = SOULS_TEAM_CHAOS + CHAOS_STUB * (uint32_t)k;
		if (!row) continue;
		for (size_t i = 0; i < sizeof code / sizeof *code; ++i) put16(p + 2 * i, code[i]);
		put32(p + 16, row->bn6);
		put32(p + 20, BN6_CHIP_WRITE_NOLOG | 1);
		if (thumb_bl_write(p, 6, at, at + 12)) team_chaos_on |= 1u << k;
	}
	if (team_chaos_on) emu_write(SOULS_TEAM_CHAOS, out, sizeof out);
}

/* the charge times BN4's Souls give grown entry i: id5 -1 a Soul's charged shot's, -3 his chip charge's and -4 his plain
 * shot's on BN4's own power attacks (BN4_MORE_AT on, BN4_MORE_EACH a Soul); NULL where none (a gap's, -2) */
static const uint8_t *bn4_charge_of(int i, int id5) {
	bool own = id5 == -3 || id5 == -4;
	int k = own ? (i - BN4_MORE_AT) / BN4_MORE_EACH : i - nmoved - (int)sizeof empty_powers - nchaos;
	if ((id5 != -1 && !own) || (own && i < BN4_MORE_AT) || k < 0 || k >= nbn4) return NULL;
	return id5 == -1 ? bn4s[k].charge : id5 == -3 ? bn4s[k].chip_charge : bn4s[k].plain_routine ? bn4s[k].plain_charge : bn4s[k].left_charge;
}

/* the grown power attacks' and charge times' entries past BN6's own, n in all; false where one has no routine */
static bool grown(int n, uint8_t powers[4 * 0x100], uint8_t charges[BN6_CHARGE_ROW * 0x100]) {
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	memcpy(powers, R.data + (BN6_POWER_ATTACKS - BUS), 4 * BN6_POWER_ATTACK_COUNT);
	memcpy(charges, R.data + (BN6_CHARGE_TIMES - BUS), BN6_CHARGE_ROW * BN6_POWER_ATTACK_COUNT);
	for (int i = 0; i < n - BN6_POWER_ATTACK_COUNT; ++i) {
		int id5 = 0;
		uint32_t routine = routine_of(i, &id5);
		if (!routine) return false;
		put32(powers + 4 * (BN6_POWER_ATTACK_COUNT + i), routine);
		uint8_t *row = charges + BN6_CHARGE_ROW * (BN6_POWER_ATTACK_COUNT + i);
		const uint8_t *from4 = id5 < 0 ? bn4_charge_of(i, id5) : NULL;
		if (from4) memcpy(row, from4, BN6_CHARGE_ROW);
		else if (id5 < 0) memset(row, 0, BN6_CHARGE_ROW);
		else memcpy(row, b5 + (BN5_CHARGE_TIMES - BUS) + BN6_CHARGE_ROW * (uint32_t)id5, BN6_CHARGE_ROW);
		for (int k = 0; (breaks & SOULPOWER_BREAK_CHARGE) && k < BN6_CHARGE_ROW / 2; ++k) put16(row + 2 * k, get16(row + 2 * k) + 10);
	}
	return true;
}

/* Team Colonel's and BN4's Souls' loadouts into rows; false where one of Team Colonel's names a power attack not there */
/* BN5's empty power whose charge times are a BN4 Soul's chip charge's (BN4's empty routines, their charge times alone serving),
 * its index in empty_powers, -1 none */
static int bn5_empty_of(const SoulBn4 *s) {
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	for (size_t i = 0; s->chip != NONE && i < sizeof empty_powers; ++i)
		if (!memcmp(b5 + (BN5_CHARGE_TIMES - BUS) + BN6_CHARGE_ROW * (uint32_t)empty_powers[i], s->chip_charge, BN6_CHARGE_ROW))
			return (int)i;
	return -1;
}

/* whether a BN4 Soul's chip charge takes an empty power of BN4's own (BN4_MORE_AT's), no BN5 one having its times */
static bool bn4_chip_own(const SoulBn4 *s) { return s->chip != NONE && bn5_empty_of(s) < 0; }

/* A BN4 Soul's chip charge, BN6's id: BN5's empty power of the same charge times (AquaSoul's 20 at every Charge: ToadSoul's
 * 0x1B), else BN4's own of the k-th (FireSoul's 50); NONE where it has none */
static int bn4_chip_power(int k) {
	const SoulBn4 *s = &bn4s[k];
	int i = bn5_empty_of(s);
	if (s->chip == NONE) return NONE;
	return i >= 0 ? power6(empty_powers[i]) : SOUL_POWER_FIRST + BN4_MORE_AT + BN4_MORE_EACH * k + MORE_CHIP;
}

_Static_assert(SOULS_LOADOUTS + BN6_FORM_LOADOUT_SIZE * (SOULS + SOULS_HELD_BN4_ALL) <= SOULS_LOADOUTS_END,
	"the Souls' loadouts' room holds BN5's six and BN4's twelve (soul_room.h)");
static bool rows_make(uint8_t rows[BN6_FORM_LOADOUT_SIZE * (SOULS + SOULS_BN4)]) {
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	/* (each Soul's loadout: BN5's row in BN6's ids, BN5's plain shot BN6's own 1, the same charge times) */
	memset(rows, NONE, BN6_FORM_LOADOUT_SIZE * (SOULS + SOULS_BN4));
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
	/* (each BN4 Soul's: its chip charge as BN5's empty power of the same charge times (bn4_chip_power), BN6's plain shot (+6 0,
	 * as every form's), its charged shot after the chaos ones, no B-left ability, no Chaos Unison: BN4_FORM_LOADOUTS' four bytes
	 * as BN6's six) */
	for (int k = 0; k < nbn4; ++k) {
		uint8_t *r6 = rows + BN6_FORM_LOADOUT_SIZE * (SOULS + k);
		r6[ROW_A] = (uint8_t)bn4_chip_power(k);
		/* (his own other power, on BN4's own power attacks: his plain shot's, MetlSoul's, or his B and left's, ProtSoul's
		 * shield; a Soul has one of them at most) */
		uint8_t other = (uint8_t)(SOUL_POWER_FIRST + BN4_MORE_AT + BN4_MORE_EACH * k + MORE_OTHER);
		r6[ROW_6] = bn4s[k].plain_routine ? other : 0;
		if (bn4s[k].left_routine && !bn4s[k].plain_routine) r6[ROW_LEFT] = other;
		r6[ROW_B] = (uint8_t)(BN6_POWER_ATTACK_COUNT + nmoved + (int)sizeof empty_powers + nchaos + k);
	}
	return true;
}

/* Team ProtoMan's Souls whose charged shot is moved: their rows as Team Colonel's, their chaos where its stub is written;
 * each Soul's whole or not at all, the ones that stand in team_on */
static void team_rows(uint8_t team[BN6_FORM_LOADOUT_SIZE * SOULS_TEAM]) {
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	memset(team, NONE, BN6_FORM_LOADOUT_SIZE * SOULS_TEAM);
	team_on = 0;
	for (int k = 0; nteam && k < SOULS_TEAM; ++k) {
		const uint8_t *r5 = b5 + (BN5_SOUL_LOADOUTS - BUS) + BN6_FORM_LOADOUT_SIZE * (uint32_t)(SOUL_TEAM_FIRST + k);
		uint8_t *r6 = team + BN6_FORM_LOADOUT_SIZE * k;
		int a = r5[ROW_A] == BN5_POWER_NORMAL ? BN5_POWER_NORMAL : power6(r5[ROW_A]), b = power6(r5[ROW_B]);
		int left = r5[ROW_LEFT] == NONE ? NONE : power6(r5[ROW_LEFT]);
		if (a == NONE || b == NONE || (r5[ROW_LEFT] != NONE && left == NONE)) continue;
		r6[ROW_4] = r5[ROW_4];
		r6[ROW_A] = (uint8_t)a;
		r6[ROW_6] = r5[ROW_6];
		r6[ROW_B] = (uint8_t)b;
		r6[ROW_LEFT] = (uint8_t)left;
		r6[ROW_11] = r5[ROW_11] == NONE ? NONE : (uint8_t)power6(r5[ROW_11]);   /* (NONE where its chaos stub is not written) */
		if (breaks & SOULPOWER_BREAK_FAMILY) r6[ROW_B] = (uint8_t)(SOUL_POWER_FIRST + (r6[ROW_B] - SOUL_POWER_FIRST + 1) % nmoved);
		team_on |= 1u << k;
	}
}

/* BN6's power attacks and charge times grown with the Souls', their words pointed at the copies; the Souls' rows */
static bool tables(void) {
	const SoulCode *c = &soulcode_bn5;
	nchaos = soulchaos_power(chaos_powers[0]) ? (int)sizeof chaos_powers : 0;
	/* (BN4's Souls where BN4's ROM is read and their code moved, in their order: each one's whole or none from it on) */
	for (nbn4 = 0; nbn4 < SOULS_BN4 && soulbn4_read(nbn4, &bn4s[nbn4]); ++nbn4) {}
	nmoved = c->npowers_colonel;
	nteam = c->npowers - c->npowers_colonel;
	if (nteam) team_chaos();
	int n = BN6_POWER_ATTACK_COUNT + (nteam ? TEAM_MOVED_AT + nteam : team_first());
	/* (BN4's own past Team ProtoMan's, where a BN4 Soul stands) */
	if (n > SOUL_POWER_BN4_MORE) return false;
	if (nbn4) n = SOUL_POWER_FIRST + BN4_MORE_END;
	if (SOUL_POWER_FIRST != BN6_POWER_ATTACK_COUNT || SOULS_POWERS + 4u * (uint32_t)n > SOULS_POWERS_END ||
		SOULS_CHARGES + BN6_CHARGE_ROW * (uint32_t)n > SOULS_CHARGES_END || n > NONE ||
		SOULS_LOADOUTS + BN6_FORM_LOADOUT_SIZE * (uint32_t)(SOULS + SOULS_BN4) > SOULS_LOADOUTS_END)
		return false;
	uint8_t powers[4 * 0x100], charges[BN6_CHARGE_ROW * 0x100], rows[BN6_FORM_LOADOUT_SIZE * (SOULS + SOULS_BN4)];
	uint8_t team[BN6_FORM_LOADOUT_SIZE * SOULS_TEAM];
	if (!grown(n, powers, charges) || !rows_make(rows)) return false;
	team_rows(team);
	emu_write(SOULS_POWERS, powers, 4u * (uint32_t)n);
	if (team_on) emu_write(SOULS_TEAM_LOADOUTS, team, sizeof team);
	emu_write(SOULS_CHARGES, charges, BN6_CHARGE_ROW * (uint32_t)n);
	emu_write(SOULS_LOADOUTS, rows, BN6_FORM_LOADOUT_SIZE * (uint32_t)(SOULS + nbn4));
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
	/* (Team ProtoMan's played by the Souls' own hooks and Thumb: soulteam_chips.c's propeller, soulcapsule.c's A on a grey capsule) */
	return !nteam || (xsong_install_at(XROM_BN5_COLONEL_US, SOUL_TEAM_PROPELLER_SONG, SOUL_TEAM_PROPELLER_SLOT) == SOUL_TEAM_PROPELLER_SLOT &&
		xsong_install_at(XROM_BN5_COLONEL_US, SOUL_TEAM_GREY_SONG, SOUL_TEAM_GREY_SLOT) == SOUL_TEAM_GREY_SLOT);
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
	int more = id - SOUL_POWER_FIRST - BN4_MORE_AT;
	return (nmoved && id >= SOUL_POWER_FIRST + nmoved && id < SOUL_POWER_FIRST + nmoved + (int)sizeof empty_powers) ||
		(team_on && id >= SOUL_POWER_TEAM && id < SOUL_POWER_TEAM + (int)sizeof team_empty_powers) ||
		(more >= 0 && more % BN4_MORE_EACH == MORE_CHIP && more / BN4_MORE_EACH < nbn4 && bn4_chip_own(&bn4s[more / BN4_MORE_EACH]));
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
	printf("soulpower: the Souls' abilities%s (BN4's: %d)%s\n", ok ? ": their power attacks, charge times, loadouts, elements and sounds" : " NOT placed whole", ok ? nbn4 : 0,
		brk ? " (soulbreak: parts of it broken, the differential's own test)" : "");
	return ok;
}
