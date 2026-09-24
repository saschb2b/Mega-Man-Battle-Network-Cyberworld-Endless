#include "save.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "audio.h"
#include "game.h"
#include "run.h"
#include "save_blob.h"

#define RUN_MAGIC 0x43574534u /* "CWE4": room for 32 areas */
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
	mkdir(dir, 0755);
	save_path(file, sizeof file, name);
	snprintf(tmp, sizeof tmp, "%s.tmp", file);
	FILE *f = fopen(tmp, "wb");
	if (!f) return false;
	uint32_t hdr[3] = { magic, (uint32_t)n, checksum(data, n) };
	bool ok = fwrite(hdr, sizeof hdr, 1, f) == 1 && fwrite(data, n, 1, f) == 1;
	ok = fflush(f) == 0 && ok;
	fclose(f);
	/* Atomic replace so a power cut never leaves a half-written save. */
	return ok && rename(tmp, file) == 0;
}

bool save_read_blob(const char *name, uint32_t magic, void *data, size_t n) {
	char file[600];
	save_path(file, sizeof file, name);
	FILE *f = fopen(file, "rb");
	if (!f) return false;
	uint32_t hdr[3];
	bool ok = fread(hdr, sizeof hdr, 1, f) == 1 && hdr[0] == magic && hdr[1] == n && fread(data, n, 1, f) == 1 && checksum(data, n) == hdr[2];
	fclose(f);
	return ok;
}

void save_state_path(char *out, size_t n) { save_path(out, n, "run.state"); }

void save_init(void) {
	legacy_move_state();
	if (!save_read_blob("profile.sav", PROFILE_MAGIC, &profile, sizeof profile)) memset(&profile, 0, sizeof profile);
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

bool save_run(void) {
	if (!run.active) return false;
	return save_write_blob("run.sav", RUN_MAGIC, &run, sizeof run);
}

bool peek_run(Run *out) { return save_read_blob("run.sav", RUN_MAGIC, out, sizeof *out) && out->active; }

/* Colonel was navi 17 before this version (the enemy table's unnamed
 * navi); he is 18. */
static void upgrade_run(void) {
	for (int b = 0; b < MAX_BIOMES; ++b) if (run.boss_order[b] == 17) run.boss_order[b] = 18;
}

bool load_run(void) {
	Run tmp;
	if (save_read_blob("run.sav", RUN_MAGIC, &tmp, sizeof tmp) && tmp.active) { run = tmp; upgrade_run(); return true; }
	if (!legacy_load_run()) return false;
	upgrade_run();
	return true;
}

void save_delete(void) {
	char file[600];
	save_path(file, sizeof file, "run.sav");
	remove(file);
	save_state_path(file, sizeof file);
	remove(file);
}

void profile_save(void) { save_write_blob("profile.sav", PROFILE_MAGIC, &profile, sizeof profile); }

void profile_record_run(void) {
	profile.runs++;
	profile.last_depth = run.depth;
	if (run.depth > profile.best_depth) profile.best_depth = run.depth;
	profile.bosses += run.bosses_beaten;
	profile.viruses += run.viruses_deleted;
	if (run.secret_cleared) profile.secret_clears++;
	profile_save();
}
