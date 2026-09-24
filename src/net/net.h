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

enum { C_VOID = 0, C_PATH = 1 };

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
} ObjType;

typedef struct {
	int type;
	float x, y;
	int param;
	bool solid;
	int npc_line;
} NetObj;

/* Room kinds: where points of interest go (docs/LEVEL_DESIGN.md). */
enum { ROOM_PLATFORM, ROOM_PAD, ROOM_FIELD };

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
} Layer;

extern Layer layer;

/* Generation is deterministic for a given seed. `stair_dirs` (bit per
 * STAIR_UP_*) are the stairs the area can draw, `rise` their height. */
void layer_generate(uint32_t seed, int depth, int biome, int kind, unsigned stair_dirs, int rise);
/* Lifts dead-end rooms onto stairs (net_height.c). */
void layer_raise_rooms(uint32_t seed, unsigned dirs, int rise);
int biome_for_depth(int depth);
bool is_boss_depth(int depth);

#endif
