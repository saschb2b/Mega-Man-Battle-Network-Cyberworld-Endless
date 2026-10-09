/* Synthetic ROM-free transfer fixtures: no player saves or game assets. */
#include "test_backup.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

#include "backup.h"
#include "backup_fixture.h"
#include "backup_fixture_v2.h"
#include "backup_internal.h"
#include "save.h"
#include "rom.h"
#include "save_format.h"
#include "save_layout.h"
#include "version.h"

static int failures;
static const char *unreadable_profile;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

uint8_t *__wrap_backup_read_file(const char *path, size_t *n) {
	if (unreadable_profile && !strcmp(path, unreadable_profile)) { *n = 0; return NULL; }
	return __real_backup_read_file(path, n);
}

/* a save blob as save.c writes one: magic, size, FNV-1a checksum, bytes */
static void blob_file(const char *path, uint32_t magic, const void *data, uint32_t n) {
	const uint8_t *b = data;
	uint32_t sum = 2166136261u;
	for (uint32_t i = 0; i < n; ++i) sum = (sum ^ b[i]) * 16777619u;
	FILE *f = fopen(path, "wb");
	if (!f) return;
	uint32_t hdr[3] = { magic, n, sum };
	fwrite(hdr, sizeof hdr, 1, f);
	fwrite(data, n, 1, f);
	fclose(f);
}

/* a folder and all in it gone */
static void remove_tree(const char *dir) {
	DIR *d = opendir(dir);
	if (!d) return;
	for (struct dirent *e; (e = readdir(d));) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
		char path[512];
		struct stat st;
		snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
		if (!stat(path, &st) && S_ISDIR(st.st_mode)) remove_tree(path);
		else remove(path);
	}
	closedir(d);
	rmdir(dir);
}

static bool same_file(const char *a, const char *b) {
	size_t na = 0, nb = 0;
	uint8_t *x = backup_read_file(a, &na), *y = backup_read_file(b, &nb);
	bool same = x && y && na == nb && !memcmp(x, y, na);
	free(x);
	free(y);
	return same;
}

/* The saves in one file (backup.c, issue #97): what a phone keeps beside its
 * ROMs and the browser's page exports */
