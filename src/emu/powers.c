/* Flags found by setting them in a battle and opening CROSSSELECT (see
 * docs/ROM_DATA.md). */
#include "powers.h"

#include "bn6.h"
#include "flags.h"
#include "run.h"

/* Gregar's Crosses by navi index: HeatMan 1 .. ChargeMan 5 */
static const struct { int navi, flag; } crosses[] = {
	{ 1, BN6_FLAG_HEAT_CROSS }, { 2, BN6_FLAG_ELEC_CROSS }, { 3, BN6_FLAG_SLASH_CROSS },
	{ 4, BN6_FLAG_ERASE_CROSS }, { 5, BN6_FLAG_CHARGE_CROSS },
};

void powers_after_boss(int navi, int biome) {
	for (unsigned i = 0; i < sizeof crosses / sizeof *crosses; ++i)
		if (crosses[i].navi == navi) flag_set(crosses[i].flag);
	/* the Graveyard's guardian wakes the Cybeast */
	if (biome == BIOME_GRAVEYARD) flag_set(BN6_FLAG_BEAST_OUT);
}
