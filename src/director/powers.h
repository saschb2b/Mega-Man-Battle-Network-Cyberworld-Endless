/* What MegaMan keeps from the Navis he deletes: the game's own Crosses and
 * Beast Out, switched on through the event flags the story sets. */
#ifndef CW_POWERS_H
#define CW_POWERS_H

/* After a won boss battle against `navi` (navi index) in `biome`. */
void powers_after_boss(int navi, int biome);
/* What the player is told after that battle, or NULL. */
const char *powers_reward_text(int navi, int biome);

#endif
