/* online_square.h. The Net Square built into the running game (docs/
 * ONLINE.md): square_plan.c's plan from the server's seed, drawn by the
 * layers' own map builder in one of BN6's areas' tiles, its signs and Board
 * BN6's own map objects, its warps BN6's gem pads (a teleport within the
 * map, as a layer's pair), its pad home BN6's return pad, its channel
 * keeper a Mr.Prog, and the other players' MegaMen NPCs whose scripts the
 * Square moves between frames (BN6 walks them: its own animation and
 * depth). */
#include "online_square.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "director.h"
#include "emu.h"
#include "flags.h"
#include "gamecall.h"
#include "layer_objs.h"
#include "mapslot.h"
#include "netmap.h"
#include "npc.h"
#include "pad_art.h"
#include "rom.h"
#include "square_plan.h"

#define PEER_SPRITE 55     /* list 6's MegaMan, bn6f npcSpritePtrs[0x37] */
#define PROG_SPRITE 61     /* list 6's Mr.Prog: the keeper, the greeter, the referee */
#define RETURN_PAD 0x81    /* list 7's return pad, as a side layer's way back */
/* the puppets' script: shown or hidden, animation, position, a frame's
 * pause, and a jump back, or once through a sound first */
#define SCRIPT_LOOP 9
#define SCRIPT_VISIBLE SCRIPT_LOOP
#define SCRIPT_ANIM (SCRIPT_LOOP + 2)
#define SCRIPT_POSITION (SCRIPT_LOOP + 4)
#define SCRIPT_JUMP (SCRIPT_LOOP + 13)
#define SCRIPT_SOUND (SCRIPT_LOOP + 17)
#define SQUARE_WRITER "online square"
#define PROG_TEXT 1024
#define DESK_SPRITE 0x78   /* list 7's reception desk, two robot clerks at it: Trade Central */
#define DESK_ANIM 0        /* its one animation: the clerks at work */
#define DESK_RADIUS 28     /* walked into and spoken to as far from its middle */
#define TALK_ROOM SQUARE_TALK_ROOM   /* the Square's chats (online_chat.c), one archive at a time */
/* The pad home draws in front of its middle, as Lan's HP's portals draw
 * in front of their cells: a player stood on it as drawn, 21 units
 * towards the screen's front (-x, +y) of its middle, outside a zone of 12
 * each way, and nothing asked (session 74). Its zone is its art, the
 * return pad's ring (list 7's 0x81), as MegaMan stands on it, as every
 * pad's is (pad_art.h). */
#define FIXTURES (SQUARE_PROGS + 1)  /* the Mr.Progs and the desk, for A */

/* the ambient actors' sprites (list, number): GreenNavi and GirlNavi
 * walking (layer_objs.c's), BN6's log-in beam (stage_npc.c's), the "!"
 * over a head (lanhp.c's courier's) */
static const int extra_sprites[SQUARE_ACTORS][2] = { { 6, 62 }, { 6, 69 }, { 7, 0x00 }, { 5, 0x07 } };
/* where the Mr.Progs stand and which way they face (BN6's 0-7, up first:
 * the keeper down-left, grid +y, and the greeter up-right, grid -y, to
 * the arrivals between them; the referee up-left, grid -x, to the ring's
 * stair) */
static const int prog_at[SQUARE_PROGS] = { SQ_KEEPER, SQ_GREETER, SQ_REFEREE };
static const int prog_face[SQUARE_PROGS] = { 5, 1, 7 };
#define ARRIVE_FACE 7      /* MegaMan arrives facing the landmark (grid -x) */
#define WARP_ENTRY 2       /* the first warp's entry in the map's warp list (1 is a layer's exit) */

