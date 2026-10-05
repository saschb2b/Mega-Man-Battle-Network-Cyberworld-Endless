#include "save.h"

#include <errno.h>
#include <stdio.h>

#include "compat.h"
#include <string.h>

#include "audio.h"
#include "game.h"
#include "loot.h"
#include "platform.h"
#include "run.h"
#include "save_blob.h"

#define RUN_MAGIC 0x43574537u /* "CWE7": the board's programs (docs/NAVICUST.md) */
/* (Run is saved as it lies in memory: a change to it does not compile
 * until someone decides about the saves before it, issue #19) */
_Static_assert(sizeof(Run) == 96, "Run changed: old saves no longer read as they are; bump RUN_MAGIC (and read the "
	"previous one where it can carry over), then set this size");
#define PROFILE_MAGIC 0x43575032u

Profile profile;

void save_path(char *out, size_t n, const char *name) {
	snprintf(out, n, "%s/savedata/%s", g_data_dir, name);
}

static uint32_t checksum(const void *p, size_t n) {
	const uint8_t *b = p;
	uint32_t h = 2166136261u;
	for (size_t i = 0; i < n; ++i) h = (h ^ b[i]) * 16777619u;
	return h;
}

bool save_write_blob(const char *name, uint32_t magic, const void *data, size_t n) {
	char dir[600], file[600], tmp[620];
	snprintf(dir, sizeof dir, "%s/savedata", g_data_dir);
	cw_mkdir(dir);
	save_path(file, sizeof file, name);
	snprintf(tmp, sizeof tmp, "%s.tmp", file);
	FILE *f = fopen(tmp, "wb");
	if (!f) return false;
	uint32_t hdr[3] = { magic, (uint32_t)n, checksum(data, n) };
	bool ok = fwrite(hdr, sizeof hdr, 1, f) == 1 && fwrite(data, n, 1, f) == 1;
	ok = fflush(f) == 0 && ok;
	fclose(f);
	/* Atomic replace so a power cut never leaves a half-written save. */
	ok = ok && cw_rename(tmp, file);
	platform_persist();
	return ok;
}

static bool read_blob(const char *name, uint32_t magic, void *data, size_t n, bool upto) {
	char file[600];
	save_path(file, sizeof file, name);
	FILE *f = fopen(file, "rb");
	if (!f) return false;
	uint32_t hdr[3];
	bool ok = fread(hdr, sizeof hdr, 1, f) == 1 && hdr[0] == magic && (upto ? hdr[1] > 0 && hdr[1] <= n : hdr[1] == n);
	if (ok) {
		memset(data, 0, n);
		ok = fread(data, hdr[1], 1, f) == 1 && checksum(data, hdr[1]) == hdr[2];
	}
	fclose(f);
	return ok;
}

bool save_read_blob(const char *name, uint32_t magic, void *data, size_t n) { return read_blob(name, magic, data, n, false); }
bool save_read_blob_upto(const char *name, uint32_t magic, void *data, size_t n) { return read_blob(name, magic, data, n, true); }

void save_state_path(char *out, size_t n) { save_path(out, n, "run.state"); }

void save_init(void) {
	legacy_move_state();
	if (!save_read_blob_upto("profile.sav", PROFILE_MAGIC, &profile, sizeof profile)) memset(&profile, 0, sizeof profile);
	if (!profile.music_volume) profile.music_volume = 9;
	if (!profile.sfx_volume) profile.sfx_volume = 9;
	audio_set_volume(profile.music_volume - 1, profile.sfx_volume - 1);
}

bool save_exists(void) {
	Run tmp;
	if (save_read_blob("run.sav", RUN_MAGIC, &tmp, sizeof tmp) && tmp.active) return true;
	Run keep = run;
	bool old = legacy_load_run();
	run = keep;
	return old;
}

#define BATTLE_MAGIC 0x43574232u   /* "CWB2": the last two battles, loot.h */

bool save_run(void) {
	if (!run.active) return false;
	LootMemory m;
	loot_memory(&m);
	save_write_blob("battle.sav", BATTLE_MAGIC, &m, sizeof m);
	return save_write_blob("run.sav", RUN_MAGIC, &run, sizeof run);
}

