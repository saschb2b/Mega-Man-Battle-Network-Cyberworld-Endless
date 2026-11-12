/* The original run records retain their route, progress and old guardians;
 * the guardians' first records their counts. */
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "rivals.h"
#include "run.h"
#include "save_blob.h"
#include "save_format.h"

Run run;
char g_data_dir[512] = ".";
static uint8_t fixture[64];
static size_t fixture_size;
static uint32_t fixture_magic;
static int failures;

#define CHECK(test, message) do { if (!(test)) { fprintf(stderr, "%s\n", message); ++failures; } } while (0)

/* rivals.sav as its first version wrote it ("CWR1": four ints a navi),
 * as its second did ("CWR2": a Rival each), both for navis 0-31, as its
 * third did ("CWR3": navis 0-58, BN4's too), as its fourth did ("CWR4":
 * navis 0-64, Team ProtoMan's too), and what was written over them */
#define RIVALS_OLD 32
#define RIVALS_THIRD 59
#define RIVALS_FOURTH 65
#define RIVALS_NOW 0x43575235u   /* "CWR5" */
static int rivals_v1[RIVALS_OLD][4];
static Rival rivals_v2[RIVALS_OLD];
static Rival rivals_v3[RIVALS_THIRD];
static Rival rivals_v4[RIVALS_FOURTH];
static uint32_t rivals_magic;
static Rival rivals_written[RIVAL_NAVIS];

bool save_write_blob(const char *name, uint32_t magic, const void *data, size_t n) {
	if (strcmp(name, "rivals.sav") || n != sizeof rivals_written) return false;
	rivals_magic = magic;
	memcpy(rivals_written, data, n);
	return true;
}

bool save_read_blob(const char *name, uint32_t magic, void *data, size_t n) {
	if (!strcmp(name, "rivals.sav")) {
		if (magic != rivals_magic) return false;
		if (rivals_magic == 0x43575231u && n == sizeof rivals_v1) memcpy(data, rivals_v1, n);
		else if (rivals_magic == 0x43575232u && n == sizeof rivals_v2) memcpy(data, rivals_v2, n);
		else if (rivals_magic == 0x43575233u && n == sizeof rivals_v3) memcpy(data, rivals_v3, n);
		else if (rivals_magic == 0x43575234u && n == sizeof rivals_v4) memcpy(data, rivals_v4, n);
		else return false;
		return true;
	}
	if (strcmp(name, "run.sav") || magic != fixture_magic || n != fixture_size) return false;
	memcpy(data, fixture, n);
	return true;
}

/* (a Run as it lies today, under `today_magic`) */
static Run today_fixture;
static uint32_t today_magic;

