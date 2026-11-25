/* BN4 Red Sun's Souls in BN6's engine (soulbn4.h). */
#include "soulbn4.h"

#include <string.h>

#include "bn4.h"
#include "bn4_navis.h"
#include "bn4_souls.h"
#include "bn6.h"
#include "bn6_chips.h"
#include "bn6_link.h"
#include "bytes.h"
#include "chips.h"
#include "hook.h"
#include "rom.h"
#include "soulkinds.h"
#include "soulpassive.h"
#include "soulroute.h"
#include "thumb_reloc.h"
#include "xchips_code.h"

#define BUS 0x08000000u
#define NONE 0xFF
#define FORMS 16                /* BN4_FORM_LOADOUTS', BN4_FORM_ELEMENTS' rows: forms 0-15 */
#define CHARGE_ROW 10           /* BN4_CHARGE_TIMES' row, a power attack's (BN6_CHARGE_ROW) */
/* a loadout row's bytes (BN4_FORM_LOADOUTS): +1 the plain shot's power attack, +2 the chip charge's, +3 the charged shot's */
enum { ROW4_0, ROW4_PLAIN, ROW4_CHIP, ROW4_CHARGED };
#define POWER_PLAIN4 0x00       /* BN4's plain shot (form 0's +1): BN6's own +6 byte, 0, for every form */

/* BN6's BN4 Souls by k: BN4's Soul, its form (ThunSoul 6, GutsSoul 2, AquaSoul 11, SrchSoul 4, NumSoul 8, FireSoul 5, MetlSoul 9,
 * WoodSoul 12, WindSoul 3, ProtSoul 7, RollSoul 1, JunkSoul 10) */
static const uint8_t forms[SOULS_BN4] = { 6, 2, 11, 4, 8, 5, 9, 12, 3, 7, 1, 10 };
#define FORM_GUTS 2
#define FORM_NUM 8
#define FORM_AQUA 11
#define FORM_FIRE 5
#define FORM_METL 9
#define FORM_WOOD 12
#define FORM_WIND 3
#define FORM_PROT 7
#define OP_MOVS_R1 0x2100u   /* movs r1,#imm8 */
#define OP_MOVS_R0 0x2000u   /* movs r0,#imm8 */
#define OP_MOVS_R4 0x2400u   /* movs r4,#imm8 */
#define OP_BEQ_NEXT 0xD001u  /* beq past the next bl */
#define OP_CMP_R1_B 0x2900u   /* cmp r1,#imm8 (the bonus's kinds) */
#define OP_BL_HI 0xF000u      /* a bl's first half */
#define FORM_THUN 6
#define OP_CMP_R0 0x2800u    /* cmp r0,#imm8 */
#define OP_CMP_R2 0x2A00u    /* cmp r2,#imm8 */
#define OP_CMP_R4 0x2C00u    /* cmp r4,#imm8 */
#define OP_CMP_R1 0x2900u    /* cmp r1,#imm8 */
#define OP_ADDS_R1 0x3100u   /* adds r1,#imm8 */
#define HAND_END 0xFFFFu     /* a hand's ids after its last */

unsigned soulbn4_held(unsigned bn4) {
	unsigned out = 0;
	for (int k = 0; k < SOULS_BN4; ++k)
		if (bn4 >> (forms[k] - 1) & 1) out |= 1u << (SOULS_HELD_BN4 + k);
	return out;
}

int soulbn4_form(int k) { return k >= 0 && k < SOULS_BN4 ? forms[k] : 0; }

