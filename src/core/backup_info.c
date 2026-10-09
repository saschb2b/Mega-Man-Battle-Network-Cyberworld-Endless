/* Transfer preflight and the summaries read from checksummed save blobs. */
#include "backup_internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "layer_make.h"
#include "rom.h"
#include "save.h"
#include "save_format.h"
#include "version.h"

/* The profile's two volume fields are preferences, even in old saves. */
void backup_normalize_profile(uint8_t *data, size_t n, const uint8_t *local, size_t local_n) {
	if (n < 12 + 36 || backup_get32(data) != PROFILE_MAGIC || backup_get32(data + 4) != n - 12 ||
		backup_get32(data + 8) != backup_fnv(data + 12, n - 12, 2166136261u)) return;
	memset(data + 12 + 28, 0, 8);
	if (local && local_n >= 12 + 36 && backup_get32(local) == PROFILE_MAGIC &&
		backup_get32(local + 4) == local_n - 12 && backup_get32(local + 8) == backup_fnv(local + 12, local_n - 12, 2166136261u))
		memcpy(data + 12 + 28, local + 12 + 28, 8);
	backup_put32(data + 8, backup_fnv(data + 12, n - 12, 2166136261u));
}

static bool good_blob(const uint8_t *data, uint32_t size) {
	return size >= 12 && backup_get32(data + 4) == size - 12 &&
		backup_get32(data + 8) == backup_fnv(data + 12, size - 12, 2166136261u);
}

static bool run_format(uint32_t magic, uint32_t size) {
	static const uint32_t sizes[] = { 0, 248, 56, 64, 0, 84, 88, 96, 100, sizeof(Run) };
	if ((magic & 0xffffff00u) != (RUN_MAGIC & 0xffffff00u) || (magic & 255u) < '1' || (magic & 255u) > '9') return false;
	unsigned int format = (magic & 255u) - '0';
	return sizes[format] && size == sizes[format];
}

typedef struct { BackupInfo info, manifest; int area; bool ok, have_manifest, have_area; } Read;

static void read_run(Read *r, const uint8_t *data, uint32_t size) {
	BackupInfo *i = &r->info;
	i->run_magic = backup_get32(data);
	if (!run_format(i->run_magic, size - 12)) { i->status = BACKUP_INCOMPATIBLE_RUN; return; }
	i->has_run = data[12] != 0;
	if (!i->has_run) return;
	size_t offset = (i->run_magic & 255u) == '1' ? 172 : 8;
	i->run_depth = (int)backup_get32(data + 12 + offset);
	i->run_area = (int)backup_get32(data + 12 + offset + 4);
	i->run_act = i->run_depth > 0 ? (i->run_depth - 1) / 3 + 1 : 0;
	if (i->run_depth <= 0 || i->run_depth > 100000 || i->run_area < 0 || i->run_area >= MAX_BIOMES) r->ok = false;
}

static void read_summary(Read *r, const char *name, const uint8_t *data, uint32_t size) {
	BackupInfo *i = &r->info;
	if (!strcmp(name, "savedata/profile.sav")) {
		if (backup_get32(data) != PROFILE_MAGIC || size < 20) { r->ok = false; return; }
		if (size - 12 > sizeof(Profile)) i->status = BACKUP_INCOMPATIBLE_RUN;
		i->has_profile = true;
		i->runs = (int)backup_get32(data + 12);
		i->best = (int)backup_get32(data + 16);
		if (i->runs < 0 || i->best < 0) r->ok = false;
	} else if (!strcmp(name, "savedata/run.sav")) read_run(r, data, size);
	else if (!strcmp(name, "savedata/run.make") && size == 16) i->layer_make = backup_get32(data + 12);
	else if (!strcmp(name, "savedata/run.area") && size == 16) {
		r->area = (int)backup_get32(data + 12);
		r->have_area = true;
	}
}

