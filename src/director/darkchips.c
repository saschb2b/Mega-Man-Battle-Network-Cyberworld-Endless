/* The run's DarkChips (darkchips.h). */
#include "darkchips.h"

#include <string.h>

#include "save_blob.h"

#define DARK_MAGIC 0x444B4331u   /* "DKC1" */

static struct { uint32_t seed; uint8_t count[DARK_KINDS]; } dark;
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
	if (!save_read_blob("run.dark", DARK_MAGIC, &dark, sizeof dark) || dark.seed != seed) dark_new_run(seed);
}

int dark_count(int k) { return k >= 0 && k < DARK_KINDS ? dark.count[k] : 0; }

void dark_give(int k) {
	if (k >= 0 && k < DARK_KINDS) dark.count[k] = 1;   /* (one of each, as BN5's folder takes them) */
}

void dark_set_counts(const uint8_t counts[DARK_KINDS]) { memcpy(dark.count, counts, sizeof dark.count); }