static SquarePlan plan;
static uint8_t cells[SQUARE_H][SQUARE_W];
static int group, number, rise;
static uint32_t world_hash, prog_text[SQUARE_PROGS], talk_room, trade_text;
static int exit_x, exit_y;
static int gems[SQUARE_WARPS * 2][2];   /* the warps' gems, the plaza's and its part's in turn (panels) */
static int fixture[FIXTURES][3];   /* the Mr.Progs' and the desk's middles and reach, for A */
typedef struct {
	uint32_t script, id;
	int x, y, z, moving, anim, sound;
	bool visible;
} Puppet;
static Puppet actors[SQUARE_PEERS], extras[SQUARE_ACTORS];

static uint32_t hash_byte(uint32_t h, uint8_t b) { return (h ^ b) * 16777619u; }

/* the plan's hash and the tiles the client drew from it: two clients with
 * the same seed and ROM draw the same Square */
static uint32_t geometry_hash(void) {
	uint32_t h = square_plan_hash(&plan);
	int tw, th;
	const uint16_t *tiles = netmap_last_tiles(&tw, &th);
	for (int i = 0; i < tw * th * 2; ++i) {
		h = hash_byte(h, (uint8_t)tiles[i]);
		h = hash_byte(h, (uint8_t)(tiles[i] >> 8));
	}
	return h;
}

/* A puppet of sprite `index` of list `category`, moved by the Square
 * between frames: the mutable loop uses existing NPC bytecode (visible or
 * hidden, animation, position, a frame's pause, a jump), and a jump through
 * a sound (0x28) once when one is asked for. ns_set_animation only reloads
 * a changed animation, so a walker keeps its cadence (bn6f npc.s). */
static uint32_t puppet_script(int category, int index) {
	uint8_t script[] = {
		0x09, 0x25, (uint8_t)index, (uint8_t)(category * 4), 0x0A, 0, 0x1F, 0x0E, 0x13,
		0x09, 0x16, 0, 0x14, 0, 0, 0, 0, 0, 0,
		0x10, 1, 0x02, 0, 0, 0, 0,
		0x28, 0, 0, 0x02, 0, 0, 0, 0,
	};
	uint32_t at = mapslot_alloc(script, sizeof script);
	if (!at) return 0;
	put32(script + SCRIPT_JUMP, at + SCRIPT_LOOP);
	put32(script + sizeof script - 4, at + SCRIPT_LOOP);
	emu_write(at, script, sizeof script);
	return at;
}

/* the plan's props as BN6's map objects (layer_objs.c's pieces), those on
 * the stage's and the ring's rims at their height */
static void props(NpcList *npcs) {
	static uint8_t recs[(SQUARE_PROPS * 3 + 1) * 20];
	int n = 0;
	for (int i = 0; i < plan.nprops; ++i) {
		const NetProp *p = &plan.props[i];
		n = layer_prop_objects(npcs, p, plan.level[p->y][p->x] ? rise : 0, recs, n, SQUARE_PROPS * 3);
	}
	memset(recs + n * 20, 0, 4);
	recs[n * 20] = 0xFF;
	npcs->objects = n ? mapslot_alloc(recs, n * 20 + 4) : 0;
}

static void put_jump(uint32_t script, uint32_t to) {
	uint8_t b[4];
	put32(b, to);
	emu_write(script + SCRIPT_JUMP, b, sizeof b);
}

static void position(uint32_t at, int x, int y, int z) {
	uint8_t p[6];
	put16(p, (uint16_t)x);
	put16(p + 2, (uint16_t)y);
	put16(p + 4, (uint16_t)z);
	emu_write(at + SCRIPT_POSITION, p, sizeof p);
}

static int level_z(int gx, int gy) { return plan.level[gy][gx] ? rise : 0; }

/* the channel keeper, the greeter and the referee: Mr.Progs each with his
 * own words' room */