/* AquaSoul's rule in BN4's code, its numbers read there: false where the code is not that */
static bool aqua_rule(int *kind) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!b4) return false;
	uint16_t tf = get16(b4 + (BN4_AQUA_TEST_FORM - BUS)), tk = get16(b4 + (BN4_AQUA_TEST_KIND - BUS));
	uint16_t bf = get16(b4 + (BN4_AQUA_BONUS_FORM - BUS)), bk = get16(b4 + (BN4_AQUA_BONUS_KIND - BUS));
	if (tf != (OP_CMP_R0 | FORM_AQUA) || bf != (OP_CMP_R2 | FORM_AQUA) || (tk & 0xFF00u) != OP_CMP_R4 || bk != (0x2900u | (tk & 0xFF)))
		return false;
	*kind = tk & 0xFF;
	return true;
}

/* FireSoul's rule in BN4's code (BN4_FIRE_TEST_FORM, BN4_FIRE_TEST_KIND): his chips' kind, no flag tested; false where the
 * code is not that */
static bool fire_rule(int *kind) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!b4) return false;
	uint16_t f = get16(b4 + (BN4_FIRE_TEST_FORM - BUS)), k = get16(b4 + (BN4_FIRE_TEST_KIND - BUS));
	if (f != (OP_CMP_R0 | FORM_FIRE) || (k & 0xFF00u) != OP_CMP_R4) return false;
	*kind = k & 0xFF;
	return true;
}

/* MetlSoul's rules in BN4's code: the test's two kinds, its flags (not left alone, its power shown), and the bonus's same two;
 * false where the code is not that */
static bool metl_rule(int *kind1, int *kind2) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!b4) return false;
	uint16_t f = get16(b4 + (BN4_METL_TEST_FORM - BUS)), k1 = get16(b4 + (BN4_METL_TEST_KIND - BUS)), k2 = get16(b4 + (BN4_METL_TEST_KIND2 - BUS));
	uint16_t bf = get16(b4 + (BN4_METL_BONUS_FORM - BUS)), b1 = get16(b4 + (BN4_METL_BONUS_KIND - BUS)), b2 = get16(b4 + (BN4_METL_BONUS_KIND2 - BUS));
	if (f != (OP_CMP_R0 | FORM_METL) || (k1 & 0xFF00u) != OP_CMP_R4 || (k2 & 0xFF00u) != OP_CMP_R4 || bf != (OP_CMP_R2 | FORM_METL) ||
		b1 != (OP_CMP_R1_B | (k1 & 0xFF)) || b2 != (OP_CMP_R1_B | (k2 & 0xFF)))
		return false;
	*kind1 = k1 & 0xFF;
	*kind2 = k2 & 0xFF;
	return true;
}

/* ProtSoul's rules in BN4's code: the test's kind (BN4_PROT_TEST_KIND, a chip not left alone) and the bonus's the same kind;
 * false where the code is not that */
static bool prot_rule(int *kind) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!b4) return false;
	uint16_t f = get16(b4 + (BN4_PROT_TEST_FORM - BUS)), k = get16(b4 + (BN4_PROT_TEST_KIND - BUS));
	uint16_t bf = get16(b4 + (BN4_PROT_BONUS_FORM - BUS)), bk = get16(b4 + (BN4_PROT_BONUS_KIND - BUS));
	if (f != (OP_CMP_R0 | FORM_PROT) || (k & 0xFF00u) != OP_CMP_R4 || bf != (OP_CMP_R2 | FORM_PROT) || bk != (OP_CMP_R1_B | (k & 0xFF)))
		return false;
	*kind = k & 0xFF;
	return true;
}

/* the kind a Soul charges chips of that are not left alone, AquaSoul's and ProtSoul's, by his form's rule; -1 none */
static int alone_kind(int form) {
	int kind;
	if (form == FORM_AQUA && aqua_rule(&kind)) return kind;
	if (form == FORM_PROT && prot_rule(&kind)) return kind;
	return -1;
}

