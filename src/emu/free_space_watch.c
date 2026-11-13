#include "free_space_watch.h"

#include <stdio.h>
#include <stdlib.h>

#include "free_space.h"

typedef struct { const char *name; uint32_t at, end; } Region;

#define PLAN_REGION(name, at, end, parent, zone) { name, at, end },
static const Region regions[] = { FREE_SPACE_ROM(PLAN_REGION) };
#define NREGIONS ((int)(sizeof regions / sizeof *regions))
#define TOLD_MAX 8   /* the strays told on stderr; the rest counted */

static int strays;
static bool quiet;

/* (a measured run's: a bit a MEASURE_GRAIN of the free space, set by every
 * write into it, so that a room written again per battle or per layer
 * keeps the most any of them reached, and a region with a table at its end
 * shows what lies unwritten before it) */
#define MEASURE_GRAIN 16u
#define MEASURE_BITS ((FREE_END - EMU_FREE) / MEASURE_GRAIN)
static uint8_t *written;

void free_space_measure(void) {
	if (!written) written = calloc(MEASURE_BITS / 8, 1);
}

static void measure(uint32_t at, uint32_t len) {
	uint32_t lo = at < EMU_FREE ? EMU_FREE : at, hi = at + len > FREE_END ? FREE_END : at + len;
	for (uint32_t g = (lo - EMU_FREE) / MEASURE_GRAIN; lo < hi && g <= (hi - 1 - EMU_FREE) / MEASURE_GRAIN; ++g)
		written[g / 8] |= (uint8_t)(1u << (g % 8));
}

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
	if (written) measure(at, len);
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
	/* (the spans written during the run, "~ START END" each) */
	for (uint32_t g = 0, from = 0; written && g <= MEASURE_BITS; ++g) {
		bool set = g < MEASURE_BITS && (written[g / 8] >> (g % 8) & 1);
		if (set && (!g || !(written[(g - 1) / 8] >> ((g - 1) % 8) & 1))) from = g;
		if (!set && g && (written[(g - 1) / 8] >> ((g - 1) % 8) & 1))
			fprintf(f, "~ %08x %08x\n", (unsigned)(EMU_FREE + from * MEASURE_GRAIN), (unsigned)(EMU_FREE + g * MEASURE_GRAIN));
	}
	return fclose(f) == 0;
}
