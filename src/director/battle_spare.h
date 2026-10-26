/* A battle MegaMan is spared in: ProtoMan's netbattle (docs/game/rival.md, The
 * netbattle). The blow that would delete him leaves him 1 HP, and the
 * battle ends at the fight's next frame as BN6's own draw ends one (result
 * 3: no reward, his HP as it stands, as an escape ends), without its DRAW!
 * banner: Lan stops the battle. */
#ifndef CW_BATTLE_SPARE_H
#define CW_BATTLE_SPARE_H

#include <stdbool.h>
#include <stdint.h>

/* The hooks on BN6's battle loop, once (idle until armed) */
void battle_spare_install(void);
/* The next battle spared (or none), as the director sets it between frames */
void battle_spare_arm(bool on);
/* Whether the sparing blow came in the battle armed */
bool battle_spare_down(void);
/* object_subtractHP's hook, for MegaMan's object at `hp` (above 0): the
 * damage he may take of `damage` */
uint32_t battle_spare_blow(uint32_t damage, uint32_t hp);

#endif