bool soulbn4_charges(int k, int chip) {
	int kind, kind2;
	if (k >= 0 && k < SOULS_BN4 && forms[k] == FORM_FIRE && fire_rule(&kind)) return chip > 0 && soulkinds_bn4_of(chip) == kind;
	if (k >= 0 && k < SOULS_BN4 && forms[k] == FORM_METL && metl_rule(&kind, &kind2)) {
		uint8_t flags = chip > 0 ? hook_read8(chips_table_bus() + (uint32_t)chip * BN6_CHIP_RECORD_SIZE + BN6_CHIP_EFFECT_FLAGS) : 0;
		int kd = soulkinds_bn4_of(chip);
		return chip > 0 && !(flags & BN6_CHIP_LEFT_ALONE) && (flags & BN6_CHIP_SHOWS_POWER) && (kd == kind || kd == kind2);
	}
	if (k < 0 || k >= SOULS_BN4 || (kind = alone_kind(forms[k])) < 0) return false;
	uint32_t rec = chips_table_bus() + (uint32_t)chip * BN6_CHIP_RECORD_SIZE;
	return chip > 0 && !(hook_read8(rec + BN6_CHIP_EFFECT_FLAGS) & BN6_CHIP_LEFT_ALONE) && soulkinds_bn4_of(chip) == kind;
}

/* ThunSoul's uncharged bonus in BN4's code (BN4_CHIP_BONUS): its form and two kinds; false where the code is not that */
static bool thun_rule(int *kind1, int *kind2) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!b4) return false;
	uint16_t f = get16(b4 + (BN4_THUN_BONUS_FORM - BUS)), k1 = get16(b4 + (BN4_THUN_BONUS_KIND - BUS)), k2 = get16(b4 + (BN4_THUN_BONUS_KIND2 - BUS));
	if (f != (OP_CMP_R2 | FORM_THUN) || (k1 & 0xFF00u) != OP_CMP_R1 || (k2 & 0xFF00u) != OP_CMP_R1) return false;
	*kind1 = k1 & 0xFF;
	*kind2 = k2 & 0xFF;
	return true;
}

unsigned soulbn4_chip_bonus(int k, int chip) {
	int kind1, kind2;
	if (k < 0 || k >= SOULS_BN4 || forms[k] != FORM_THUN || chip <= 0 || !thun_rule(&kind1, &kind2)) return 0;
	uint8_t flags = hook_read8(chips_table_bus() + (uint32_t)chip * BN6_CHIP_RECORD_SIZE + BN6_CHIP_EFFECT_FLAGS);
	int kind = soulkinds_bn4_of(chip);
	return (flags & BN6_CHIP_SHOWS_POWER) && !(flags & BN6_CHIP_LEFT_ALONE) && (kind == kind1 || kind == kind2) ? BN4_PARALYSING : 0;
}

unsigned soulbn4_charge_bonus(int k) {
	int kind, kind2;
	/* (BN4_CHIP_CHARGE_BONUS by form: AquaSoul's Aqua, ProtSoul's Sword, MetlSoul's two kinds; the chips he charges are those
	 * alone) */
	return k >= 0 && k < SOULS_BN4 && (alone_kind(forms[k]) >= 0 || (forms[k] == FORM_METL && metl_rule(&kind, &kind2))) ?
		BN4_CHARGED_DOUBLE : 0;
}

uint32_t soulbn4_charged_copy(int k) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	uint32_t at;
	/* (BN4's chip use: its test of his form, then its bl to his attack where the chip's setup would be) */
	if (!b4 || k < 0 || k >= SOULS_BN4 || forms[k] != FORM_FIRE || get16(b4 + (BN4_CHARGED_FORM - BUS)) != (OP_CMP_R0 | FORM_FIRE) ||
		(get16(b4 + (BN4_FIRE_CHARGED_CALL - BUS)) & 0xF800u) != OP_BL_HI || !thumb_bl_read(b4 + (BN4_FIRE_CHARGED_CALL - BUS), 0,
		BN4_FIRE_CHARGED_CALL, &at) || at != BN4_FIRE_CHARGED)
		return 0;
	return xchips_bn4_moved_at(BN4_FIRE_CHARGED | 1u);
}