static void test_roundtrip(void) {
	/* (a folder of this run's own, emptied first: an earlier run's saves
	 * are no part of it, and Docker's pids come again) */
	char base[64], from[96], to[96], path[256], other[256];
	snprintf(base, sizeof base, "build/host/test-backup-%d", (int)getpid());
	remove_tree(base);
	snprintf(from, sizeof from, "%s/from", base);
	snprintf(to, sizeof to, "%s/to", base);
	mkdir(base, 0755);
	mkdir(from, 0755);
	mkdir(to, 0755);
	snprintf(path, sizeof path, "%s/savedata", from);
	mkdir(path, 0755);
	snprintf(path, sizeof path, "%s/savedata", to);
	mkdir(path, 0755);
	size_t n = 0;
	uint8_t *none = backup_pack(from, 1, &n);
	CHECK(!none, "backup: no profile, nothing to keep");
	free(none);
	int prof[16] = { 12, 9, 0, 0, 0, 0, 0, 4, 5 };   /* (runs, best: the profile's first two) */
	snprintf(path, sizeof path, "%s/savedata/profile.sav", from);
	blob_file(path, 0x43575032u, prof, sizeof prof);
	int runbuf[25] = { 1, 7, 4 };   /* (active, seed, layer: a run's first three) */
	snprintf(path, sizeof path, "%s/savedata/run.sav", from);
	blob_file(path, 0x43574538u, runbuf, sizeof runbuf);
	snprintf(path, sizeof path, "%s/savedata/run.state", from);
	FILE *state = fopen(path, "wb");
	if (state) { fputs("synthetic test checkpoint", state); fclose(state); }
	snprintf(path, sizeof path, "%s/keys.ini", from);
	FILE *f = fopen(path, "w");
	if (f) { fputs("A = J\n", f); fclose(f); }
	snprintf(path, sizeof path, "%s/savedata/run.sav.tmp", from);
	f = fopen(path, "w");
	if (f) { fputs("half", f); fclose(f); }
	uint8_t *b = backup_pack(from, 1700000000u, &n), *c = NULL;
	BackupInfo info, again;
	CHECK(b && backup_info(b, n, &info), "backup: packed and read back");
	if (!b) return;
	CHECK(info.runs == 12 && info.best == 9 && info.run_depth == 4, "backup: runs %d, best %d, a run on layer %d", info.runs, info.best, info.run_depth);
	CHECK(info.files == 3 && info.format == 2 && info.stamp == 1700000000u, "backup: %d files (a .tmp left out), stamp %llu", info.files, (unsigned long long)info.stamp);
	size_t m = 0;
	c = backup_pack(from, 1800000000u, &m);
	CHECK(c && backup_info(c, m, &again) && again.hash == info.hash && n == m && !memcmp(b, c, n), "backup: unchanged saves keep their manifest and exact bytes");
	free(c);
	b[n / 2] ^= 1;
	CHECK(!backup_info(b, n, &again) && !backup_unpack(to, b, n), "backup: a damaged one refused");
	b[n / 2] ^= 1;
	CHECK(!backup_info(b, n - 1, &again), "backup: a cut one refused");
	snprintf(path, sizeof path, "%s/savedata/profile.sav", to);
	int local_prof[16] = { 1, 1, 0, 0, 0, 0, 0, 8, 9 };
	blob_file(path, PROFILE_MAGIC, local_prof, sizeof local_prof);
	snprintf(path, sizeof path, "%s/keys.ini", to);
	f = fopen(path, "w");
	if (f) { fputs("A = K\n", f); fclose(f); }
	CHECK(backup_unpack(to, b, n), "backup: unpacked");
	snprintf(path, sizeof path, "%s/savedata/run.sav", from);
	snprintf(other, sizeof other, "%s/savedata/run.sav", to);
	CHECK(same_file(path, other), "backup: run.sav comes back as it was");
	snprintf(path, sizeof path, "%s/keys.ini", from);
	snprintf(other, sizeof other, "%s/keys.ini", to);
	CHECK(!same_file(path, other), "backup: source keys.ini never replaces local controls");
	size_t key_n = 0;
	uint8_t *keys = backup_read_file(other, &key_n);
	CHECK(keys && key_n == 6 && !memcmp(keys, "A = K\n", 6), "backup: existing local controls kept exactly");
	free(keys);
	snprintf(path, sizeof path, "%s/savedata/profile.sav", to);
	size_t profile_n = 0;
	uint8_t *local = backup_read_file(path, &profile_n);
	CHECK(local && profile_n >= 48 && backup_get32(local + 12 + 28) == 8 && backup_get32(local + 12 + 32) == 9,
		"backup: incoming progress keeps the receiving device's volume settings");
	free(local);
	snprintf(other, sizeof other, "%s/savedata.old/profile.sav", to);
	f = fopen(other, "r");
	CHECK(f != NULL, "backup: the saves it replaced kept aside in savedata.old");
	if (f) fclose(f);
	CHECK(backup_local_info(to, &again) && again.hash == info.hash, "backup: the saves unpacked are the saves packed");
	free(b);
	remove_tree(base);
}

/* Find a member in the synthetic carriers without using the writer. */
static uint8_t *member(uint8_t *b, size_t n, const char *name) {
	size_t at = BACKUP_HEAD;
	while (at + 6 < n - 4) {
		size_t len = (size_t)b[at] | (size_t)b[at + 1] << 8;
		uint32_t size = backup_get32(b + at + 2 + len);
		if (strlen(name) == len && !memcmp(b + at + 2, name, len)) return b + at + 6 + len;
		at += 6 + len + size;
	}
	return NULL;
}

static void rechecksum(uint8_t *b, size_t n) { backup_put32(b + n - 4, backup_fnv(b, n - 4, 2166136261u)); }

