/* V2's portable identity. Its timestamp/revision change when progress does,
 * so starting or exporting twice never invents a new version of a save. */
#include "backup_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "compat.h"
#include "layer_make.h"
#include "save_format.h"
#include "version.h"

#ifdef _WIN32
#include <windows.h>
#endif

static char device_name[64];

void backup_set_device(const char *name) {
	snprintf(device_name, sizeof device_name, "%s", name ? name : "");
	for (char *p = device_name; *p; ++p) if ((unsigned char)*p < 32) *p = ' ';
}

/* Text with a fixed schema and order, also readable beside the local saves. */
bool backup_manifest_read(const uint8_t *data, size_t n, BackupInfo *info) {
	if (!n || n >= BACKUP_MANIFEST_MAX || memchr(data, 0, n)) return false;
	char text[BACKUP_MANIFEST_MAX];
	memcpy(text, data, n);
	text[n] = 0;
	unsigned long long stamp;
	unsigned int format, needs;
	int end = 0;
	memset(info, 0, sizeof *info);
	int fields = sscanf(text, "format=%u\nversion=%63[^\n]\nrun_magic=%x\nlayer_make=%u\nstamp=%llu\nsystem=%23[^\n]\ndevice=%63[^\n]\nsave_id=%32[^\n]\nrevision=%u\nparent_id=%32[^\n]\nparent_revision=%u\npayload_hash=%x\nfiles=%d\nruns=%d\nbest=%d\nrun_depth=%d\nrun_area=%d\nrun_act=%d\nneeds_bn5=%u\n%n",
		&format, info->version, &info->run_magic, &info->layer_make, &stamp, info->system, info->device,
		info->save_id, &info->revision, info->parent_id, &info->parent_revision, &info->hash, &info->files,
		&info->runs, &info->best, &info->run_depth, &info->run_area, &info->run_act, &needs, &end);
	if (fields != 19 || end != (int)n || format > 100 || needs > 1 || !info->revision ||
		strlen(info->save_id) != 32 ||
		(strlen(info->parent_id) != 32 && strcmp(info->parent_id, "-"))) return false;
	info->format = (int)format;
	info->stamp = (uint64_t)stamp;
	info->needs_bn5 = needs != 0;
	if (!strcmp(info->parent_id, "-")) info->parent_id[0] = 0;
	return true;
}

static bool manifest_text(const BackupInfo *i, char *out, size_t n) {
	int wrote = snprintf(out, n, "format=%d\nversion=%s\nrun_magic=%08x\nlayer_make=%u\nstamp=%llu\nsystem=%s\ndevice=%s\nsave_id=%s\nrevision=%u\nparent_id=%s\nparent_revision=%u\npayload_hash=%08x\nfiles=%d\nruns=%d\nbest=%d\nrun_depth=%d\nrun_area=%d\nrun_act=%d\nneeds_bn5=%d\n",
		i->format, i->version, i->run_magic, i->layer_make, (unsigned long long)i->stamp, i->system, i->device,
		i->save_id, i->revision, i->parent_id[0] ? i->parent_id : "-", i->parent_revision, i->hash, i->files,
		i->runs, i->best, i->run_depth, i->run_area, i->run_act, i->needs_bn5 ? 1 : 0);
	return wrote >= 0 && (size_t)wrote < n;
}

static void random_id(char out[33]) {
	uint32_t words[4];
	FILE *f = fopen("/dev/urandom", "rb");
	bool got = f && fread(words, sizeof words, 1, f) == 1;
	if (f) fclose(f);
	if (!got) {
		static uint32_t serial;
		uint32_t h = (uint32_t)time(NULL) ^ (uint32_t)clock() ^ (uint32_t)(uintptr_t)out ^ ++serial;
		if (!h) h = 0x9e3779b9u;
		for (int i = 0; i < 4; ++i) { h ^= h << 13; h ^= h >> 17; h ^= h << 5; words[i] = h; }
	}
	for (int i = 0; i < 4; ++i) snprintf(out + 8 * i, 9, "%08x", words[i]);
}

static const char *system_name(void) {
#if defined(__EMSCRIPTEN__)
	return "browser";
#elif defined(__3DS__)
	return "3DS";
#elif defined(__ANDROID__)
	return "Android";
#elif defined(CW_IOS)
	return "iOS";
#elif defined(_WIN32)
	return "Windows";
#elif defined(__APPLE__)
	return "macOS";
#elif defined(CW_DESKTOP)
	return "Linux";
#else
	return "PortMaster";
#endif
}

static void local_device(char out[64]) {
	if (!device_name[0]) {
#ifdef _WIN32
		DWORD size = sizeof device_name;
		if (!GetComputerNameA(device_name, &size)) device_name[0] = 0;
#elif !defined(__EMSCRIPTEN__) && !defined(__3DS__)
		if (gethostname(device_name, sizeof device_name - 1)) device_name[0] = 0;
#endif
		if (!device_name[0]) backup_set_device(system_name());
	}
	snprintf(out, 64, "%s", device_name);
}

bool backup_manifest_update(const char *data_dir, uint64_t stamp, BackupInfo *info, char *out, size_t n) {
	char path[1100];
	snprintf(path, sizeof path, "%s/%s", data_dir, BACKUP_MANIFEST);
	size_t old_n = 0;
	uint8_t *old = backup_read_file(path, &old_n);
	BackupInfo previous;
	bool have = old && backup_manifest_read(old, old_n, &previous);
	if (have && previous.hash == info->hash) {
		*info = previous;
		bool ok = old_n < n;
		if (ok) { memcpy(out, old, old_n); out[old_n] = 0; }
		free(old);
		return ok;
	}
	free(old);
	info->format = BACKUP_FORMAT;
	info->stamp = stamp ? stamp : (uint64_t)time(NULL);
	if (!info->run_magic) info->run_magic = RUN_MAGIC;
	if (!info->layer_make) info->layer_make = LAYER_MAKE;
	snprintf(info->version, sizeof info->version, "%s", CW_VERSION);
	snprintf(info->system, sizeof info->system, "%s", system_name());
	local_device(info->device);
	if (have) {
		if (previous.revision == UINT32_MAX) return false;
		memcpy(info->save_id, previous.save_id, sizeof info->save_id);
		memcpy(info->parent_id, previous.save_id, sizeof info->parent_id);
		info->parent_revision = previous.revision;
		info->revision = previous.revision + 1;
	} else { random_id(info->save_id); info->revision = 1; }
	return manifest_text(info, out, n) && backup_write_file(path, (const uint8_t *)out, strlen(out));
}