uint32_t soulbn4_enter_copy(int k) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!b4 || k < 0 || k >= SOULS_BN4) return 0;
	return xchips_bn4_moved_at(get32(b4 + (BN4_FORM_ENTERS - BUS) + 4u * forms[k]) | 1u);
}

int soulbn4_k(int soul) { return soul >= SOUL_BN4_FIRST && soul < SOUL_BN4_FIRST + SOULS_BN4 ? soul - SOUL_BN4_FIRST : -1; }

/* a power attack's routine one of BN4's empty ones (its charge times alone serve) */
static bool empty4(uint32_t routine) { return routine == (BN4_POWER_EMPTY | 1) || routine == (BN4_POWER_EMPTY2 | 1); }

/* BN4's power attack `power` as BN6 runs it: its code moved with a unit, else its copy as a chip's shot (soulroute.c); 0 none */
static uint32_t power_routine(const uint8_t *b4, int power) {
	uint32_t moved = xchips_bn4_moved_at(get32(b4 + (BN4_POWER_ATTACKS - BUS) + 4u * (uint32_t)power));
	return moved ? moved : soulroute_at(power);
}

bool soulbn4_read(int k, SoulBn4 *out) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	memset(out, 0, sizeof *out);
	if (!b4 || k < 0 || k >= SOULS_BN4) return false;
	int form = forms[k];
	const uint8_t *row = b4 + (BN4_FORM_LOADOUTS - BUS) + BN4_FORM_LOADOUT_SIZE * (uint32_t)form;
	/* (the shape the move reads: no plain shot or chip charge of its own, a charged shot among BN4's power attacks; the moved
	 * Souls' others are later ones') */
	/* (a chip charge only where its power attack is one of BN4's empty routines, its charge times serving alone: AquaSoul's,
	 * FireSoul's) */
	bool chip = row[ROW4_CHIP] != NONE;
	/* (a plain shot of his own where its code is moved with his unit, MetlSoul's, or is a chip's shot, WindSoul's AirShot) */
	uint32_t plain = row[ROW4_PLAIN] != POWER_PLAIN4 && row[ROW4_PLAIN] < BN4_POWER_ATTACK_COUNT ? power_routine(b4, row[ROW4_PLAIN]) : 0;
	if (form < 1 || form >= FORMS || row[ROW4_0] != NONE || (row[ROW4_PLAIN] != POWER_PLAIN4 && !plain) || row[ROW4_CHARGED] >= BN4_POWER_ATTACK_COUNT ||
		(chip && (row[ROW4_CHIP] >= BN4_POWER_ATTACK_COUNT || !empty4(get32(b4 + (BN4_POWER_ATTACKS - BUS) + 4u * row[ROW4_CHIP])))))
		return false;
	out->chip = row[ROW4_CHIP];
	out->plain = row[ROW4_PLAIN];
	out->plain_routine = plain;
	if (plain) memcpy(out->plain_charge, b4 + (BN4_CHARGE_TIMES - BUS) + CHARGE_ROW * (uint32_t)out->plain, CHARGE_ROW);
	if (chip) memcpy(out->chip_charge, b4 + (BN4_CHARGE_TIMES - BUS) + CHARGE_ROW * (uint32_t)out->chip, CHARGE_ROW);
	out->form = (uint8_t)form;
	out->kind = b4[BN4_SOUL_KINDS - BUS + (uint32_t)(form - 1)];
	out->element = b4[BN4_FORM_ELEMENTS - BUS + (uint32_t)form];
	out->power = row[ROW4_CHARGED];
	out->routine = power_routine(b4, out->power);
	/* (his B and left's, where his taking of the form sets it: ProtSoul's shield) */
	out->left = NONE;
	if (form == FORM_PROT && get32(b4 + (BN4_FORM_ENTERS - BUS) + 4u * (uint32_t)form) == (BN4_PROT_ENTER | 1) &&
		(get16(b4 + (BN4_PROT_LEFT - BUS)) & 0xFF00u) == OP_MOVS_R1) {
		out->left = b4[BN4_PROT_LEFT - BUS];
		out->left_routine = out->left < BN4_POWER_ATTACK_COUNT ? power_routine(b4, out->left) : 0;
		if (!out->left_routine) return false;
		memcpy(out->left_charge, b4 + (BN4_CHARGE_TIMES - BUS) + CHARGE_ROW * (uint32_t)out->left, CHARGE_ROW);
	}
	memcpy(out->charge, b4 + (BN4_CHARGE_TIMES - BUS) + CHARGE_ROW * (uint32_t)out->power, CHARGE_ROW);
	return out->routine != 0;
}

