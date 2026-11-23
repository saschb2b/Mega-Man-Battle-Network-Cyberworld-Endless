/* A moved chip's computed power as the PET shows it (xchips_shown.h). */
#include "xchips_shown.h"

#include <stdint.h>

#include "bn6_chips.h"
#include "bn6_pet.h"
#include "chips.h"
#include "emu.h"
#include "hook.h"
#include "xchips_code.h"

/* (an entry of BN6's grown table past its own: BN5's from XC_POWER_FIRST, BN4's from XREG4_POWER_FIRST past them) */
static bool grown(uint16_t power) { return power >= 1000u + XC_POWER_FIRST; }

/* (BN6_NAVI_POWER and BN6_NAVI_POWER_MID, their first instruction, r0 the chip, lr their caller's) a grown entry's power
 * computed as the battle computes it for side 0: BN6_CHIP_POWER_OF returns to the caller in their place */
static HookAct shown(HookRegs *r, void *user) {
	(void)user;
	uint32_t chip = r->r[0];
	if (chip == 0 || chip >= CHIPS_IDS) return HOOK_CONTINUE;
	uint16_t power = hook_read16(chips_table_bus() + chip * BN6_CHIP_RECORD_SIZE + BN6_CHIP_ATTACK_POWER);
	if (!grown(power)) return HOOK_CONTINUE;
	r->r[1] = 0;
	r->r[12] = BN6_CHIP_POWER_OF | 1u;
	return HOOK_JUMP;
}

static int card_chip = -1, card_power;   /* (what the card last drew: written on the core's thread, read between frames) */

/* (BN6_CARD_POWER_DRAWN: read only, the instruction runs on) */
static HookAct card(HookRegs *r, void *user) {
	(void)user;
	card_chip = hook_read16(r->r[5] + BN6_CARD_CHIP);
	card_power = (int)r->r[0];
	return HOOK_CONTINUE;
}

bool xchips_shown_install(void) {
	return emu_hook(BN6_NAVI_POWER, shown, NULL) && emu_hook(BN6_NAVI_POWER_MID, shown, NULL) && emu_hook(BN6_CARD_POWER_DRAWN, card, NULL);
}

bool xchips_shown_card(int *chip, int *power) {
	if (card_chip < 0) return false;
	*chip = card_chip;
	*power = card_power;
	return true;
}
