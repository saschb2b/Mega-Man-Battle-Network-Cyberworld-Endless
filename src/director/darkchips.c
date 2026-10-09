/* The run's DarkChips (darkchips.h). */
#include "darkchips.h"

#include <string.h>
#include <stddef.h>

#include "save_blob.h"

#define DARK_MAGIC 0x444B4331u   /* "DKC1" */

/* (it grows at its end, so a run saved before the flame's layer was kept
 * reads with none kept: save_read_blob_upto) */
typedef struct {
	uint32_t seed;
	uint8_t count[DARK_KINDS];
	uint16_t flame_depth;        /* the layer whose flame is kept, 0 none */
	uint8_t flame_side;          /* ... its side kind */
	uint8_t flame_kind;          /* ... the kind it holds + 1 (0: none there), DARK_FLAME_BN6 where it is BN6's */
} DarkSave;
static DarkSave dark;
_Static_assert(sizeof(DarkSave) == 20 && offsetof(DarkSave, seed) == 0 && offsetof(DarkSave, count) == 4 &&
	offsetof(DarkSave, flame_depth) == 16 && offsetof(DarkSave, flame_side) == 18 && offsetof(DarkSave, flame_kind) == 19,
	"DarkChip save layout changed");
#define DARK_FLAME_BN6 0x80
uint16_t dark_dev_mask;

void dark_new_run(uint32_t seed) {
	memset(&dark, 0, sizeof dark);
	dark.seed = seed;
	for (int k = 0; k < DARK_KINDS; ++k) dark.count[k] = (uint8_t)(dark_dev_mask >> k & 1);
}

void dark_begin(uint32_t seed) {
	if (dark.seed != seed) dark_new_run(seed);
}

void dark_save(void) { save_write_blob("run.dark", DARK_MAGIC, &dark, sizeof dark); }

void dark_load(uint32_t seed) {
	if (!save_read_blob_upto("run.dark", DARK_MAGIC, &dark, sizeof dark) || dark.seed != seed) dark_new_run(seed);
}

int dark_count(int k) { return k >= 0 && k < DARK_KINDS ? dark.count[k] : 0; }

void dark_give(int k) {
	if (k >= 0 && k < DARK_KINDS) dark.count[k] = 1;   /* (one of each, as BN5's folder takes them) */
}

void dark_set_counts(const uint8_t counts[DARK_KINDS]) { memcpy(dark.count, counts, sizeof dark.count); }

/* BN6's five by DarkChipID, as BN5's kinds (DrkSword 188, DarkThnd 193,
 * DrkRecov 194, DarkInvs 189, DarkPlus 190: paired by name, docs/ROM_DATA.md) */
static const int8_t bn6_kinds[DARK_BN6_COUNT] = { 1, 6, 7, 2, 3 };

int dark_bn6_kind(int id) { return id >= DARK_BN6_FIRST && id < DARK_BN6_FIRST + DARK_BN6_COUNT ? bn6_kinds[id - DARK_BN6_FIRST] : -1; }

int dark_bn6_id(int k) {
	for (int i = 0; i < DARK_BN6_PLAYED; ++i)
		if (bn6_kinds[i] == k) return DARK_BN6_FIRST + i;
	return 0;
}

int dark_flame_pick(uint32_t seed, int depth, bool bn6, const uint8_t held[DARK_KINDS]) {
	int n = bn6 ? DARK_BN6_PLAYED : DARK_KINDS;
	uint32_t h = seed ^ (uint32_t)depth * 2654435761u;
	/* (BN5's as it was; BN6's mixed first, so seeds that differ in their
	 * low bits alone, a test's 1 to 12, start apart too) */
	int k0 = (int)((bn6 ? (h * 2246822519u) >> 16 : h >> 8) % (uint32_t)n);
	for (int j = 0; j < n; ++j) {
		int i = (k0 + j) % n, k = bn6 ? bn6_kinds[i] : i;
		if (!held[k]) return k;
	}
	return -1;
}

int dark_flame_of(uint32_t seed, int depth, int side, bool bn6) {
	uint8_t net = bn6 ? DARK_FLAME_BN6 : 0;
	if (dark.flame_depth == depth && dark.flame_side == side && dark.flame_kind && (dark.flame_kind & DARK_FLAME_BN6) == net)
		return (dark.flame_kind & ~DARK_FLAME_BN6) - 1;
	int k = dark_flame_pick(seed, depth, bn6, dark.count);
	dark.flame_depth = (uint16_t)depth;
	dark.flame_side = (uint8_t)side;
	dark.flame_kind = (uint8_t)(k >= 0 ? (k + 1) | net : 0);
	return k;
}

void dark_price_hp(int base, int max, int hp, int out[3]) {
	out[0] = base > DARK_PRICE + 10 ? base - DARK_PRICE : 10;
	out[1] = max - (base - out[0]);
	out[2] = hp > out[1] ? out[1] : hp;
}
