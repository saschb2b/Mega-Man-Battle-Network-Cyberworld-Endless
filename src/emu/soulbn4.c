/* BN4 Red Sun's Souls in BN6's engine (soulbn4.h). */
#include "soulbn4.h"

#include <string.h>

#include "bn4.h"
#include "bn4_souls.h"
#include "bn6.h"
#include "bn6_chips.h"
#include "bytes.h"
#include "chips.h"
#include "hook.h"
#include "rom.h"
#include "soulkinds.h"
#include "soulpassive.h"
#include "thumb_reloc.h"
#include "xchips_code.h"

#define BUS 0x08000000u
#define NONE 0xFF
#define FORMS 16                /* BN4_FORM_LOADOUTS', BN4_FORM_ELEMENTS' rows: forms 0-15 */
#define CHARGE_ROW 10           /* BN4_CHARGE_TIMES' row, a power attack's (BN6_CHARGE_ROW) */
/* a loadout row's bytes (BN4_FORM_LOADOUTS): +1 the plain shot's power attack, +2 the chip charge's, +3 the charged shot's */
enum { ROW4_0, ROW4_PLAIN, ROW4_CHIP, ROW4_CHARGED };
#define POWER_PLAIN4 0x00       /* BN4's plain shot (form 0's +1): BN6's own +6 byte, 0, for every form */

/* BN6's BN4 Souls by k: BN4's Soul, its form (ThunSoul 6, GutsSoul 2, AquaSoul 11, SrchSoul 4, NumSoul 8, FireSoul 5, MetlSoul 9) */
static const uint8_t forms[SOULS_BN4] = { 6, 2, 11, 4, 8, 5, 9 };
#define FORM_GUTS 2
#define FORM_NUM 8
#define FORM_AQUA 11
#define FORM_FIRE 5
#define FORM_METL 9
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

bool soulbn4_charges(int k, int chip) {
	int kind, kind2;
	if (k >= 0 && k < SOULS_BN4 && forms[k] == FORM_FIRE && fire_rule(&kind)) return chip > 0 && soulkinds_bn4_of(chip) == kind;
	if (k >= 0 && k < SOULS_BN4 && forms[k] == FORM_METL && metl_rule(&kind, &kind2)) {
		uint8_t flags = chip > 0 ? hook_read8(chips_table_bus() + (uint32_t)chip * BN6_CHIP_RECORD_SIZE + BN6_CHIP_EFFECT_FLAGS) : 0;
		int kd = soulkinds_bn4_of(chip);
		return chip > 0 && !(flags & BN6_CHIP_LEFT_ALONE) && (flags & BN6_CHIP_SHOWS_POWER) && (kd == kind || kd == kind2);
	}
	if (k < 0 || k >= SOULS_BN4 || forms[k] != FORM_AQUA || !aqua_rule(&kind)) return false;
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
	/* (BN4_CHIP_CHARGE_BONUS by form: AquaSoul's Aqua, MetlSoul's two kinds; the chips he charges are those alone) */
	return k >= 0 && k < SOULS_BN4 && ((forms[k] == FORM_AQUA && aqua_rule(&kind)) || (forms[k] == FORM_METL && metl_rule(&kind, &kind2))) ?
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
	/* (a plain shot of his own where its code is moved with his unit: MetlSoul's) */
	uint32_t plain = row[ROW4_PLAIN] != POWER_PLAIN4 && row[ROW4_PLAIN] < BN4_POWER_ATTACK_COUNT ?
		xchips_bn4_moved_at(get32(b4 + (BN4_POWER_ATTACKS - BUS) + 4u * row[ROW4_PLAIN])) : 0;
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
	out->routine = xchips_bn4_moved_at(get32(b4 + (BN4_POWER_ATTACKS - BUS) + 4u * out->power));
	memcpy(out->charge, b4 + (BN4_CHARGE_TIMES - BUS) + CHARGE_ROW * (uint32_t)out->power, CHARGE_ROW);
	return out->routine != 0;
}

/* The Souls whose taking of the form passes over the hand (BN4_FORM_ENTERS), each its routine's places: GutsSoul's +30 to chips
 * of no kind or panel-cracking, NumSoul's +10 to those of no kind (one kind: kind2 0) */
static const struct { uint8_t form; uint32_t enter, kind, kind2, add; } passes[] = {
	{ FORM_GUTS, BN4_GUTS_ENTER, BN4_GUTS_PASS_KIND, BN4_GUTS_PASS_KIND2, BN4_GUTS_PASS_ADD },
	{ FORM_NUM, BN4_NUM_ENTER, BN4_NUM_PASS_KIND, 0, BN4_NUM_PASS_ADD },
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

void soulbn4_enter(uint32_t obj, int k) {
	int kind1, kind2, add;
	if (k < 0 || k >= SOULS_BN4 || (soulpassive_breaks() & SOULPASSIVE_BREAK_PLUS) || !pass_of(forms[k], &kind1, &kind2, &add))
		return;
	uint32_t hand = BN6_BATTLE_HAND + (uint32_t)(hook_read8(obj + BN6_T1_ALLIANCE) & 1) * BN6_HAND_SIZE;
	for (uint32_t i = hook_read8(hand + BN6_HAND_AT); i < BN6_HAND_MAX; ++i) {
		uint16_t chip = hook_read16(hand + BN6_HAND_CHIPS + 2 * i);
		if (chip == HAND_END) break;
		int kind = soulkinds_bn4_of(chip);
		uint32_t rec = chips_table_bus() + (uint32_t)chip * BN6_CHIP_RECORD_SIZE;
		if ((kind == kind1 || kind == kind2) && (hook_read8(rec + BN6_CHIP_EFFECT_FLAGS) & BN6_CHIP_SHOWS_POWER))
			hook_write16(hand + BN6_HAND_BONUS + 2 * i, (uint16_t)(hook_read16(hand + BN6_HAND_BONUS + 2 * i) + add));
	}
}
