/* saves.h. The screen's course: opened over the title or the ROMs screen,
 * home first; a file to import compared with this device's saves, focus
 * on the side with more progress; B goes back a view, and leaves the
 * comparison keeping this device's saves. Its buttons' work is
 * saves_actions.c's, its picture saves_draw.c's. */
#include "saves.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "audio.h"
#include "launcher_saves.h"
#include "mirror.h"
#include "pick.h"
#include "platform.h"
#include "rom.h"
#include "saves_actions.h"
#include "saves_cloud.h"
#include "saves_code.h"
#include "saves_home.h"
#include "saves_draw.h"
#include "saves_state.h"
#include "saves_text.h"
#include "touch.h"

Saves SV;

void saves_refresh(void) {
	SV.have = mirror_local(&SV.here);
	SV.unreadable = !SV.have && SV.here.status != BACKUP_OK;
	SV.undo = backup_can_undo(g_data_dir);
}

void saves_clear_file(void) {
	free(SV.bytes);
	SV.bytes = NULL;
	SV.size = 0;
	SV.from_place = false;
	SV.from_cloud = false;
	SV.from_code = false;
}

void saves_result(const char *text) {
	if (text != SV.note) snprintf(SV.note, sizeof SV.note, "%s", text);
	SV.view = SAVES_RESULT;
	SV.offer_undo = SV.undone = SV.resume = false;
	SV.focus = SAVES_DONE;
}

int saves_pages(SavesPage *out) {
	int n = 0;
	if (saves_cloud_here()) out[n++] = SAVES_PAGE_CLOUD;
	if (saves_code_here()) out[n++] = SAVES_PAGE_CODE;
	out[n++] = SAVES_PAGE_FILE;
	return n;
}

void saves_home_page(SavesPage page) {
	SavesPage pages[SAVES_PAGES];
	int n = saves_pages(pages), at = 0;
	for (int i = 0; i < n; ++i) if (pages[i] == page) at = i;
	SV.view = SAVES_HOME;
	SV.page = pages[at];
	SV.paged = true;
	int lines[SAVES_ACTIONS], k = saves_actions(lines);
	SV.focus = k ? lines[0] : SAVES_DONE;
	/* (the line this page was left on, where it still stands) */
	for (int i = 0; i < k; ++i) if (lines[i] == SV.page_focus[SV.page]) SV.focus = lines[i];
}

/* (home: the page that needs the player, the cloud's trouble or saves
 * waiting in the copy's place; else the page last shown; the first there
 * is at the first opening) */
static void home(void) {
	MirrorStatus m;
	mirror_status(&m);
	saves_cloud_look();
	if (saves_cloud_needs_you()) saves_home_page(SAVES_PAGE_CLOUD);
	else if (m.state == MIRROR_HELD) {
		SV.page_focus[SAVES_PAGE_FILE] = SAVES_IMPORT;
		saves_home_page(SAVES_PAGE_FILE);
	} else saves_home_page(SV.paged ? SV.page : SAVES_PAGE_CLOUD);
}

/* (L and R: the page before or after, round) */
static void turn_page(int step) {
	SavesPage pages[SAVES_PAGES];
	int n = saves_pages(pages), at = 0;
	if (n < 2) return;
	for (int i = 0; i < n; ++i) if (pages[i] == SV.page) at = i;
	saves_home_page(pages[(at + step + n) % n]);
	SV.said[0] = 0;
	if (R.data) audio_sfx(SFX_CURSOR);
}

bool saves_open(void) {
	const Scene *now = scene_current();
	if (now == &scene_saves) return true;
	if (now == &scene_emu) return false;
	SV.back = now == &scene_launcher ? &scene_launcher : &scene_title;
	SV.busy = false;
	SV.said[0] = 0;
	saves_clear_file();
	/* (the player asked: the copy's place looked at, a desktop's Downloads
	 * too) */
	mirror_look();
	saves_refresh();
	home();
	scene_set(&scene_saves);
	return true;
}

/* (a record's deepest layer: its best, or the run on now where deeper) */
static int deepest(const BackupInfo *info) { return info->best > info->run_depth ? info->best : info->run_depth; }

/* (the side with more progress: more runs, then the deeper layer, this
 * device's at a tie; a reinstall's empty saves never) */
static bool file_ahead(void) {
	if (!SV.have) return true;
	if (SV.file.runs != SV.here.runs) return SV.file.runs > SV.here.runs;
	return deepest(&SV.file) > deepest(&SV.here);
}