static void progs(NpcList *npcs, const uint8_t *empty) {
	bool sprite = npc_need_sprite(npcs, 6, PROG_SPRITE);
	for (int k = 0; k < SQUARE_PROGS; ++k) {
		int gx = plan.at[prog_at[k]][0], gy = plan.at[prog_at[k]][1], x, y;
		netmap_world(gx, gy, &x, &y);
		fixture[k][0] = x; fixture[k][1] = y; fixture[k][2] = 16;
		fprintf(stderr, "online: Mr.Prog %d at %d,%d\n", k, x, y);
		prog_text[k] = mapslot_alloc(empty, PROG_TEXT);
		if (!prog_text[k] || !sprite) continue;
		uint32_t prog = npc_talker(6, PROG_SPRITE, x, y, level_z(gx, gy), prog_face[k], prog_text[k], 0, -1, false);
		if (prog) npcs->script[npcs->n++] = prog;
	}
}

/* Trade Central's desk in its wing, its clerks' words its own */
static void desk(NpcList *npcs, const uint8_t *empty) {
	int x, y;
	trade_text = mapslot_alloc(empty, PROG_TEXT);
	netmap_world(plan.at[SQ_TRADE][0], plan.at[SQ_TRADE][1], &x, &y);
	fixture[SQUARE_PROGS][0] = x; fixture[SQUARE_PROGS][1] = y; fixture[SQUARE_PROGS][2] = DESK_RADIUS;
	fprintf(stderr, "online: Trade Central's desk at %d,%d\n", x, y);
	if (trade_text && npc_need_sprite(npcs, 7, DESK_SPRITE)) {
		const NpcDesk d = { 7, DESK_SPRITE, x, y, DESK_ANIM, trade_text, 0, 0, 0, DESK_RADIUS };
		uint32_t at = npc_desk(&d);
		if (at) npcs->script[npcs->n++] = at;
	} else fprintf(stderr, "online: Trade Central's desk found no room for its sprite\n");
}

/* the pad home, the Mr.Progs, the desk, the peers' puppets, then the
 * ambient actors' (BN6 runs 16 NPCs: the peers' first) into `npcs` */
static bool people(NpcList *npcs) {
	int x, y;
	netmap_world(plan.at[SQ_EXIT][0], plan.at[SQ_EXIT][1], &x, &y);
	exit_x = x + 8; exit_y = y + 8;   /* (the middle of its four panels) */
	if (npc_need_sprite(npcs, 7, RETURN_PAD)) npcs->script[npcs->n++] = npc_prop(7, RETURN_PAD, exit_x, exit_y, 0, 0);
	static const uint8_t empty[TALK_ROOM] = { 2, 0, 0xE6 };
	talk_room = mapslot_alloc(empty, TALK_ROOM);
	progs(npcs, empty);
	desk(npcs, empty);
	if (!npc_need_sprite(npcs, 6, PEER_SPRITE)) return false;
	for (int i = 0; i < SQUARE_PEERS; ++i) {
		actors[i].script = puppet_script(6, PEER_SPRITE);
		if (!actors[i].script) return false;
		npcs->script[npcs->n++] = actors[i].script;
	}
	for (int k = 0; k < SQUARE_ACTORS && npcs->n < BN6_NPC_COUNT; ++k) {
		if (!npc_need_sprite(npcs, extra_sprites[k][0], extra_sprites[k][1])) continue;
		extras[k].script = puppet_script(extra_sprites[k][0], extra_sprites[k][1]);
		if (extras[k].script) npcs->script[npcs->n++] = extras[k].script;
	}
	return true;
}

/* the stage and the ring raised where the area's maps have Sky's stairs */
static void heights(NetLayout *lay) {
	LayerKit kit;
	netmap_kit(plan.area, &kit);
	rise = 0;
	if (!(kit.stair_dirs & 1u << STAIR_UP_NY) || kit.rise < 32) return;
	rise = kit.rise;
	lay->level = &plan.level[0][0];
	lay->rise = rise;
	lay->stairs = plan.stairs;
	lay->nstairs = plan.nstairs;
}

/* The warps: each gem's trigger takes its entry of the map's warp list,
 * which beams MegaMan to its pair as BN6's gem pads do (departure 12, the
 * camera scrolling along: mapslot_teleport). BN6 then steps him 20 units a
 * world axis the way he faces, and takes a trigger he stands near when it
 * lets him go: landed on the gem's middle, he was beamed straight back,
 * and back again. So he lands on its rim the way he faces, 16 units a
 * world axis out, and steps on to the pad's corner, clear of it. */
