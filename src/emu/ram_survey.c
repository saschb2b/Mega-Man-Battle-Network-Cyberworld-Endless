#include "ram_survey.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EWRAM 0x02000000u
#define EWRAM_SIZE 0x40000u
#define IWRAM 0x03000000u
#define IWRAM_SIZE 0x8000u
#define CANARY 0x5Au
#define CANARIES 8
#define TOLD_MAX 6   /* a stretch's changes told on stderr; the rest counted */

static const char *seen_path;
static uint8_t *seen;   /* EWRAM_SIZE + IWRAM_SIZE: 1 where a byte was not zero */
static struct { uint32_t at, len; int told; } canaries[CANARIES];
static int ncanaries, strays;
static unsigned long frames;

static void survey_write(void) {
	if (seen_path) {
		FILE *f = fopen(seen_path, "wb");
		if (!f || fwrite(seen, 1, EWRAM_SIZE + IWRAM_SIZE, f) != EWRAM_SIZE + IWRAM_SIZE) fprintf(stderr, "ram survey: %s not written\n", seen_path);
		if (f) fclose(f);
	}
	if (ncanaries) fprintf(stderr, "ram canary: %d changed bytes over %lu frames\n", strays, frames);
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
	if (seen_path || ncanaries) atexit(survey_write);
	return seen_path || ncanaries;
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