bool save_read_blob_upto(const char *name, uint32_t magic, void *data, size_t n) {
	if (strcmp(name, "run.sav") || !today_magic || magic != today_magic || n != sizeof today_fixture) return false;
	memcpy(data, &today_fixture, n);
	return true;
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

/* A player's first rivals.sav loads whole: every count and last result
 * kept, how the battles went not known, and the file written anew in the
 * second version; Colonel's old place (17) still moves to his own (18). */
static void check_rivals(void) {
	for (int i = 0; i < RIVALS_OLD; ++i) {
		rivals_v1[i][0] = i + 3;
		rivals_v1[i][1] = i;
		rivals_v1[i][2] = i % 4;
		rivals_v1[i][3] = i % 3;
	}
	rivals_v1[18][0] = 0;
	rivals_magic = 0x43575231u;
	rivals_load();
	CHECK(rivals_magic == RIVALS_NOW, "rivals: the first version was not written anew");
	for (int i = RIVALS_OLD; i < RIVAL_NAVIS; ++i)
		CHECK(!memcmp(rival(i), &(Rival){ 0 }, sizeof(Rival)) && !memcmp(&rivals_written[i], &(Rival){ 0 }, sizeof(Rival)),
			"rivals: a navi past the first version's not blank");
	for (int i = 0; i < RIVALS_OLD; ++i) {
		const Rival *r = rival(i);
		int from = i == 18 ? 17 : i;
		bool moved = i == 17;
		CHECK(r->met == (moved ? 0 : rivals_v1[from][0]) && r->megaman_won == (moved ? 0 : rivals_v1[from][1]) &&
			r->navi_won == (moved ? 0 : rivals_v1[from][2]) && r->last == (moved ? 0 : rivals_v1[from][3]), "rivals: a first version's count changed");
		CHECK(!r->form && !r->left && !r->run, "rivals: a first version's battle has an ending");
		CHECK(!memcmp(r, &rivals_written[i], sizeof *r), "rivals: written other than loaded");
	}
	/* and a battle after it keeps how it went */
	rival_result(4, RIVAL_NAVI_WON, &(RivalEnd){ 2, 1, 1000 }, 7);
	CHECK(rival(4)->navi_won == 1 && rival(4)->last == RIVAL_NAVI_WON && rival(4)->form == 2 && rival(4)->left == 1 && rival(4)->run == 7,
		"rivals: a battle's ending not kept");
	rival_result(4, RIVAL_MEGAMAN_WON, &(RivalEnd){ 0, 400, 400 }, 8);
	CHECK(rival(4)->megaman_won == 5 && !rival(4)->form && rival(4)->left == RIVAL_LEFT_FULL && rival(4)->run == 8, "rivals: a full win's ending");
}

/* The second version's rivals.sav (navis 0-31, how each last battle
 * ended) loads whole into the third's (BN4's Navis past them, from
 * GUARDIAN_BN4_FIRST), written anew at its size, and a BN4 Navi's battle
 * is kept after it */
static void check_rivals_second(void) {
	for (int i = 0; i < RIVALS_OLD; ++i)
		rivals_v2[i] = (Rival){ i + 1, i * 2, i % 5, i % 3, (uint8_t)(i % 12), (uint8_t)(i % RIVAL_LEFT_FULL + 1), (uint16_t)(100 + i) };
	rivals_v2[17].met = 0;   /* (Colonel's old place met never: nothing moves) */
	memset(rivals_written, 0xA5, sizeof rivals_written);
	rivals_magic = 0x43575232u;
	rivals_load();
	CHECK(rivals_magic == RIVALS_NOW, "rivals: the second version was not written anew");
	for (int i = 0; i < RIVAL_NAVIS; ++i) {
		CHECK(!memcmp(rival(i), i < RIVALS_OLD ? &rivals_v2[i] : &(Rival){ 0 }, sizeof(Rival)), "rivals: a second version's record changed");
		CHECK(!memcmp(&rivals_written[i], rival(i), sizeof(Rival)), "rivals: the second version written other than loaded");
	}
	rival_met(35);
	rival_result(35, RIVAL_MEGAMAN_WON, &(RivalEnd){ 0, 300, 600 }, 12);
	CHECK(rival(35)->met == 1 && rival(35)->megaman_won == 1 && rival(35)->run == 12 && rivals_written[35].megaman_won == 1,
		"rivals: BN4's GutsMan (navi 35) not kept");
	CHECK(!memcmp(rival(RIVAL_NAVIS), &(Rival){ 0 }, sizeof(Rival)), "rivals: a navi past them not blank");
}

/* The third version's rivals.sav (navis 0-58, BN4's among them) loads
 * whole into today's (Team ProtoMan's and Nebula's past them, from
 * GUARDIAN_TEAM_FIRST, blank), written anew at its size, and a Team
 * ProtoMan Navi's battle is kept after it */
static void check_rivals_third(void) {
	for (int i = 0; i < RIVALS_THIRD; ++i)
		rivals_v3[i] = (Rival){ i + 2, i * 3, i % 7, i % 3, (uint8_t)(i % 9), (uint8_t)(i % RIVAL_LEFT_FULL + 1), (uint16_t)(200 + i) };
	rivals_v3[17].met = 0;   /* (Colonel's old place met never: nothing moves) */
	memset(rivals_written, 0xA5, sizeof rivals_written);
	rivals_magic = 0x43575233u;
	rivals_load();
	CHECK(rivals_magic == RIVALS_NOW, "rivals: the third version was not written anew");
	for (int i = 0; i < RIVAL_NAVIS; ++i) {
		CHECK(!memcmp(rival(i), i < RIVALS_THIRD ? &rivals_v3[i] : &(Rival){ 0 }, sizeof(Rival)), "rivals: a third version's record changed");
		CHECK(!memcmp(&rivals_written[i], rival(i), sizeof(Rival)), "rivals: the third version written other than loaded");
	}
	rival_met(62);
	rival_result(62, RIVAL_NAVI_WON, &(RivalEnd){ 0, 500, 1000 }, 14);
	CHECK(rival(62)->met == 1 && rival(62)->navi_won == 1 && rival(62)->run == 14 && rivals_written[62].navi_won == 1,
		"rivals: Team ProtoMan's NapalmMan (navi 62) not kept");
}

/* The fourth version's rivals.sav (navis 0-64, Team ProtoMan's among them)
 * loads whole into the fifth's (Nebula's LarkMan and GridMan past them,
 * from GUARDIAN_NEBULA_FIRST, blank), written anew at its size, and
 * GridMan's battle is kept after it */
static void check_rivals_fourth(void) {
	for (int i = 0; i < RIVALS_FOURTH; ++i)
		rivals_v4[i] = (Rival){ i + 3, i * 2, i % 5, i % 3, (uint8_t)(i % 7), (uint8_t)(i % RIVAL_LEFT_FULL + 1), (uint16_t)(300 + i) };
	rivals_v4[17].met = 0;   /* (Colonel's old place met never: nothing moves) */
	memset(rivals_written, 0xA5, sizeof rivals_written);
	rivals_magic = 0x43575234u;
	rivals_load();
	CHECK(rivals_magic == RIVALS_NOW, "rivals: the fourth version was not written anew");
	for (int i = 0; i < RIVAL_NAVIS; ++i) {
		CHECK(!memcmp(rival(i), i < RIVALS_FOURTH ? &rivals_v4[i] : &(Rival){ 0 }, sizeof(Rival)), "rivals: a fourth version's record changed");
		CHECK(!memcmp(&rivals_written[i], rival(i), sizeof(Rival)), "rivals: the fourth version written other than loaded");
	}
	rival_met(66);
	rival_result(66, RIVAL_MEGAMAN_WON, &(RivalEnd){ 0, 700, 1000 }, 15);
	CHECK(rival(66)->met == 1 && rival(66)->megaman_won == 1 && rival(66)->run == 15 && rivals_written[66].megaman_won == 1,
		"rivals: Nebula's GridMan (navi 66) not kept");
}

/* Ten-bit chip ids (save_format.h): a run of "CWEB" or "CWEC", its state's
 * folders nine-bit, read as today's Run (load_run converts them once); one
 * of RUN_MAGIC is no older format's */
static void check_nine_bit_runs(void) {
	const uint32_t nine[2] = { RUN_MAGIC_NINE, RUN_MAGIC_NINE_STOP };
	fixture_magic = 0;
	for (int k = 0; k < 2; ++k) {
		memset(&today_fixture, 0, sizeof today_fixture);
		today_fixture.active = true;
		today_fixture.seed = 9u + (uint32_t)k;
		today_fixture.depth = 4;
		today_fixture.extras_off = 2;
		today_magic = nine[k];
		memset(&run, 0, sizeof run);
		CHECK(legacy_load_run() && run.seed == 9u + (uint32_t)k && run.depth == 4 && run.extras_off == 2,
			"ten bits: a CWEB or CWEC run read as today's, its extras kept");
	}
	today_magic = RUN_MAGIC;
	memset(&run, 0, sizeof run);
	CHECK(RUN_MAGIC == RUN_MAGIC_TEN || !legacy_load_run(), "ten bits: RUN_MAGIC's run is no older format's");
	CHECK(RUN_MAGIC != RUN_MAGIC_TEN || !legacy_load_run(), "ten bits: CWED is no older format's");
	today_magic = 0;
}

int main(void) {
	check_nine_bit_runs();
	check_format(8);
	check_format(16);
	check_rivals();
	check_rivals_second();
	check_rivals_third();
	check_rivals_fourth();
	if (!failures) puts("legacy: CWE2 and CWE3 route, progress, guardian bounds and RNG, CWR1's, CWR2's and CWR3's rivals, CWEB and CWEC as nine-bit runs passed");
	return failures ? 1 : 0;
}
