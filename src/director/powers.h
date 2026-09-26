/* What MegaMan keeps from the Navis he deletes: the game's own Crosses and
 * BeastOut, switched on through the event flags the story sets. */
#ifndef CW_POWERS_H
#define CW_POWERS_H

/* After a won boss battle against `navi` (navi index) in `biome`. */
void powers_after_boss(int navi, int biome);
/* What the player is told after that battle at `depth` (ta_talk's boxes:
 * a Cross MegaMan did not have yet, BeastOut in the first Graveyard), or
 * NULL. */
const char *powers_reward_text(int navi, int biome, int depth);

#endif
