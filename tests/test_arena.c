/* The Arena's ROM-free parts (issue #120): the keys' buffer and its
 * delay, the local link's latency, and a carried MegaMan checked and
 * written into a made-up core's memory. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "arena_input.h"
#include "arena_link.h"
#include "arena_loadout.h"
#include "bn6.h"

static void buffer(void) {
	ArenaInput in;
	arena_input_init(&in, 1);
	assert(in.delay == ARENA_DELAY_MIN);
	arena_input_init(&in, 1000);
	assert(in.delay == ARENA_DELAY_MAX);
	arena_input_init(&in, 6);
	/* the first D frames released on both sides: frame 0 needs 0-3, known */
	assert(arena_input_ready(&in) && arena_input_runnable(&in) == 3);
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

int main(void) {
	buffer();
	link_latency();
	check();
	install();
	puts("test_arena: ok");
	return 0;
}
