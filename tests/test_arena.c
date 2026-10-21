/* The Arena's ROM-free parts (issue #120): the keys' buffer and its
 * delay, the local link's latency, a carried MegaMan checked and written
 * into a made-up core's memory, the messages on the wire (as the server's
 * protocol.py packs them) and two network links back to back; the
 * stands' messages, a watched battle's keys and its report (issue #128). */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena_format.h"
#include "arena_input.h"
#include "arena_link.h"
#include "arena_link_net.h"
#include "arena_loadout.h"
#include "arena_sync.h"
#include "arena_text.h"
#include "arena_wire.h"
#include "bn6.h"
#include "stands_report.h"
#include "stands_wire.h"
#include "watch_keys.h"

static void buffer(void) {
	ArenaInput in;
	arena_input_init(&in, 1);
	assert(in.delay == ARENA_DELAY_MIN);
	arena_input_init(&in, 1000);
	assert(in.delay == ARENA_DELAY_MAX);
	arena_input_init(&in, 6);
	/* the first D frames released on both sides: frame 0 needs 0 and 1, known */
	assert(arena_input_ready(&in, 0));
	uint16_t keys[ARENA_LOOKAHEAD][2];
	for (unsigned f = 0; f < 5; ++f) assert(arena_input_take(&in, 0, keys) && keys[0][0] == 0 && keys[1][1] == 0);
	assert(!arena_input_ready(&in, 0) && !arena_input_take(&in, 0, keys));
	/* each stream in order: a resent frame left alone, a gap refused */
	assert(arena_input_put(&in, 0, 6, 0xFFFF) == ARENA_PUT_OK);
	assert(arena_input_put(&in, 0, 6, 1) == ARENA_PUT_OLD);
	assert(arena_input_put(&in, 0, 8, 1) == ARENA_PUT_GAP);
	assert(arena_input_put(&in, 2, 6, 1) == ARENA_PUT_GAP);
	assert(!arena_input_ready(&in, 0));   /* (the peer's frame 6 not yet in) */
	assert(arena_input_put(&in, 1, 6, 0x10) == ARENA_PUT_OK);
	assert(arena_input_take(&in, 0, keys));
	/* frame 5's keys, and the one after: frame 6, masked to ten buttons */
	assert(keys[0][0] == 0 && keys[1][0] == ARENA_KEYS_MASK && keys[1][1] == 0x10 && in.next == 6);
	/* no further ahead than the ring holds */
	for (uint32_t f = 7; f < in.next + ARENA_RING; ++f) assert(arena_input_put(&in, 0, f, (uint16_t)f) == ARENA_PUT_OK);
	assert(arena_input_put(&in, 0, in.next + ARENA_RING, 1) == ARENA_PUT_FULL);
}

/* A rollback window (issue #128): frames run before the other's keys
 * come, each guessed as his last held; one that comes otherwise says the
 * first frame that read it, and the frames from there take what is known */