/* The Souls whose taking of the form passes over the hand (BN4_FORM_ENTERS), each its routine's places: GutsSoul's +30 to chips
 * of no kind or panel-cracking, NumSoul's +10 to those of no kind, WindSoul's +10 to Wind chips (one kind: kind2 0) */
static const struct { uint8_t form; uint32_t enter, kind, kind2, add; } passes[] = {
	{ FORM_GUTS, BN4_GUTS_ENTER, BN4_GUTS_PASS_KIND, BN4_GUTS_PASS_KIND2, BN4_GUTS_PASS_ADD },
	{ FORM_NUM, BN4_NUM_ENTER, BN4_NUM_PASS_KIND, 0, BN4_NUM_PASS_ADD },
	{ FORM_WIND, BN4_WIND_ENTER, BN4_WIND_PASS_KIND, 0, BN4_WIND_PASS_ADD },
};

/* form's pass's numbers read from BN4's code: its kinds (kind2 -1 none) and its add; false where it has none, or the code is not
 * that */
static bool pass_of(int form, int *kind1, int *kind2, int *add) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	for (size_t i = 0; b4 && i < sizeof passes / sizeof *passes; ++i) {
		if (passes[i].form != form) continue;
		if (get32(b4 + (BN4_FORM_ENTERS - BUS) + 4u * (uint32_t)form) != (passes[i].enter | 1)) return false;
		uint16_t k1 = get16(b4 + (passes[i].kind - BUS)), a = get16(b4 + (passes[i].add - BUS));
		uint16_t k2 = passes[i].kind2 ? get16(b4 + (passes[i].kind2 - BUS)) : OP_CMP_R1 | 0xFFu;
		if ((k1 & 0xFF00u) != OP_CMP_R1 || (k2 & 0xFF00u) != OP_CMP_R1 || (a & 0xFF00u) != OP_ADDS_R1) return false;
		*kind1 = k1 & 0xFF;
		*kind2 = passes[i].kind2 ? (k2 & 0xFF) : -1;
		*add = a & 0xFF;
		return true;
	}
	return false;
}

/* whether chip (BN6's id) shows its power (BN4's +9 0x02, BN6's BN6_CHIP_SHOWS_POWER) */
static bool shows_power(uint16_t chip) {
	return hook_read8(chips_table_bus() + (uint32_t)chip * BN6_CHIP_RECORD_SIZE + BN6_CHIP_EFFECT_FLAGS) & BN6_CHIP_SHOWS_POWER;
}

/* WoodSoul's merge in BN4's code (BN4_WOOD_MERGE), its two kinds read there: false where form has none, or the code is not that */
static bool merge_rule(int form, int *kind1, int *kind2) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	uint32_t to;
	if (!b4 || form != FORM_WOOD || get32(b4 + (BN4_FORM_ENTERS - BUS) + 4u * (uint32_t)form) != (BN4_WOOD_ENTER | 1) ||
		!thumb_bl_read(b4 + (BN4_WOOD_ENTER + 4 - BUS), 0, BN4_WOOD_ENTER + 4, &to) || to != BN4_WOOD_MERGE)
		return false;
	uint16_t k1 = get16(b4 + (BN4_WOOD_MERGE_KIND - BUS)), k2 = get16(b4 + (BN4_WOOD_MERGE_KIND2 - BUS));
	if ((k1 & 0xFF00u) != OP_CMP_R1 || (k2 & 0xFF00u) != OP_CMP_R1) return false;
	*kind1 = k1 & 0xFF;
	*kind2 = k2 & 0xFF;
	return true;
}

