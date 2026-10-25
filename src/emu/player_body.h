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

#endif
