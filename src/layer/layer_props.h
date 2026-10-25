/* The sprite props' map objects on a layer (layer_props.c; docs/
 * LEVEL_DESIGN.md, Props), for layer_objs.c. */
#ifndef CW_LAYER_PROPS_H
#define CW_LAYER_PROPS_H

#include <stdint.h>

#include "npc.h"

/* The layer's props, another game's, Rush's and the obstacles' map
 * objects, their sprites asked of `npcs`: their records' bus address, 0
 * for none. */
uint32_t layer_props_objects(NpcList *npcs);

#endif
