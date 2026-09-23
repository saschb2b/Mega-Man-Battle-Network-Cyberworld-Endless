#include "save.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "audio.h"
#include "game.h"
#include "run.h"

#define RUN_MAGIC 0x43574531u /* "CWE1" */
#define PROFILE_MAGIC 0x43575032u

Profile profile;

static void path(char *out, size_t n, const char *name) {
	snprintf(out, n, "%s/savedata/%s", g_data_dir, name);
}

static uint32_t checksum(const void *p, size_t n) {
	const uint8_t *b = p;
	uint32_t h = 2166136261u;
	for (size_t i = 0; i < n; ++i) h = (h ^ b[i]) * 16777619u;
	return h;
}

static bool write_blob(const char *name, uint32_t magic, const void *data, size_t n) {
	char dir[600], file[600], tmp[620];
	snprintf(dir, sizeof dir, "%s/savedata", g_data_dir);
	mkdir(dir, 0755);
	path(file, sizeof file, name);
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

static bool read_blob(const char *name, uint32_t magic, void *data, size_t n) {
	char file[600];
	path(file, sizeof file, name);
	FILE *f = fopen(file, "rb");
	if (!f) return false;
	uint32_t hdr[3];
	bool ok = fread(hdr, sizeof hdr, 1, f) == 1 && hdr[0] == magic && hdr[1] == n && fread(data, n, 1, f) == 1 && checksum(data, n) == hdr[2];
	fclose(f);
	return ok;
}

void save_init(void) {
	if (!read_blob("profile.sav", PROFILE_MAGIC, &profile, sizeof profile)) memset(&profile, 0, sizeof profile);
	if (!profile.music_volume) profile.music_volume = 9;
	if (!profile.sfx_volume) profile.sfx_volume = 9;
	audio_set_volume(profile.music_volume - 1, profile.sfx_volume - 1);
}

bool save_exists(void) {
	Run tmp;
	return read_blob("run.sav", RUN_MAGIC, &tmp, sizeof tmp) && tmp.active;
}

bool save_run(void) {
	if (!run.active) return false;
	return write_blob("run.sav", RUN_MAGIC, &run, sizeof run);
}

bool load_run(void) {
	Run tmp;
	if (!read_blob("run.sav", RUN_MAGIC, &tmp, sizeof tmp) || !tmp.active) return false;
	run = tmp;
	return true;
}

void save_delete(void) {
	char file[600];
	path(file, sizeof file, "run.sav");
	remove(file);
}

void profile_save(void) { write_blob("profile.sav", PROFILE_MAGIC, &profile, sizeof profile); }

void profile_record_run(void) {
	profile.runs++;
	if (run.depth > profile.best_depth) profile.best_depth = run.depth;
	if (run.score > profile.best_score) profile.best_score = run.score;
	profile.bosses += run.bosses_beaten;
	profile.viruses += run.viruses_deleted;
	if (run.secret_cleared) profile.secret_clears++;
	profile_save();
}
