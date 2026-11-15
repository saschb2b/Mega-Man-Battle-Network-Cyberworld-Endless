/* The looks the director switches the Glitch warp and a Glitch layer's
 * copy between (layer_objs.h, LayerObjs.look_*; docs/game/glitch-layer.md):
 * the warp's three pads, the copy's forms' bodies, each with the bit of
 * glitch_looks saying the map loads its sprite. */
#include "layer_objs.h"

#include "guardians.h"
#include "npc.h"

/* The big framed warp pad, the green oval pad and the orange-framed pad
 * (list 7's 0x06, 0x22, 0x36): steady every frame, where the dark warp's
 * flame (0x44) is drawn three frames in five */
const uint8_t glitch_look_sprite[GLITCH_LOOKS] = { 0x06, 0x22, 0x36 };

static void look_set(LayerObjs *out, int k, int list, int sprite, int anim) {
	out->look_list[k] = (uint8_t)list;
	out->look_sprite[k] = (uint8_t)sprite;
	out->look_anim[k] = (uint8_t)anim;
}

void glitch_warp_looks(NpcList *npcs, LayerObjs *out) {
	out->glitch_looks = 0;
	for (int k = 0; k < GLITCH_LOOKS; ++k) {
		look_set(out, k, 7, glitch_look_sprite[k], 0);
		if (npc_need_sprite(npcs, 7, glitch_look_sprite[k])) out->glitch_looks |= (uint8_t)(1u << k);
	}
}

void glitch_copy_looks(NpcList *npcs, LayerObjs *out, const int *forms, int n) {
	for (int k = 0; k < GLITCH_LOOKS; ++k) {
		NpcBody f = guardian_body(k < n ? forms[k] : n ? forms[0] : 1, 5);
		look_set(out, k, f.list, f.index, f.anim);
	}
	out->glitch_looks = npc_need_sprite(npcs, out->look_list[0], out->look_sprite[0]) ? 1 : 0;
}

/* (the others loaded where the map's sprite list still has room once the
 * layer's props took theirs: most are uncompressed and take none; a look
 * whose sprite did not load is never switched to, as it would draw as
 * noise) */
void glitch_copy_looks_more(NpcList *npcs, LayerObjs *out) {
	if (!out->glitch_looks || out->look_list[0] == 7) return;
	for (int k = 1; k < GLITCH_LOOKS; ++k)
		if (out->look_sprite[k] != out->look_sprite[0] && npc_need_sprite(npcs, out->look_list[k], out->look_sprite[k])) out->glitch_looks |= (uint8_t)(1u << k);
}
