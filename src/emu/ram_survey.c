#include "ram_survey.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/core.h>
#include <mgba/internal/arm/arm.h>

#define EWRAM 0x02000000u
#define EWRAM_SIZE 0x40000u
#define IWRAM 0x03000000u
#define IWRAM_SIZE 0x8000u
#define CANARY 0x5Au
#define CANARIES 8
#define TOLD_MAX 6   /* a stretch's changes told on stderr; the rest counted */

#define SRAM 0x0E000000u
#define SRAM_SIZE 0x8000u
#define WATCHES 8
#define PCS_MAX 48   /* a stretch's places told, each once; the rest counted */

static const char *seen_path;
static uint8_t *seen;   /* EWRAM_SIZE + IWRAM_SIZE: 1 where a byte was not zero */
static struct { uint32_t at, len; int told; } canaries[CANARIES];
static int ncanaries, strays;
static unsigned long frames;

/* ---- the watch: every read and write the game's code makes of a stretch ---- */

typedef struct { uint32_t pc; uint8_t write, size; } WatchPlace;
static struct {
	uint32_t at, len;
	unsigned long reads, writes, wide;   /* (wide: an access of more than a byte, counted in the others too) */
	uint32_t lo[2], hi[2];               /* the lowest and highest byte read, written */
	WatchPlace places[PCS_MAX];
	int nplaces, more;
} watches[WATCHES];
static int nwatches;
static uint32_t watch_lo = UINT32_MAX, watch_hi;
static struct ARMMemory board;   /* the board's own calls, under ours */
static bool board_kept;

/* an address as its memory's first image: the work RAM's and the save RAM's mirrors folded */
static uint32_t unmirrored(uint32_t a) {
	switch (a >> 24) {
	case 0x02: return EWRAM | (a & (EWRAM_SIZE - 1));
	case 0x03: return IWRAM | (a & (IWRAM_SIZE - 1));
	case 0x0E: case 0x0F: return SRAM | (a & (SRAM_SIZE - 1));
	default: return a;
	}
}

static void watch_seen(struct ARMCore *cpu, uint32_t a, uint32_t n, int write, int size) {
	a = unmirrored(a);
	if (a >= watch_hi || a + n <= watch_lo) return;
	for (int k = 0; k < nwatches; ++k) {
		uint32_t lo = a > watches[k].at ? a : watches[k].at, hi = a + n < watches[k].at + watches[k].len ? a + n : watches[k].at + watches[k].len;
		if (lo >= hi) continue;
		uint32_t pc = (uint32_t)cpu->gprs[ARM_PC] - (cpu->executionMode == MODE_THUMB ? 4u : 8u);
		bool first = !(write ? watches[k].writes : watches[k].reads);
		++*(write ? &watches[k].writes : &watches[k].reads);
		watches[k].wide += size > 1;
		if (first || lo < watches[k].lo[write]) watches[k].lo[write] = lo;
		if (first || hi - 1 > watches[k].hi[write]) watches[k].hi[write] = hi - 1;
		int i = 0;
		while (i < watches[k].nplaces && (watches[k].places[i].pc != pc || watches[k].places[i].write != write || watches[k].places[i].size != size)) ++i;
		if (i < watches[k].nplaces) continue;
		if (i == PCS_MAX) { ++watches[k].more; continue; }
		watches[k].places[watches[k].nplaces++] = (WatchPlace){ pc, (uint8_t)write, (uint8_t)size };
		fprintf(stderr, "ram watch: %08x+%x %s %08x, %d byte%s, at %08x lr %08x frame %lu\n", (unsigned)watches[k].at, (unsigned)watches[k].len,
			write ? "written" : "read", (unsigned)lo, size, size > 1 ? "s" : "", (unsigned)pc, (unsigned)cpu->gprs[ARM_LR], frames);
	}
}

