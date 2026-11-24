/* layer_objs.h's key items in a layer's Mystery Data: the run's Spin
 * (docs/game/meta/spins.md), a ScrtData, and the run's rare find of
 * colours (docs/game/looks-earning.md, The rare find). */
#include "layer_objs.h"

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "looks.h"
#include "looks_routes.h"
#include "run.h"

/* (a key item, as the game's Mystery Data hold one: kind 4) */
static void key_item(uint8_t out[8], int item) {
	const uint8_t c[8] = { 4, 0x20, 0xFF, 0xFF, (uint8_t)item, 0, 0, 0 };
	memcpy(out, c, 8);
}

/* (the NaviCust turns programs of colour c with key item 0x4F + c: bn6f
 * sub_8136364) */
void layer_spin_content(uint8_t out[8], int colour) { key_item(out, 0x4F + colour); }

void layer_fragment_content(uint8_t out[8]) { key_item(out, SCRIPTS_SECRET_DATA); }

int layer_look_find(void) {
	int look = looks_find_palette();
	bool here = look == LOOK_GLITCH ? run.side_kind == LAYER_GLITCH
		: look == LOOK_MYSTERY && run.side_kind == LAYER_NORMAL && run.depth == looks_find_depth();
	return here ? look : -1;
}

/* (its content rolled as any other's first, then swapped: a profile that
 * owns the palette finds the layer otherwise the same) */
int layer_look_md(int spin_md, int reg_md) {
	int best = -1;
	for (int i = 0, k = 0; i < layer.nobj && k < 16; ++i) {
		const NetObj *o = &layer.obj[i];
		if (o->type != OBJ_MYSTERY) continue;
		++k;
		if (i == spin_md || i == reg_md || o->param == MD_PURPLE || o->param < 1) continue;
		if (best < 0 || o->param >= layer.obj[best].param) best = i;
	}
	return best;
}

int layer_look_put(MysteryData *m, int palette) {
	key_item(m->content, SCRIPTS_COLOR_DATA);
	if (emu_debug_on()) fprintf(stderr, "looks: palette %d in the Mystery Data at %d %d\n", palette, m->x, m->y);
	return palette;
}
