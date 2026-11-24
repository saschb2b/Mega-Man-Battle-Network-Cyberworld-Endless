/* The palettes' moments on the map (docs/game/looks-earning.md, Earning): the
 * milestones looked for, AsterLand's sales granted, and MegaMan's word on
 * each palette got, said once a talk can start. */
#ifndef CW_LOOKS_DUE_H
#define CW_LOOKS_DUE_H

#include <stdio.h>

/* Each frame on a layer or at home. */
void looks_due_frame(void);
/* The key item the game gave (EV_ITEM_GIVEN): the run's rare find picked
 * up, where `palette` is the layer's (layer_objs.h's look_palette). */
void looks_item_given(int item, int palette);
/* A headless run's state: `looksowned` the palettes owned and `lookstold`
 * those said (a bit each, 1 << LookPalette), `lookfind` the run's rare
 * find (-1 none), `lookdepth` the layer a Mystery find lies on. */
void looks_routes_describe(FILE *f);

#endif
