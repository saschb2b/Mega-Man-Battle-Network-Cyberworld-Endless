/* The Arena's ROM-free parts (issue #120): the keys' buffer and its
 * delay, the local link's latency, a carried MegaMan checked and written
 * into a made-up core's memory, the messages on the wire (as the server's
 * protocol.py packs them) and two network links back to back. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena_format.h"
#include "arena_input.h"
#include "arena_link.h"
#include "arena_link_net.h"
#include "arena_loadout.h"
#include "arena_wire.h"
#include "bn6.h"

static void buffer(void) {
	ArenaInput in;
	arena_input_init(&in, 1);
	assert(in.delay == ARENA_DELAY_MIN);
	arena_input_init(&in, 1000);
	assert(in.delay == ARENA_DELAY_MAX);
	arena_input_init(&in, 6);
	/* the first D frames released on both sides: frame 0 needs 0-3, known */
	assert(arena_input_ready(&in));
	uint16_t keys[ARENA_LOOKAHEAD][2];
	for (unsigned f = 0; f < 3; ++f) assert(arena_input_take(&in, keys) && keys[0][0] == 0 && keys[3][1] == 0);
	assert(!arena_input_ready(&in) && !arena_input_take(&in, keys));
	/* each stream in order: a resent frame left alone, a gap refused */
	assert(arena_input_put(&in, 0, 6, 0xFFFF) == ARENA_PUT_OK);
	assert(arena_input_put(&in, 0, 6, 1) == ARENA_PUT_OLD);
	assert(arena_input_put(&in, 0, 8, 1) == ARENA_PUT_GAP);
	assert(arena_input_put(&in, 2, 6, 1) == ARENA_PUT_GAP);
	assert(!arena_input_ready(&in));   /* (the peer's frame 6 not yet in) */
	assert(arena_input_put(&in, 1, 6, 0x10) == ARENA_PUT_OK);
	assert(arena_input_take(&in, keys));
	/* frame 3's keys, and the three after: frame 6 is the fourth, masked to ten buttons */
	assert(keys[0][0] == 0 && keys[3][0] == ARENA_KEYS_MASK && keys[3][1] == 0x10 && in.next == 4);
	/* no further ahead than the ring holds */
	for (uint32_t f = 7; f < in.next + ARENA_RING; ++f) assert(arena_input_put(&in, 0, f, (uint16_t)f) == ARENA_PUT_OK);
	assert(arena_input_put(&in, 0, in.next + ARENA_RING, 1) == ARENA_PUT_FULL);
}

static uint16_t counting_peer(void *context, uint32_t frame) {
	++*(unsigned *)context;
	return (uint16_t)(frame * 7);
}

/* The arena's tick against the local link: ticks it stalled in `ticks` */
static unsigned play(unsigned delay, unsigned latency, unsigned ticks, unsigned *ran) {
	ArenaLocalLink local;
	unsigned made = 0;
	ArenaLink *link = arena_link_local(&local, delay, latency, counting_peer, &made);
	ArenaInput in;
	arena_input_init(&in, link->delay);
	unsigned stalls = 0;
	*ran = 0;
	for (uint32_t t = 0; t < ticks; ++t) {
		link->tick(link);
		uint32_t frame = t + in.delay;
		assert(arena_input_put(&in, 0, frame, (uint16_t)t) == ARENA_PUT_OK && link->send(link, frame, (uint16_t)t));
		uint16_t keys;
		while (link->poll(link, in.known[1], &keys)) {
			assert(keys == (uint16_t)(in.known[1] * 7) % (ARENA_KEYS_MASK + 1));
			assert(arena_input_put(&in, 1, in.known[1], keys) == ARENA_PUT_OK);
		}
		assert(!link->broken(link));
		/* (as arena_tick: frame N due at tick N) */
		uint16_t frame_keys[ARENA_LOOKAHEAD][2];
		if (in.next <= t && !arena_input_ready(&in)) ++stalls;
		for (unsigned k = 0; k < 2 && in.next <= t && arena_input_take(&in, frame_keys); ++k) {
			uint32_t n = in.next - 1;
			assert(frame_keys[0][0] == (n < in.delay ? 0 : (uint16_t)(n - in.delay)));
			if (n % 60 == 0) link->send_hash(link, n, 0x1234 + n);
			++*ran;
		}
	}
	/* the hashes come back after the latency, and only those sent */
	uint64_t h;
	assert(link->poll_hash(link, 540, &h) && h == 0x1234 + 540 && !link->poll_hash(link, 541, &h));
	assert(made == ticks);
	link->close(link);
	return stalls;
}

