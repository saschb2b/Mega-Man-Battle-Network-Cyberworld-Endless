/* The layer guardian's sequence, from the arena to the open exit (boss.c). */
#ifndef CW_BOSS_H
#define CW_BOSS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "foes.h"
#include "guardian_objs.h"

/* A new layer: its guardian (navi 0 for none) with its scripts in `archive`. */
void boss_begin_layer(uint32_t archive, const GuardianStage *g);
/* Once a frame while MegaMan is on the map. */
void boss_update(void);
/* The guardian's battle was started and has not come back yet. */
bool boss_fighting(void);
/* ... his battle as it was forced, and the HP he spawns with into *hp */
const Encounter *boss_battle(int *hp);
/* A frame of the guardian's battle: how it stands, for his record. */
void boss_battle_frame(void);
/* Back from the guardian's battle. */
void boss_battle_over(bool won);
/* MegaMan was deleted: in the guardian's battle, it remembers. */
void boss_lost(void);
/* The exit pad works: no guardian, or it is beaten and its data taken. */
bool boss_exit_open(void);
bool boss_beaten(void);
/* The guardian's scene holds the stage (from its entrance to its logout). */
bool boss_cinematic(void);
/* Its Guardian Data shows, not yet taken. */
bool boss_reward_waiting(void);
/* ... and has shown for less than a second: an A then was pressed at the
 * guardian's last word, not at it (director_keys.c). */
bool boss_reward_fresh(void);
/* Its Guardian Data has been taken. */
bool boss_done(void);
/* Nothing of the guardian under way: not met yet, or all done (a run can
 * be saved then). */
bool boss_idle(void);
/* Whether the guardian has just fallen and the run wants a checkpoint
 * (once: the call clears it). */
bool boss_take_checkpoint(void);
/* Whether MegaMan has just stepped into the arena and the run wants saving
 * at its door, this frame, before the staging takes him (once an approach:
 * the call clears it; the staging begins on the next frame). */
bool boss_take_door(void);
/* The shared dive (docs/online/shared-dive.md): whether MegaMan stands in
 * the arena's mouth, saved at its door, the guardian still waiting; while
 * boss_door_hold (a part above's, NULL none) says so, the staging waits
 * there; and the party (1 alone) the next guardian's fight is scaled for:
 * the version a diver alone meets, his HP times the party (at most
 * PARTY_HP_MOST; issue #754). */
bool boss_at_door(void);
extern bool (*boss_door_hold)(void);
void boss_party_set(int n);
#define PARTY_HP_MOST 9999
/* The dev's (tools/party_guardian.py, docs/devtools/party-guardian.md):
 * every guardian scaled for a party of `n` (0 the run's own), its HP
 * `hp_per` percent of his own more for each fighter past the first (-1
 * the party's own rule, times the party) and his version `steps` on
 * (-1 the party's own ladder). Play's rule is not changed by it. */
void boss_party_dev(int n, int hp_per, int steps);
/* The guardian's battle fought elsewhere (the shared dive's party core,
 * party_play.h; NULL none): given his Encounter, his own HP and the HP he
 * is fought at, true where it takes the fight, and the run's core does not
 * start it; its end comes back through director_party_over */
extern bool (*boss_party_fight)(const Encounter *e, int own_hp, int fought_hp);
/* After a run saved mid-layer is loaded: the guardian's state from its
 * flags. */
void boss_resume(void);
/* The Navi MegaMan fights or is about to: the guardian's, or in a
 * tournament's bracket the round's (docs/game/tournament.md) */
int boss_foe(void);
/* The tournament's bracket in a state's words: bracket, round, roundfoe,
 * roundchips (director_describe) */
void boss_bracket_describe(FILE *f);

/* His course in the state's words: his version, whether he is known, his
 * Guardian Data (hidden, fresh, waiting, taken) and the exit pad; in his
 * battle the HP he is fought at. */
void boss_describe(FILE *f);

/* Where the test autopilot heads while the guardian stands: into the
 * arena, then to its Guardian Data (to check, *talk). */
bool boss_goal(int *x, int *y, bool *talk);

#endif
