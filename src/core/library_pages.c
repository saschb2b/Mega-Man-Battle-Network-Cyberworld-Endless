/* library_pages.h. */
#include "library_pages.h"

#include <stddef.h>

#include "bn6_chips.h"
#include "chip_pool.h"
#include "chips.h"
#include "meta.h"
#include "rom.h"
#include "xchips.h"
#include "xchips_registry.h"

int library_page_of(const LibSlot *rows, int n, int cls, bool (*plays)(int chip), LibSlot *out, int most) {
	int k = 0;
	for (int i = 0; i < n; ++i) {
		if ((cls >= 0 && rows[i].cls != cls) || !plays(rows[i].chip)) continue;
		if (out && k < most) out[k] = rows[i];
		if (!out || k < most) ++k;
	}
	return k;
}

void library_tally(const LibSlot *rows, int n, int cls, bool (*plays)(int chip), bool (*has)(int chip), int *held, int *of) {
	*held = *of = 0;
	for (int i = 0; i < n; ++i) {
		if ((cls >= 0 && rows[i].cls != cls) || !plays(rows[i].chip)) continue;
		++*of;
		*held += has(rows[i].chip);
	}
}

int library_rows_check(const LibSlot *rows, int n) {
	for (int i = 0; i < n; ++i) {
		const LibSlot *r = &rows[i];
		if (r->cls >= LIB_CLASSES || !r->number || !r->own || !r->chip || r->chip >= XREG_IDS) return i + 1;
		if (i && (r->cls < r[-1].cls || (r->cls == r[-1].cls && r->number <= r[-1].number))) return i + 1;
		for (int j = 0; j < i; ++j)
			if (rows[j].own == r->own) return i + 1;
	}
	return 0;
}

bool library_plays(int chip) {
	const XChipReg *r = xreg_at(chip);
	if (r) return r->type <= XCHIPS_GIGA && (r->how == XREG_RECORD || r->how == XREG_MOVED);
	return chip > 0 && chip < XCHIPS_BN6 && chip_pool_tier(chip) >= 0;
}

/* BN6's own slots, read from its records: the chips a run can hold, each
 * with its number on its tab of BN6's Library, by class and then number */
static int bn6_rows(const LibSlot **rows) {
	static LibSlot slots[XCHIPS_BN6];
	static const uint8_t *of;
	static int n;
	*rows = slots;
	if (of == R.data) return n;
	of = R.data;
	n = 0;
	for (int cls = 0; R.data && cls < LIB_CLASSES; ++cls) {
		int first = n;
		for (int id = 1; id < XCHIPS_BN6; ++id) {
			const uint8_t *c = chip_pool_class(id) == cls ? chips_record(id) : NULL;
			if (!c) continue;
			/* (kept in their numbers' order: a few records) */
			int at = n++;
			for (; at > first && slots[at - 1].number > c[BN6_CHIP_LIBRARY_NUMBER]; --at) slots[at] = slots[at - 1];
			slots[at] = (LibSlot){ (uint16_t)id, (uint16_t)id, c[BN6_CHIP_LIBRARY_NUMBER], (uint8_t)cls };
		}
	}
	return n;
}

static int rows_of(int entry, const LibSlot **rows) {
	*rows = NULL;
	switch (entry) {
	case LIB_BN6: return bn6_rows(rows);
	case LIB_BN5: *rows = library_bn5_rows; return library_bn5_count;
	case LIB_BN4: *rows = library_bn4_rows; return library_bn4_count;
	default: return 0;
	}
}

int library_page(int entry, int cls, LibSlot *out, int most) {
	const LibSlot *rows;
	int n = rows_of(entry, &rows);
	return library_page_of(rows, n, cls, library_plays, out, most);
}

void library_count(int entry, int cls, int *held, int *of) {
	const LibSlot *rows;
	int n = rows_of(entry, &rows);
	library_tally(rows, n, cls, library_plays, meta_library_has, held, of);
}

bool library_whole(int entry, int cls) {
	int held, of;
	library_count(entry, cls, &held, &of);
	return of > 0 && held >= of;
}

bool library_entry_read(int entry) {
	if (entry == LIB_BN6) return R.data != NULL;
	if (entry == LIB_BN5) return chips_from_bn5();
	return entry == LIB_BN4 && XR[XROM_BN4_RED_SUN_US].data != NULL;
}