static void link_latency(void) {
	unsigned ran;
	/* a delay that covers the lookahead and the latency never waits */
	assert(play(6, 3, 600, &ran) == 0 && ran == 600);
	assert(play(ARENA_DELAY_MIN, 0, 600, &ran) == 0 && ran == 600);
	/* one that does not waits as many ticks as it falls short, once, and
	 * stays that far behind */
	assert(play(6, 5, 600, &ran) == 2 && ran == 598);
	/* the peer's keys never arrive early: none before its latency */
	ArenaLocalLink local;
	ArenaLink *link = arena_link_local(&local, 6, 4, NULL, NULL);
	uint16_t keys;
	for (unsigned t = 0; t < 4; ++t) { link->tick(link); assert(!link->poll(link, 6, &keys)); }
	link->tick(link);   /* (tick 4: tick 0's message there) */
	assert(link->poll(link, 6, &keys) && keys == 0 && !link->poll(link, 7, &keys));
	/* my keys go out in order, once each */
	assert(link->send(link, 6, 1) && !link->send(link, 6, 1) && !link->send(link, 8, 1) && link->send(link, 7, 1));
}

static OnlineLoadout starter(void) {
	OnlineLoadout l;
	memset(&l, 0, sizeof l);
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) l.folder[i] = (uint16_t)((1 + i % 20) | (i % 27) << 9);
	l.pack[5][0] = 3;
	l.board[0] = 0x2A;
	l.grid[24] = 1;
	l.hp = 40;
	l.max_hp = 400;
	l.base_hp = 300;
	l.attack = 2;
	l.crosses = 0x05;
	l.beast = 1;
	l.tag[0] = l.tag[1] = 0xFF;
	return l;
}

static void check(void) {
	char why[128];
	OnlineLoadout l = starter();
	assert(arena_loadout_check(&l, why, sizeof why));
	l.souls = 0x04;
	assert(!arena_loadout_check(&l, why, sizeof why) && strstr(why, "Souls"));
	l = starter();
	l.folder[7] = (uint16_t)(ARENA_BN6_CHIP_LAST + 1);
	assert(!arena_loadout_check(&l, why, sizeof why) && strstr(why, "chip 320"));
	l = starter();
	l.folder[7] = 205;   /* (a blank record of BN6's: a BN5 chip on a game with BN5's ROM) */
	assert(!arena_loadout_check(&l, why, sizeof why) && strstr(why, "chip 205"));
	l = starter();
	l.pack[314][0] = 1;
	assert(!arena_loadout_check(&l, why, sizeof why) && strstr(why, "pack holds chip 314"));
	l = starter();
	l.folder[7] = 0xFFFF;   /* (an empty entry is no chip) */
	assert(arena_loadout_check(&l, NULL, 0));
	l.folder[8] = (uint16_t)(1 | 27u << 9);
	assert(!arena_loadout_check(&l, why, sizeof why) && strstr(why, "code 27"));
	l = starter();
	l.pack[BN6_PACK_IDS + 3][2] = 1;
	assert(!arena_loadout_check(&l, why, sizeof why) && strstr(why, "pack"));
	l = starter();
	l.max_hp = 0;
	assert(!arena_loadout_check(&l, why, sizeof why) && strstr(why, "HP"));
}

/* ---- a made-up core: EWRAM in an array ---- */

static uint8_t ewram[BN6_EWRAM_END - BN6_EWRAM];
static unsigned outside;

