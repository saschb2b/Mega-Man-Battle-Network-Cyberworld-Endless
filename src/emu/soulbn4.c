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
#include "xchips_code.h"

#define BUS 0x08000000u
#define NONE 0xFF
#define FORMS 16                /* BN4_FORM_LOADOUTS', BN4_FORM_ELEMENTS' rows: forms 0-15 */
#define CHARGE_ROW 10           /* BN4_CHARGE_TIMES' row, a power attack's (BN6_CHARGE_ROW) */
/* a loadout row's bytes (BN4_FORM_LOADOUTS): +1 the plain shot's power attack, +2 the chip charge's, +3 the charged shot's */
enum { ROW4_0, ROW4_PLAIN, ROW4_CHIP, ROW4_CHARGED };
#define POWER_PLAIN4 0x00       /* BN4's plain shot (form 0's +1): BN6's own +6 byte, 0, for every form */

/* BN6's BN4 Souls by k: BN4's Soul, its form (ThunSoul 6, GutsSoul 2) */
static const uint8_t forms[SOULS_BN4] = { 6, 2 };
#define FORM_GUTS 2
#define OP_CMP_R1 0x2900u    /* cmp r1,#imm8 */
#define OP_ADDS_R1 0x3100u   /* adds r1,#imm8 */
#define HAND_END 0xFFFFu     /* a hand's ids after its last */

unsigned soulbn4_held(unsigned bn4) {
	unsigned out = 0;
	for (int k = 0; k < SOULS_BN4; ++k)
		if (bn4 >> (forms[k] - 1) & 1) out |= 1u << (SOULS_HELD_BN4 + k);
	return out;
}

int soulbn4_k(int soul) { return soul >= SOUL_BN4_FIRST && soul < SOUL_BN4_FIRST + SOULS_BN4 ? soul - SOUL_BN4_FIRST : -1; }

bool soulbn4_read(int k, SoulBn4 *out) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	memset(out, 0, sizeof *out);
	if (!b4 || k < 0 || k >= SOULS_BN4) return false;
	int form = forms[k];
	const uint8_t *row = b4 + (BN4_FORM_LOADOUTS - BUS) + BN4_FORM_LOADOUT_SIZE * (uint32_t)form;
	/* (the shape the move reads: no plain shot or chip charge of its own, a charged shot among BN4's power attacks; the moved
	 * Souls' others are later ones') */
	if (form < 1 || form >= FORMS || row[ROW4_0] != NONE || row[ROW4_PLAIN] != POWER_PLAIN4 || row[ROW4_CHIP] != NONE ||
		row[ROW4_CHARGED] >= BN4_POWER_ATTACK_COUNT)
		return false;
	out->form = (uint8_t)form;
	out->kind = b4[BN4_SOUL_KINDS - BUS + (uint32_t)(form - 1)];
	out->element = b4[BN4_FORM_ELEMENTS - BUS + (uint32_t)form];
	out->power = row[ROW4_CHARGED];
	out->routine = xchips_bn4_moved_at(get32(b4 + (BN4_POWER_ATTACKS - BUS) + 4u * out->power));
	memcpy(out->charge, b4 + (BN4_CHARGE_TIMES - BUS) + CHARGE_ROW * (uint32_t)out->power, CHARGE_ROW);
	return out->routine != 0;
}

/* GutsSoul's pass's numbers read from BN4's code (BN4_GUTS_PASS): its two kinds and its add; false where the code is not that */
static bool guts_pass(int *kind1, int *kind2, int *add) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!b4 || get32(b4 + (BN4_FORM_ENTERS - BUS) + 4u * FORM_GUTS) != (BN4_GUTS_ENTER | 1)) return false;
	uint16_t k1 = get16(b4 + (BN4_GUTS_PASS_KIND - BUS)), k2 = get16(b4 + (BN4_GUTS_PASS_KIND2 - BUS)), a = get16(b4 + (BN4_GUTS_PASS_ADD - BUS));
	if ((k1 & 0xFF00u) != OP_CMP_R1 || (k2 & 0xFF00u) != OP_CMP_R1 || (a & 0xFF00u) != OP_ADDS_R1) return false;
	*kind1 = k1 & 0xFF;
	*kind2 = k2 & 0xFF;
	*add = a & 0xFF;
	return true;
}

void soulbn4_enter(uint32_t obj, int k) {
	int kind1, kind2, add;
	if (k < 0 || k >= SOULS_BN4 || forms[k] != FORM_GUTS || (soulpassive_breaks() & SOULPASSIVE_BREAK_PLUS) ||
	    !guts_pass(&kind1, &kind2, &add))
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