/* Add a known run blob to the fixed v1 carrier, without the exporter. */
static uint8_t *run_carrier(uint32_t magic, const uint8_t *run_bytes, uint32_t size, size_t *n) {
	const char *name = "savedata/run.sav";
	size_t len = strlen(name), at = sizeof backup_fixture - 4;
	*n = sizeof backup_fixture + 6 + len + 12 + size;
	uint8_t *b = malloc(*n);
	if (!b) return NULL;
	memcpy(b, backup_fixture, at);
	backup_put32(b + 16, 2);
	b[at] = (uint8_t)len;
	b[at + 1] = 0;
	memcpy(b + at + 2, name, len);
	backup_put32(b + at + 2 + len, size + 12);
	uint8_t *blob = b + at + 6 + len;
	backup_put32(blob, magic);
	backup_put32(blob + 4, size);
	backup_put32(blob + 8, backup_fnv(run_bytes, size, 2166136261u));
	memcpy(blob + 12, run_bytes, size);
	rechecksum(b, *n);
	return b;
}

static void test_pre_emulator_run(void) {
	char dir[80];
	snprintf(dir, sizeof dir, "build/host/test-pre-emu-%d", (int)getpid());
	remove_tree(dir);
	mkdir(dir, 0755);
	uint8_t legacy[248] = { 1 };  /* CWE1: seed at 4, depth/biome at 172/176 */
	backup_put32(legacy + 4, 7);
	backup_put32(legacy + 172, 4);
	size_t n = 0;
	uint8_t *b = run_carrier(0x43574531u, legacy, sizeof legacy, &n);
	BackupInfo info, before = { 0 }, after;
	CHECK(b && backup_info(b, n, &info) && info.format == 1 && info.has_run && !info.has_state && info.run_depth == 4 &&
		backup_check(&info, true) == BACKUP_OK, "backup: pre-emulator CWE1 needs no state to resume its layer fresh");
	CHECK(b && backup_restore(dir, b, n, true) == BACKUP_OK && backup_local_info(dir, &before),
		"backup: a synthetic pre-emulator v1 carrier restores and exports again");
	free(b);
	Run modern = { .active = true, .seed = 7, .depth = 4 };
	b = run_carrier(RUN_MAGIC, (const uint8_t *)&modern, sizeof modern, &n);
	CHECK(b && !backup_info(b, n, &info) && backup_restore(dir, b, n, true) == BACKUP_DAMAGED,
		"backup: a modern active run still requires its emulator checkpoint");
	CHECK(backup_local_info(dir, &after) && after.hash == before.hash,
		"backup: refusing a modern run without state keeps the previous imported progress");
	free(b);
	remove_tree(dir);
}

static void test_golden(void) {
	BackupInfo info;
	CHECK(backup_info(backup_fixture, sizeof backup_fixture, &info) && info.format == 1 && info.files == 1,
		"backup: synthetic v1 golden envelope stays readable");
	CHECK(backup_info(backup_fixture_v2, sizeof backup_fixture_v2, &info) && info.format == 2 && info.files == 1 &&
		!strcmp(info.device, "Synthetic fixture") && !strcmp(info.save_id, "00000000000000000000000000000001"),
		"backup: synthetic v2 golden envelope and fixed manifest stay readable");
	uint8_t corrupt[sizeof backup_fixture];
	memcpy(corrupt, backup_fixture, sizeof corrupt);
	corrupt[sizeof corrupt - 5] ^= 1;
	rechecksum(corrupt, sizeof corrupt);
	CHECK(!backup_info(corrupt, sizeof corrupt, &info), "backup: damaged profile refused despite valid carrier checksum");
	memcpy(corrupt, backup_fixture, sizeof corrupt);
	corrupt[30] = '\\';
	rechecksum(corrupt, sizeof corrupt);
	CHECK(!backup_info(corrupt, sizeof corrupt, &info), "backup: path aliases refused before import");
}