static uint32_t fake_read32(void *context, uint32_t a) {
	(void)context;
	if (a < BN6_EWRAM || a + 4 > BN6_EWRAM_END) { ++outside; return 0; }
	const uint8_t *p = ewram + (a - BN6_EWRAM);
	return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24;
}

static void fake_write8(void *context, uint32_t a, uint8_t v) {
	(void)context;
	if (a < BN6_EWRAM || a >= BN6_EWRAM_END) { ++outside; return; }
	ewram[a - BN6_EWRAM] = v;
}

static uint8_t at(uint32_t a) { return ewram[a - BN6_EWRAM]; }
static uint16_t at16(uint32_t a) { return (uint16_t)(at(a) | at(a + 1) << 8); }

static void put32(uint32_t a, uint32_t v) {
	for (unsigned i = 0; i < 4; ++i) ewram[a - BN6_EWRAM + i] = (uint8_t)(v >> (8 * i));
}

static bool flag(unsigned f) { return at(BN6_EVENT_FLAGS + f / 8) & (0x80u >> (f & 7)); }

static void install(void) {
	const ArenaMemory m = { fake_read32, fake_write8, NULL };
	OnlineLoadout l = starter();
	char why[128];
	/* the folder and pack not where BN6 keeps them: refused, nothing written */
	memset(ewram, 0xEE, sizeof ewram);
	put32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS, 0);
	assert(!arena_loadout_install(&m, &l, why, sizeof why) && strstr(why, "folder"));
	put32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS, 0x02002178u);
	put32(BN6_TOOLKIT + BN6_TOOLKIT_PACK, 0x02002230u);
	l.souls = 1;
	assert(!arena_loadout_install(&m, &l, why, sizeof why) && at16(0x02002178u) == 0xEEEE);
	l = starter();
	assert(arena_loadout_install(&m, &l, why, sizeof why) && !outside);
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) assert(at16(0x02002178u + 2 * i) == l.folder[i]);
	assert(at(0x02002230u + 5 * BN6_PACK_ENTRY) == 3 && at(0x02002230u + 6 * BN6_PACK_ENTRY) == 0);
	assert(at(BN6_PACK_EXT) == 0xEE);   /* (the extension is BN5's: left as it was) */
	assert(at(BN6_NAVICUST_PLACED) == 0x2A && at(BN6_NAVICUST_GRID + 24) == 1 && at(BN6_NAVICUST_GRID) == 0);
	/* a NetBattle begins at full HP */
	assert(at16(BN6_NAVI_HP) == 400 && at16(BN6_NAVI_MAX_HP) == 400 && at16(BN6_NAVI_BASE_MAX_HP) == 300);
	assert(at(BN6_NAVI_ATTACK) == 2 && at(BN6_NAVI_FOLDER1_TAG) == 0xFF);
	assert(flag(BN6_FLAG_HEAT_CROSS) && !flag(BN6_FLAG_HEAT_CROSS + 1) && flag(BN6_FLAG_HEAT_CROSS + 2) && flag(BN6_FLAG_BEAST_OUT));
	l.crosses = 0;
	l.beast = 0;
	assert(arena_loadout_install(&m, &l, NULL, 0) && !flag(BN6_FLAG_HEAT_CROSS) && !flag(BN6_FLAG_BEAST_OUT));
}

/* ---- the wire: the server's struct shapes (protocol.py), byte for byte ---- */