static uint32_t w_load32(struct ARMCore *cpu, uint32_t a, int *c) { watch_seen(cpu, a & ~3u, 4, 0, 4); return board.load32(cpu, a, c); }
static uint32_t w_load16(struct ARMCore *cpu, uint32_t a, int *c) { watch_seen(cpu, a & ~1u, 2, 0, 2); return board.load16(cpu, a, c); }
static uint32_t w_load8(struct ARMCore *cpu, uint32_t a, int *c) { watch_seen(cpu, a, 1, 0, 1); return board.load8(cpu, a, c); }
static void w_store32(struct ARMCore *cpu, uint32_t a, int32_t v, int *c) { watch_seen(cpu, a & ~3u, 4, 1, 4); board.store32(cpu, a, v, c); }
static void w_store16(struct ARMCore *cpu, uint32_t a, int16_t v, int *c) { watch_seen(cpu, a & ~1u, 2, 1, 2); board.store16(cpu, a, v, c); }
static void w_store8(struct ARMCore *cpu, uint32_t a, int8_t v, int *c) { watch_seen(cpu, a, 1, 1, 1); board.store8(cpu, a, v, c); }
/* (a load or store multiple's words, as mGBA's GBALoadMultiple places them from the base, the mask and the direction) */
static void watch_multiple(struct ARMCore *cpu, uint32_t a, int mask, enum LSMDirection d, int write) {
	uint32_t n = 0;
	for (int m = mask & 0xFFFF; m; m >>= 1) n += (uint32_t)(m & 1);
	if (!n) n = 16;
	if (d & LSM_D) a -= (n << 2) - 4;
	if (d & LSM_B) a += d & LSM_D ? (uint32_t)-4 : 4u;
	watch_seen(cpu, a & ~3u, n * 4, write, 4);
}
static uint32_t w_load_multiple(struct ARMCore *cpu, uint32_t a, int mask, enum LSMDirection d, int *c) {
	watch_multiple(cpu, a, mask, d, 0);
	return board.loadMultiple(cpu, a, mask, d, c);
}
static uint32_t w_store_multiple(struct ARMCore *cpu, uint32_t a, int mask, enum LSMDirection d, int *c) {
	watch_multiple(cpu, a, mask, d, 1);
	return board.storeMultiple(cpu, a, mask, d, c);
}

void ram_survey_watch(struct mCore *core) {
	if (!nwatches || !core) return;
	struct ARMCore *cpu = core->cpu;
	if (cpu->memory.load8 == w_load8) return;
	/* (every GBA core's calls are the board's same six and two; another board's are left alone) */
	if (board_kept && (cpu->memory.load8 != board.load8 || cpu->memory.store32 != board.store32)) return;
	board = cpu->memory;
	board_kept = true;
	cpu->memory.load32 = w_load32; cpu->memory.load16 = w_load16; cpu->memory.load8 = w_load8;
	cpu->memory.store32 = w_store32; cpu->memory.store16 = w_store16; cpu->memory.store8 = w_store8;
	cpu->memory.loadMultiple = w_load_multiple; cpu->memory.storeMultiple = w_store_multiple;
}

static void watch_write(void) {
	for (int k = 0; k < nwatches; ++k) {
		fprintf(stderr, "ram watch: %08x+%x in all: %lu reads", (unsigned)watches[k].at, (unsigned)watches[k].len, watches[k].reads);
		if (watches[k].reads) fprintf(stderr, " (%08x-%08x)", (unsigned)watches[k].lo[0], (unsigned)watches[k].hi[0]);
		fprintf(stderr, ", %lu writes", watches[k].writes);
		if (watches[k].writes) fprintf(stderr, " (%08x-%08x)", (unsigned)watches[k].lo[1], (unsigned)watches[k].hi[1]);
		fprintf(stderr, ", %lu wider than a byte, from %d places%s over %lu frames\n", watches[k].wide, watches[k].nplaces + watches[k].more,
			watches[k].more ? " (not all told)" : "", frames);
	}
}

static void survey_write(void) {
	watch_write();
	if (seen_path) {
		FILE *f = fopen(seen_path, "wb");
		if (!f || fwrite(seen, 1, EWRAM_SIZE + IWRAM_SIZE, f) != EWRAM_SIZE + IWRAM_SIZE) fprintf(stderr, "ram survey: %s not written\n", seen_path);
		if (f) fclose(f);
	}
	if (ncanaries) fprintf(stderr, "ram canary: %d changed bytes over %lu frames\n", strays, frames);
}

