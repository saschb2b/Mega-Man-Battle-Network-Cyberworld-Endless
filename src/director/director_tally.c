/* The viruses a battle deleted, for the run's tally that RESULT reads
 * (director_state.h; docs/game/meta/design.md, Teaching it): a battle lost
 * or run from counts those it deleted too. BattleState's side 1 left
 * (BN6_STATE_SIDE1_LEFT, set from the side's count as its foes spawn and
 * lowered as each one's dying begins) is followed through the battle: the
 * most that stood, and how many fell since. It is read once the battle
 * loop is past its first phase, its foes all in: until they are, the
 * battle before's count stands. A won battle counts its foes, as before;
 * a lost one ends in GAME OVER, where the director counts it from here. */
#include "director_state.h"

#include <stdio.h>

#include "bn6.h"
#include "bn6_battle_loop.h"
#include "emu.h"
#include "run.h"

static int most, down;

void tally_battle_start(void) {
	most = down = 0;
}

void tally_battle_frame(void) {
	if (!emu_read8(BN6_LOOP)) return;
	int left = emu_read8(BN6_BATTLE_STATE + BN6_STATE_SIDE1_LEFT);
	if (left > most) most = left;
	down = most - left;
}

int tally_deleted(bool won, int foes) {
	return (won || down > foes) ? foes : down;
}

/* The tally in the state's words (director_describe's; tests/scenarios
 * reads them): the run's viruses and Navis deleted, as RESULT will count
 * them, the layer's random battles fought, and in a battle its enemies,
 * the most that stood and those down. Reads alone. */
void tally_describe(FILE *f) {
	fprintf(f, "viruses %d\nnavis %d\nbattles %d\n", run.viruses_deleted, run.bosses_beaten, D.battles);
	if (D.in_battle && !D.battler) fprintf(f, "foes %d\ndown %d\n", most, down);
}