static void wire(void) {
	uint8_t m[ONLINE_MAX_MESSAGE];
	/* ARENA !BBIHB: a challenge of session 0x01020304 at 40 ms by BN6's rules; latency clamped, a zero ref
	 * and rules past full refused */
	assert(arena_wire_action(m, ARENA_ACT_CHALLENGE, 0x01020304u, 40, ONLINE_FORMAT_BN6) == ONLINE_ARENA_BYTES);
	static const uint8_t challenge[] = { 17, 1, 1, 2, 3, 4, 0, 40, 0 };
	assert(!memcmp(m, challenge, sizeof challenge));
	assert(arena_wire_action(m, ARENA_ACT_CHALLENGE, 1, 0, ONLINE_FORMAT_FULL) && m[8] == ONLINE_FORMAT_FULL);
	assert(!arena_wire_action(m, ARENA_ACT_CHALLENGE, 1, 0, ONLINE_FORMAT_FULL + 1));
	assert(arena_wire_action(m, ARENA_ACT_OVER, 9, 99999, 0) && m[6] == 0x13 && m[7] == 0x88);
	assert(!arena_wire_action(m, ARENA_ACT_ACCEPT, 0, 0, 0) && !arena_wire_action(m, 5, 1, 0, 0) && !arena_wire_action(m, 0, 1, 0, 0));
	/* ARENA_STATE !BBII16sBBB: START, match 7, the other 9, side 1, delay 8, BN6's rules */
	uint8_t st[ONLINE_ARENA_STATE_BYTES] = { 18, 3, 0, 0, 0, 7, 0, 0, 0, 9 };
	memset(st + 10, 0xAB, ONLINE_IDENTITY_BYTES);
	st[26] = 1;
	st[27] = 8;
	ArenaWireState s;
	assert(arena_wire_state(st, sizeof st, &s) && s.stage == ARENA_WIRE_START && s.match == 7 && s.other == 9);
	assert(s.side == 1 && s.delay == 8 && s.identity[15] == 0xAB && s.rules == ONLINE_FORMAT_BN6);
	st[28] = ONLINE_FORMAT_FULL;
	assert(arena_wire_state(st, sizeof st, &s) && s.rules == ONLINE_FORMAT_FULL);
	st[28] = ONLINE_FORMAT_FULL + 1;
	assert(!arena_wire_state(st, sizeof st, &s));
	st[28] = 0;
	assert(!arena_wire_state(st, sizeof st - 1, &s));
	st[1] = 5;
	assert(!arena_wire_state(st, sizeof st, &s));
	/* ARENA_INPUT !BIIB + n u16: keys from frame 0x100 */
	const uint16_t keys[3] = { 0x0001, 0x0210, 0x03FF };
	assert(arena_wire_input(m, sizeof m, 7, 0x100, keys, 3) == 16);
	static const uint8_t input[] = { 20, 0, 0, 0, 7, 0, 0, 1, 0, 3, 0, 1, 2, 0x10, 3, 0xFF };
	assert(!memcmp(m, input, sizeof input));
	uint32_t match, frame;
	uint16_t back[ONLINE_ARENA_INPUT_MAX];
	unsigned count;
	assert(arena_wire_read_input(m, 16, &match, &frame, back, &count) && match == 7 && frame == 0x100 && count == 3 && back[2] == 0x3FF);
	assert(!arena_wire_read_input(m, 15, &match, &frame, back, &count));
	assert(!arena_wire_input(m, sizeof m, 7, 0, keys, 0) && !arena_wire_input(m, sizeof m, 7, 0, keys, ONLINE_ARENA_INPUT_MAX + 1));
	/* ARENA_HASH !BIIQ */
	assert(arena_wire_hash(m, 7, 59, 0x0123456789ABCDEFull) == 17 && m[0] == 21 && m[9] == 0x01 && m[16] == 0xEF);
	uint64_t h;
	assert(arena_wire_read_hash(m, 17, &match, &frame, &h) && match == 7 && frame == 59 && h == 0x0123456789ABCDEFull);
	/* ARENA_LOADOUT !BIH + bytes */
	const uint8_t bytes[3] = { 1, 0, 0 };
	assert(arena_wire_loadout(m, sizeof m, 7, bytes, 2) == 9 && m[0] == 19 && m[5] == 0 && m[6] == 2);
	const uint8_t *got;
	size_t n;
	assert(arena_wire_read_loadout(m, 9, &match, &got, &n) && match == 7 && n == 2 && got[0] == 1);
	assert(!arena_wire_read_loadout(m, 10, &match, &got, &n) && !arena_wire_loadout(m, sizeof m, 7, bytes, ONLINE_ARENA_LOADOUT_MAX + 1));
}

/* A carried MegaMan's bytes: back as he went, the pack left home; the
 * game's start's in two bytes; bytes not one refused */