static void test_manifest(void) {
	char dir[80], path[160];
	snprintf(dir, sizeof dir, "build/host/test-transfer-%d", (int)getpid());
	remove_tree(dir);
	mkdir(dir, 0755);
	CHECK(backup_restore(dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_OK, "backup: legacy golden restores");
	Profile p = { .runs = 3, .best_depth = 9, .music_volume = 5, .sfx_volume = 2 };
	snprintf(path, sizeof path, "%s/savedata/profile.sav", dir);
	blob_file(path, PROFILE_MAGIC, &p, sizeof p);
	struct utimbuf checkpoint_time = { 1700000000, 1700000000 };
	CHECK(utime(path, &checkpoint_time) == 0, "backup: synthetic checkpoint write time set");
	backup_set_device("Test device");
	size_t n = 0, m = 0;
	uint8_t *a = backup_pack(dir, 0, &n);
	BackupInfo i, j;
	CHECK(a && backup_info(a, n, &i) && !strcmp(i.device, "Test device") && i.revision == 1 && !i.parent_id[0] && i.stamp == 1700000000u,
		"backup: manifest identifies first profile revision and source device");
	if (!a) { remove_tree(dir); return; }
	uint8_t *profile_bytes = member(a, n, "savedata/profile.sav");
	CHECK(profile_bytes && !backup_get32(profile_bytes + 12 + 28) && !backup_get32(profile_bytes + 12 + 32),
		"backup: profile volumes do not travel");
	p.music_volume = 9;
	blob_file(path, PROFILE_MAGIC, &p, sizeof p);
	uint8_t *b = backup_pack(dir, 1800000000u, &m);
	CHECK(b && n == m && !memcmp(a, b, n), "backup: local volume changes leave the transfer byte-stable");
	free(b);
	++p.runs;
	blob_file(path, PROFILE_MAGIC, &p, sizeof p);
	b = backup_pack(dir, 1800000000u, &m);
	CHECK(b && backup_info(b, m, &j) && j.revision == i.revision + 1 && !strcmp(i.save_id, j.save_id) &&
		!strcmp(j.parent_id, i.save_id) && j.parent_revision == i.revision && j.stamp == 1800000000u,
		"backup: changed progress advances the same identity with a parent revision");
	free(b);
	uint8_t *manifest = member(a, n, BACKUP_MANIFEST);
	manifest[7] = '3';
	rechecksum(a, n);
	CHECK(backup_info(a, n, &j) && backup_check(&j, true) == BACKUP_NEWER_FORMAT,
		"backup: a future manifest is readable for preview and refused for install");
	manifest[7] = '2';
	rechecksum(a, n);
	CHECK(backup_info(a, n, &j), "backup: original format restored");
	/* Test the build.py '+' notation against this build's own release. */
	unsigned int major, minor, patch;
	if (sscanf(CW_VERSION, "%u.%u.%u", &major, &minor, &patch) == 3) {
		snprintf(j.version, sizeof j.version, "999.0.0");
		CHECK(backup_check(&j, true) == BACKUP_NEWER_BUILD, "backup: a newer game release is refused");
		snprintf(j.version, sizeof j.version, "%u.%u.%u+4294967294.gtest", major, minor, patch);
		CHECK(backup_check(&j, true) == BACKUP_NEWER_BUILD, "backup: a later build of the same release is refused");
	}
	free(a);
	remove_tree(dir);
}

static void test_run_preflight(void) {
	char dir[80], path[160];
	snprintf(dir, sizeof dir, "build/host/test-run-transfer-%d", (int)getpid());
	remove_tree(dir);
	mkdir(dir, 0755);
	backup_restore(dir, backup_fixture, sizeof backup_fixture, true);
	Run r = { .active = true, .seed = 7, .depth = 4, .biome = BIOME_CENTRAL };
	snprintf(path, sizeof path, "%s/savedata/run.sav", dir);
	blob_file(path, RUN_MAGIC, &r, sizeof r);
	size_t n = 0;
	uint8_t *b = backup_pack(dir, 1, &n);
	CHECK(!b, "backup: an active run without its checkpoint is refused");
	free(b);
	snprintf(path, sizeof path, "%s/savedata/run.state", dir);
	backup_write_file(path, (const uint8_t *)"synthetic checkpoint", 20);
	int area = NET_AREAS;
	snprintf(path, sizeof path, "%s/savedata/run.area", dir);
	blob_file(path, 0x43415231u, &area, sizeof area);
	b = backup_pack(dir, 2, &n);
	BackupInfo i;
	CHECK(b && backup_info(b, n, &i) && i.run_depth == 4 && i.run_act == 2 && i.run_area == NET_AREAS && i.needs_bn5,
		"backup: a saved BN5 area is distinguished from the run's BN6 biome");
	if (b) {
		CHECK(backup_check(&i, false) == BACKUP_NEEDS_BN5 && backup_check(&i, true) == BACKUP_OK,
			"backup: optional ROM requirement is checked before replacement");
		uint8_t *run_bytes = member(b, n, "savedata/run.sav");
		run_bytes[12 + 8] ^= 1;
		rechecksum(b, n);
		CHECK(!backup_info(b, n, &i), "backup: a damaged run metadata blob is refused");
	}
	free(b);
	r.active = false;
	snprintf(path, sizeof path, "%s/savedata/run.sav", dir);
	blob_file(path, RUN_MAGIC + 1, &r, sizeof r);
	b = backup_pack(dir, 3, &n);
	CHECK(!b, "backup: an unknown run format cannot be exported as compatible");
	free(b);
	remove_tree(dir);
}

static void test_undo_recovery(void) {
	char dir[80], saved[160], old[160], swap[160], undo[160], path[180];
	snprintf(dir, sizeof dir, "build/host/test-undo-%d", (int)getpid());
	remove_tree(dir);
	mkdir(dir, 0755);
	snprintf(saved, sizeof saved, "%s/savedata", dir);
	snprintf(old, sizeof old, "%s/savedata.old", dir);
	snprintf(swap, sizeof swap, "%s/savedata.swap", dir);
	snprintf(undo, sizeof undo, "%s/savedata.undo", dir);
	backup_restore(dir, backup_fixture, sizeof backup_fixture, true);
	Profile p = { .runs = 8, .music_volume = 6, .sfx_volume = 7 };
	snprintf(path, sizeof path, "%s/profile.sav", saved);
	blob_file(path, PROFILE_MAGIC, &p, sizeof p);
	CHECK(backup_restore(dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_OK && backup_can_undo(dir),
		"backup: import keeps the previous complete folder");
	p.runs = 0;
	p.music_volume = 2;
	blob_file(path, PROFILE_MAGIC, &p, sizeof p);
	CHECK(backup_undo(dir), "backup: last import undone");
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	CHECK(b && n >= 48 && backup_get32(b + 12) == 8 && backup_get32(b + 12 + 28) == 2,
		"backup: undo restores progress while keeping today's local volume");
	free(b);
	CHECK(rename(saved, swap) == 0 && backup_recover(dir), "backup: interrupted undo after first rename recovers");
	CHECK(rename(saved, swap) == 0 && rename(old, saved) == 0 && backup_recover(dir),
		"backup: interrupted undo after second rename recovers");
	CHECK(rename(old, undo) == 0 && rename(saved, old) == 0 && backup_recover(dir),
		"backup: interrupted import restores prior saves and prior undo");
	CHECK(backup_can_undo(dir), "backup: recovery keeps undo available");
	/* A pre-existing non-directory stage blocks an import without touching progress. */
	snprintf(path, sizeof path, "%s/savedata.import", dir);
	backup_write_file(path, (const uint8_t *)"blocked", 7);
	CHECK(backup_restore(dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_IO && backup_can_undo(dir),
		"backup: failed staging leaves both current saves and undo in place");
	remove_tree(dir);
}

static void test_undo_empty(void) {
	char dir[80], path[160];
	snprintf(dir, sizeof dir, "build/host/test-empty-undo-%d", (int)getpid());
	remove_tree(dir);
	mkdir(dir, 0755);
	CHECK(backup_restore(dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_OK && backup_can_undo(dir),
		"backup: a first import into a fresh device can be undone");
	BackupInfo empty;
	CHECK(backup_undo_info(dir, &empty) && !empty.has_profile && !empty.runs && !empty.has_run &&
		backup_check(&empty, false) == BACKUP_OK, "backup: the empty prior folder previews as valid zero progress");
	snprintf(path, sizeof path, "%s/savedata.old/transfer.manifest", dir);
	CHECK(access(path, F_OK) != 0, "backup: an empty Undo preview never creates a manifest");
	Profile p = { .runs = 4, .music_volume = 3, .sfx_volume = 4 };
	snprintf(path, sizeof path, "%s/savedata/profile.sav", dir);
	blob_file(path, PROFILE_MAGIC, &p, sizeof p);
	CHECK(backup_undo(dir), "backup: a first import returns to empty progress");
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	CHECK(b && n >= 48 && backup_get32(b + 12) == 0 && backup_get32(b + 12 + 28) == 3 && backup_get32(b + 12 + 32) == 4,
		"backup: undo to empty progress preserves the current device's volumes");
	free(b);
	remove_tree(dir);
}

static bool read_matches(const char *path, const uint8_t *expected, size_t n) {
	size_t got = 0;
	uint8_t *b = backup_read_file(path, &got);
	bool same = b && expected && got == n && !memcmp(b, expected, n);
	free(b);
	return same;
}

static void preview_read_failure(const char *dir, const char *path) {
	char kept[220];
	snprintf(kept, sizeof kept, "%s.kept", path);
	CHECK(rename(path, kept) == 0 && mkdir(path, 0755) == 0, "backup: present unreadable preview path is set up");
	BackupInfo info;
	CHECK(!backup_undo_info(dir, &info) && info.status == BACKUP_IO,
		"backup: present unreadable progress or metadata refuses Undo preview");
	CHECK(rmdir(path) == 0 && rename(kept, path) == 0, "backup: unreadable preview fixture resets without changing bytes");
}

static void test_undo_info(void) {
	char dir[80], path[180], old_profile[180], old_manifest[180];
	snprintf(dir, sizeof dir, "build/host/test-undo-preview-%d", (int)getpid());
	remove_tree(dir);
	mkdir(dir, 0755);
	BackupInfo info, prior = { 0 };
	CHECK(!backup_undo_info(dir, &info) && info.status == BACKUP_IO,
		"backup: a missing prior folder cannot preview or create progress");
	CHECK(backup_restore(dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_OK, "backup: Undo preview fixture restores");
	Profile profile_before = { .runs = 11, .best_depth = 7, .music_volume = 4, .sfx_volume = 5 };
	snprintf(path, sizeof path, "%s/savedata/profile.sav", dir);
	blob_file(path, PROFILE_MAGIC, &profile_before, sizeof profile_before);
	Run run_before = { .active = true, .seed = 7, .depth = 4 };
	snprintf(path, sizeof path, "%s/savedata/run.sav", dir);
	blob_file(path, RUN_MAGIC, &run_before, sizeof run_before);
	snprintf(path, sizeof path, "%s/savedata/run.state", dir);
	backup_write_file(path, (const uint8_t *)"synthetic checkpoint", 20);
	int area = NET_AREAS;
	snprintf(path, sizeof path, "%s/savedata/run.area", dir);
	blob_file(path, 0x43415231u, &area, sizeof area);
	size_t n = 0;
	uint8_t *packed = backup_pack(dir, 1700000000u, &n);
	CHECK(packed && backup_info(packed, n, &prior), "backup: prior profile and BN5 checkpoint are captured");
	free(packed);
	CHECK(backup_restore(dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_OK, "backup: another import retains that prior folder");
	snprintf(old_profile, sizeof old_profile, "%s/savedata.old/profile.sav", dir);
	snprintf(old_manifest, sizeof old_manifest, "%s/savedata.old/transfer.manifest", dir);
	size_t profile_n = 0, manifest_n = 0;
	uint8_t *profile_bytes = backup_read_file(old_profile, &profile_n), *manifest_bytes = backup_read_file(old_manifest, &manifest_n);
	CHECK(backup_undo_info(dir, &info) && info.runs == 11 && info.best == 7 && info.run_depth == 4 && info.run_act == 2 &&
		info.run_area == NET_AREAS && info.stamp == prior.stamp && !strcmp(info.save_id, prior.save_id) &&
		backup_check(&info, false) == BACKUP_NEEDS_BN5, "backup: Undo preview describes the exact prior progress, metadata and ROM requirement");
	CHECK(read_matches(old_profile, profile_bytes, profile_n) && read_matches(old_manifest, manifest_bytes, manifest_n),
		"backup: Undo preview never normalizes local volumes or rewrites the prior manifest");
	preview_read_failure(dir, old_profile);
	preview_read_failure(dir, old_manifest);
	CHECK(read_matches(old_profile, profile_bytes, profile_n) && read_matches(old_manifest, manifest_bytes, manifest_n),
		"backup: refused Undo previews leave both prior progress and metadata unchanged");
	profile_before.runs = 12;
	blob_file(old_profile, PROFILE_MAGIC, &profile_before, sizeof profile_before);
	CHECK(backup_undo_info(dir, &info) && info.runs == 12 && !info.version[0] &&
		read_matches(old_manifest, manifest_bytes, manifest_n), "backup: later valid progress gets an accurate preview without refreshing stale source metadata");
	CHECK(backup_write_file(old_profile, profile_bytes, profile_n), "backup: prior profile fixture resets");
	if (manifest_bytes) {
		manifest_bytes[7] = '3';
		CHECK(backup_write_file(old_manifest, manifest_bytes, manifest_n) && backup_undo_info(dir, &info) &&
			backup_check(&info, true) == BACKUP_NEWER_FORMAT, "backup: future prior format remains readable for preflight refusal");
		manifest_bytes[7] = '2';
		CHECK(backup_write_file(old_manifest, (const uint8_t *)"broken", 6) && !backup_undo_info(dir, &info) && info.status == BACKUP_DAMAGED,
			"backup: damaged present metadata refuses Undo preview");
		backup_write_file(old_manifest, manifest_bytes, manifest_n);
	}
	if (profile_bytes) {
		profile_bytes[12] ^= 1;
		CHECK(backup_write_file(old_profile, profile_bytes, profile_n) && !backup_undo_info(dir, &info) && info.status == BACKUP_DAMAGED,
			"backup: damaged present progress refuses Undo preview");
	}
	free(profile_bytes);
	free(manifest_bytes);
	remove_tree(dir);
}

static void test_profile_read_failure(void) {
	char dir[80], current[160], old[160], swap[160], stage[160];
	snprintf(dir, sizeof dir, "build/host/test-undo-io-%d", (int)getpid());
	remove_tree(dir);
	mkdir(dir, 0755);
	CHECK(backup_restore(dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_OK, "backup: undo I/O fixture restores");
	snprintf(current, sizeof current, "%s/savedata/profile.sav", dir);
	snprintf(old, sizeof old, "%s/savedata.old/profile.sav", dir);
	snprintf(swap, sizeof swap, "%s/savedata.swap", dir);
	snprintf(stage, sizeof stage, "%s/savedata.import", dir);
	Profile now = { .runs = 1, .music_volume = 2, .sfx_volume = 3 };
	Profile prior = { .runs = 42, .music_volume = 7, .sfx_volume = 8 };
	blob_file(current, PROFILE_MAGIC, &now, sizeof now);
	blob_file(old, PROFILE_MAGIC, &prior, sizeof prior);
	size_t current_n = 0, old_n = 0;
	uint8_t *current_bytes = backup_read_file(current, &current_n), *old_bytes = backup_read_file(old, &old_n);
	unreadable_profile = old;
	CHECK(!backup_undo(dir), "backup: a present but unreadable Undo profile is never synthesized as empty progress");
	unreadable_profile = NULL;
	CHECK(read_matches(current, current_bytes, current_n) && read_matches(old, old_bytes, old_n) && access(swap, F_OK) != 0,
		"backup: an Undo-profile read failure keeps both complete folders and volumes byte-for-byte");
	unreadable_profile = current;
	CHECK(!backup_undo(dir), "backup: an unreadable current profile refuses Undo rather than resetting its local volumes");
	CHECK(backup_restore(dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_IO,
		"backup: an unreadable current profile refuses Import rather than resetting its local volumes");
	unreadable_profile = NULL;
	CHECK(read_matches(current, current_bytes, current_n) && read_matches(old, old_bytes, old_n) &&
		access(swap, F_OK) != 0 && access(stage, F_OK) != 0,
		"backup: a current-profile read failure keeps both folders and volumes untouched without staging an Import");
	free(current_bytes);
	free(old_bytes);
	remove_tree(dir);
}

int test_backup(void) {
	failures = 0;
	test_roundtrip();
	test_golden();
	test_pre_emulator_run();
	test_manifest();
	test_run_preflight();
	test_undo_recovery();
	test_undo_empty();
	test_undo_info();
	test_profile_read_failure();
	return failures;
}