#define LAND_OUT 16
static bool warps(void) {
	/* (BN6's eight ways in world units, 0 up first) */
	static const int way[8][2] = { { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 } };
	CoordPad pads[SQUARE_WARPS * 2];
	for (int k = 0; k < SQUARE_WARPS; ++k)
		for (int e = 0; e < 2; ++e) {
			int i = k * 2 + e, x, y, px, py;
			netmap_world(plan.gem[k][e][0], plan.gem[k][e][1], &x, &y);
			netmap_world(plan.gem[k][!e][0], plan.gem[k][!e][1], &px, &py);
			pads[i] = (CoordPad){ x, y, WARP_ENTRY + i };
			fprintf(stderr, "online: warp %d's gem %d at %d,%d\n", k, e, x, y);
			int face = plan.gem_face[k][!e];
			mapslot_teleport(WARP_ENTRY + i, group, number, px + way[face][0] * LAND_OUT, py + way[face][1] * LAND_OUT, face);
		}
	return netmap_set_pads(pads, SQUARE_WARPS * 2);
}

static bool install_square(uint32_t seed) {
	square_plan(seed, &plan);
	memcpy(cells, plan.cell, sizeof cells);
	memset(&layer, 0, sizeof layer);
	memset(actors, 0, sizeof actors);
	memset(extras, 0, sizeof extras);
	for (int k = 0; k < SQUARE_WARPS * 2; ++k) { gems[k][0] = plan.gem[k / 2][k % 2][0]; gems[k][1] = plan.gem[k / 2][k % 2][1]; }
	NetLayout lay = { .gw = SQUARE_W, .gh = SQUARE_H, .cell = &cells[0][0], .seed = seed, .pad = &plan.pad[0][0],
		.props = plan.props, .nprops = plan.nprops, .gems = (const int (*)[2])gems, .ngems = SQUARE_WARPS * 2 };
	heights(&lay);
	if (!netmap_build(plan.area, &lay)) return false;
	world_hash = geometry_hash();
	group = R.layout->net_area[plan.area].group;
	number = R.layout->net_area[plan.area].number;
	if (!warps()) return false;
	mapslot_town(false);
	mapslot_reset();
	NpcList npcs;
	memset(&npcs, 0, sizeof npcs);
	props(&npcs);
	npc_objects_sprites(&npcs);
	if (!people(&npcs)) return false;
	fprintf(stderr, "online: square %d NPCs, %d sprites of 0x%X bytes, rise %d\n", npcs.n, npcs.nsprites, (unsigned)npcs.sprite_bytes, rise);
	if (!mapslot_install(group, number, &npcs, NULL, 0)) return false;
	/* (the Square's own music, whatever area it stands in: BN6's Navi
	 * Customizer theme, the owner's pick, docs/ONLINE.md) */
	return mapslot_music(group, number, BN6_SONG_NAVI_CUSTOMIZER);
}

bool online_square_build(uint32_t seed, uint32_t player_id) {
	claims_begin(SQUARE_WRITER);
	bool ok = install_square(seed);
	claims_end();
	if (!ok) return false;
	int x, y;
	netmap_world(plan.at[SQ_ARRIVE][0], plan.at[SQ_ARRIVE][1], &x, &y);
	/* (most of a panel apart by player, so arrivals at once stand apart) */
	x -= (int)(player_id & 1u) * 24;
	y -= (int)(player_id >> 1 & 1u) * 24;
	int cx, cy;
	netmap_world(plan.at[SQ_CENTER][0], plan.at[SQ_CENTER][1], &cx, &cy);
	fprintf(stderr, "online: square area %d, arrival at %d,%d, the landmark at %d,%d\n", plan.area, x, y, cx, cy);
	director_map_label("Net Square", "Net Square");
	emu_warp(group, number, x, y, ARRIVE_FACE);
	return true;
}