static void loadout_bytes(void) {
	OnlineLoadout l = starter(), back;
	l.board[3 * 8 + 5] = 0x77;
	l.grid[30] = 4;
	uint8_t b[ONLINE_ARENA_LOADOUT_MAX];
	bool carried;
	char why[128];
	size_t n = arena_loadout_pack(&l, b, sizeof b);
	assert(n && n <= ONLINE_ARENA_LOADOUT_MAX);
	assert(arena_loadout_unpack(b, n, &back, &carried, why, sizeof why) && carried);
	assert(!memcmp(back.folder, l.folder, sizeof l.folder) && !memcmp(back.board, l.board, sizeof l.board));
	assert(!memcmp(back.grid, l.grid, sizeof l.grid) && back.max_hp == 400 && back.hp == 400 && back.base_hp == 300);
	assert(back.attack == 2 && back.crosses == 5 && back.beast == 1 && back.tag[1] == 0xFF);
	assert(back.pack[5][0] == 0);   /* (the pack stays home) */
	/* the same bytes again from what came: both games install the same MegaMan */
	uint8_t again[ONLINE_ARENA_LOADOUT_MAX];
	assert(arena_loadout_pack(&back, again, sizeof again) == n && !memcmp(again, b, n));
	assert(arena_loadout_pack(NULL, b, sizeof b) == 2 && arena_loadout_unpack(b, 2, &back, &carried, NULL, 0) && !carried);
	n = arena_loadout_pack(&l, b, sizeof b);
	assert(!arena_loadout_unpack(b, n - 1, &back, &carried, why, sizeof why) && strstr(why, "size"));
	b[0] = 2;
	assert(!arena_loadout_unpack(b, n, &back, &carried, why, sizeof why) && strstr(why, "version"));
	/* the receiver's refusals are the local ones */
	l.souls = 2;
	n = arena_loadout_pack(&l, b, sizeof b);
	assert(arena_loadout_unpack(b, n, &back, &carried, NULL, 0) && !arena_loadout_check(&back, why, sizeof why) && strstr(why, "Souls"));
	/* a NaviCust past what the message holds does not go */
	l = starter();
	memset(l.board, 1, sizeof l.board);
	assert(!arena_loadout_pack(&l, b, sizeof b));
}

/* ---- two network links back to back, through a relay that takes `latency` ticks ---- */

#define PIPE 2048
typedef struct {
	uint8_t data[PIPE][ARENA_INPUT_HEAD + 2 * ONLINE_ARENA_INPUT_MAX];
	size_t size[PIPE];
	uint32_t at[PIPE];
	unsigned head, tail;
	uint32_t now, latency;
	bool full;   /* (an outbox with no room) */
} Pipe;

static bool pipe_send(void *context, const uint8_t *data, size_t size) {
	Pipe *p = context;
	if (p->full || p->tail - p->head == PIPE || size > sizeof p->data[0]) return false;
	unsigned i = p->tail++ % PIPE;
	memcpy(p->data[i], data, size);
	p->size[i] = size;
	p->at[i] = p->now + p->latency;
	return true;
}

static void pipe_deliver(Pipe *p, ArenaNetLink *to) {
	while (p->head != p->tail && p->at[p->head % PIPE] <= p->now) {
		unsigned i = p->head++ % PIPE;
		assert(arena_link_net_take(to, p->data[i], p->size[i]));
	}
}

/* one side's tick as arena_tick runs it: its keys for frame f are
 * f * (side + 3); every frame run sees both sides', in order; true where
 * it stalled */
