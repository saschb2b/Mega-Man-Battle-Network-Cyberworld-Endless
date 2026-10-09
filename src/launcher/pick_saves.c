#include "pick_saves.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "backup.h"
#include "game.h"
#include "pick.h"
#include "platform.h"
#ifdef CW_DESKTOP
#include "desktop.h"
#endif

static char loaded[512], folder[1024];
static bool automatic;

static void prefs_load(void) {
	if (!strcmp(loaded, g_data_dir)) return;
	snprintf(loaded, sizeof loaded, "%s", g_data_dir);
	folder[0] = 0;
#if defined(__ANDROID__) || defined(CW_IOS)
	automatic = true;
#else
	automatic = false;
#endif
	char path[600], line[1200];
	snprintf(path, sizeof path, "%s/saves.ini", g_data_dir);
	FILE *f = fopen(path, "r");
	if (!f) return;
	while (fgets(line, sizeof line, f)) {
		line[strcspn(line, "\r\n")] = 0;
		if (!strncmp(line, "auto=", 5)) automatic = atoi(line + 5) != 0;
		else if (!strncmp(line, "folder=", 7)) snprintf(folder, sizeof folder, "%.*s", (int)sizeof folder - 1, line + 7);
	}
	fclose(f);
}

static bool prefs_write(void) {
	char path[600], text[1200];
	snprintf(path, sizeof path, "%s/saves.ini", g_data_dir);
	int len = snprintf(text, sizeof text, "auto=%d\nfolder=%s\n", automatic, folder);
	bool ok = len > 0 && (size_t)len < sizeof text && backup_write_file(path, (const uint8_t *)text, (size_t)len);
	if (ok) platform_persist();
	return ok;
}

#ifdef CW_DESKTOP
static bool saves_folder_kept(char *out, size_t n) {
	prefs_load();
	if (n) snprintf(out, n, "%s", folder);
	return folder[0] != 0;
}
#endif

bool saves_folder_keep(const char *path) {
	prefs_load();
	if (strlen(path) >= sizeof folder || strpbrk(path, "\r\n")) return false;
	snprintf(folder, sizeof folder, "%s", path);
	return prefs_write();
}

unsigned pick_saves_capabilities(void) {
#ifdef __EMSCRIPTEN__
	return 0;
#elif defined(CW_DESKTOP)
	return SAVES_CAN_AUTO | (!desktop_big_screen() && desktop_can_choose() ? SAVES_CAN_FOLDER : 0);
#elif defined(__ANDROID__) || defined(CW_IOS)
	return SAVES_CAN_AUTO | SAVES_CAN_FOLDER;
#else
	return SAVES_CAN_AUTO;
#endif
}

bool pick_saves_auto_enabled(void) {
	prefs_load();
	return (pick_saves_capabilities() & SAVES_CAN_AUTO) && automatic;
}

void pick_saves_auto(bool enabled) {
	prefs_load();
	if (!(pick_saves_capabilities() & SAVES_CAN_AUTO)) return;
	automatic = enabled;
	prefs_write();
}

bool saves_copy(const char *from, const char *to) {
	size_t n = 0;
	uint8_t *b = backup_read_file(from, &n);
	bool ok = b && backup_write_file(to, b, n);
	free(b);
	return ok;
}

bool saves_stage(const char *from, const char *to) {
	struct stat st;
	if (stat(from, &st) != 0 || !S_ISREG(st.st_mode)) return false;
	if (saves_copy(from, to)) return true;
	static const uint8_t refused[] = "The incoming saves file could not be read.";
	return backup_write_file(to, refused, sizeof refused - 1);
}

bool pick_saves_place(char *name, size_t n) {
#if defined(__ANDROID__) || defined(CW_IOS)
	return saves_phone_place(name, n);
#elif defined(CW_DESKTOP)
	if (saves_folder_kept(name, n)) return true;
	return desktop_saves_default(name, n);
#elif defined(__EMSCRIPTEN__)
	if (n) snprintf(name, n, "Downloads");
	return true;
#else
	if (n) snprintf(name, n, "%s", g_data_dir);
	return true;
#endif
}

bool pick_saves_scope(char *out, size_t n) {
#if defined(__ANDROID__) || defined(CW_IOS)
	return saves_phone_scope(out, n);
#elif defined(__EMSCRIPTEN__)
	if (n) out[0] = 0;
	return false;
#else
	return pick_saves_place(out, n);
#endif
}

/* Keep the first damaged candidate too: the screen can explain the
 * refusal, and an automatic export must not erase it on the way there. */
#ifndef __EMSCRIPTEN__
static bool different(const char *path, const BackupInfo *local, bool have, uint32_t ignored, uint32_t exported) {
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	uint32_t hash = 2166136261u;
	for (size_t i = 0; b && i < n; ++i) hash = (hash ^ b[i]) * 16777619u;
	BackupInfo info;
	bool found = false;
	if (b && (!ignored || hash != ignored)) {
		bool valid = backup_info(b, n, &info) && info.status == BACKUP_OK;
		found = !valid || ((!exported || hash != exported) && (!have || info.hash != local->hash));
	}
	free(b);
	return found;
}
#endif

static bool find(const char *to, uint32_t ignored, uint32_t exported) {
#ifdef __EMSCRIPTEN__
	(void)to;
	(void)ignored;
	(void)exported;
	return false;
#else
	BackupInfo local;
	bool have = backup_local_info(g_data_dir, &local);
	if (pick_saves_get(to) && different(to, &local, have, ignored, exported)) return true;
#ifdef CW_DESKTOP
	char place[1024], path[1100];
	if (desktop_saves_default(place, sizeof place)) {
		snprintf(path, sizeof path, "%s/%s", place, BACKUP_NAME);
		if (saves_stage(path, to) && different(to, &local, have, ignored, exported)) return true;
	}
#else
	char path[1100];
#endif
	snprintf(path, sizeof path, "%s/%s", g_data_dir, BACKUP_NAME);
	if (saves_stage(path, to) && different(to, &local, have, ignored, exported)) return true;
	remove(to);
	return false;
#endif
}

bool pick_saves_find(const char *to) { return find(to, 0, 0); }

bool pick_saves_discover(const char *to, uint32_t ignored_hash, uint32_t exported_hash) {
	return find(to, ignored_hash, exported_hash);
}
