/* The stands' messages on the wire (stands_wire.h). */
#include "stands_wire.h"

#include <string.h>

static void put16(uint8_t *o, unsigned v) { o[0] = (uint8_t)(v >> 8); o[1] = (uint8_t)v; }
static void put32(uint8_t *o, uint32_t v) { put16(o, v >> 16); put16(o + 2, v & 0xFFFFu); }
static uint16_t get16(const uint8_t *i) { return (uint16_t)((unsigned)i[0] << 8 | i[1]); }
static uint32_t get32(const uint8_t *i) { return (uint32_t)get16(i) << 16 | get16(i + 2); }

static bool report_ok(const ArenaReport *r) {
	for (int s = 0; s < 2; ++s)
		if (r->hp[s] > r->max_hp[s] || r->max_hp[s] > 9999 || r->chip[s] > ONLINE_CHIP_LAST) return false;
	return r->winner <= 2;
}

/* (the report's 15 bytes, as ARENA_STATUS and a STANDS entry hold them) */
static void report_put(uint8_t *o, const ArenaReport *r) {
	put16(o, r->hp[0]); put16(o + 2, r->max_hp[0]); put16(o + 4, r->hp[1]); put16(o + 6, r->max_hp[1]);
	put16(o + 8, r->chip[0]); put16(o + 10, r->chip[1]);
	o[12] = r->used[0]; o[13] = r->used[1]; o[14] = r->winner;
}

static ArenaReport report_get(const uint8_t *i) {
	ArenaReport r = { .hp = { get16(i), get16(i + 4) }, .max_hp = { get16(i + 2), get16(i + 6) }, .chip = { get16(i + 8), get16(i + 10) },
		.used = { i[12], i[13] }, .winner = i[14] };
	return r;
}

int stands_wire_read(const uint8_t *data, size_t size, StandsEntry out[ONLINE_STANDS_MAX]) {
	if (size < 2 || data[0] != ONLINE_STANDS || data[1] > ONLINE_STANDS_MAX || size != 2u + data[1] * (size_t)ONLINE_STANDS_ENTRY_BYTES) return -1;
	for (int k = 0; k < data[1]; ++k) {
		const uint8_t *e = data + 2 + k * ONLINE_STANDS_ENTRY_BYTES;
		StandsEntry s = { .match = get32(e), .side = { get32(e + 4), get32(e + 8) }, .stage = e[44], .watchers = e[45], .cheers = e[46],
			.report = report_get(e + 47), .bout = e[62] };
		for (int side = 0; side < 2; ++side) {
			memcpy(s.name[side], e + 12 + 16 * side, ONLINE_NAME_BYTES);
			s.name[side][ONLINE_NAME_BYTES] = 0;
		}
		if (!s.match || s.stage < STANDS_WAY_IN || s.stage > STANDS_STOPPED || !report_ok(&s.report)) return -1;
		out[k] = s;
	}
	return data[1];
}

size_t stands_wire_status(uint8_t out[ONLINE_ARENA_STATUS_BYTES], uint32_t match, const ArenaReport *r) {
	if (!match || !report_ok(r)) return 0;
	out[0] = ONLINE_ARENA_STATUS;
	put32(out + 1, match);
	report_put(out + 5, r);
	return ONLINE_ARENA_STATUS_BYTES;
}

size_t stands_wire_watch(uint8_t out[ONLINE_WATCH_BYTES], unsigned action, uint32_t match) {
	if ((action != WATCH_ACT_WATCH && action != WATCH_ACT_STOP) || !match) return 0;
	out[0] = ONLINE_WATCH;
	out[1] = (uint8_t)action;
	put32(out + 2, match);
	return ONLINE_WATCH_BYTES;
}

bool stands_wire_state(const uint8_t *data, size_t size, WatchState *out) {
	if (size != ONLINE_WATCH_STATE_BYTES || data[0] != ONLINE_WATCH_STATE) return false;
	WatchState s = { .stage = data[1], .match = get32(data + 2), .side = { get32(data + 6), get32(data + 10) }, .delay = data[14],
		.form = data[15], .partner = { get32(data + 16), get32(data + 20) } };
	if ((s.stage != WATCH_ON && s.stage != WATCH_OVER) || !s.match || s.form > 1) return false;
	*out = s;
	return true;
}

bool stands_wire_keys(const uint8_t *data, size_t size, uint32_t *match, unsigned *side, uint32_t *first, const uint8_t **keys,
	unsigned *count) {
	if (size < ONLINE_WATCH_KEYS_HEAD || data[0] != ONLINE_WATCH_KEYS || data[5] >= ONLINE_SEATS) return false;
	unsigned n = get16(data + 10);
	if (!n || n > ONLINE_WATCH_KEYS_MAX || size != ONLINE_WATCH_KEYS_HEAD + 2u * n) return false;
	*match = get32(data + 1);
	*side = data[5];
	*first = get32(data + 6);
	*keys = data + ONLINE_WATCH_KEYS_HEAD;
	*count = n;
	return true;
}

bool stands_wire_relay(const uint8_t *data, size_t size, unsigned *side, const uint8_t **inner, size_t *n) {
	if (size < 3 || data[0] != ONLINE_WATCH_RELAY || data[1] >= ONLINE_SEATS) return false;
	if (data[2] != ONLINE_ARENA_LOADOUT && data[2] != ONLINE_ARENA_HASH) return false;
	*side = data[1];
	*inner = data + 2;
	*n = size - 2;
	return true;
}