static bool side_tick(ArenaNetLink *net, ArenaInput *in, unsigned side, uint32_t t) {
	ArenaLink *l = &net->link;
	l->tick(l);
	uint32_t frame = t + in->delay;
	uint16_t mine = (uint16_t)(frame * (side + 3) & ARENA_KEYS_MASK);
	assert(arena_input_put(in, side, frame, mine) == ARENA_PUT_OK && l->send(l, frame, mine));
	uint16_t keys;
	while (l->poll(l, in->known[side ^ 1], &keys)) assert(arena_input_put(in, side ^ 1, in->known[side ^ 1], keys) == ARENA_PUT_OK);
	assert(!l->broken(l));
	uint16_t frame_keys[ARENA_LOOKAHEAD][2];
	bool stalled = in->next <= t && !arena_input_ready(in);
	for (unsigned k = 0; k < 2 && in->next <= t && arena_input_take(in, frame_keys); ++k) {
		uint32_t n = in->next - 1;
		for (unsigned p = 0; p < 2; ++p) assert(frame_keys[0][p] == (n < in->delay ? 0 : (uint16_t)(n * (p + 3) & ARENA_KEYS_MASK)));
		if (n % 60 == 59) l->send_hash(l, n, 0x5000 + n);
		uint64_t theirs;
		if (n >= 120 && n % 60 == 59 && l->poll_hash(l, n - 60, &theirs)) assert(theirs == 0x5000 + n - 60);
	}
	return stalled;
}

/* both sides from the ready's exchange; side 0's ticks stalled */
static unsigned play_net(unsigned delay, unsigned latency, unsigned ticks) {
	ArenaNetLink net[2];
	Pipe *pipes = calloc(2, sizeof *pipes);   /* (pipes[i]: side i's messages to the other) */
	ArenaInput in[2];
	unsigned stalls = 0;
	for (unsigned side = 0; side < 2; ++side) {
		pipes[side].latency = latency;
		arena_link_net(&net[side], 77, side, delay, pipe_send, &pipes[side]);
		arena_input_init(&in[side], net[side].link.delay);
		assert(arena_link_net_ready(&net[side], 0xFEED));
	}
	uint64_t h;
	assert(!arena_link_net_peer_ready(&net[0], &h));
	for (uint32_t t = 0; t < ticks; ++t)
		for (unsigned side = 0; side < 2; ++side) {
			pipes[0].now = pipes[1].now = t;
			pipe_deliver(&pipes[side ^ 1], &net[side]);
			if (t >= latency) assert(arena_link_net_peer_ready(&net[side], &h) && h == 0xFEED);
			if (side_tick(&net[side], &in[side], side, t) && side == 0) ++stalls;
		}
	/* the battle over on side 0's: side 1 spends the keys come, then the link breaks, saying so */
	ArenaLink *l1 = &net[1].link;
	arena_link_net_ended(&net[1]);
	for (unsigned t = 0; t < ARENA_NET_GRACE; ++t) { l1->tick(l1); assert(!l1->broken(l1)); }
	l1->tick(l1);
	assert(l1->broken(l1) && strstr(l1->broken(l1), "left"));
	free(pipes);
	return stalls;
}

static void link_net(void) {
	/* a delay past the lookahead and the one way never waits; one short
	 * of it waits */
	assert(play_net(6, 2, 900) == 0);
	assert(play_net(6, 3, 300) == 0);
	assert(play_net(ARENA_DELAY_MIN + 1, 0, 300) == 0);   /* (side 1 sends after side 0 has run its tick) */
	assert(play_net(4, 4, 600) > 0);
	/* another battle's message, a gap */
	ArenaNetLink l;
	Pipe *p = calloc(1, sizeof *p);
	arena_link_net(&l, 5, 1, 6, pipe_send, p);
	uint8_t m[ONLINE_MAX_MESSAGE];
	uint16_t keys[2] = { 1, 2 };
	size_t n = arena_wire_input(m, sizeof m, 9, 6, keys, 2);
	assert(!arena_link_net_take(&l, m, n) && !l.link.broken(&l.link));
	n = arena_wire_input(m, sizeof m, 5, 7, keys, 2);
	assert(arena_link_net_take(&l, m, n) && strstr(l.link.broken(&l.link), "order"));
	/* my keys: in order, once each, a frame a message */
	arena_link_net(&l, 5, 0, 6, pipe_send, p);
	assert(l.link.send(&l.link, 6, 1) && !l.link.send(&l.link, 6, 1) && !l.link.send(&l.link, 8, 1));
	assert(p->tail == 1 && p->size[0] == ARENA_INPUT_HEAD + 2);
	/* ... the outbox full: kept, and sent together once it has room */
	p->full = true;
	for (uint32_t f = 7; f < 10; ++f) assert(l.link.send(&l.link, f, (uint16_t)f) && !l.link.broken(&l.link));
	p->full = false;
	assert(l.link.send(&l.link, 10, 10) && p->tail == 2 && p->size[1] == ARENA_INPUT_HEAD + 8);
	uint32_t match, frame;
	unsigned count;
	uint16_t got[ONLINE_ARENA_INPUT_MAX];
	assert(arena_wire_read_input(p->data[1], p->size[1], &match, &frame, got, &count) && frame == 7 && count == 4 && got[3] == 10);
	/* ... and full for as many frames as a message holds: broken */
	p->full = true;
	for (uint32_t f = 11; f < 11 + ONLINE_ARENA_INPUT_MAX; ++f) assert(l.link.send(&l.link, f, 0));
	assert(strstr(l.link.broken(&l.link), "sent"));
	/* the connection gone: broken at once */
	arena_link_net(&l, 5, 0, 6, pipe_send, p);
	arena_link_net_lost(&l, "gone");
	assert(!strcmp(l.link.broken(&l.link), "gone"));
	free(p);
}