bool online_square_arrived(void) {
	return emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_GAME &&
		emu_read8(BN6_GAMESTATE) == BN6_SUB_MAP &&
		emu_read8(BN6_MAP_GROUP) == group && emu_read8(BN6_MAP_NUMBER) == number;
}

uint32_t online_square_hash(void) { return world_hash; }

bool online_square_gem(int k, int end, int *x, int *y) {
	if (k < 0 || k >= SQUARE_WARPS || end < 0 || end > 1) return false;
	netmap_world(plan.gem[k][end][0], plan.gem[k][end][1], x, y);
	return true;
}

bool online_square_prog_say(int who, const uint8_t *archive, int n) {
	if (who < 0 || who >= SQUARE_PROGS || !prog_text[who] || n <= 0 || n > PROG_TEXT) return false;
	emu_write(prog_text[who], archive, (size_t)n);
	return true;
}

bool online_square_keeper_say(const uint8_t *archive, int n) { return online_square_prog_say(SQUARE_PROG_KEEPER, archive, n); }

const SquarePlan *online_square_plan(void) { return &plan; }


uint32_t online_square_talk_room(void) { return talk_room; }

bool online_square_trade_say(const uint8_t *archive, int n) {
	if (!trade_text || n <= 0 || n > PROG_TEXT) return false;
	emu_write(trade_text, archive, (size_t)n);
	return true;
}

static bool near_exit(int x, int y, int reach) {
	PadArt art = { exit_x, exit_y, PAD_RING_ACROSS, PAD_RING_DOWN };
	return pad_art_on(&art, x, y, reach);
}

bool online_square_on_exit(int x, int y) { return near_exit(x, y, 0); }

bool online_square_exit_ahead(int x, int y, int face) {
	/* (BN6's own probe for A, then two and three times as far: the pad is
	 * flat, and MegaMan checks it from its edge) */
	int dx, dy;
	npc_probe(face, &dx, &dy);
	for (int m = 0; m <= 3; ++m)
		if (near_exit(x + dx * m, y + dy * m, 2)) return true;
	return false;
}

static void peer_at(unsigned slot, const OnlinePeer *peer) {
	uint32_t at = actors[slot].script;
	bool visible = peer != NULL;
	emu_write8(at + SCRIPT_VISIBLE, visible ? 0x08 : 0x09);
	actors[slot].visible = visible;
	if (!visible) { actors[slot].id = 0; return; }
	bool moved = actors[slot].id == peer->id && (actors[slot].x != peer->x || actors[slot].y != peer->y);
	if (moved) actors[slot].moving = 8;
	else if (actors[slot].moving) --actors[slot].moving;
	emu_write8(at + SCRIPT_ANIM, (uint8_t)(peer->facing + (actors[slot].moving ? 8 : 0)));
	position(at, peer->x, peer->y, peer->z);
	/* (a sound asked for last frame has played: the loop again) */
	if (actors[slot].sound) { put_jump(at, at + SCRIPT_LOOP); actors[slot].sound = 0; }
	actors[slot].id = peer->id;
	actors[slot].x = peer->x;
	actors[slot].y = peer->y;
	actors[slot].z = peer->z;
}

void online_square_actor(int k, bool visible, int anim, int x, int y, int z) {
	Puppet *a = &extras[k];
	if (k < 0 || k >= SQUARE_ACTORS || !a->script) return;
	claims_begin(SQUARE_WRITER);
	emu_write8(a->script + SCRIPT_VISIBLE, visible ? 0x08 : 0x09);
	emu_write8(a->script + SCRIPT_ANIM, (uint8_t)anim);
	position(a->script, x, y, z);
	/* (a sound asked for last frame has played: the loop again) */
	if (a->sound) { put_jump(a->script, a->script + SCRIPT_LOOP); a->sound = 0; }
	claims_end();
	a->visible = visible; a->anim = anim; a->x = x; a->y = y; a->z = z;
}