static void read_file(const char *name, const uint8_t *data, uint32_t size, void *user) {
	Read *r = user;
	if (!strcmp(name, BACKUP_MANIFEST)) {
		r->have_manifest = true;
		r->ok = backup_manifest_read(data, size, &r->manifest) && r->ok;
		return;
	}
	if (!backup_progress_name(name)) return;
	++r->info.files;
	if (!strcmp(name, "savedata/run.state")) { r->info.has_state = size > 0; if (!size) r->ok = false; }
	else {
		if (!good_blob(data, size)) { r->ok = false; return; }
		read_summary(r, name, data, size);
	}
	r->info.hash = backup_fnv((const uint8_t *)name, strlen(name) + 1, r->info.hash);
	uint8_t len[4];
	backup_put32(len, size);
	r->info.hash = backup_fnv(len, sizeof len, r->info.hash);
	if (!strcmp(name, "savedata/profile.sav")) {
		uint8_t *copy = malloc(size);
		if (!copy) { r->ok = false; return; }
		memcpy(copy, data, size);
		backup_normalize_profile(copy, size, NULL, 0);
		r->info.hash = backup_fnv(copy, size, r->info.hash);
		free(copy);
	} else r->info.hash = backup_fnv(data, size, r->info.hash);
}

static bool same_summary(const BackupInfo *a, const BackupInfo *b) {
	return a->hash == b->hash && a->runs == b->runs && a->best == b->best && a->run_depth == b->run_depth &&
		a->run_area == b->run_area && a->run_act == b->run_act && a->needs_bn5 == b->needs_bn5 && a->files == b->files &&
		(!a->run_magic || a->run_magic == b->run_magic) && (!a->layer_make || a->layer_make == b->layer_make);
}

bool backup_info(const uint8_t *bytes, size_t n, BackupInfo *info) {
	Read r = { .info = { .format = 1, .hash = 2166136261u }, .ok = true };
	memset(info, 0, sizeof *info);
	info->status = BACKUP_DAMAGED;
	if (!backup_walk(bytes, n, NULL, NULL) || !backup_walk(bytes, n, read_file, &r) || !r.ok || !r.info.has_profile) return false;
	if (r.info.has_run && !r.info.has_state) return false;
	if (r.info.has_run && r.have_area) {
		r.info.run_area = r.area;
		r.info.needs_bn5 = r.area >= NET_AREAS;
	}
	for (int i = 0; i < 8; ++i) r.info.stamp |= (uint64_t)bytes[8 + i] << (8 * i);
	if (r.have_manifest) {
		if (!same_summary(&r.info, &r.manifest) || r.info.stamp != r.manifest.stamp) return false;
		BackupStatus status = r.info.status;
		bool profile_present = r.info.has_profile, run_present = r.info.has_run, state_present = r.info.has_state;
		r.info = r.manifest;
		r.info.has_profile = profile_present;
		r.info.has_run = run_present;
		r.info.has_state = state_present;
		r.info.status = status;
	}
	*info = r.info;
	info->status = backup_check(info, true);
	return true;
}

/* Release version and git-describe's commit count; an unversioned dev build
 * cannot be ordered against a release and still has the run-format check. */
static bool newer_build(const char *incoming) {
	unsigned int a[4] = { 0 }, b[4] = { 0 };
	if (sscanf(incoming, "%u.%u.%u", &a[0], &a[1], &a[2]) < 3 ||
		sscanf(CW_VERSION, "%u.%u.%u", &b[0], &b[1], &b[2]) < 3) return false;
	const char *asuffix = strpbrk(incoming, "+-"), *bsuffix = strpbrk(CW_VERSION, "+-");
	if (asuffix) a[3] = (unsigned int)strtoul(asuffix + 1 + (!strncmp(asuffix + 1, "git", 3) ? 3 : 0), NULL, 10);
	if (bsuffix) b[3] = (unsigned int)strtoul(bsuffix + 1 + (!strncmp(bsuffix + 1, "git", 3) ? 3 : 0), NULL, 10);
	for (int i = 0; i < 4; ++i) { if (a[i] != b[i]) return a[i] > b[i]; }
	return false;
}

BackupStatus backup_check(const BackupInfo *info, bool bn5) {
	if (info->status != BACKUP_OK) return info->status;
	if (info->format > BACKUP_FORMAT) return BACKUP_NEWER_FORMAT;
	if (info->format < 1) return BACKUP_DAMAGED;
	if (newer_build(info->version) || info->layer_make > LAYER_MAKE) return BACKUP_NEWER_BUILD;
	if (info->has_run && info->run_magic > RUN_MAGIC) return BACKUP_INCOMPATIBLE_RUN;
	if (info->has_run && info->needs_bn5 && !bn5) return BACKUP_NEEDS_BN5;
	return BACKUP_OK;
}
