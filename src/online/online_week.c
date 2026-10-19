#include "online_week.h"

#include <string.h>

#include "online_transport.h"
#include "online_wire.h"

#define HEAD 6   /* u8 kind, u16 week, u8 count, u8 last week's, u8 place */

static WeekBoard K;

void online_week_reset(void) { memset(&K, 0, sizeof K); }

const WeekBoard *online_week_board(void) { return &K; }

int64_t online_week_start(uint16_t week) { return ONLINE_WEEK_EPOCH + (int64_t)week * 7 * 24 * 3600; }

static uint16_t get16(const uint8_t *p) { return (uint16_t)(p[0] << 8 | p[1]); }

/* an entry: its name (16 bytes, zero-padded), layer, act, won, minutes */
static bool entry(const uint8_t *e, WeekEntry *out, bool blank_ok) {
	memset(out, 0, sizeof *out);
	memcpy(out->name, e, ONLINE_NAME_BYTES);
	out->depth = get16(e + 16);
	out->act = e[18];
	out->won = e[19] & 1;
	out->minutes = get16(e + 20);
	if (blank_ok && !out->depth && !out->name[0]) return true;
	return online_wire_name(out->name) && out->depth >= 1 && out->depth <= ONLINE_WEEK_LAYERS && out->act >= 1 && out->act <= 4 && e[19] <= 1;
}

bool online_week_mail(const uint8_t *data, size_t size) {
	if (size < HEAD || data[0] != ONLINE_WEEK_BOARD || data[3] > ONLINE_WEEK_TOP || data[4] > ONLINE_WEEK_LAST_TOP) return false;
	int ntop = data[3], nlast = data[4];
	if (size != HEAD + (size_t)(1 + ntop + nlast) * ONLINE_WEEK_ENTRY_BYTES) return false;
	WeekBoard got = { 0 };
	got.week = get16(data + 1);
	got.place = data[5];
	const uint8_t *e = data + HEAD;
	if (!entry(e, &got.mine, true) || (got.place && !got.mine.depth)) return false;
	for (int k = 0; k < ntop; ++k)
		if (!entry(e + (size_t)(1 + k) * ONLINE_WEEK_ENTRY_BYTES, &got.top[k], false)) return false;
	for (int k = 0; k < nlast; ++k)
		if (!entry(e + (size_t)(1 + ntop + k) * ONLINE_WEEK_ENTRY_BYTES, &got.last[k], false)) return false;
	got.ntop = ntop;
	got.nlast = nlast;
	got.serial = K.serial + 1;
	K = got;
	return true;
}

bool online_week_post(uint16_t week, uint32_t seed, int depth, int act, bool won, unsigned minutes, uint32_t hash) {
	uint8_t m[ONLINE_WEEK_POST_BYTES] = { ONLINE_WEEK_POST };
	online_put16(m + 1, week);
	online_put32(m + 3, seed);
	online_put16(m + 7, (uint16_t)depth);
	m[9] = (uint8_t)act;
	m[10] = won ? 1 : 0;
	online_put16(m + 11, (uint16_t)(minutes > 0xFFFF ? 0xFFFF : minutes));
	online_put32(m + 13, hash);
	return online_transport_send(m, sizeof m);
}
