/* layer_objs.h's layer_look_find: whether the run's rare find of colours
 * lies on the layer being made (docs/game/looks-earning.md, Earning). */
#include "layer_objs.h"

#include "looks.h"
#include "looks_routes.h"
#include "net.h"
#include "run.h"

int layer_look_find(void) {
	int look = looks_find_palette();
	bool here = look == LOOK_GLITCH ? run.side_kind == LAYER_GLITCH
		: look == LOOK_MYSTERY && run.side_kind == LAYER_NORMAL && run.depth == looks_find_depth();
	return here ? look : -1;
}
