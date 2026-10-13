/* pick.h where there is no launcher: the 3DS, a PortMaster handheld (its
 * ROM in the port's own rom/ folder) and the browser (its page chooses
 * the ROMs, web/play/app.js). SAVES' import and export there: the data
 * folder's cyberworld-endless.cwsave on the handhelds, the page's own
 * file chooser and download in the browser. */
#include "pick.h"

#if !defined(CW_DESKTOP) && !defined(CW_IOS) && !defined(__ANDROID__)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "backup.h"
#include "game.h"
#include "pick_text.h"
#include "save_queue.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

#define PAGE_WAIT_MS 60000   /* the page's file chooser waited for this long at most */

EM_JS(int, browser_saves_import, (void), {
	if (!Module.savesPick) return 0;
	Module.savesPick();
	return 1;
});

EM_JS(int, browser_saves_export, (const char *from, const char *name), {
	return Module.savesDownload && Module.savesDownload(UTF8ToString(from), UTF8ToString(name)) ? 1 : 0;
});

static uint32_t waiting_since;
#endif

static PickResult saves_result;
static bool saves_done, waiting;

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

static void finish(int status, const char *text) {
	memset(&saves_result, 0, sizeof saves_result);
	saves_result.status = status;
	snprintf(saves_result.text, sizeof saves_result.text, "%s", text);
	saves_done = true;
}

bool pick_saves_import(void) {
#ifdef __EMSCRIPTEN__
	/* (the page asks for a file with its own button: a browser opens its
	 * chooser only from the player's own click, web/play/app.js) */
	if (!browser_saves_import()) return false;
	waiting = true;
	waiting_since = SDL_GetTicks();
	return true;
#else
	char path[700];
	snprintf(path, sizeof path, "%s/%s", g_data_dir, BACKUP_NAME);
	FILE *f = fopen(path, "rb");
	if (f) fclose(f);
	if (f) finish(1, path);
	else {
		char text[800];
		pick_text_none_found(g_data_dir, text, sizeof text);
		finish(-2, text);
	}
	saves_result.path = saves_result.status > 0;
	return true;
#endif
}

bool pick_saves_export(const char *from) {
	char text[800];
#ifdef __EMSCRIPTEN__
	bool ok = browser_saves_export(from, BACKUP_NAME) != 0;
	snprintf(text, sizeof text, "%s", ok ? pick_text_downloads() : pick_text_not_written());
#else
	char to[700];
	snprintf(to, sizeof to, "%s/%s", g_data_dir, BACKUP_NAME);
	save_queue_wait();
	size_t n = 0;
	uint8_t *b = backup_read_file(from, &n);
	bool ok = b && backup_write_file(to, b, n);
	free(b);
	snprintf(text, sizeof text, "%s", ok ? to : pick_text_not_written());
#endif
	finish(ok ? 1 : -2, text);
	return true;
}

bool pick_saves_choose_folder(void) { return false; }

bool pick_saves_busy(void) {
#ifdef __EMSCRIPTEN__
	if (waiting && SDL_GetTicks() - waiting_since >= PAGE_WAIT_MS) pick_saves_cancel();
#endif
	return saves_done || waiting;
}

void pick_saves_cancel(void) {
	if (!waiting) return;
	waiting = false;
	finish(-1, "");
}

bool pick_saves_done(PickResult *r) {
	if (!saves_done) return false;
	*r = saves_result;
	saves_done = false;
	return true;
}

#ifdef __EMSCRIPTEN__
void pick_saves_page(const char *path) {
	waiting = false;
	finish(path && *path ? 1 : -1, path ? path : "");
	saves_result.path = saves_result.status > 0;
}
#endif
#else
typedef int pick_none_unused;
#endif
