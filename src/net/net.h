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
	OBJ_MYSTERY,     /* param: content quality 0 (green), 1-2 (blue), MD_PURPLE */
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

/* A gap Rush bridges (issue #14): from floor panel (x, y), MegaMan's
 * stand, `len` void panels towards DIR_* `dir` to floor of the same
 * height; the far floor is an island holding one thing, or a shortcut. The
 * gap's panels stay C_VOID in `cell`: the map side makes them walkable
 * once Rush lies there (docs/LEVEL_DESIGN.md, Set pieces). */
typedef struct { int x, y, dir, len; bool island; } NetGap;
#define MAX_GAPS 2

/* A Link Navi obstacle in a walkway's mouth (issue #42): BN6's own, which
 * the run's Crosses clear as Gregar's Link Navis do, closing a pocket off
 * the way that holds one thing. (x, y) the walkway's first panel, dir the
 * DIR_* from the room into it, (rx, ry) its reward's panel. */
enum { BLOCK_WATER, BLOCK_TREE, BLOCK_FLAMES, BLOCK_CYCLONE, BLOCK_CLOUD, BLOCK_KINDS,
	BLOCK_PCODE = BLOCK_KINDS, BLOCK_TOLL };   /* (and BN6's security cubes, issue #45: a P-Code's, a toll's) */
typedef struct { int x, y, dir, kind, rx, ry; } NetBlock;
#define MAX_BLOCKS 2

/* A lane of arrow panels (issue #43): BN6's own, which carry MegaMan one
 * way, input held, until he is past them. From the floor at (x, y) its
 * `len` panels run towards dir (DIR_*) to the floor past them. */
typedef struct { int x, y, dir, len; } NetLane;
#define MAX_LANES 1

/* An invisible path (issue #46): BN6's floor drawn as void, from a
 * walkway's tip `len` panels on to a lonely pad holding one thing; a gap's
 * shape (NetGap), its panels void in cell[] (drawn so) and floor to the
 * map's walls. */
#define MAX_PATHS 1

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
	NetGap gap[MAX_GAPS];
	int ngaps;
	NetBlock block[MAX_BLOCKS];
	int nblocks;
	int teller;        /* the navi who tells a P-Code cube's code: its object + 1, 0 none */
	int teleport_x[2], teleport_y[2];   /* a teleport pair's two panels, when nteleports is 1 (issue #44): */
	int nteleports;                     /* the first a pad of its own past the void when teleport_island */
	bool teleport_island;
	NetLane lane[MAX_LANES];
	int nlanes;
	NetGap path[MAX_PATHS];
	int npaths;
	int hinter;        /* the navi who hints at an invisible path: its object + 1, 0 none */
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
	bool gem;         /* its maps mark their teleport pads with BN6's gem (issue #44) */
	unsigned arrows;  /* the ways its maps draw an arrow panel (bit per DIR_*, issue #43) */
} LayerKit;

/* Generation is deterministic for a given seed and kit. */
void layer_generate(uint32_t seed, int depth, int biome, int kind, const LayerKit *kit);
/* The game runs at most 16 NPCs on a map (BN6_NPC_COUNT) and leaves the
 * rest of a map's list out without a word; every object but the warp-in
 * takes one, the guardian two (himself and his data). The layer's count. */
#define LAYER_NPC_MAX 16
int layer_npcs(void);
/* Lifts dead-end rooms onto stairs (net_height.c). */
void layer_raise_rooms(uint32_t seed, unsigned dirs, int rise);
/* The one-wide walkways the way from the arrival to (gx, gy) crosses, past
 * the area's cap widened to two panels (net_way.c, docs/LEVEL_DESIGN.md,
 * Navigation); and how many it crosses, from (sx, sy). */
void layer_widen_way(int biome, int gx, int gy);
int layer_way_runs(int sx, int sy, int gx, int gy);
int layer_way_cap(int biome);
/* Whether panel (x, y) lies on the layer's way from its arrival to its exit
 * or guardian, or beside it (net_gen.c: what stands keeps off it). */
bool layer_by_way(int x, int y);
/* ... on the way itself. */
bool layer_on_way(int x, int y);
/* How many panels a walk from the way to (x, y) takes, -1 off the floor
 * (once the layer's data are placed: net_gen.c, Detours). */
int layer_detour(int x, int y);
/* The DIR_* an arrow lane's panel (x, y) carries MegaMan towards, -1 none
 * (issue #43); and whether a walk may step from panel (x, y) to the panel
 * beside it (nx, ny): onto a lane and along it only the way its arrows run
 * (against them, its first panel carries him back). */
int layer_lane_dir(int x, int y);
bool layer_step_ok(int x, int y, int nx, int ny);
/* A purple Mystery Data's param: locked until an Unlocker opens it, the
 * best a layer holds (issue #41). */
#define MD_PURPLE 3
/* A layer's set pieces (net_pieces.c, epic #49): BN6's own interactables,
 * from the run's seed and the depth alone. How many of the act's layers
 * from `depth` on hold `piece` (a Net Dealer stocks its key); a Rush gap's
 * length in panels. */
enum { PIECE_PURPLE = 1, PIECE_RUSH = 2, PIECE_TELEPORT = 4, PIECE_OBSTACLE = 8, PIECE_CUBE = 16, PIECE_ARROW = 32, PIECE_HIDDEN = 64 };
unsigned layer_pieces(int depth, int biome, int kind);
/* (dev: --dev pieces=MASK) set pieces every layer of an area that has them holds */
extern unsigned layer_pieces_forced;
/* The Crosses MegaMan holds as layer `depth` begins (a bit per navi 1-5),
 * which of them clear obstacle `kind` (BLOCK_*), and the obstacle a
 * layer's pocket takes (BLOCK_*, -1 none): the area's own kinds, mostly
 * one the run can clear. */
unsigned layer_crosses(int depth);
unsigned block_openers(int kind);
int layer_block_kind(int depth, int biome);
/* The security cube an area's layers set (BLOCK_PCODE or BLOCK_TOLL). */
int layer_cube_kind(int biome);
bool layer_purple(int depth, int biome, int kind);
int layer_pieces_ahead(int depth, unsigned piece);
int layer_rush_len(int depth, int biome);
int biome_for_depth(int depth);
bool is_boss_depth(int depth);
/* The layer's place in its act, 0-2 (the Nest counts as a first). */
int layer_in_act(int depth);

#endif