/* a hand's u16 arrays by place: those the merge adds, and those a place's move takes (its id first); its u8 arrays */
static const uint8_t sums[] = { BN6_HAND_POWERS, BN6_HAND_BONUS, BN6_HAND_EXTRA };
static const uint8_t moves16[] = { BN6_HAND_CHIPS, BN6_HAND_POWERS, BN6_HAND_BONUS, BN6_HAND_EXTRA, BN6_HAND_ENTRIES };
static const uint8_t moves8[] = { BN6_HAND_SCREEN, BN6_HAND_MARKS };

/* the hand's chips from place j on moved up a place, its last emptied */
static void hand_up(uint32_t hand, uint32_t j) {
	for (; j + 1 < BN6_HAND_MAX; ++j) {
		for (size_t a = 0; a < sizeof moves16; ++a) hook_write16(hand + moves16[a] + 2 * j, hook_read16(hand + moves16[a] + 2 * (j + 1)));
		for (size_t a = 0; a < sizeof moves8; ++a) hook_write8(hand + moves8[a] + j, hook_read8(hand + moves8[a] + j + 1));
		if (hook_read16(hand + BN6_HAND_CHIPS + 2 * j) == HAND_END) return;
	}
	hook_write16(hand + BN6_HAND_CHIPS + 2 * j, HAND_END);
}

/* WoodSoul's merge of a hand (BN4_WOOD_MERGE): from the chip up next, each of kind1 whose next is of kind2 and shows its power
 * takes that chip's power, bonus and extra, and the chips after it move up over it. Its two u8 arrays move a place each,
 * as BN6 keeps them, where BN4 moves its by two bytes */
static void hand_merge(uint32_t hand, int kind1, int kind2) {
	for (uint32_t i = hook_read8(hand + BN6_HAND_AT); i + 1 < BN6_HAND_MAX; ++i) {
		uint16_t chip = hook_read16(hand + BN6_HAND_CHIPS + 2 * i), next = hook_read16(hand + BN6_HAND_CHIPS + 2 * (i + 1));
		if (chip == HAND_END || next == HAND_END) return;
		if (soulkinds_bn4_of(chip) != kind1 || soulkinds_bn4_of(next) != kind2 || !shows_power(next)) continue;
		for (size_t a = 0; a < sizeof sums; ++a) {
			uint32_t at = hand + sums[a] + 2 * i;
			hook_write16(at, (uint16_t)(hook_read16(at) + hook_read16(at + 2)));
		}
		hand_up(hand, i + 1);
	}
}

void soulbn4_enter(uint32_t obj, int k) {
	int kind1, kind2, add;
	if (k < 0 || k >= SOULS_BN4 || (soulpassive_breaks() & SOULPASSIVE_BREAK_PLUS)) return;
	uint32_t hand = BN6_BATTLE_HAND + (uint32_t)(hook_read8(obj + BN6_T1_ALLIANCE) & 1) * BN6_HAND_SIZE;
	if (merge_rule(forms[k], &kind1, &kind2)) {
		hand_merge(hand, kind1, kind2);
		return;
	}
	if (!pass_of(forms[k], &kind1, &kind2, &add)) return;
	for (uint32_t i = hook_read8(hand + BN6_HAND_AT); i < BN6_HAND_MAX; ++i) {
		uint16_t chip = hook_read16(hand + BN6_HAND_CHIPS + 2 * i);
		if (chip == HAND_END) break;
		int kind = soulkinds_bn4_of(chip);
		if ((kind == kind1 || kind == kind2) && shows_power(chip))
			hook_write16(hand + BN6_HAND_BONUS + 2 * i, (uint16_t)(hook_read16(hand + BN6_HAND_BONUS + 2 * i) + add));
	}
}