static void guesses(void) {
	ArenaInput in;
	arena_input_init(&in, 1);
	uint16_t keys[ARENA_LOOKAHEAD][2];
	/* frame 0 needs frame 1's keys: mine sampled, his not yet come */
	assert(arena_input_put(&in, 0, 1, 5) == ARENA_PUT_OK);
	assert(!arena_input_ready(&in, 0) && arena_input_ready(&in, 3));
	assert(arena_input_take(&in, 3, keys) && keys[1][0] == 5 && keys[1][1] == 0);
	assert(arena_input_put(&in, 0, 2, 6) == ARENA_PUT_OK && arena_input_take(&in, 3, keys));
	assert(arena_input_put(&in, 0, 3, 7) == ARENA_PUT_OK && arena_input_take(&in, 3, keys));
	/* three frames on guesses: as many as the window */
	assert(arena_input_put(&in, 0, 4, 8) == ARENA_PUT_OK && !arena_input_ready(&in, 3) && arena_input_settled(&in) == 0);
	/* his frame 1 as guessed: nothing to roll back, frame 0 settled */
	assert(arena_input_put(&in, 1, 1, 0) == ARENA_PUT_OK && arena_input_wrong(&in) == ARENA_NO_FRAME && arena_input_settled(&in) == 1);
	/* his frame 2 otherwise: frames 1 and 2 read it, back to frame 1 */
	assert(arena_input_put(&in, 1, 2, 9) == ARENA_PUT_OK && arena_input_wrong(&in) == 1);
	arena_input_rewind(&in, arena_input_wrong(&in));
	assert(in.next == 1 && arena_input_wrong(&in) == ARENA_NO_FRAME);
	/* run again: frame 1 with his frame 2 as it came, frame 2 with his 3 guessed anew, held from it */
	assert(arena_input_take(&in, 3, keys) && keys[0][0] == 5 && keys[0][1] == 0 && keys[1][0] == 6 && keys[1][1] == 9);
	assert(arena_input_take(&in, 3, keys) && keys[1][0] == 7 && keys[1][1] == 9 && in.next == 3);
	/* his frame 3 as the new guess: nothing wrong */
	assert(arena_input_put(&in, 1, 3, 9) == ARENA_PUT_OK && arena_input_wrong(&in) == ARENA_NO_FRAME && arena_input_settled(&in) == 3);
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
		if (in.next <= t && !arena_input_ready(&in, 0)) ++stalls;
		for (unsigned k = 0; k < 2 && in.next <= t && arena_input_take(&in, 0, frame_keys); ++k) {
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
	assert(play(6, 7, 600, &ran) == 2 && ran == 598);
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
	l.folder[7] = 0xFFFF;   /* (an empty entry is no chip) */
	assert(arena_loadout_check(&l, NULL, 0));
	l.folder[8] = (uint16_t)(1 | 27u << 9);
	assert(!arena_loadout_check(&l, why, sizeof why) && strstr(why, "code 27"));
	l = starter();
	l.max_hp = 0;
	assert(!arena_loadout_check(&l, why, sizeof why) && strstr(why, "HP"));
	/* full rules, every NetBattle's (no ROM here, as on either game): BN5's chips in the folder, BN6's blanks the registry
	 * fills, the pack's extension and the Souls go; the codes and HP are held as BN6 holds them */
	l = starter();
	l.folder[7] = (uint16_t)(411 | 12u << 9);   /* MrkCan1 M */
	l.folder[8] = 205;                         /* WideSht2, a blank of BN6's */
	l.pack[411][0] = 2;
	l.pack[511][3] = 1;
	l.souls = 0x04;
	assert(arena_loadout_check(&l, why, sizeof why));
	l.folder[9] = (uint16_t)(411 | 27u << 9);
	assert(!arena_loadout_check(&l, why, sizeof why) && strstr(why, "code 27"));
	l.folder[9] = 0xFFFF;
	l.max_hp = 10000;
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
	l.folder[8] = (uint16_t)(1 | 27u << 9);
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
	/* ARENA !BBIH (v9: no rules byte): a challenge of session 0x01020304 at 40 ms; latency clamped, a zero ref refused */
	assert(arena_wire_action(m, ARENA_ACT_CHALLENGE, 0x01020304u, 40) == ONLINE_ARENA_BYTES && ONLINE_ARENA_BYTES == 8);
	static const uint8_t challenge[] = { 17, 1, 1, 2, 3, 4, 0, 40 };
	assert(!memcmp(m, challenge, sizeof challenge));
	assert(arena_wire_action(m, ARENA_ACT_OVER, 9, 99999) && m[6] == 0x13 && m[7] == 0x88);
	assert(!arena_wire_action(m, ARENA_ACT_ACCEPT, 0, 0) && !arena_wire_action(m, 5, 1, 0) && !arena_wire_action(m, 0, 1, 0));
	/* ARENA_STATE !BBII16sBB: START, match 7, the other 9, side 1, delay 8 */
	uint8_t st[ONLINE_ARENA_STATE_BYTES] = { 18, 3, 0, 0, 0, 7, 0, 0, 0, 9 };
	memset(st + 10, 0xAB, ONLINE_IDENTITY_BYTES);
	st[26] = 1;
	st[27] = 8;
	ArenaWireState s;
	assert(arena_wire_state(st, sizeof st, &s) && s.stage == ARENA_WIRE_START && s.match == 7 && s.other == 9);
	assert(s.side == 1 && s.delay == 8 && s.identity[15] == 0xAB && ONLINE_ARENA_STATE_BYTES == 28);
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
	l.folder[8] = (uint16_t)(1 | 27u << 9);
	n = arena_loadout_pack(&l, b, sizeof b);
	assert(arena_loadout_unpack(b, n, &back, &carried, NULL, 0) && !arena_loadout_check(&back, why, sizeof why) && strstr(why, "code 27"));
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
	bool stalled = in->next <= t && !arena_input_ready(in, 0);
	for (unsigned k = 0; k < 2 && in->next <= t && arena_input_take(in, 0, frame_keys); ++k) {
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
		/* (the installs' record first, side 1's another: told apart from the ready's hash, kept beside it) */
		assert(arena_link_net_record(&net[side], side ? 0xB5B5 : 0xB5));
		assert(arena_link_net_ready(&net[side], 0xFEED));
	}
	uint64_t h;
	assert(!arena_link_net_peer_ready(&net[0], &h) && !arena_link_net_peer_record(&net[0], &h));
	for (uint32_t t = 0; t < ticks; ++t)
		for (unsigned side = 0; side < 2; ++side) {
			pipes[0].now = pipes[1].now = t;
			pipe_deliver(&pipes[side ^ 1], &net[side]);
			if (t >= latency) assert(arena_link_net_peer_ready(&net[side], &h) && h == 0xFEED);
			if (t >= latency) assert(arena_link_net_peer_record(&net[side], &h) && h == (side ? 0xB5u : 0xB5B5u));
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

/* Two games' clocks, the second started `offset` ticks after the first (so
 * the first leads), each `half[i]` ticks from the server, which passes the
 * keys on at once; each game's sync told its round trip to the server;
 * ticks of wall time run, each game holding as its sync says. Returns the
 * holds of each, and in `late` the keys that came after their frame was
 * due, from the fifth second on */
typedef struct { uint32_t holds[2], late, last_hold[2]; } SyncRun;

static SyncRun sync_run(unsigned offset, const unsigned half[2], unsigned delay, unsigned ticks) {
	enum { QUEUE = 512 };
	ArenaSync sync[2];
	uint32_t clock[2] = { 0, 0 };   /* ticks sampled: each game's next tick */
	struct { uint32_t at, frame; } q[2][QUEUE];
	unsigned head[2] = { 0, 0 }, tail[2] = { 0, 0 };
	SyncRun r = { { 0, 0 }, 0, { 0, 0 } };
	for (unsigned i = 0; i < 2; ++i) {
		arena_sync_init(&sync[i]);
		arena_sync_rtt(&sync[i], (uint32_t)(2 * half[i] * 1000 / 60));
	}
	for (uint32_t w = 0; w < ticks; ++w) {
		for (unsigned i = 0; i < 2; ++i) {
			if (i == 1 && w < offset) continue;
			if (arena_sync_hold(&sync[i])) {
				assert(!r.holds[i] || w - r.last_hold[i] >= ARENA_SYNC_SPREAD);
				++r.holds[i];
				r.last_hold[i] = w;
			} else {
				unsigned k = tail[i]++ % QUEUE;
				q[i][k].at = w + half[0] + half[1];
				q[i][k].frame = clock[i]++ + delay;
			}
			/* the other's keys come: their lag, and whether they came in time (frame - 3 not yet due) */
			unsigned o = i ^ 1;
			while (head[o] != tail[o] && q[o][head[o] % QUEUE].at <= w) {
				uint32_t frame = q[o][head[o]++ % QUEUE].frame;
				int32_t now = (int32_t)clock[i] - 1;
				arena_sync_arrived(&sync[i], now - (int32_t)(frame - delay));
				if (w > 300 && (int32_t)frame - 3 < now) ++r.late;
			}
		}
	}
	return r;
}

static void sync(void) {
	/* the target: the round trip's ticks and one */
	ArenaSync s;
	arena_sync_init(&s);
	arena_sync_rtt(&s, 100);
	arena_sync_rtt(&s, 0);
	arena_sync_rtt(&s, 120);
	assert(s.rtt_ms == 100 && arena_sync_target(&s) == 7);
	/* no hold before a second's keys are judged, nor without a round trip */
	ArenaSync none;
	arena_sync_init(&none);
	for (int i = 0; i < 200; ++i) { arena_sync_arrived(&none, 40); assert(!arena_sync_hold(&none)); }
	/* the game whose ready came last leads by the one way (here four ticks, each 2 from the server) and
	 * at the server's delay (3, a tick, the round trip) its keys came late: it holds, spread out, until
	 * its lag is its round trip and a tick, then never again; the other never holds; nothing is late */
	const unsigned even[2] = { 2, 2 };
	SyncRun r = sync_run(4, even, 3 + 1 + 4, 1800);
	assert(r.holds[0] == 3 && r.holds[1] == 0 && r.last_hold[0] < 300 && r.late == 0);
	/* asymmetric halves (the server nearer the first): each holds its lag at its own round trip */
	const unsigned uneven[2] = { 1, 3 };
	r = sync_run(4, uneven, 3 + 1 + 6, 1800);
	assert(r.holds[0] == 5 && r.holds[1] == 0 && r.late == 0);
	/* clocks already even: nobody holds */
	r = sync_run(0, even, 3 + 1 + 4, 1800);
	assert(r.holds[0] == 0 && r.holds[1] == 0 && r.late == 0);
	/* the round trips said for the delay: mean and twice the spread, one a ping */
	ArenaPings p;
	memset(&p, 0, sizeof p);
	assert(arena_pings_latency(&p) == 0);
	arena_pings_note(&p, 50, 1000);
	arena_pings_note(&p, 90, 1500);   /* (within the same ping: not kept) */
	assert(p.n == 1 && arena_pings_latency(&p) == 50);
	arena_pings_note(&p, 70, 3000);
	assert(p.n == 2 && arena_pings_latency(&p) == 80);   /* 60 and twice 10 */
	for (uint32_t t = 5000; t < 40000; t += 2000) arena_pings_note(&p, 60, t);
	assert(p.n == ARENA_SYNC_PINGS && arena_pings_latency(&p) == 60);
	/* the window a frame's cost affords (the Nova's 1.07 ms a battle frame: 9; this machine's 0.9: 10, the most), and the latency said less what it covers */
	assert(arena_window_for(1.07) == 9 && arena_window_for(0.9) == ARENA_WINDOW_MOST && arena_window_for(6.0) == 0 && arena_window_for(0) == 0);
	assert(arena_ask_ms(100, 0) == 100 && arena_ask_ms(100, 4) == 34 && arena_ask_ms(60, 4) == 0);
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

/* the folders that hold chips, and the carried MegaMan's folder (every
 * NetBattle by full rules: his Souls and BN5's chips go) */
static void formats(void) {
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
	assert(arena_folders_fitting(&l) == 3u);
	l.folders[1][0] = 0xFFFF;
	for (unsigned i = 1; i < BN6_FOLDER_ENTRIES; ++i) l.folders[1][i] = 0;
	assert(arena_folders_fitting(&l) == 1u);
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) l.folders[1][i] = (uint16_t)(1 + i);
	arena_format_carry(&l, 1, &out);
	assert(!memcmp(out.folder, l.folders[1], sizeof out.folder) && out.regular == 4 && out.tag[0] == 2 && out.tag[1] == 7);
	assert(out.souls == 0x21 && out.pack[411][0] == 3 && out.pack[214][1] == 1 && out.pack[5][2] == 2);
	arena_format_carry(&l, 0, &out);
	assert(out.souls == 0x21 && out.pack[411][0] == 3 && out.folder[0] == (411 | 12u << 9));
	/* (what full rules carry, both games check alike) */
	char why[160];
	out.max_hp = 100;
	assert(arena_loadout_check(&out, why, sizeof why));
}

/* the stands (issue #128): their messages as the server's protocol.py packs them */
static void stands(void) {
	ArenaReport r = { .hp = { 80, 0 }, .max_hp = { 100, 100 }, .chip = { 54, 1 }, .used = { 2, 1 }, .winner = 1 };
	uint8_t m[ONLINE_MAX_MESSAGE];
	assert(stands_wire_status(m, 7, &r) == ONLINE_ARENA_STATUS_BYTES && m[0] == ONLINE_ARENA_STATUS && m[4] == 7);
	assert(m[5] == 0 && m[6] == 80 && m[8] == 100 && m[14] == 54 && m[17] == 2 && m[19] == 1);
	ArenaReport bad = r;
	bad.hp[1] = 101;
	assert(!stands_wire_status(m, 7, &bad) && !stands_wire_status(m, 0, &r));
	bad = r;
	bad.chip[0] = ONLINE_CHIP_LAST + 1;
	assert(!stands_wire_status(m, 7, &bad));
	assert(stands_wire_watch(m, WATCH_ACT_WATCH, 9) == ONLINE_WATCH_BYTES && m[0] == ONLINE_WATCH && m[1] == 1 && m[5] == 9);
	assert(!stands_wire_watch(m, 3, 9) && !stands_wire_watch(m, WATCH_ACT_STOP, 0));
	/* STANDS: one battle on, its report */
	uint8_t s[2 + ONLINE_STANDS_ENTRY_BYTES] = { ONLINE_STANDS, 1 };
	uint8_t *e = s + 2;
	e[3] = 7; e[7] = 1; e[11] = 2;
	memcpy(e + 12, "Alpha", 5);
	memcpy(e + 28, "Beta", 4);
	e[44] = STANDS_FIGHTING; e[45] = 2; e[46] = 3;
	e[48] = 80; e[50] = 100; e[52] = 20; e[54] = 100; e[56] = 54; e[58] = 1; e[59] = 2; e[60] = 1;
	StandsEntry got[ONLINE_STANDS_MAX];
	assert(stands_wire_read(s, sizeof s, got) == 1 && got[0].match == 7 && got[0].side[0] == 1 && got[0].side[1] == 2);
	assert(!strcmp(got[0].name[0], "Alpha") && !strcmp(got[0].name[1], "Beta") && got[0].watchers == 2 && got[0].cheers == 3);
	assert(got[0].report.hp[0] == 80 && got[0].report.hp[1] == 20 && got[0].report.chip[0] == 54 && got[0].report.used[0] == 2);
	assert(stands_wire_read(s, sizeof s - 1, got) < 0);
	e[44] = 9;
	assert(stands_wire_read(s, sizeof s, got) < 0);
	/* WATCH_STATE, WATCH_KEYS, WATCH_RELAY */
	uint8_t st[ONLINE_WATCH_STATE_BYTES] = { ONLINE_WATCH_STATE, WATCH_ON, 0, 0, 0, 7, 0, 0, 0, 1, 0, 0, 0, 2, 6 };
	WatchState w;
	assert(stands_wire_state(st, sizeof st, &w) && w.match == 7 && w.side[1] == 2 && w.delay == 6);
	st[1] = 3;
	assert(!stands_wire_state(st, sizeof st, &w));
	uint8_t k[ONLINE_WATCH_KEYS_HEAD + 4] = { ONLINE_WATCH_KEYS, 0, 0, 0, 7, 1, 0, 0, 0, 6, 0, 2, 0x01, 0x02, 0x03, 0xFF };
	uint32_t match, first;
	unsigned side, count;
	const uint8_t *keys;
	assert(stands_wire_keys(k, sizeof k, &match, &side, &first, &keys, &count) && match == 7 && side == 1 && first == 6 && count == 2);
	assert(!stands_wire_keys(k, sizeof k - 1, &match, &side, &first, &keys, &count));
	uint8_t hashed[2 + ARENA_HASH_BYTES] = { ONLINE_WATCH_RELAY, 0 };
	assert(arena_wire_hash(hashed + 2, 7, 59, 0x1234));
	const uint8_t *inner;
	size_t n;
	assert(stands_wire_relay(hashed, sizeof hashed, &side, &inner, &n) && side == 0 && n == ARENA_HASH_BYTES && inner[0] == ONLINE_ARENA_HASH);
	hashed[2] = ONLINE_ARENA_INPUT;
	assert(!stands_wire_relay(hashed, sizeof hashed, &side, &inner, &n));
}

/* a watched battle's keys: from the first frame, a stretch known left, a gap refused */
static void watch_keys(void) {
	WatchKeys w = { 0 };
	uint8_t be[2 * 300];
	for (unsigned i = 0; i < 300; ++i) { be[2 * i] = (uint8_t)(i >> 8 | 0xFC); be[2 * i + 1] = (uint8_t)i; }
	assert(watch_keys_put(&w, 0, 0, be, 6) && watch_keys_runnable(&w) == 0);
	assert(watch_keys_put(&w, 1, 0, be, 4) && watch_keys_runnable(&w) == 3);
	assert(!watch_keys_put(&w, 1, 5, be, 1));   /* (frame 4 missing) */
	assert(watch_keys_put(&w, 1, 2, be, 300) && w.known[1] == 302 && watch_keys_runnable(&w) == 5);
	assert(watch_keys_put(&w, 0, 6, be + 12, 294) && watch_keys_runnable(&w) == 299);
	uint16_t out[ARENA_LOOKAHEAD][2];
	watch_keys_take(&w, 4, out);
	/* (the keys' ten bits: the top six of the byte pair dropped) */
	assert(out[0][0] == 4 && out[0][1] == 2 && out[1][0] == 5 && out[1][1] == 3);
	watch_keys_free(&w);
	assert(!w.known[0] && !w.keys[0]);
}

/* the report off the player objects: a chip counted as it gives way under a chip's action, a charged shot not */
static void report(void) {
	ArenaReport r = { 0 };
	ReportSeen seen = { 0 };
	stands_report_side(&r, &seen, 0, 100, 100, 0x08, 54);
	stands_report_side(&r, &seen, 0, 100, 100, 0x12, 0xFFFF);
	assert(r.used[0] == 1 && r.chip[0] == 54);
	stands_report_side(&r, &seen, 0, 100, 100, 0x08, 0xFFFF);
	stands_report_side(&r, &seen, 0, 100, 100, 0x16, 0xFFFF);
	assert(r.used[0] == 1);
	stands_report_side(&r, &seen, 1, 120, 100, 0x08, 1);
	assert(r.hp[1] == 100 && r.max_hp[1] == 100 && !r.used[1]);
}

/* the PET's words on the Arena's screens (issue #167): every stop names
 * no frame, the other's named where he is the cause; the set's score, mine
 * first; a carried MegaMan's Crosses and Souls by name, "---" none */
static void pet_text(void) {
	for (int why = ARENA_STOP_ERROR; why <= ARENA_STOP_MEMORY; ++why) {
		const char *s = arena_stop_line((ArenaStop)why, "Alpha");
		assert(*s && !strstr(s, "frame") && !strstr(s, "lag") && !strstr(s, "delay"));
	}
	assert(strstr(arena_stop_line(ARENA_STOP_LEFT, "Alpha"), "Alpha left"));
	assert(strstr(arena_stop_line(ARENA_STOP_SILENT, "Alpha"), "Alpha's PET"));
	assert(!strcmp(arena_score(2, 1), "2 - 1"));
	assert(!strcmp(arena_crosses_named(0x5), "Heat Slash"));
	assert(!strcmp(arena_souls_named(0x21), "Colonel Toad"));
	assert(!strcmp(arena_souls_named(0), arena_none));
	assert(strstr(arena_waiting_for("Beta"), "Beta's PET"));
}

int main(void) {
	pet_text();
	formats();
	buffer();
	guesses();
	link_latency();
	check();
	install();
	wire();
	loadout_bytes();
	link_net();
	sync();
	stands();
	watch_keys();
	report();
	puts("test_arena: ok");
	return 0;
}
