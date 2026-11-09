#include "free_space_watch.h"

#include <stdio.h>

#include "free_space.h"

typedef struct { const char *name; uint32_t at, end; } Region;

#define PLAN_REGION(name, at, end, parent, zone) { name, at, end },
static const Region regions[] = { FREE_SPACE_ROM(PLAN_REGION) };
#define NREGIONS ((int)(sizeof regions / sizeof *regions))
#define TOLD_MAX 8   /* the strays told on stderr; the rest counted */

static int strays;
static bool quiet;

/* the innermost region holding [at, end): a part after its parent in the
 * table, so the last that holds it */
static const Region *holding(uint32_t at, uint32_t end) {
	const Region *found = NULL;
	for (int i = 0; i < NREGIONS && regions[i].at <= at; ++i)
		if (at >= regions[i].at && end <= regions[i].end) found = &regions[i];
	return found;
}

const char *free_space_region(uint32_t at) {
	const Region *r = at >= EMU_FREE && at < FREE_END ? holding(at, at + 1) : NULL;
	return r ? r->name : NULL;
}

bool free_space_write(uint32_t at, uint32_t len) {
	if (!len || at >= FREE_END || at + len <= EMU_FREE) return true;
	if (at >= EMU_FREE && at + len <= FREE_END && holding(at, at + len)) return true;
	if (!quiet && strays < TOLD_MAX) {
		const char *from = free_space_region(at), *to = free_space_region(at + len - 1);
		fprintf(stderr, "free space: a write of %u bytes at %08x-%08x runs from %s into %s (src/emu/free_space.plan)\n", (unsigned)len,
			(unsigned)at, (unsigned)(at + len - 1), from ? from : "a gap", to ? to : "a gap");
	}
	++strays;
	return false;
}

int free_space_strays(void) { return strays; }

void free_space_quiet(bool on) { quiet = on; }

bool free_space_usage(const char *path, uint8_t (*read8)(uint32_t)) {
	FILE *f = fopen(path, "w");
	if (!f) return false;
	for (int i = 0; i < NREGIONS; ++i) {
		uint32_t end = regions[i].end;
		while (end > regions[i].at && read8(end - 1) == 0xFF) --end;
		fprintf(f, "%s %08x\n", regions[i].name, (unsigned)(end > regions[i].at ? end : 0u));
	}
	return fclose(f) == 0;
}