/* CYBERWORLD_RAM_WATCH's stretches */
static void watch_open(const char *c) {
	while (c && *c && nwatches < WATCHES) {
		char *end;
		uint32_t at = (uint32_t)strtoul(c, &end, 0), len = *end == ':' ? (uint32_t)strtoul(end + 1, &end, 0) : 0;
		if (len && unmirrored(at) == at && at + len > at) {
			watches[nwatches].at = at; watches[nwatches++].len = len;
			if (at < watch_lo) watch_lo = at;
			if (at + len > watch_hi) watch_hi = at + len;
		} else fprintf(stderr, "ram watch: %08x:%x is no stretch (a mirror's address, or no length)\n", (unsigned)at, (unsigned)len);
		c = *end == ',' ? end + 1 : end + strlen(end);
	}
}

bool ram_survey_open(void) {
	seen_path = getenv("CYBERWORLD_RAM_SURVEY");
	const char *c = getenv("CYBERWORLD_RAM_CANARY");
	if (seen_path) {
		seen = calloc(1, EWRAM_SIZE + IWRAM_SIZE);
		FILE *f = seen ? fopen(seen_path, "rb") : NULL;
		if (f) {
			if (fread(seen, 1, EWRAM_SIZE + IWRAM_SIZE, f) != EWRAM_SIZE + IWRAM_SIZE) memset(seen, 0, EWRAM_SIZE + IWRAM_SIZE);
			fclose(f);
		}
		if (!seen) seen_path = NULL;
	}
	while (c && *c && ncanaries < CANARIES) {
		char *end;
		uint32_t at = (uint32_t)strtoul(c, &end, 0), len = *end == ':' ? (uint32_t)strtoul(end + 1, &end, 0) : 0;
		bool in_e = at >= EWRAM && at + len <= EWRAM + EWRAM_SIZE, in_i = at >= IWRAM && at + len <= IWRAM + IWRAM_SIZE;
		if (len && (in_e || in_i)) { canaries[ncanaries].at = at; canaries[ncanaries++].len = len; }
		else fprintf(stderr, "ram canary: %08x:%x is no stretch of the work RAM\n", (unsigned)at, (unsigned)len);
		c = *end == ',' ? end + 1 : end + strlen(end);
	}
	watch_open(getenv("CYBERWORLD_RAM_WATCH"));
	if (seen_path || ncanaries || nwatches) atexit(survey_write);
	return seen_path || ncanaries || nwatches;
}

void ram_survey_frame(uint8_t *ewram, uint8_t *iwram) {
	++frames;
	if (seen) {
		for (uint32_t i = 0; i < EWRAM_SIZE; ++i) seen[i] |= ewram[i] != 0;
		for (uint32_t i = 0; i < IWRAM_SIZE; ++i) seen[EWRAM_SIZE + i] |= iwram[i] != 0;
	}
	for (int k = 0; k < ncanaries; ++k) {
		uint8_t *p = canaries[k].at >= IWRAM ? iwram + (canaries[k].at - IWRAM) : ewram + (canaries[k].at - EWRAM);
		uint32_t zero = 0, changed = 0;
		for (uint32_t i = 0; i < canaries[k].len; ++i) {
			zero += p[i] == 0;
			changed += p[i] != CANARY;
		}
		/* (all zero: the power-on clear's, or a state's from before the canary) */
		if (changed && zero != canaries[k].len) {
			for (uint32_t i = 0; i < canaries[k].len; ++i)
				if (p[i] != CANARY && canaries[k].told++ < TOLD_MAX)
					fprintf(stderr, "ram canary: %08x holds %02x at frame %lu\n", (unsigned)(canaries[k].at + i), p[i], frames);
			strays += (int)changed;
		}
		if (changed) memset(p, CANARY, canaries[k].len);
	}
}
