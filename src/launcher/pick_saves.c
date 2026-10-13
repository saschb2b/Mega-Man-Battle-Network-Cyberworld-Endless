/* pick_saves.h, and pick.h's saves where the systems share them: the
 * device-local saves.ini (the automatic copy on or off, the folder kept),
 * and the transfer place where it is a plain folder (a desktop's, the
 * data folder of a PortMaster handheld or the 3DS). */
#include "pick_saves.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "backup.h"
#include "compat.h"
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
	return len > 0 && (size_t)len < sizeof text && backup_write_file(path, (const uint8_t *)text, (size_t)len);
}

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

#if defined(__ANDROID__) || defined(CW_IOS)
int saves_read_through(int (*get)(const char *to), uint8_t **bytes, size_t *n) {
	char tmp[600];
	snprintf(tmp, sizeof tmp, "%s/saves-in.tmp", g_data_dir);
	*bytes = NULL;
	int r = get(tmp);
	if (r > 0) {
		*bytes = backup_read_file(tmp, n);
		if (!*bytes) r = -1;
	}
	remove(tmp);
	return r;
}

bool saves_put_through(bool (*put)(const char *from), const uint8_t *bytes, size_t n) {
	char tmp[600];
	snprintf(tmp, sizeof tmp, "%s/saves-out.tmp", g_data_dir);
	bool ok = backup_write_file(tmp, bytes, n) && put(tmp);
	remove(tmp);
	return ok;
}
#else
/* (a desktop's Downloads asked of the system once: its user-dirs.dirs
 * read, never a folder made, until something is written there) */
bool pick_saves_place(char *name, size_t n) {
#if defined(CW_DESKTOP)
	static char downloads[1024];
	static bool asked, have;
	prefs_load();
	if (folder[0]) {
		snprintf(name, n, "%s", folder);
		return true;
	}
	if (!asked) {
		have = desktop_saves_default(downloads, sizeof downloads);
		asked = true;
	}
	if (have) snprintf(name, n, "%s", downloads);
	return have;
#elif defined(__EMSCRIPTEN__)
	(void)name;
	(void)n;
	return false;
#else
	snprintf(name, n, "%s", g_data_dir);
	return true;
#endif
}

bool pick_saves_scope(char *out, size_t n) { return pick_saves_place(out, n); }

static bool place_file(char *path, size_t n) {
	char place[1024];
	if (!pick_saves_place(place, sizeof place)) return false;
	return snprintf(path, n, "%s/%s", place, BACKUP_NAME) < (int)n;
}

int pick_saves_read(uint8_t **bytes, size_t *n) {
	char path[1100];
	struct stat st;
	*bytes = NULL;
	if (!place_file(path, sizeof path) || stat(path, &st) != 0) return 0;
	if (!S_ISREG(st.st_mode)) return -1;
	*bytes = backup_read_file(path, n);
	return *bytes ? 1 : -1;
}

bool pick_saves_stat(uint64_t *size, int64_t *mtime) {
	char path[1100];
	struct stat st;
	if (!place_file(path, sizeof path) || stat(path, &st) != 0) return false;
	*size = (uint64_t)st.st_size;
	*mtime = (int64_t)st.st_mtime;
	return true;
}

bool pick_saves_put(const uint8_t *bytes, size_t n) {
	char place[1024], path[1100];
	if (!pick_saves_place(place, sizeof place)) return false;
	cw_mkdir(place);   /* (a desktop's Downloads, made at the first write there) */
	snprintf(path, sizeof path, "%s/%s", place, BACKUP_NAME);
	return backup_write_file(path, bytes, n);
}
#endif