void saves_compare(uint8_t *bytes, size_t n, bool from_place) {
	if (!saves_open()) {
		free(bytes);
		return;
	}
	saves_clear_file();
	SV.bytes = bytes;
	SV.size = n;
	SV.from_place = from_place;
	saves_refresh();
	memset(&SV.file, 0, sizeof SV.file);
	SV.status = bytes && backup_info(bytes, n, &SV.file) ? backup_check(&SV.file, XR[XROM_BN5_COLONEL_US].data != NULL) : BACKUP_DAMAGED;
	/* (one that opened by itself, the cloud's or the copy's place's, waits
	 * half a second: a START mashed through the title answers nothing) */
	SV.guard_input = from_place || SV.from_cloud ? 30 : 1;
	if (SV.status == BACKUP_OK && SV.have && SV.here.hash == SV.file.hash) {
		if (from_place) mirror_answer(true);
		saves_clear_file();
		saves_result(saves_same());
		return;
	}
	SV.view = SAVES_COMPARE;
	if (SV.status != BACKUP_OK) {
		saves_refusal(SV.status, &SV.file, SV.note, sizeof SV.note);
		SV.focus = SAVES_DONE;
	} else {
		MirrorStatus m;
		mirror_status(&m);
		bool place = from_place && m.state != MIRROR_OFF;
		saves_compare_note(&SV.here, SV.have, SV.unreadable, place ? m.place : NULL, SV.note, sizeof SV.note);
		SV.focus = file_ahead() ? SAVES_TAKE : SV.have ? SAVES_KEEP : SAVES_DONE;
	}
	if (R.data) audio_sfx(SFX_REVEAL);
}

void saves_compare_cloud(uint8_t *bytes, size_t n) {
	saves_compare(bytes, n, false);
	if (!SV.bytes) return;
	SV.from_cloud = true;
	SV.guard_input = 30;
	if (SV.status == BACKUP_OK) saves_cloud_compare_note(&SV.here, SV.have, SV.unreadable, SV.note, sizeof SV.note);
}

bool saves_compare_held(void) {
	const uint8_t *held;
	size_t n;
	if (!mirror_held(&held, &n)) return false;
	uint8_t *bytes = malloc(n ? n : 1);
	if (!bytes) return false;
	memcpy(bytes, held, n);
	saves_compare(bytes, n, true);
	return true;
}

void saves_import(const char *path) {
	if (!path || !*path) return;
	size_t n = 0;
	uint8_t *bytes = backup_read_file(path, &n);
	saves_compare(bytes, bytes ? n : 0, false);
}

/* KEEP THESE (the place's file: the copy may write over it now) or DONE
 * and B (the place's file: not opened by itself again, no copy over it) */
void saves_leave_compare(bool keep_these) {
	if (SV.from_place) mirror_answer(keep_these);
	if (SV.from_cloud && keep_these) saves_cloud_answered(false);
	saves_clear_file();
	home();
}

static bool back(void) {
	if (SV.busy) {
		/* (the browser's wait for its page's file let go) */
		pick_saves_cancel();
		return true;
	}
	if (SV.view == SAVES_COMPARE) saves_leave_compare(false);
	else if (SV.view == SAVES_RESULT) home();
	else if (SV.view == SAVES_SEND_CODE || SV.view == SAVES_RECEIVE_CODE) {
		saves_code_leave();
		home();
	}
	else {
		if (SV.back == &scene_title) title_menu = true;
		scene_set(SV.back ? SV.back : &scene_title);
	}
	if (R.data) audio_sfx(SFX_CANCEL);
	return true;
}

static void enter(void) {
	/* (--scene saves, a capture's start: as from the title) */
	if (!SV.back) {
		SV.back = &scene_title;
		mirror_look();
		saves_refresh();
		home();
	}
	platform_own_taps(true);
	platform_on_back(back);
	platform_second_screen(NULL, NULL);
	touch_release();
}

static void leave(void) {
	platform_own_taps(false);
	platform_on_back(NULL);
	saves_code_leave();
	saves_clear_file();
}

