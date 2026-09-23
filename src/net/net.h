/* The procedurally generated Cyberworld: layers of net area. */
#ifndef NET_H
#define NET_H

#include <stdbool.h>
#include <stdint.h>

#define MAP_W 64
#define MAP_H 64
#define MAX_OBJS 64
#define MAX_ROOMS 14

enum { C_VOID = 0, C_PATH = 1 };

typedef enum {
	OBJ_WARP_IN,
	OBJ_EXIT,
	OBJ_MYSTERY,     /* param: content quality 0-2 */
	OBJ_SHOP,
	OBJ_HEAL,
	OBJ_TRADER,
	OBJ_BUGTRADER,
	OBJ_BOSS,        /* the layer's guardian, before the exit; param: navi */
	OBJ_UNDERNET,    /* warp to an Undernet layer */
	OBJ_SECRET_GATE, /* needs three secret fragments */
	OBJ_NPC,
	OBJ_CHALLENGE,   /* optional hard battle */
	OBJ_PROGRAMS,    /* NaviCust program vendor */
	OBJ_RETURN,      /* leave a side layer */
} ObjType;

typedef struct {
	int type;
	float x, y;
	int param;
	bool solid;
	int npc_line;
} NetObj;

typedef struct {
	int x, y, w, h;
} Room;

enum { LAYER_NORMAL, LAYER_UNDERNET, LAYER_SECRET };

typedef struct {
	uint8_t cell[MAP_H][MAP_W];
	Room rooms[MAX_ROOMS];
	int nrooms;
	NetObj obj[MAX_OBJS];
	int nobj;
	int biome;
	int kind;
	bool boss_layer;
	bool boss_beaten;
	int boss_navi;
	int exit_room;
} Layer;

extern Layer layer;

/* Generation is deterministic for a given seed. */
void layer_generate(uint32_t seed, int depth, int biome, int kind);
int biome_for_depth(int depth);
bool is_boss_depth(int depth);

#endif
