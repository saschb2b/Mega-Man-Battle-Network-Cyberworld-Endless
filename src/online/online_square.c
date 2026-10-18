/* online_square.h. The Net Square built into the running game (docs/
 * ONLINE.md): square_plan.c's plan from the server's seed, drawn by the
 * layers' own map builder in one of BN6's areas' tiles, its sign and Board
 * BN6's own map objects, its pad home BN6's return pad, its channel keeper
 * a Mr.Prog, and the other players' MegaMen NPCs whose scripts the Square
 * moves between frames (BN6 walks them: its own animation and depth). */
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
#include "mapslot.h"
#include "netmap.h"
#include "npc.h"
#include "rom.h"
#include "square_plan.h"

#define PEER_SPRITE 55     /* list 6's MegaMan, bn6f npcSpritePtrs[0x37] */
#define KEEPER_SPRITE 61   /* list 6's Mr.Prog */
#define KEEPER_FACE 5      /* -x: towards the plaza's middle */
#define RETURN_PAD 0x81    /* list 7's return pad, as a side layer's way back */
#define SCRIPT_LOOP 9
#define SCRIPT_VISIBLE SCRIPT_LOOP
#define SCRIPT_ANIM (SCRIPT_LOOP + 2)
#define SCRIPT_POSITION (SCRIPT_LOOP + 4)
#define SQUARE_WRITER "online square"
#define KEEPER_TEXT 1024
#define DESK_SPRITE 0x78   /* list 7's reception desk, two robot clerks at it: Trade Central */
#define DESK_ANIM 0        /* its one animation: the clerks at work */
#define DESK_RADIUS 28     /* walked into and spoken to as far from its middle */
#define TALK_ROOM SQUARE_TALK_ROOM   /* the Square's chats (online_chat.c), one archive at a time */
#define OW_MAP_OBJECTS 0x0A4F24u   /* bn6f OverworldMapObjects (layer_objs.c's props) */
#define EXIT_REACH 12

/* the sign's and the Board's map objects (layer_objs.c's prop pieces) */
static const struct { int look; uint8_t id; int dx, dy, dz; } pieces[] = {
	{ LOOK_SIGN, 0x3E, 14, 18, 0 }, { LOOK_BBS, 0x47, 14, 18, -24 },
};

static SquarePlan plan;
static uint8_t cells[SQUARE_H][SQUARE_W];
static int group, number;
static uint32_t world_hash, keeper_text, talk_room, trade_text;
static int exit_x, exit_y;
static int fixture[2][3];   /* the keeper's and the desk's middles and reach, for A */
static struct {
	uint32_t script, id;
	int x, y, z, moving;
	bool visible;
} actors[SQUARE_PEERS];

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

/* The mutable loop uses existing NPC bytecode: visible/hidden, animation,
 * position, one-frame pause, jump. ns_set_animation only reloads a changed
 * animation, so a walking peer keeps its cadence (bn6f npc.s). */
static uint32_t peer_script(void) {
	uint8_t script[] = {
		0x09, 0x25, PEER_SPRITE, 6 * 4, 0x0A, 0, 0x1F, 0x0E, 0x13,
		0x09, 0x16, 0, 0x14, 0, 0, 0, 0, 0, 0,
		0x10, 1, 0x02, 0, 0, 0, 0,
	};
	uint32_t at = mapslot_alloc(script, sizeof script);
	if (!at) return 0;
	put32(script + sizeof script - 4, at + SCRIPT_LOOP);
	emu_write(at, script, sizeof script);
	return at;
}

/* the plan's sign and Board as BN6's map objects, their sprites asked for */
static void props(NpcList *npcs) {
	static uint8_t recs[(SQUARE_PROPS + 1) * 20];
	int n = 0;
	for (int i = 0; i < plan.nprops; ++i)
		for (size_t k = 0; k < sizeof pieces / sizeof *pieces; ++k) {
			if (pieces[k].look != plan.props[i].look) continue;
			const uint8_t *e = R.data + OW_MAP_OBJECTS + pieces[k].id * 16u;
			if (!npc_need_sprite(npcs, e[0] / 4, e[1])) continue;
			int cx, cy;
			netmap_world(plan.props[i].x, plan.props[i].y, &cx, &cy);
			uint8_t *r = recs + n++ * 20;
			memset(r, 0, 20);
			r[0] = 5;
			put32(r + 4, (uint32_t)((int32_t)(cx - 16 + pieces[k].dx) * 65536));
			put32(r + 8, (uint32_t)((int32_t)(cy - 16 + pieces[k].dy) * 65536));
			put32(r + 12, (uint32_t)((int32_t)pieces[k].dz * 65536));
			put32(r + 16, pieces[k].id);
		}
	memset(recs + n * 20, 0, 4);
	recs[n * 20] = 0xFF;
	npcs->objects = n ? mapslot_alloc(recs, n * 20 + 4) : 0;
}

