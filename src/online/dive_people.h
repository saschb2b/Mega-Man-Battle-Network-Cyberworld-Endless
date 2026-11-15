/* The other divers on MegaMan's layer (the shared dive, issue #589;
 * docs/online/shared-dive.md): BN6's MegaMen as puppets in the NPC slots
 * the layer leaves free (BN6 runs 16), each placed every frame where the
 * server's SNAPSHOT of the layer's room says he stands; walked through,
 * never in the way. */
#ifndef CW_DIVE_PEOPLE_H
#define CW_DIVE_PEOPLE_H

#include <stdbool.h>

#include "online_types.h"

#define DIVE_PEOPLE 7   /* a room's eight, MegaMan among them */

/* The puppets made as each layer is built (layer_objs_more), or not. */
void dive_people_on(bool on);

/* The words' room this layer gave the dive's chats (0 none), and the
 * diver whose puppet stands ahead of MegaMan within reach (0 none). */
uint32_t dive_people_talk_room(void);
uint32_t dive_people_ahead(void);

/* Each frame on a layer: the room's divers as the snapshot has them. */
void dive_people_frame(const OnlineSnapshot *s);

#endif