static void update(void) {
	saves_tick();
	if (saves_code_tick()) { back(); return; }
	if (saves_picked() || SV.guard_input) {
		int x, y;
		if (SV.guard_input) --SV.guard_input;
		(void)platform_tap(&x, &y);
		return;
	}
	if (btn_pressed(BTN_B)) {
		if (!saves_home_back()) back();
		return;
	}
	if (SV.busy || saves_pointer(back, turn_page)) return;
	if (SV.view == SAVES_HOME) { saves_home_keys(turn_page); return; }
	if (btn_repeat(BTN_UP)) saves_move_focus(0, -1);
	if (btn_repeat(BTN_DOWN)) saves_move_focus(0, 1);
	if (btn_repeat(BTN_LEFT)) saves_move_focus(-1, 0);
	if (btn_repeat(BTN_RIGHT)) saves_move_focus(1, 0);
	/* (the comparison's answers by A alone: START opened nothing here) */
	if (btn_pressed(BTN_A) || (SV.view != SAVES_COMPARE && btn_pressed(BTN_START))) saves_activate(SV.focus);
}

const Scene scene_saves = { "saves", enter, update, saves_draw, leave, NULL };

void saves_describe(FILE *f) {
	if (scene_current() != &scene_saves) return;
	static const char *const views[] = { "home", "compare", "result", "send", "receive" }, *const pages[SAVES_PAGES] = { "cloud", "code", "file" };
	static const char *const actions[SAVES_DIGIT + 1] = { "export", "import", "send", "receive", "undo", "auto", "folder", "keep", "take", "done",
		"continue", "erase", "get", "turnon", "link", "add", "unlink", "delete", "digit" };
	fprintf(f, "savesview %s\nsavespage %s\nsavesfocus %s\n", views[SV.view], pages[SV.page], actions[SV.focus < SAVES_DIGIT ? SV.focus : SAVES_DIGIT]);
	if (SV.have) fprintf(f, "here %08x\nherelayer %d\nhereruns %d\n", (unsigned)SV.here.hash, SV.here.run_depth, SV.here.runs);
	else fprintf(f, "here none\n");
	if (SV.view == SAVES_COMPARE)
		fprintf(f, "theirs %08x\ntheirlayer %d\ntheirstatus %s\n", (unsigned)SV.file.hash, SV.file.run_depth, SV.status == BACKUP_OK ? "ok" : "refused");
	fprintf(f, "undo %s\nautocopy %s\n", SV.undo ? "yes" : "no", pick_saves_auto_enabled() ? "on" : "off");
	if (saves_home_asking() >= 0) fprintf(f, "savesasks %s\n", saves_home_yes() ? "yes" : "no");
	if (SV.busy) fprintf(f, "savesbusy %s\n", SV.said);
	if (SV.view == SAVES_HOME || !SV.note[0]) return;
	/* (its words in one line) */
	fprintf(f, "savesnote ");
	for (const char *c = SV.note; *c; ++c) fputc(*c == '\n' ? ' ' : *c, f);
	fputc('\n', f);
}

bool saves_key(void) {
	static bool alone;
	if (btn_pressed(BTN_L)) alone = true;
	if (btn_held(BTN_L) && btn_pressed(BTN_SELECT)) alone = false;
	if (!btn_released(BTN_L) || !alone) return false;
	alone = false;
	if (saves_open() && R.data) audio_sfx(SFX_SELECT);
	return true;
}

void saves_tick(void) {
	const Scene *scene = scene_current();
	if (scene != &scene_title && scene != &scene_launcher && scene != &scene_saves) return;
	saves_cloud_tick();
	if (SV.busy || pick_busy() || pick_saves_busy() || SV.view == SAVES_COMPARE) return;
	char path[1100];
	/* (the ROMs screen takes its own drops: a ROM or a .cwsave) */
	if (scene != &scene_launcher && platform_dropped(path, sizeof path)) {
		size_t n = strlen(path);
		if (n > 7 && !SDL_strcasecmp(path + n - 7, ".cwsave")) {
			saves_import(path);
			return;
		}
	}
	/* (the copy's place holding saves not answered: compared at once, once
	 * a look) */
	if (scene != &scene_saves && mirror_incoming()) saves_compare_held();
}

/* (the ROMs screen's two hooks: launcher_saves.h) */
const LauncherSaves launcher_saves = { saves_import, saves_tick };

#ifdef __EMSCRIPTEN__
/* The browser page's menu (web/play/app.js): SAVES over the title, 0 in a
 * run (the page says to go to the title first) */
EMSCRIPTEN_KEEPALIVE
int cw_saves_open(void) { return saves_open() ? 1 : 0; }
/* ... and the file its chooser gave, "" for none */
EMSCRIPTEN_KEEPALIVE
void cw_saves_import(const char *path) {
	if (!saves_open()) return;
	pick_saves_page(path);
}
#endif
