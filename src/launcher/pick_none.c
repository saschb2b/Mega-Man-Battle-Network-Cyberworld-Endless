/* pick.h where there is no launcher: the 3DS, a PortMaster handheld (its
 * ROM in the port's own rom/ folder) and the browser (its page chooses
 * the ROMs, web/play/app.js). */
#include "pick.h"

#if !defined(CW_DESKTOP) && !defined(CW_IOS) && !defined(__ANDROID__)
#include <stdio.h>
#include <string.h>

#include "backup.h"
#include "game.h"
#include "pick_saves.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(int, browser_saves_import, (void), {
	if (!Module.savesPick) return 0;
	Module.savesPick();
	return 1;
});

EM_JS(int, browser_saves_export, (const char *from, const char *name), {
	return Module.savesDownload && Module.savesDownload(UTF8ToString(from), UTF8ToString(name)) ? 1 : 0;
});
#endif

static PickResult saves_result;
static bool saves_done;

unsigned pick_kinds(void) { return 0; }

bool pick_open(int kind, int slot) {
	(void)kind;
	(void)slot;
	return false;
}

bool pick_busy(void) { return false; }

bool pick_done(PickResult *r) {
	(void)r;
	return false;
}

int pick_look(char *msg, size_t n) {
	(void)msg;
	(void)n;
	return -1;
}

bool pick_folder(char *name, size_t n) {
	(void)name;
	(void)n;
	return false;
}

bool pick_saves_put(const char *from) {
	char to[600];
	snprintf(to, sizeof to, "%s/%s", g_data_dir, BACKUP_NAME);
	return saves_copy(from, to);
}

bool pick_saves_get(const char *to) {
	char from[600];
	snprintf(from, sizeof from, "%s/%s", g_data_dir, BACKUP_NAME);
	return saves_stage(from, to);
}

bool pick_saves_import(void) {
#ifdef __EMSCRIPTEN__
	return browser_saves_import() != 0;
#else
	memset(&saves_result, 0, sizeof saves_result);
	snprintf(saves_result.text, sizeof saves_result.text, "%s/saves-found.cwsave", g_data_dir);
	saves_result.path = true;
	saves_result.status = pick_saves_find(saves_result.text) ? 1 : -2;
	if (saves_result.status < 0) snprintf(saves_result.text, sizeof saves_result.text, "No saves file found in %s.", g_data_dir);
	saves_done = true;
	return true;
#endif
}

bool pick_saves_export(const char *from) {
	memset(&saves_result, 0, sizeof saves_result);
#ifdef __EMSCRIPTEN__
	saves_result.status = browser_saves_export(from, BACKUP_NAME) ? 1 : -2;
	snprintf(saves_result.text, sizeof saves_result.text, "Downloads/%s", BACKUP_NAME);
#else
	saves_result.status = pick_saves_put(from) ? 1 : -2;
	snprintf(saves_result.text, sizeof saves_result.text, "%s/%s", g_data_dir, BACKUP_NAME);
#endif
	saves_done = true;
	return true;
}

bool pick_saves_choose_folder(void) { return false; }
bool pick_saves_busy(void) { return false; }

bool pick_saves_done(PickResult *r) {
	if (!saves_done) return false;
	*r = saves_result;
	saves_done = false;
	return true;
}
#else
typedef int pick_none_unused;
#endif
