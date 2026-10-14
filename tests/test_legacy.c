/* The original run records retain their route, progress and old guardians. */
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "run.h"
#include "save_blob.h"

Run run;
char g_data_dir[512] = ".";
static uint8_t fixture[64];
static size_t fixture_size;
static uint32_t fixture_magic;
static int failures;

#define CHECK(test, message) do { if (!(test)) { fprintf(stderr, "%s\n", message); ++failures; } } while (0)

bool save_read_blob(const char *name, uint32_t magic, void *data, size_t n) {
	if (strcmp(name, "run.sav") || magic != fixture_magic || n != fixture_size) return false;
	memcpy(data, fixture, n);
	return true;
}

bool save_read_blob_upto(const char *name, uint32_t magic, void *data, size_t n) {
	(void)name; (void)magic; (void)data; (void)n;
	return false;
}

void save_queue_wait(void) {}
void save_path(char *out, size_t n, const char *name) { snprintf(out, n, "%s/savedata/%s", g_data_dir, name); }

/* Newly added areas get fresh guardians. A migration may overwrite only
 * the old format's eight or sixteen entries, and must keep RNG untouched. */
void run_new(uint32_t seed) {
	memset(&run, 0, sizeof run);
	run.active = true;
	run.seed = seed;
	for (unsigned i = 0; i < MAX_BIOMES; ++i) run.boss_order[i] = (uint8_t)(200u + i);
	(void)rng_next();
}

static void put_word(size_t at, uint32_t value) {
	for (unsigned i = 0; i < 4; ++i) fixture[at + i] = (uint8_t)(value >> (i * 8));
}

static void check_format(unsigned guardians) {
	memset(fixture, 0, sizeof fixture);
	fixture_magic = guardians == 8 ? 0x43574532u : 0x43574533u;
	fixture_size = guardians == 8 ? 56 : 64;
	fixture[0] = 1;
	put_word(4, 123); put_word(8, 7); put_word(12, 4); put_word(16, 2); put_word(20, 0x8989);
	for (unsigned i = 0; i < 6; ++i) fixture[24 + i] = (uint8_t)(5u - i);
	for (unsigned i = 0; i < guardians; ++i) fixture[30 + i] = (uint8_t)(60u + i);
	size_t progress = guardians == 8 ? 40 : 48;
	put_word(progress, 31); put_word(progress + 4, 707); put_word(progress + 8, 9);
	fixture[progress + 12] = 1;
	rng_seed(0x12345678);
	uint32_t before = rng_state();
	CHECK(legacy_load_run(), "legacy: supported record refused");
	CHECK(run.active && run.seed == 123 && run.depth == 7 && run.biome == 4 && run.side_kind == 2 && run.layer_seed == 0x8989,
		"legacy: route changed");
	CHECK(!memcmp(run.biome_order, fixture + 24, 6), "legacy: area order changed");
	for (unsigned i = 0; i < MAX_BIOMES; ++i)
		CHECK(run.boss_order[i] == (i < guardians ? 60u + i : 200u + i), "legacy: old or generated guardian changed");
	CHECK(run.bosses_beaten == 31 && run.viruses_deleted == 707 && run.fragments == 9 && run.secret_cleared,
		"legacy: progress changed");
	CHECK(rng_state() == before, "legacy: migration advanced RNG");
	Run preserved = run;
	fixture[0] = 0;
	CHECK(!legacy_load_run() && !memcmp(&run, &preserved, sizeof run), "legacy: inactive record replaced the current run");
}

int main(void) {
	check_format(8);
	check_format(16);
	if (!failures) puts("legacy: CWE2 and CWE3 route, progress, guardian bounds and RNG passed");
	return failures ? 1 : 0;
}
