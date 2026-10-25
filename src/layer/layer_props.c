/* A layer's sprite props as BN6's own map objects (docs/LEVEL_DESIGN.md,
 * Props), and the other objects of the map's object list: another game's
 * prop, Rush and his bones, the obstacles. */
#include "layer_props.h"

#include <stdio.h>
#include <string.h>

#include "blockers.h"
#include "bytes.h"
#include "debug.h"
#include "layer_objs.h"
#include "netmap.h"
#include "rom.h"
#include "rush.h"
#include "xnavi.h"

/* The sprite props' map objects (docs/LEVEL_DESIGN.md, Props): the
 * originals' objects by their OverworldMapObjects ids, each at its panel's
 * spot +(14, 18) from the corner, as the originals set theirs, a set
 * piece's pieces where the original has them. */
#define OW_MAP_OBJECTS 0x0A4F24u
static const struct { uint8_t look; uint8_t id; int8_t dx, dy; int16_t dz; } prop_pieces[] = {
	{ LOOK_TREE, 0x3D, 14, 18, 0 },            /* a cybertree (Green Area 2, ACDC Area) */
	{ LOOK_GIANT_TREE, 0x7F, 14, 18, 0 },      /* the giant cybertree's body, */
	{ LOOK_GIANT_TREE, 0x80, 38, -6, -24 },    /* its base */
	{ LOOK_GIANT_TREE, 0x81, -10, 42, 154 },   /* and its crown (Green Area 2) */
	{ LOOK_STATUE, 0x9C, 14, 18, -12 },        /* the Undernet's statue */
	{ LOOK_BRAZIER, 0x9F, 14, 18, 0 },         /* a brazier (Undernet 1 and Zero) */
	{ LOOK_MONUMENT, 0xD3, 16, 16, 0 },        /* the Graveyard's monument */
	{ LOOK_GRAVE, 0xCA, 14, 18, 0 },           /* a gravestone (the Graveyard's rows) */
	{ LOOK_SIGN, 0x3E, 14, 18, 0 },            /* the NetCafe's WELCOME sign (Central Area 1) */
	{ LOOK_BBS, 0x47, 14, 18, -24 },           /* a BBS (Seaside Area 1) */
};

#define RUSH_OBJECTS_MAX 12   /* Rush and its bones in each panel of two gaps */

/* BN5's dark hole on Nebula Area's layers (XLOOK_DARK_HOLE): a vortex in
 * the void past the back rim of the layer's best room, where its own maps
 * stand theirs past a platform's edge, the Graveyard's monument's place;
 * the layer itself unchanged (a handler-3 object of blockers.c's, as
 * BN6's map objects' table has no room for an id of the engine's) */
static int xprops_objects(uint8_t *recs, int n, int max) {
	int x, y, cx, cy;
	if (!(layer_objs_xlooks & XLOOK_DARK_HOLE) || xnavi_object(XOBJ_DARK_HOLE) < 0 || !layer_landmark_void(&x, &y)) return n;
	netmap_world(x, y, &cx, &cy);
	if (emu_debug_on()) fprintf(stderr, "prop: BN5's dark hole at %d,%d (world %d,%d)\n", x, y, cx, cy);
	return blockers_xprop(recs, n, max, cx, cy);
}

int layer_prop_objects(NpcList *npcs, const NetProp *p, int cz, uint8_t *recs, int n, int max) {
	/* (its sprites first: a piece without its sprite would draw noise) */
	bool loaded = true;
	for (size_t k = 0; k < sizeof prop_pieces / sizeof *prop_pieces; ++k)
		if (prop_pieces[k].look == p->look) {
			const uint8_t *e = R.data + OW_MAP_OBJECTS + prop_pieces[k].id * 16u;
			loaded &= npc_need_sprite(npcs, e[0] / 4, e[1]);
		}
	if (emu_debug_on()) fprintf(stderr, "prop look %d at %d,%d%s%s\n", p->look, p->x, p->y, cz ? " (raised)" : "", loaded ? "" : " - no room for its sprite");
	if (!loaded) return n;
	int cx, cy;
	netmap_world(p->x, p->y, &cx, &cy);   /* (the panel's centre, 16 in from its corner) */
	for (size_t k = 0; k < sizeof prop_pieces / sizeof *prop_pieces && n < max; ++k) {
		if (prop_pieces[k].look != p->look) continue;
		uint8_t *r = recs + n++ * 20;
		memset(r, 0, 20);
		r[0] = 5;   /* a map object, its handler looked up by its id */
		put32(r + 4, (uint32_t)((int32_t)(cx - 16 + prop_pieces[k].dx) * 65536));
		put32(r + 8, (uint32_t)((int32_t)(cy - 16 + prop_pieces[k].dy) * 65536));
		put32(r + 12, (uint32_t)((int32_t)(cz + prop_pieces[k].dz) * 65536));
		put32(r + 16, prop_pieces[k].id);
	}
	return n;
}

uint32_t layer_props_objects(NpcList *npcs) {
	static uint8_t recs[(MAX_PROPS * 3 + 1 + RUSH_OBJECTS_MAX + MAX_BLOCKS + 1) * 20];
	int n = 0;
	for (int i = 0; i < layer.nprops; ++i) {
		const NetProp *p = &layer.props[i];
		if (p->kind != PROP_SPRITE) continue;
		n = layer_prop_objects(npcs, p, layer.level[p->y][p->x] ? layer.rise : 0, recs, n, MAX_PROPS * 3);
	}
	n = xprops_objects(recs, n, MAX_PROPS * 3 + 1);
	n = rush_objects(recs, n, MAX_PROPS * 3 + 1 + RUSH_OBJECTS_MAX);
	n = blockers_objects(recs, n, MAX_PROPS * 3 + 1 + RUSH_OBJECTS_MAX + MAX_BLOCKS);
	if (!n) return 0;
	memset(recs + n * 20, 0, 4);
	recs[n * 20] = 0xFF;
	return mapslot_alloc(recs, n * 20 + 4);
}

