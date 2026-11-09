/* Another game's chips as BN6's, by name (xchips.h). */
#include "xchips.h"

#include "chip_entry.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "bn4.h"
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
		int id = (int)chip_entry_id(folder[i]);
		if (folder[i] == 0xFFFF || !id || (id < n6 && to_x[id])) continue;
		bool again = false;
		for (int e = 0; e < i && !again; ++e) again = folder[e] != 0xFFFF && (int)chip_entry_id(folder[e]) == id;
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

/* BN6's own chips' names, a registry's id of its own "" (none of another
 * game's chips comes back as one of them yet) */
static char names6[XCHIPS_BN6][XCHIP_NAME];

static void bn6_names(void) {
	for (int id = 1; id < XCHIPS_BN6; ++id) {
		ChipInfo ci;
		chip_info(id, &ci);
		snprintf(names6[id], sizeof names6[id], "%s", xreg_at(id) ? "" : ci.name);
	}
}

/* (once, where both ROMs are read; BN5's names in BN6's text codes first:
 * its "M-Cannon" spells the dash otherwise, rom.h's xrom_text_bn6). BN6's
 * own chips by name; the registry's chips of BN5's in BN6's tables
 * (xchips_registry.h) BN5's chips they are, but none of BN5's comes back
 * as one of them yet (a reward of one is the stand-in's zenny, until
 * issue #82's run systems bring BN5's chips into runs) */
static char names5[BN5_CHIPS][XCHIP_NAME];   /* (BN5's names in BN6's codes, once pair() has read them) */

static void pair(void) {
	if (paired || !XR[XROM_BN5_COLONEL_US].data || !R.data) return;
	for (int i = 1; i < BN5_CHIPS; ++i)
		xrom_text(XROM_BN5_COLONEL_US, i < 256 ? BN5_CHIP_NAMES_LOW : BN5_CHIP_NAMES_HIGH, i < 256 ? i : i - 256, names5[i], sizeof names5[i]);
	bn6_names();
	paired = xchips_pair((const char (*)[XCHIP_NAME])names6, XCHIPS_BN6, (const char (*)[XCHIP_NAME])names5, BN5_CHIPS, to_bn5, from_bn5);
	for (int i = 0; i < xreg_count; ++i)
		if (xreg_own(&xreg_rows[i])) to_bn5[xreg_rows[i].bn6] = xreg_rows[i].bn5;
}

