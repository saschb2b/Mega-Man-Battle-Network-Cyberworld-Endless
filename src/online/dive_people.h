/* The other divers on MegaMan's layer (the shared dive, issue #589;
 * docs/online/shared-dive.md): BN6's MegaMen as puppets in the NPC slots
 * the layer leaves free (BN6 runs 16), each placed every frame where the
 * server's SNAPSHOT of the layer's room says he stands; walked through,
 * never in the way, each with his name over his head (issue #712). */
#ifndef CW_DIVE_PEOPLE_H
#define CW_DIVE_PEOPLE_H

#include <stdbool.h>
#include <stdio.h>

#include "online_types.h"

#define DIVE_PEOPLE 7   /* a room's eight, MegaMan among them */

/* The puppets made as each layer is built (layer_objs_more), or not. */
void dive_people_on(bool on);

/* Each frame on a layer: the room's divers as the snapshot has them. */
void dive_people_frame(const OnlineSnapshot *s);

/* The divers' names over their MegaMen while the layer's map shows, as
 * the Square names its players (map_label.h); divers on one spot have
 * theirs a row over each other, and those the picture's top leaves no row
 * for are counted on the topmost ("Beta +2"). `chat`: BN6's chat box is
 * open, and the names it would lie under are left out. */
void dive_people_draw(bool chat);
/* The names shown, in the state's words: `tags N: NAMES` (NAME+K: K more
 * under it). */
void dive_people_describe(FILE *f);

#endif