/* the pair's common ground: BN6's rules for any pair until full ones are
 * built; each folder fits or not; the carried MegaMan's folder and what
 * BN6's rules leave home */
static void formats(void) {
	assert(arena_format_for(0, 0) == ONLINE_FORMAT_BN6 && arena_format_for(ONLINE_ROMS_BN5, 0) == ONLINE_FORMAT_BN6);
	assert(arena_format_for(ONLINE_ROMS_BN5, ONLINE_ROMS_BN5) == (ARENA_FULL_RULES ? ONLINE_FORMAT_FULL : ONLINE_FORMAT_BN6));
	assert(arena_format_ok(ONLINE_FORMAT_BN6, 0) && !arena_format_ok(ONLINE_FORMAT_FULL, 0));
	static OnlineLoadout l, out;
	memset(&l, 0, sizeof l);
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		l.folders[0][i] = (uint16_t)(411 | 12u << 9);      /* BN5's MrkCan1 M */
		l.folders[1][i] = (uint16_t)(1 + i);                 /* BN6's own, in A */
	}
	l.folders[0][3] = 5;
	l.regulars[1] = 4; l.tags[1][0] = 2; l.tags[1][1] = 7;
	l.souls = 0x21;
	l.pack[411][0] = 3; l.pack[214][1] = 1; l.pack[5][2] = 2;
	assert(arena_folder_fit(&l, ONLINE_FORMAT_BN6, 0) == ARENA_FOLDER_BN5);
	assert(arena_folder_fit(&l, ONLINE_FORMAT_FULL, 0) == ARENA_FOLDER_FITS);
	assert(arena_folder_fit(&l, ONLINE_FORMAT_BN6, 1) == ARENA_FOLDER_FITS);
	assert(arena_folder_fit(&l, ONLINE_FORMAT_BN6, 2) == ARENA_FOLDER_EMPTY);
	assert(arena_folders_fitting(&l, ONLINE_FORMAT_BN6) == 2u && arena_folders_fitting(&l, ONLINE_FORMAT_FULL) == 3u);
	arena_format_carry(&l, ONLINE_FORMAT_BN6, 1, &out);
	assert(!memcmp(out.folder, l.folders[1], sizeof out.folder) && out.regular == 4 && out.tag[0] == 2 && out.tag[1] == 7);
	assert(!out.souls && !out.pack[411][0] && !out.pack[214][1] && out.pack[5][2] == 2);
	char why[160];
	out.max_hp = 100;
	assert(arena_loadout_check(&out, why, sizeof why));
	arena_format_carry(&l, ONLINE_FORMAT_FULL, 0, &out);
	assert(out.souls == 0x21 && out.pack[411][0] == 3 && out.folder[0] == (411 | 12u << 9));
}

int main(void) {
	formats();
	buffer();
	link_latency();
	check();
	install();
	wire();
	loadout_bytes();
	link_net();
	puts("test_arena: ok");
	return 0;
}
