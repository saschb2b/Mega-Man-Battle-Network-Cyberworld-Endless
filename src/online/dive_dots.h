/* The other divers on the layer's map (the shared dive, issue #728;
 * docs/online/shared-dive.md, Dots on the map): every diver in the
 * layer's room where the server's SNAPSHOT has him, lent to the map's
 * drawing (map_divers.h) while the dive is linked, a crewmate in the
 * crew's colour (crew_look.h), the others dimmer. A diver on another
 * layer is in another room, and on no map of this one. */
#ifndef CW_DIVE_DOTS_H
#define CW_DIVE_DOTS_H

#include <stdbool.h>
#include <stdio.h>

#include "online_types.h"

/* Lent to the map while the dive is linked, or not. */
void dive_dots_on(bool on);
/* Each frame of the link on a layer (dive_people_frame): the room's
 * divers as the snapshot has them. Without it for half a second (the
 * link down, MegaMan off the layer) they are no one's dots. */
void dive_dots_frame(const OnlineSnapshot *s);
/* The dots in the state's words: `mapdots N: NAME(DX,DY)` each, his
 * panel from MegaMan's, `*` a crewmate's. */
void dive_dots_describe(FILE *f);

#endif
