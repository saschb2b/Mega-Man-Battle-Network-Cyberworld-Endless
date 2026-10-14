#include "online_square.h"

#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "flags.h"
#include "gamecall.h"
#include "mapslot.h"
#include "netmap.h"
#include "npc.h"
#include "rom.h"

#define SQUARE_SIDE 24
#define SQUARE_PEERS 8
#define PEER_SPRITE 55  /* list 6's MegaMan, bn6f npcSpritePtrs[0x37] */
#define SCRIPT_LOOP 9
#define SCRIPT_VISIBLE SCRIPT_LOOP
#define SCRIPT_ANIM (SCRIPT_LOOP + 2)
#define SCRIPT_POSITION (SCRIPT_LOOP + 4)
#define SQUARE_WRITER "online square"

static uint8_t cells[SQUARE_SIDE * SQUARE_SIDE];
static NetLayout square;
static int group, number;
static uint32_t world_hash;
static struct {
	uint32_t script, id;
	int x, y, z, moving;
	bool visible;
} actors[SQUARE_PEERS];

static uint32_t hash_byte(uint32_t h, uint8_t b) { return (h ^ b) * 16777619u; }

static uint32_t geometry_hash(uint32_t seed) {
	uint32_t h = 2166136261u;
	for (unsigned i = 0; i < 4; ++i) h = hash_byte(h, (uint8_t)(seed >> (i * 8)));
	for (unsigned i = 0; i < sizeof cells; ++i) h = hash_byte(h, cells[i]);
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

static bool install_square(uint32_t seed) {
	memset(cells, 0, sizeof cells);
	memset(&layer, 0, sizeof layer);
	memset(actors, 0, sizeof actors);
	/* A generous shared floor and a seed-selected alcove. This prototype's
	 * geometry is independent of a run, its inventory, and all emulator RAM. */
	for (int y = 6; y < 18; ++y)
		for (int x = 6; x < 18; ++x) cells[y * SQUARE_SIDE + x] = C_PATH;
	int ax = seed & 1u ? 18 : 3;
	for (int y = 10; y < 14; ++y)
		for (int x = ax; x < ax + 3; ++x) cells[y * SQUARE_SIDE + x] = C_PATH;
	square = (NetLayout){ .gw = SQUARE_SIDE, .gh = SQUARE_SIDE, .cell = cells, .seed = seed };
	if (!netmap_build(0, &square)) return false;
	world_hash = geometry_hash(seed);
	mapslot_town(false);
	mapslot_reset();
	NpcList npcs = { 0 };
	if (!npc_need_sprite(&npcs, 6, PEER_SPRITE)) return false;
	for (int i = 0; i < SQUARE_PEERS; ++i) {
		actors[i].script = peer_script();
		if (!actors[i].script) return false;
		npcs.script[npcs.n++] = actors[i].script;
	}
	group = R.layout->net_area[0].group;
	number = R.layout->net_area[0].number;
	if (!mapslot_install(group, number, &npcs, NULL, 0)) return false;
	return mapslot_music(group, number, R.layout->net_area[0].song);
}

bool online_square_build(uint32_t seed, uint32_t player_id) {
	claims_begin(SQUARE_WRITER);
	bool ok = install_square(seed);
	claims_end();
	if (!ok) return false;
	int x, y;
	netmap_world(11, 11, &x, &y);
	/* Separated by half a panel, peers can see the other arrival without
	 * changing how BN6 moves or collides with the map. */
	x += (int)(player_id % 4u) * 16;
	emu_warp(group, number, x, y, 3);
	return true;
}

bool online_square_arrived(void) {
	return emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_GAME &&
		emu_read8(BN6_GAMESTATE) == BN6_SUB_MAP &&
		emu_read8(BN6_MAP_GROUP) == group && emu_read8(BN6_MAP_NUMBER) == number;
}

uint32_t online_square_hash(void) { return world_hash; }

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

void online_square_peers(const OnlineSnapshot *snapshot) {
	/* No battles, jacking out, or cartridge saves from the technical room.
	 * EnterMap clears the no-encounters bit, so set it after each frame. */
	flag_set(BN6_FLAG_NO_ENCOUNTERS);
	flag_set(BN6_FLAG_NO_JACK);
	flag_set(BN6_FLAG_NO_PET_SAVE);
	claims_begin(SQUARE_WRITER);
	for (unsigned i = 0; i < SQUARE_PEERS; ++i)
		peer_at(i, snapshot->connected && i < snapshot->peer_count ? &snapshot->peers[i] : NULL);
	claims_end();
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
