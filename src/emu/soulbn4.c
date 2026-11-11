/* BN4 Red Sun's Souls in BN6's engine (soulbn4.h). */
#include "soulbn4.h"

#include <string.h>

#include "bn4.h"
#include "bytes.h"
#include "rom.h"
#include "xchips_code.h"

#define BUS 0x08000000u
#define NONE 0xFF
#define FORMS 16                /* BN4_FORM_LOADOUTS', BN4_FORM_ELEMENTS' rows: forms 0-15 */
#define CHARGE_ROW 10           /* BN4_CHARGE_TIMES' row, a power attack's (BN6_CHARGE_ROW) */
/* a loadout row's bytes (BN4_FORM_LOADOUTS): +1 the plain shot's power attack, +2 the chip charge's, +3 the charged shot's */
enum { ROW4_0, ROW4_PLAIN, ROW4_CHIP, ROW4_CHARGED };
#define POWER_PLAIN4 0x00       /* BN4's plain shot (form 0's +1): BN6's own +6 byte, 0, for every form */

/* BN6's BN4 Souls by k: BN4's Soul, its form (ThunSoul 6) */
static const uint8_t forms[SOULS_BN4] = { 6 };

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
