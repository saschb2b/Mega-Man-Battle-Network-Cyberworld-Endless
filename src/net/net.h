/* The procedurally generated Cyberworld: layers of net area. */
#ifndef NET_H
#define NET_H

#include <stdbool.h>
#include <stdint.h>

#define MAP_W 64
#define MAP_H 64
#define MAX_OBJS 64
#define MAX_ROOMS 24
#define MAX_STAIRS 4

/* A cell: void, floor, or floor a prop stands on (drawn as floor, never
 * walked in the layout's terms): C_SOLID is walled off in the game too (a
 * counter's aisle, where its navi stands), C_PROPPED keeps the floor and
 * takes the prop's own walls (the counter's panels, whose front half
 * MegaMan can step on up to the counter). */
enum { C_VOID = 0, C_PATH = 1, C_SOLID = 2, C_PROPPED = 3 };

typedef enum {
	OBJ_WARP_IN,
	OBJ_EXIT,
	OBJ_MYSTERY,     /* param: content quality 0-2 */
	OBJ_SHOP,
	OBJ_HEAL,
	OBJ_TRADER,
	OBJ_BUGTRADER,
	OBJ_BOSS,        /* the layer's guardian, in its arena; param: navi */
	OBJ_UNDERNET,    /* warp to an Undernet layer */
	OBJ_SECRET_GATE, /* needs three secret fragments */
	OBJ_NPC,
	OBJ_CHALLENGE,   /* optional hard battle */
	OBJ_PROGRAMS,    /* NaviCust program vendor */
	OBJ_RETURN,      /* leave a side layer */
	OBJ_GIFT,        /* the run's first layer: a gift to choose */
	OBJ_NAVI_GATE,   /* a gate sealed with a Navi's code, his SP inside (docs/META.md, gates); param: navi */
	OBJ_VAULT,       /* a collector's vault: a Library's count opens it, three chips inside (docs/META.md, gates) */
	OBJ_DUEL,        /* ProtoMan, the rival: a busting duel against his time (docs/RIVAL.md) */
	OBJ_OFFICIAL,    /* an official gate: Chaud's clearance opens it (docs/RIVAL.md); param: its level, 1-2 */
} ObjType;

typedef struct {
	int type;
	float x, y;
	int param;
	bool solid;
	int npc_line;
	int prop;         /* the prop it stands behind (a counter), -1 none */
} NetObj;

/* Props: what the originals set on their floors (docs/LEVEL_DESIGN.md,
 * Props). A counter faces the camera: along grid y with its front towards
 * grid +x (screen down-right), or along grid x with its front towards
 * grid +y (down-left); its navi stands in the aisle behind it. A sprite
 * prop is one of the originals' map objects on a cell: in the void past
 * the floor's edge, or on a walled-off cell (C_SOLID) in the floor, a hole
 * as the originals set their statues and stones in. An emblem is art in
 * the floor itself, walked over (the Graveyard's crosses). */
enum { PROP_COUNTER, PROP_SPRITE, PROP_EMBLEM };
enum { FACES_X, FACES_Y };
/* the sprite props' looks (the map side knows their objects) */
enum { LOOK_TREE, LOOK_GIANT_TREE, LOOK_STATUE, LOOK_BRAZIER, LOOK_MONUMENT, LOOK_GRAVE, LOOK_SIGN, LOOK_BBS, LOOK_COUNT };
#define MAX_PROPS 96

typedef struct {
	int kind, faces;
	int x, y, len;    /* a counter's first cell (lowest along its run); a sprite's cell */
	int look;         /* a sprite's LOOK_* */
} NetProp;

/* Room kinds: where points of interest go (docs/LEVEL_DESIGN.md). */
enum { ROOM_PLATFORM, ROOM_PAD, ROOM_FIELD, ROOM_LEG };   /* ROOM_LEG: a stretch of a winding path, never raised */

/* A platform or pad: its box, a floor cell in it (ax, ay) and its kind. */
typedef struct {
	int x, y, w, h;
	int ax, ay;
	int kind;
} Room;

enum { LAYER_NORMAL, LAYER_UNDERNET, LAYER_SECRET };

/* Which way a stair climbs: towards grid -x or -y (see src/map/stairs.h). */
enum { STAIR_UP_NX, STAIR_UP_NY };

/* A stair: 2 x 2 cells from (x, y). */
typedef struct { int x, y, dir; } Stair;

typedef struct {
	uint8_t cell[MAP_H][MAP_W];
	uint8_t level[MAP_H][MAP_W];   /* 1: a raised room's floor */
	Stair stair[MAX_STAIRS];
	int nstairs;
	int rise;                      /* world z of the raised floor */
	Room rooms[MAX_ROOMS];
	int nrooms;
	NetObj obj[MAX_OBJS];
	int nobj;
	int biome;
	int kind;
	bool boss_layer;
	int boss_navi;
	int exit_room;
	int arena;                     /* the guardian's arena room, -1 for none */
	int ante;                      /* the room before it, with the last services */
	int arena_dir;                 /* DIR_* from the antechamber into the arena */
	int layout;                    /* LAYOUT_* (net_layouts.h) */
	NetProp props[MAX_PROPS];
	int nprops;
} Layer;

extern Layer layer;

/* What an area's original maps give its layers to draw (learned from the
 * ROM, src/map): the stairs it has (bit per STAIR_UP_*) and their height,
 * and a counter's length in panels for each way it can face (FACES_*, 0
 * none). */
typedef struct {
	unsigned stair_dirs;
	int rise;
	int counter_len[2];
	unsigned looks;   /* the sprite props its maps have (bit per LOOK_*) */
	bool emblem;      /* its maps set an emblem in their floors */
} LayerKit;

/* Generation is deterministic for a given seed and kit. */
void layer_generate(uint32_t seed, int depth, int biome, int kind, const LayerKit *kit);
/* Lifts dead-end rooms onto stairs (net_height.c). */
void layer_raise_rooms(uint32_t seed, unsigned dirs, int rise);
/* Whether panel (x, y) lies on the layer's way from its arrival to its exit
 * or guardian, or beside it (net_gen.c: what stands keeps off it). */
bool layer_by_way(int x, int y);
/* ... on the way itself. */
bool layer_on_way(int x, int y);
int biome_for_depth(int depth);
bool is_boss_depth(int depth);
/* The layer's place in its act, 0-2 (the Nest counts as a first). */
int layer_in_act(int depth);

#endif
