/* The week's traces on a dive layer (issue #589; docs/online/shared-dive.md,
 * As built): the room's deletion marks, phrases, first finds and opened
 * Mystery Data as the server keeps them for the week (DIVE_TRACES), shown
 * on the layer after its own objects (layer_objs_more, as the divers are):
 * a mark as BN6's skull burst where a diver fell, a phrase as BN6's
 * sparkle on its panel, and a sparkle over a Mystery Data others opened
 * this week. A on a mark says who fell and to what; A on a phrase shows
 * it, and MegaMan waves at it (DIVE wave). The layer's seed is untouched:
 * the traces are presentation, placed after it. */
#ifndef CW_DIVE_TRACES_H
#define CW_DIVE_TRACES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dive_wire.h"
#include "mapslot.h"

/* The NPC slots the traces take as a layer is built, of the `room` left
 * after the layer's own: none, one (the marks') or two (and the sparkle's). */
int dive_traces_more(NpcList *npcs, int room);
/* A new layer's room entered: its traces forgotten till the server's come. */
void dive_traces_room(DiveWay way);
/* A DIVE_TRACES come in. */
void dive_traces_mail(const DiveTraces *t);
/* Each frame on the layer: the objects placed by MegaMan; A on one. */
void dive_traces_frame(void);
/* The week's first finder of Mystery Data `index` ("" none known). */
const char *dive_traces_finder(unsigned index);
/* Whether another diver opened Mystery Data `index` this week. */
bool dive_traces_opened(unsigned index);
/* The layer's split records come in (DIVE_TRACES' splits): the quickest
 * bust of battle `index` at party size 1, 0 none. */
unsigned dive_traces_best(unsigned index);

#endif