uint32_t soulbn4_wind(int k) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	uint32_t to;
	if (!b4 || k < 0 || k >= SOULS_BN4 || forms[k] != FORM_WIND ||
		get32(b4 + (BN4_FORM_ENTERS - BUS) + 4u * (uint32_t)forms[k]) != (BN4_WIND_ENTER | 1) ||
		!thumb_bl_read(b4 + (BN4_WIND_SPAWN_CALL - BUS), 0, BN4_WIND_SPAWN_CALL, &to) || to != BN4_WIND_SPAWN)
		return 0;
	uint16_t f = get16(b4 + (BN4_WIND_FLAGS - BUS));
	return (f & 0xFF00u) == OP_MOVS_R0 ? (f & 0xFFu) : 0;
}

bool soulbn4_wind_coll(SoulBn4CollSet *out) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!b4) return false;
	uint16_t a = get16(b4 + (BN4_WIND_COLL - BUS)), b = get16(b4 + (BN4_WIND_COLL + 2 - BUS)), c = get16(b4 + (BN4_WIND_COLL + 4 - BUS));
	if ((a & 0xFF00u) != OP_MOVS_R1 || (b & 0xFF00u) != 0x2200u || (c & 0xFF00u) != 0x2300u) return false;
	out->region = a & 0xFF;
	out->region2 = b & 0xFF;
	out->arg = c & 0xFF;
	return true;
}

int soulbn4_wind_level(void) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	uint16_t m = b4 ? get16(b4 + (BN4_WIND_LEVEL - BUS)) : 0;
	return (m & 0xFF00u) == OP_MOVS_R4 ? (m & 0xFF) : -1;
}

bool soulbn4_guarded(int k) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	uint32_t to;
	if (!b4 || k < 0 || k >= SOULS_BN4 || get16(b4 + (BN4_STATUS_GUARD_FORM - 6 - BUS)) != (OP_MOVS_R1 | BN4_NAVI_FORM) ||
		get16(b4 + (BN4_STATUS_GUARD_FORM + 2 - BUS)) != OP_BEQ_NEXT ||
		!thumb_bl_read(b4 + (BN4_STATUS_GUARD_SKIP - BUS), 0, BN4_STATUS_GUARD_SKIP, &to) || to != BN4_STATUS_TAKE)
		return false;
	uint16_t f = get16(b4 + (BN4_STATUS_GUARD_FORM - BUS));
	return (f & 0xFF00u) == OP_CMP_R0 && (f & 0xFF) == forms[k];
}

bool soulbn4_heal(int k, SoulBn4Heal *out) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!b4 || k < 0 || k >= SOULS_BN4) return false;
	uint16_t f = get16(b4 + (BN4_ROLL_HEAL_FORM - BUS)), kind = get16(b4 + (BN4_ROLL_HEAL_KIND - BUS));
	uint16_t share = get16(b4 + (BN4_ROLL_HEAL_SHARE - BUS)), effect = get16(b4 + (BN4_ROLL_HEAL_EFFECT - BUS));
	uint16_t sound = get16(b4 + (BN4_ROLL_HEAL_SOUND - BUS));
	if (f != (OP_CMP_R0 | forms[k]) || (kind & 0xFF00u) != OP_CMP_R0 || (share & 0xFF00u) != OP_MOVS_R1 || !(share & 0xFFu) ||
		(effect & 0xFF00u) != OP_MOVS_R4 || (sound & 0xFF00u) != OP_MOVS_R0)
		return false;
	out->kind = (uint8_t)kind;
	out->share = (uint8_t)share;
	out->effect = (uint8_t)effect;
	out->sound = (uint8_t)sound;
	return true;
}