static int xchips_paired(void) {
	pair();
	return paired;
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


int xchips_darkloid_chip(int ai, int version) {
	/* (BN5's V1 chip of each Darkloid the registry holds, its SP and DS
	 * after it: its records, family 0x41 by the Navi's subfamily) */
	static const struct { uint8_t ai; uint16_t v1; } V1[] = { { BN5_NAVI_SHADEMAN, 260 }, { BN5_NAVI_BLIZZARDMAN, 263 } };
	int step = version == XCHIPS_VERSION_SP ? 1 : version == XCHIPS_VERSION_DS ? 2 : version ? -1 : 0;
	for (size_t i = 0; i < sizeof V1 / sizeof *V1 && step >= 0; ++i)
		if (V1[i].ai == ai) return xchips_run_chip(V1[i].v1 + step);
	return 0;
}

/* BN6's chip `id`'s four codes as its record holds them (0-25 A-Z, 26 *,
 * 0xFF none: the All * helper's aside, which the battle's results apply
 * as they lean, encounter.c) */
static void codes_of(int id, uint8_t out[4]) {
	const uint8_t *rec = chips_record(id);
	for (uint32_t i = 0; i < 4; ++i) out[i] = rec ? rec[BN6_CHIP_CODES + i] : 0xFF;
}

/* A chip entry as BN6's chip `id` in its code (the stand-in's zenny where
 * there is none, or where it sits out of BN6's battles) */
static uint16_t reward_as(int id, uint16_t v) {
	if (!id || chips_sits_out(id)) return (uint16_t)(1u << 14 | XCHIPS_STAND_IN);
	uint8_t codes[4];
	codes_of(id, codes);
	return reward_chip((unsigned)id, (unsigned)xchips_code(codes, (int)(chip9_code(v) & 0x1F)));
}

uint16_t xchips_reward_bn6(uint16_t v) {
	if (v == 0xFFFF || v >> 14) return v;
	/* (the registry's id where it registers the chip, BN6's of its name
	 * otherwise; none, or one that sits out of BN6's battles, the
	 * stand-in's zenny: docs/multirom/one-net.md, BN5's viruses' own
	 * rewards) */
	return reward_as(xchips_run_chip((int)chip9_id(v)), v);
}

/* ---- BN4 Red Sun's chips as a run's (docs/multirom/bn4.md, Its viruses'
 * rewards; docs/multirom/bn4-chips.md, The build plan's unit 0) ---- */

#define BUS 0x08000000u
static uint16_t from_bn4[BN4_CHIP_COUNT];
static bool bn4_paired;

int xchips_v1_of(const uint8_t *version, const uint16_t *navi, const uint8_t *cls, int n, int x) {
	if (x <= 0 || x >= n || cls[x] != 1 || (version[x] != XCHIPS_VERSION_SP && version[x] != XCHIPS_VERSION_DS)) return 0;
	int v = x - (version[x] - XCHIPS_VERSION_SP + 1);
	return v > 0 && cls[v] == 1 && !version[v] && navi[v] == navi[x] ? v : 0;
}

bool xchips_sp_named(const char *name) {
	size_t n = strlen(name);
	return n && name[n - 1] == XCHIPS_SP_MARK;
}

static const uint8_t *bn4_record(const uint8_t *d4, int x) { return d4 + (BN4_CHIP_RECORDS - BUS) + BN6_CHIP_RECORD_SIZE * (uint32_t)x; }

/* the registry's own id for BN5's chip named `name` (BN5's ROM read), 0
 * none or a Program Advance (no reward names one) */
static int registry_named(const char *name) {
	if (!paired) return 0;
	for (int j = 1; j < BN5_CHIPS; ++j) {
		const XChipReg *r = same(name, names5[j]) ? xreg_of_bn5(j) : NULL;
		if (r && xreg_own(r) && r->how != XREG_PA) return r->bn6;
	}
	return 0;
}

/* The run's chip of V1 `y`'s Navi in BN4's version `version`: the
 * registry's row of BN5's id as many after the V1's, its record of that
 * version (BN5 orders them V1, SP, DS as BN4 does); else BN6's own chip of
 * the V1's family and subfamily whose name carries the SP mark (BN6's
 * Navi chips are V1, EX and SP: none is a DS); 0 none */
static int run_version(int y, int version) {
	const XChipReg *v1 = xreg_at(y);
	if (v1) {
		const XChipReg *r = paired ? xreg_of_bn5(v1->bn5 + version - XCHIPS_VERSION_SP + 1) : NULL;
		const uint8_t *d5 = XR[XROM_BN5_COLONEL_US].data;
		return r && xreg_own(r) && d5[BN5_CHIP_RECORDS + BN6_CHIP_RECORD_SIZE * (uint32_t)r->bn5 + BN6_CHIP_PARAMS] == version ? r->bn6 : 0;
	}
	const uint8_t *ry = chips_record(y);
	if (version != XCHIPS_VERSION_SP || !ry) return 0;
	for (int i = 1; i < XCHIPS_BN6; ++i) {
		const uint8_t *ri = chips_record(i);
		if (ri && ri[BN6_CHIP_FAMILY] == ry[BN6_CHIP_FAMILY] && ri[BN6_CHIP_SUBFAMILY] == ry[BN6_CHIP_SUBFAMILY] && xchips_sp_named(names6[i])) return i;
	}
	return 0;
}

/* (once, where both ROMs are read; BN5's registry rows where its ROM is
 * read too) A BN4 chip pays the run's chip it is, by three rules, each
 * one chip per name as the registry's (docs/multirom/bn4-chips.md, unit
 * 0): by name, BN6's own of its name whatever their classes (BN6's ten
 * standard chips BN4 calls Mega: Muramasa to SuprVulc, a folder chip in
 * both); else by name, the registry's own id of BN5's chip of its name
 * (Z Saver, the moved, the playing records, a DarkChip: its 34 that a
 * reward can name, none of its Program Advances); else by Navi and
 * version, BN4's SP and DS (spelt with letters where BN6 and BN5 mark
 * them) as the run's chip of their V1's Navi in that version. BN4's
 * pseudo-chips and Program Advances pair none (its Giga FinalGun's
 * namesake in BN6 is a pseudo-chip: it waits for an id of its own) */
static void pair_bn4(void) {
	const uint8_t *d4 = XR[XROM_BN4_RED_SUN_US].data;
	if (bn4_paired || !d4 || !R.data) return;
	pair();
	static char names4[BN4_CHIP_COUNT][XCHIP_NAME];
	static uint16_t to_bn4[XCHIPS_BN6], navi[BN4_CHIP_COUNT];
	static uint8_t cls[BN4_CHIP_COUNT], version[BN4_CHIP_COUNT];
	for (int i = 1; i < BN4_CHIP_COUNT; ++i) {
		xrom_text(XROM_BN4_RED_SUN_US, (i < 256 ? BN4_CHIP_NAMES : BN4_CHIP_NAMES_HI) - BUS, i & 0xFF, names4[i], sizeof names4[i]);
		const uint8_t *rec = bn4_record(d4, i);
		cls[i] = rec[BN4_CHIP_CLASS];
		version[i] = rec[BN6_CHIP_PARAMS];
		navi[i] = (uint16_t)(rec[BN6_CHIP_FAMILY] << 8 | rec[BN6_CHIP_SUBFAMILY]);
	}
	bn6_names();
	xchips_pair((const char (*)[XCHIP_NAME])names6, XCHIPS_BN6, (const char (*)[XCHIP_NAME])names4, BN4_CHIP_COUNT, to_bn4, from_bn4);
	for (int x = 1; x < BN4_CHIP_COUNT; ++x)
		if (cls[x] > XCHIPS_GIGA) from_bn4[x] = 0;
		else if (!from_bn4[x]) from_bn4[x] = (uint16_t)registry_named(names4[x]);
	for (int x = 1; x < BN4_CHIP_COUNT; ++x) {
		int v = from_bn4[x] ? 0 : xchips_v1_of(version, navi, cls, BN4_CHIP_COUNT, x);
		if (v && from_bn4[v]) from_bn4[x] = (uint16_t)run_version(from_bn4[v], version[x]);
	}
	bn4_paired = true;
}

int xchips_bn4_paid(int x) {
	pair_bn4();
	return bn4_paired && x > 0 && x < BN4_CHIP_COUNT ? from_bn4[x] : 0;
}

uint16_t xchips_reward_bn4(uint16_t v) {
	if (v == 0xFFFF || v >> 14) return v;
	return reward_as(xchips_bn4_paid((int)chip9_id(v)), v);
}
