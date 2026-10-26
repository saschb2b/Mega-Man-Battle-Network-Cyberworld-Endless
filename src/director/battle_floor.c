/* The battle floor (docs/game/progression/structure.md, The battle floor):
 * bad-luck protection on BN6's own encounter roll. bn6f sub_80AA4C0 rolls
 * once each 0x40 of the walk since the last battle, its odds a row of
 * byte_8020C5C by that walk (0x40 a row, the last from 0x400) and a column
 * by the map's encounter class; the maps the layers take over have classes
 * 3, 5 and 6, and class 6 gives nothing before 0x380, so 43% of the layers
 * of seeds 1-12 were left without a battle. */
#include "battle_floor.h"

#include "bn6.h"
#include "director_layer.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "flags.h"
#include "net.h"
#include "run.h"

#define FLOOR_DUE  0x100   /* the walk with no battle yet (about a quarter of a layer's) before the odds rise */
#define FLOOR_ODDS 0x400   /* ... to the walk of byte_8020C5C's last row, BN6's best */

/* (not a guardian's layer, whose battle he is; not a dark layer's board,
 * whose battles are its own; not a trip back, the player's errand home;
 * not where BN6's random battles are off) */
bool floor_owed(void) {
	return D.active && D.battles == 0 && !D.objs.guardian.navi && !layer.dark && run.side_kind != LAYER_BACK && !D.battler_series &&
		!flag_get(BN6_FLAG_NO_ENCOUNTERS);
}

static void steps_write(uint32_t at, int v) {
	emu_write8(at, (uint8_t)v);
	emu_write8(at + 1, (uint8_t)(v >> 8));
}

void floor_walk(void) {
	if (!floor_owed()) return;
	uint32_t steps = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_STEPS);
	if (steps < BN6_EWRAM || steps >= BN6_EWRAM_END) return;
	int walked = emu_read16(steps + BN6_STEPS_WALKED);
	/* (the roll checks at the next frame, as the walk since its last check
	 * passes 0x40) */
	if (walked >= FLOOR_DUE && walked < FLOOR_ODDS) steps_write(steps + BN6_STEPS_WALKED, FLOOR_ODDS);
}

bool floor_at_exit(void) {
	if (!floor_owed()) return false;
	if (emu_battle_forcing()) return true;
	Encounter e = D.rolled[emu_encounter_slot()];
	D.floor_fight = true;
	set_encounter(&e, true);
	return true;
}
