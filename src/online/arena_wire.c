/* The Arena's messages on the wire (arena_wire.h). */
#include "arena_wire.h"

#include <stdio.h>
#include <string.h>

static void put16(uint8_t *o, unsigned v) { o[0] = (uint8_t)(v >> 8); o[1] = (uint8_t)v; }
static void put32(uint8_t *o, uint32_t v) { put16(o, v >> 16); put16(o + 2, v & 0xFFFFu); }
static uint16_t get16(const uint8_t *i) { return (uint16_t)((unsigned)i[0] << 8 | i[1]); }
static uint32_t get32(const uint8_t *i) { return (uint32_t)get16(i) << 16 | get16(i + 2); }

size_t arena_wire_action(uint8_t out[ONLINE_ARENA_BYTES], unsigned action, uint32_t ref, uint32_t latency_ms, unsigned form) {
	if (action < ARENA_ACT_CHALLENGE || action > ARENA_ACT_SEEN || !ref || form > ONLINE_TEAM_SIDE_BY_SIDE) return 0;
	out[0] = ONLINE_ARENA;
	out[1] = (uint8_t)action;
	put32(out + 2, ref);
	put16(out + 6, latency_ms > ARENA_LATENCY_MOST ? ARENA_LATENCY_MOST : latency_ms);
	out[8] = (uint8_t)form;
	return ONLINE_ARENA_BYTES;
}

bool arena_wire_state(const uint8_t *data, size_t size, ArenaWireState *out) {
	if (size != ONLINE_ARENA_STATE_BYTES || data[0] != ONLINE_ARENA_STATE) return false;
	ArenaWireState s = { .stage = data[1], .match = get32(data + 2), .other = get32(data + 6), .side = data[26], .delay = data[27],
		.form = data[28] };
	memcpy(s.identity, data + 10, ONLINE_IDENTITY_BYTES);
	for (unsigned k = 0; k < ONLINE_SEATS; ++k) s.seats[k] = get32(data + 29 + 4 * k);
	/* (a seat past the GBAs' only in a 2v2's START) */
	bool four = s.stage == ARENA_WIRE_START && s.seats[2] && s.seats[3];
	if (s.stage < ARENA_WIRE_CHALLENGED || s.stage > ARENA_WIRE_SEEN || !s.match || s.side >= (four ? ONLINE_SEATS : 2) ||
	    s.form > ONLINE_TEAM_SIDE_BY_SIDE) return false;
	*out = s;
	return true;
}

bool arena_wire_seat(const uint8_t *data, size_t size, unsigned *seat, const uint8_t **inner, size_t *n) {
	if (size < 3 || data[0] != ONLINE_ARENA_SEAT || data[1] >= ONLINE_SEATS) return false;
	*seat = data[1];
	*inner = data + 2;
	*n = size - 2;
	return true;
}

size_t arena_wire_team(uint8_t out[ONLINE_TEAM_BYTES], unsigned action, uint32_t ref, bool value) {
	if (action < TEAM_ACT_PROPOSE || action > TEAM_ACT_ORDER || (!ref && action != TEAM_ACT_SPLIT)) return 0;
	out[0] = ONLINE_TEAM;
	out[1] = (uint8_t)action;
	put32(out + 2, ref);
	out[6] = value;
	return ONLINE_TEAM_BYTES;
}

bool arena_wire_team_state(const uint8_t *data, size_t size, TeamWireState *out) {
	if (size != ONLINE_TEAM_STATE_BYTES || data[0] != ONLINE_TEAM_STATE) return false;
	TeamWireState s = { .stage = data[1], .partner = get32(data + 2), .match = get32(data + 6), .bout = get32(data + 10),
		.left = { data[30], data[31] }, .number = data[32], .result = data[33], .hp = get16(data + 34), .form = data[36] };
	for (int k = 0; k < 4; ++k) s.players[k] = get32(data + 14 + 4 * k);
	if (s.stage < TEAM_ASKED || s.stage > TEAM_OVER || s.result > TEAM_DRAWN || s.left[0] > 2 || s.left[1] > 2 ||
	    s.form > ONLINE_TEAM_SIDE_BY_SIDE) return false;
	*out = s;
	return true;
}