/* the pad home, the channel keeper and the peers' scripts into `npcs` */
static bool people(NpcList *npcs) {
	int x, y;
	netmap_world(plan.at[SQ_EXIT][0], plan.at[SQ_EXIT][1], &x, &y);
	exit_x = x + 8; exit_y = y + 8;   /* (the middle of its four panels) */
	if (npc_need_sprite(npcs, 7, RETURN_PAD)) npcs->script[npcs->n++] = npc_prop(7, RETURN_PAD, exit_x, exit_y, 0, 0);
	static const uint8_t empty[TALK_ROOM] = { 2, 0, 0xE6 };
	keeper_text = mapslot_alloc(empty, KEEPER_TEXT);
	talk_room = mapslot_alloc(empty, TALK_ROOM);
	netmap_world(plan.at[SQ_KEEPER][0], plan.at[SQ_KEEPER][1], &x, &y);
	fixture[0][0] = x; fixture[0][1] = y; fixture[0][2] = 16;
	if (keeper_text && npc_need_sprite(npcs, 6, KEEPER_SPRITE)) {
		uint32_t keeper = npc_talker(6, KEEPER_SPRITE, x, y, 0, KEEPER_FACE, keeper_text, 0, -1, false);
		if (keeper) npcs->script[npcs->n++] = keeper;
	}
	/* Trade Central's desk in its wing, its clerks' words its own */
	trade_text = mapslot_alloc(empty, KEEPER_TEXT);
	netmap_world(plan.at[SQ_TRADE][0], plan.at[SQ_TRADE][1], &x, &y);
	fixture[1][0] = x; fixture[1][1] = y; fixture[1][2] = DESK_RADIUS;
	if (trade_text && npc_need_sprite(npcs, 7, DESK_SPRITE)) {
		const NpcDesk d = { 7, DESK_SPRITE, x, y, DESK_ANIM, trade_text, 0, 0, 0, DESK_RADIUS };
		uint32_t desk = npc_desk(&d);
		if (desk) npcs->script[npcs->n++] = desk;
	} else fprintf(stderr, "online: Trade Central's desk found no room for its sprite\n");
	if (!npc_need_sprite(npcs, 6, PEER_SPRITE)) return false;
	for (int i = 0; i < SQUARE_PEERS; ++i) {
		actors[i].script = peer_script();
		if (!actors[i].script) return false;
		npcs->script[npcs->n++] = actors[i].script;
	}
	return true;
}

static bool install_square(uint32_t seed) {
	square_plan(seed, &plan);
	memcpy(cells, plan.cell, sizeof cells);
	memset(&layer, 0, sizeof layer);
	memset(actors, 0, sizeof actors);
	NetLayout lay = { .gw = SQUARE_W, .gh = SQUARE_H, .cell = &cells[0][0], .seed = seed, .pad = &plan.pad[0][0],
		.props = plan.props, .nprops = plan.nprops };
	if (!netmap_build(plan.area, &lay)) return false;
	world_hash = geometry_hash();
	mapslot_town(false);
	mapslot_reset();
	NpcList npcs;
	memset(&npcs, 0, sizeof npcs);
	props(&npcs);
	npc_objects_sprites(&npcs);
	if (!people(&npcs)) return false;
	group = R.layout->net_area[plan.area].group;
	number = R.layout->net_area[plan.area].number;
	if (!mapslot_install(group, number, &npcs, NULL, 0)) return false;
	return mapslot_music(group, number, R.layout->net_area[plan.area].song);
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
	fprintf(stderr, "online: square area %d, arrival at %d,%d\n", plan.area, x, y);
	director_map_label("Net Square", "Net Square");
	emu_warp(group, number, x, y, 5);
	return true;
}

bool online_square_arrived(void) {
	return emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_GAME &&
		emu_read8(BN6_GAMESTATE) == BN6_SUB_MAP &&
		emu_read8(BN6_MAP_GROUP) == group && emu_read8(BN6_MAP_NUMBER) == number;
}

uint32_t online_square_hash(void) { return world_hash; }

bool online_square_keeper_say(const uint8_t *archive, int n) {
	if (!keeper_text || n <= 0 || n > KEEPER_TEXT) return false;
	emu_write(keeper_text, archive, (size_t)n);
	return true;
}

uint32_t online_square_talk_room(void) { return talk_room; }

bool online_square_trade_say(const uint8_t *archive, int n) {
	if (!trade_text || n <= 0 || n > KEEPER_TEXT) return false;
	emu_write(trade_text, archive, (size_t)n);
	return true;
}

bool online_square_on_exit(int x, int y) { return abs(x - exit_x) <= EXIT_REACH && abs(y - exit_y) <= EXIT_REACH; }

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
	uint8_t position[6];
	put16(position, (uint16_t)peer->x);
	put16(position + 2, (uint16_t)peer->y);
	put16(position + 4, (uint16_t)peer->z);
	emu_write(at + SCRIPT_POSITION, position, sizeof position);
	actors[slot].id = peer->id;
	actors[slot].x = peer->x;
	actors[slot].y = peer->y;
	actors[slot].z = peer->z;
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
	for (int k = 0; k < 2; ++k) {
		int fx = fixture[k][0] - (x + dx), fy = fixture[k][1] - (y + dy);
		if (fx * fx + fy * fy <= fixture[k][2] * fixture[k][2]) return -1;
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
