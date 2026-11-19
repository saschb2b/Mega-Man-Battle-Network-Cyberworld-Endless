/* BN6's player object in another Navi's overworld sprite (docs/game/liberation/decision.md,
 * The team): a liberation's Navi in play walks as BN6 walks its player, in
 * his own body, as BN5's board walks him. The sprite its spawn loads
 * (BN6_PLAYER_SPRITE_LOAD) is answered, and its spawn run again on the map
 * it stands on. */
#ifndef CW_PLAYER_BODY_H
#define CW_PLAYER_BODY_H

/* From now on the player object in sprite `sprite` of list 6, -1 his own
 * (MegaMan's): at every spawn (a map entered, a battle's return), and now
 * on the map where the body changes. */
void player_body(int sprite);

/* His own body (MegaMan's; Lan's in the real world) from the next spawn
 * on, the player as he stands left alone: where no board is on and a map
 * is about to load, which spawns him afresh. A board left with a teammate
 * in play (a reset, a quit, a run's end) had the next run's Lan walk the
 * town in the teammate's sprite, and his jack-in stand still (issue
 * #696). */
void player_body_own(void);

/* The list-6 sprite the player's spawn is given, -1 his own */
int player_body_now(void);

#endif