size_t arena_wire_input(uint8_t *out, size_t cap, uint32_t match, uint32_t frame, const uint16_t *keys, unsigned count) {
	size_t size = ARENA_INPUT_HEAD + 2u * count;
	if (!match || !count || count > ONLINE_ARENA_INPUT_MAX || cap < size) return 0;
	out[0] = ONLINE_ARENA_INPUT;
	put32(out + 1, match);
	put32(out + 5, frame);
	out[9] = (uint8_t)count;
	for (unsigned i = 0; i < count; ++i) put16(out + ARENA_INPUT_HEAD + 2 * i, keys[i]);
	return size;
}

bool arena_wire_read_input(const uint8_t *data, size_t size, uint32_t *match, uint32_t *frame,
	uint16_t keys[ONLINE_ARENA_INPUT_MAX], unsigned *count) {
	if (size < ARENA_INPUT_HEAD || data[0] != ONLINE_ARENA_INPUT) return false;
	unsigned n = data[9];
	if (!n || n > ONLINE_ARENA_INPUT_MAX || size != ARENA_INPUT_HEAD + 2u * n) return false;
	*match = get32(data + 1);
	*frame = get32(data + 5);
	*count = n;
	for (unsigned i = 0; i < n; ++i) keys[i] = get16(data + ARENA_INPUT_HEAD + 2 * i);
	return true;
}

size_t arena_wire_hash(uint8_t out[ARENA_HASH_BYTES], uint32_t match, uint32_t frame, uint64_t hash) {
	if (!match) return 0;
	out[0] = ONLINE_ARENA_HASH;
	put32(out + 1, match);
	put32(out + 5, frame);
	put32(out + 9, (uint32_t)(hash >> 32));
	put32(out + 13, (uint32_t)hash);
	return ARENA_HASH_BYTES;
}

bool arena_wire_read_hash(const uint8_t *data, size_t size, uint32_t *match, uint32_t *frame, uint64_t *hash) {
	if (size != ARENA_HASH_BYTES || data[0] != ONLINE_ARENA_HASH) return false;
	*match = get32(data + 1);
	*frame = get32(data + 5);
	*hash = (uint64_t)get32(data + 9) << 32 | get32(data + 13);
	return true;
}

size_t arena_wire_loadout(uint8_t *out, size_t cap, uint32_t match, const uint8_t *bytes, size_t n) {
	if (!match || !n || n > ONLINE_ARENA_LOADOUT_MAX || cap < ARENA_LOADOUT_HEAD + n) return 0;
	out[0] = ONLINE_ARENA_LOADOUT;
	put32(out + 1, match);
	put16(out + 5, (unsigned)n);
	memcpy(out + ARENA_LOADOUT_HEAD, bytes, n);
	return ARENA_LOADOUT_HEAD + n;
}

bool arena_wire_read_loadout(const uint8_t *data, size_t size, uint32_t *match, const uint8_t **bytes, size_t *n) {
	if (size < ARENA_LOADOUT_HEAD || data[0] != ONLINE_ARENA_LOADOUT) return false;
	size_t length = get16(data + 5);
	if (!length || length > ONLINE_ARENA_LOADOUT_MAX || size != ARENA_LOADOUT_HEAD + length) return false;
	*match = get32(data + 1);
	*bytes = data + ARENA_LOADOUT_HEAD;
	*n = length;
	return true;
}

/* A carried MegaMan's bytes (version 1): version, carried (0: nothing
 * follows), the folder (30 u16), the placed programs (a count, then each
 * slot's number and its 8 bytes), the grid (49), max and base HP (u16),
 * and twelve bytes: attack, speed, charge, Regular memory, MegaChip and
 * GigaChip limits, the Regular and the two TagChips, Crosses, Souls,
 * Beast Out */
#define SLOT_BYTES 8
#define HEAD_BYTES (3 + 2 * BN6_FOLDER_ENTRIES)   /* the version, carried, the folder and the count of programs */
#define TAIL_BYTES 12
#define HP_BYTES 6    /* max HP, base HP, and HP (v2) */

