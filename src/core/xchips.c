/* Another game's chips as BN6's, by name (xchips.h). */
#include "xchips.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "chips.h"
#include "data.h"
#include "rom.h"
#include "xchips_registry.h"

static bool same(const char *a, const char *b) { return a[0] && !strncmp(a, b, XCHIP_NAME); }

int xchips_pair(const char (*names6)[XCHIP_NAME], int n6, const char (*namesx)[XCHIP_NAME], int nx, uint16_t *to_x, uint16_t *from_x) {
	int paired = 0;
	for (int i = 0; i < n6; ++i) {
		to_x[i] = 0;
		for (int j = 1; i > 0 && j < nx && !to_x[i]; ++j)
			if (same(names6[i], namesx[j])) to_x[i] = (uint16_t)j;
		paired += to_x[i] != 0;
	}
	/* (and back: each of the other game's chips of a name BN6 has) */
	for (int j = 0; j < nx; ++j) {
		from_x[j] = 0;
		for (int i = 1; j > 0 && i < n6 && !from_x[j]; ++i)
			if (same(namesx[j], names6[i])) from_x[j] = (uint16_t)i;
	}
	return paired;
}

int xchips_code(const uint8_t codes[4], int code) {
	for (int i = 0; i < 4; ++i)
		if (codes[i] == code) return code;
	for (int i = 0; i < 4; ++i)
		if (codes[i] == 26) return 26;
	return codes[0] <= 26 ? codes[0] : code;
}

int xchips_out(const uint16_t *folder, int n, const uint16_t *to_x, int n6, uint16_t *out, int max) {
	int k = 0;
	for (int i = 0; i < n; ++i) {
		int id = folder[i] & 0x1FF;
		if (folder[i] == 0xFFFF || !id || (id < n6 && to_x[id])) continue;
		bool again = false;
		for (int e = 0; e < i && !again; ++e) again = folder[e] != 0xFFFF && (folder[e] & 0x1FF) == id;
		if (again) continue;
		if (k < max) out[k] = (uint16_t)id;
		++k;
	}
	return k;
}

static int has(const uint8_t codes[4], int code) {
	for (int i = 0; i < 4; ++i)
		if (codes[i] == code) return 1;
	return 0;
}

int xchips_fit(const uint8_t other[4], const uint8_t bn6[4], const uint8_t folder[3], int code, int row) {
	if (!(row & 1)) return code;
	for (int k = 0; k < 3 && folder[k]; ++k)
		if (has(other, folder[k] - 1) && has(bn6, folder[k] - 1)) return folder[k] - 1;
	return code;
}

/* ---- the run's chips and BN5's, paired at run time ---- */

static uint16_t to_bn5[XREG_IDS], from_bn5[BN5_CHIPS];
static int paired;   /* BN6's chips paired by name (on the main thread: the director's words read them) */

/* (once, where both ROMs are read; BN5's names in BN6's text codes first:
 * its "M-Cannon" spells the dash otherwise, rom.h's xrom_text_bn6). BN6's
 * own chips by name; the registry's chips of BN5's in BN6's tables
 * (xchips_registry.h) BN5's chips they are, but none of BN5's comes back
 * as one of them yet (a reward of one is the stand-in's zenny, until
 * issue #82's run systems bring BN5's chips into runs) */
static void pair(void) {
	if (paired || !XR[XROM_BN5_COLONEL_US].data || !R.data) return;
	static char names5[BN5_CHIPS][XCHIP_NAME], names6[XCHIPS_BN6][XCHIP_NAME];
	for (int i = 1; i < BN5_CHIPS; ++i)
		xrom_text(XROM_BN5_COLONEL_US, i < 256 ? BN5_CHIP_NAMES_LOW : BN5_CHIP_NAMES_HIGH, i < 256 ? i : i - 256, names5[i], sizeof names5[i]);
	for (int id = 1; id < XCHIPS_BN6; ++id) {
		ChipInfo ci;
		chip_info(id, &ci);
		snprintf(names6[id], sizeof names6[id], "%s", xreg_at(id) ? "" : ci.name);
	}
	paired = xchips_pair((const char (*)[XCHIP_NAME])names6, XCHIPS_BN6, (const char (*)[XCHIP_NAME])names5, BN5_CHIPS, to_bn5, from_bn5);
	for (int i = 0; i < xreg_count; ++i)
		if (xreg_own(&xreg_rows[i])) to_bn5[xreg_rows[i].bn6] = xreg_rows[i].bn5;
}

int xchips_paired(void) {
	pair();
	return paired;
}

int xchips_to_bn5(int id) {
	pair();
	return paired && id > 0 && id < XREG_IDS ? to_bn5[id] : 0;
}

int xchips_bn6_of(int x) {
	pair();
	return paired && x > 0 && x < BN5_CHIPS ? from_bn5[x] : 0;
}

int xchips_run_chip(int x) {
	const XChipReg *r = xchips_paired() ? xreg_of_bn5(x) : NULL;
	int id = r ? r->bn6 : xchips_bn6_of(x);
	return id && !chips_sits_out(id) ? id : 0;
}

int xchips_kind(int kind, uint16_t *out, int max) {
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	if (!xchips_paired() || !d || kind < 0) return 0;
	int n = 0;
	for (int id = 1; id < XCHIPS_BN6_STANDARD && n < max; ++id)
		if (to_bn5[id] && d[BN5_CHIP_RECORDS + 0x2Cu * to_bn5[id] + BN5_CHIP_KIND] == kind) out[n++] = (uint16_t)id;
	return n;
}

int xchips_sitting_out(const uint16_t *folder, uint16_t *out, int max) {
	return xchips_paired() ? xchips_out(folder, 30, to_bn5, XREG_IDS, out, max) : 0;
}

/* BN6's chip `id`'s four codes as its record holds them (0-25 A-Z, 26 *,
 * 0xFF none: the All * helper's aside, which the battle's results apply
 * as they lean, encounter.c) */
static void codes_of(int id, uint8_t out[4]) {
	const uint8_t *rec = chips_record(id);
	for (uint32_t i = 0; i < 4; ++i) out[i] = rec ? rec[BN6_CHIP_CODES + i] : 0xFF;
}

uint16_t xchips_reward_bn6(uint16_t v) {
	if (v == 0xFFFF || v >> 14) return v;
	/* (the registry's id where it registers the chip, BN6's of its name
	 * otherwise; none, or one that sits out of BN6's battles, the
	 * stand-in's zenny: docs/MULTIROM.md, One net, BN5's viruses' own
	 * rewards) */
	int id = xchips_run_chip(v & 0x1FF);
	if (!id) return (uint16_t)(1u << 14 | XCHIPS_STAND_IN);
	uint8_t codes[4];
	codes_of(id, codes);
	return (uint16_t)(id | xchips_code(codes, v >> 9 & 0x1F) << 9);
}