bool peek_run(Run *out) {
	if (save_read_blob("run.sav", RUN_MAGIC, out, sizeof *out) && out->active) return true;
	/* (an older format's, as CONTINUE would load it) */
	Run keep = run;
	bool old = legacy_load_run();
	*out = run;
	run = keep;
	return old && out->active;
}

/* Colonel was navi 17 before this version (the enemy table's unnamed
 * navi); he is 18. */
static void upgrade_run(void) {
	for (int b = 0; b < MAX_BIOMES; ++b) if (run.boss_order[b] == 17) run.boss_order[b] = 18;
}

/* the last battle as the run's save left it (none: forgotten) */
static void load_battle(void) {
	LootMemory m = { -1, -1, 0, 0, 0 };
	if (!save_read_blob("battle.sav", BATTLE_MAGIC, &m, sizeof m)) m = (LootMemory){ -1, -1, 0, 0, 0 };
	loot_memory_set(&m);
}

bool load_run(void) {
	Run tmp;
	if (save_read_blob("run.sav", RUN_MAGIC, &tmp, sizeof tmp) && tmp.active) { run = tmp; upgrade_run(); load_battle(); return true; }
	if (!legacy_load_run()) return false;
	upgrade_run();
	load_battle();
	return true;
}

void save_delete(void) {
	char file[600];
	save_path(file, sizeof file, "run.sav");
	remove(file);
	save_path(file, sizeof file, "battle.sav");
	remove(file);
	save_state_path(file, sizeof file);
	remove(file);
}

void profile_save(void) { save_write_blob("profile.sav", PROFILE_MAGIC, &profile, sizeof profile); }

bool profile_family_fought(int fam) { return fam >= 0 && fam < 64 && (profile.families_fought[fam >> 5] >> (fam & 31) & 1); }
void profile_family_note(int fam) { if (fam >= 0 && fam < 64) profile.families_fought[fam >> 5] |= 1u << (fam & 31); }

bool profile_code_entered(int program) { return program > 0 && program < 64 && (profile.codes_entered[program >> 3] >> (program & 7) & 1); }
void profile_code_note(int program) { if (program > 0 && program < 64) profile.codes_entered[program >> 3] |= (uint8_t)(1u << (program & 7)); }
int profile_codes_entered(void) {
	int n = 0;
	for (int p = 1; p < 64; ++p) n += profile_code_entered(p);
	return n;
}

void run_new_varied(uint32_t seed) {
	run_new(seed);
	/* (one retry left the same guardian a time in three: DiveMan guarded
	 * two new runs running; and the same area: a playtester began in the
	 * RoboDog Comp four runs running) */
	/* (the town is home, the same every run since docs/HOME.md: no longer
	 * one of them) */
	/* (and, the first tries, not the run before's either: avoiding the last
	 * alone let two of the three opening areas take turns, and a
	 * playtester met the RoboDog Comp five runs in seven, the Seaside Area
	 * never) */
	int first = run.boss_order[run.biome_order[0]], area = run.biome_order[0];
	for (int k = 1; k <= 32; ++k) {
		bool again = profile.first_guardian == first + 1 || profile.first_area == area + 1;
		if (k <= 24) again |= profile.guardian_before == first + 1 || profile.area_before == area + 1;
		if (!again) break;
		run_new(seed * 2654435761u + 0x9E37u * (uint32_t)k);
		first = run.boss_order[run.biome_order[0]];
		area = run.biome_order[0];
	}
	profile.guardian_before = profile.first_guardian;
	profile.area_before = profile.first_area;
	profile.first_guardian = (uint8_t)(first + 1);
	profile.first_area = (uint8_t)(area + 1);
	profile_save();
}

void profile_record_run(void) {
	profile.runs++;
	profile.last_depth = run.depth;
	if (run.depth > profile.best_depth) profile.best_depth = run.depth;
	profile.bosses += run.bosses_beaten;
	profile.viruses += run.viruses_deleted;
	if (run.secret_cleared) profile.secret_clears++;
	profile_save();
}