size_t arena_loadout_pack(const OnlineLoadout *l, uint8_t *out, size_t cap) {
	if (cap < 2) return 0;
	out[0] = ARENA_LOADOUT_VERSION;
	out[1] = l != NULL;
	if (!l) return 2;
	unsigned placed = 0;
	for (unsigned s = 0; s < BN6_NAVICUST_SLOTS; ++s)
		for (unsigned k = 0; k < SLOT_BYTES; ++k) if (l->board[s * SLOT_BYTES + k]) { ++placed; break; }
	size_t size = HEAD_BYTES + placed * (1 + SLOT_BYTES) + ONLINE_BOARD_CELLS + HP_BYTES + TAIL_BYTES;
	if (size > cap || size > ONLINE_ARENA_LOADOUT_MAX) return 0;
	uint8_t *o = out + 2;
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i, o += 2) put16(o, l->folder[i]);
	*o++ = (uint8_t)placed;
	for (unsigned s = 0; s < BN6_NAVICUST_SLOTS; ++s) {
		const uint8_t *record = l->board + s * SLOT_BYTES;
		bool any = false;
		for (unsigned k = 0; k < SLOT_BYTES; ++k) any = any || record[k];
		if (!any) continue;
		*o++ = (uint8_t)s;
		memcpy(o, record, SLOT_BYTES);
		o += SLOT_BYTES;
	}
	memcpy(o, l->grid, ONLINE_BOARD_CELLS);
	o += ONLINE_BOARD_CELLS;
	put16(o, l->max_hp);
	put16(o + 2, l->base_hp);
	put16(o + 4, l->hp);
	o += HP_BYTES;
	const uint8_t tail[TAIL_BYTES] = { l->attack, l->speed, l->charge, l->regular_mb, l->mega_limit, l->giga_limit,
		l->regular, l->tag[0], l->tag[1], l->crosses, l->souls, l->beast };
	memcpy(o, tail, TAIL_BYTES);
	return size;
}

#define BAD(...) ((why && whyn ? (void)snprintf(why, whyn, __VA_ARGS__) : (void)0), false)

bool arena_loadout_unpack(const uint8_t *in, size_t n, OnlineLoadout *out, bool *carried, char *why, size_t whyn) {
	memset(out, 0, sizeof *out);
	*carried = false;
	if (n < 2 || in[0] != ARENA_LOADOUT_VERSION) return BAD("the other PET's MegaMan is in another version's form");
	if (!in[1]) return n == 2 ? true : BAD("the other PET's MegaMan came with bytes too many");
	size_t head = HEAD_BYTES;
	if (n < head) return BAD("the other PET's MegaMan came short");
	const uint8_t *i = in + 2;
	for (unsigned k = 0; k < BN6_FOLDER_ENTRIES; ++k, i += 2) out->folder[k] = get16(i);
	unsigned placed = *i++;
	if (n != head + placed * (1 + SLOT_BYTES) + ONLINE_BOARD_CELLS + HP_BYTES + TAIL_BYTES) return BAD("the other PET's MegaMan came in the wrong size");
	for (unsigned k = 0; k < placed; ++k, i += 1 + SLOT_BYTES) {
		if (i[0] >= BN6_NAVICUST_SLOTS) return BAD("the other PET's NaviCust has a program out of its list");
		memcpy(out->board + i[0] * SLOT_BYTES, i + 1, SLOT_BYTES);
	}
	memcpy(out->grid, i, ONLINE_BOARD_CELLS);
	for (unsigned k = 0; k < ONLINE_BOARD_CELLS; ++k)
		if (out->grid[k] > BN6_NAVICUST_SLOTS) return BAD("the other PET's NaviCust grid names a program it has not");
	i += ONLINE_BOARD_CELLS;
	out->max_hp = get16(i);
	out->base_hp = get16(i + 2);
	out->hp = get16(i + 4);
	if (!out->hp || out->hp > out->max_hp) return BAD("the other PET's MegaMan came with HP past his max");
	i += HP_BYTES;
	out->attack = i[0]; out->speed = i[1]; out->charge = i[2];
	out->regular_mb = i[3]; out->mega_limit = i[4]; out->giga_limit = i[5];
	out->regular = i[6]; out->tag[0] = i[7]; out->tag[1] = i[8];
	out->crosses = i[9]; out->souls = i[10]; out->beast = i[11];
	*carried = true;
	return true;
}