static void puppet_sound(Puppet *a, int sound) {
	claims_begin(SQUARE_WRITER);
	emu_write8(a->script + SCRIPT_SOUND + 1, (uint8_t)sound);
	emu_write8(a->script + SCRIPT_SOUND + 2, (uint8_t)(sound >> 8));
	put_jump(a->script, a->script + SCRIPT_SOUND);
	claims_end();
	a->sound = sound;
}

void online_square_actor_sound(int k, int sound) {
	if (k < 0 || k >= SQUARE_ACTORS || !extras[k].script) return;
	puppet_sound(&extras[k], sound);
}

void online_square_peer_pose(unsigned slot, int anim, int sound) {
	if (slot >= SQUARE_PEERS || !actors[slot].script || !actors[slot].visible) return;
	claims_begin(SQUARE_WRITER);
	emu_write8(actors[slot].script + SCRIPT_ANIM, (uint8_t)anim);
	claims_end();
	if (sound > 0) puppet_sound(&actors[slot], sound);
}

int online_square_actor_ahead(int x, int y, int face) {
	int dx, dy, best = -1, best_d = 24 * 24;
	npc_probe(face, &dx, &dy);
	for (int k = 0; k < SQUARE_WALKERS; ++k) {
		if (!extras[k].visible) continue;
		int ex = extras[k].x - (x + dx), ey = extras[k].y - (y + dy), d = ex * ex + ey * ey;
		if (d < best_d) { best = k; best_d = d; }
	}
	return best;
}

void online_square_peers(const OnlineSnapshot *shown) {
	/* No battles, BN6's own jack-out, or cartridge saves in the Square.
	 * EnterMap clears the no-encounters bit, so set it after each frame. */
	flag_set(BN6_FLAG_NO_ENCOUNTERS);
	flag_set(BN6_FLAG_NO_JACK);
	flag_set(BN6_FLAG_NO_PET_SAVE);
	claims_begin(SQUARE_WRITER);
	for (unsigned i = 0; i < SQUARE_PEERS; ++i)
		peer_at(i, shown->connected && i < shown->peer_count ? &shown->peers[i] : NULL);
	claims_end();
}

int online_square_peer_ahead(int x, int y, int face) {
	int dx, dy;
	npc_probe(face, &dx, &dy);
	/* (the Square's own people first: BN6 talks to them, a peer beside
	 * them waits) */
	for (int k = 0; k < FIXTURES; ++k) {
		int fx = fixture[k][0] - (x + dx), fy = fixture[k][1] - (y + dy);
		if (fx * fx + fy * fy <= fixture[k][2] * fixture[k][2]) return -2;
	}
	int best = -1, best_d = 24 * 24;
	for (int i = 0; i < SQUARE_PEERS; ++i) {
		if (!actors[i].visible) continue;
		int ex = actors[i].x - (x + dx), ey = actors[i].y - (y + dy), d = ex * ex + ey * ey;
		if (d < best_d) { best = i; best_d = d; }
	}
	return best;
}

bool online_square_peer_position(unsigned slot, int *x, int *y, int *z) {
	if (slot >= SQUARE_PEERS || !actors[slot].visible) return false;
	/* Read the actual NPC pool, not the network buffer or script. Its
	 * whole-coordinate fields are already verified in docs/ROM_DATA.md. */
	for (unsigned i = 0; i < BN6_NPC_COUNT; ++i) {
		uint32_t at = BN6_NPC_OBJECTS + i * BN6_NPC_SIZE;
		if (!(emu_read8(at + BN6_NPC_IN_USE) & 1u)) continue;
		int nx = (int16_t)emu_read16(at + BN6_NPC_X16);
		int ny = (int16_t)emu_read16(at + BN6_NPC_Y16);
		int nz = (int16_t)emu_read16(at + BN6_NPC_Z16);
		if (nx != actors[slot].x || ny != actors[slot].y || nz != actors[slot].z) continue;
		*x = nx; *y = ny; *z = nz;
		return true;
	}
	return false;
}
