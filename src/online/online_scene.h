/* The Square while MegaMan is in it (online_visit.c): the peers moved on
 * BN6's map, their names and words over them, the Square's menu. */
#ifndef CW_ONLINE_SCENE_H
#define CW_ONLINE_SCENE_H

#include <stdbool.h>
#include <stdint.h>

#include "online_types.h"
#include "online_visit.h"

/* The Square for server seed `seed` built into the running game and
 * MegaMan warped there, as player `player`; his public card sent. */
bool online_scene_start(uint32_t seed, uint32_t player, const OnlineProfile *card);
void online_scene_end(void);
/* After a frame of the core: VISIT_END_NONE while the visit goes on. */
VisitEnd online_scene_update(void);
/* The link quiet or dropped (issue #153, online_transport.h): the core held
 * still while online_scene_wait runs in its frames' place, till the link is
 * back (VISIT_END_NONE, and the Square goes on), refused or given up (its
 * end; B gives up) */
bool online_scene_waiting(void);
VisitEnd online_scene_wait(void);
void online_scene_draw(void);
uint32_t online_scene_keys(uint32_t keys);

#endif
