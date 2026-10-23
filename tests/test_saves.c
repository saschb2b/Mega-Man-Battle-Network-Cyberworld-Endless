/* The SAVES screen (src/saves/) driven as a player drives it, by its
 * scene's update with presses, over invented saves: the real transfer
 * code (backup*.c, mirror.c), the pickers and the drawing stubbed (no ROM,
 * no dialog, no window). Its views, buttons, words, answers and Undo. */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "audio.h"
#include "backup_internal.h"
#include "compat.h"
#include "gfx.h"
#include "guardians.h"
#include "mirror.h"
#include "pick.h"
#include "pet_look.h"
#include "pixfont.h"
#include "platform.h"
#include "rivals.h"
#include "rom.h"
#include "save.h"
#include "save_format.h"
#include "saves.h"
#include "saves_actions.h"
#include "saves_draw.h"
#include "saves_state.h"
#include "saves_text.h"
#include "second_frame.h"
#include "touch.h"

Platform P;
Rom R;
XRom XR[XROM_COUNT];
char g_data_dir[512];
bool emu_resume_requested, title_menu;
const Scene scene_title = { .name = "title" };
const Scene scene_intro = { .name = "intro" };
const Scene scene_emu = { .name = "emu" };
const Scene scene_launcher = { .name = "launcher" };

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static const Scene *current = &scene_title;
static Uint32 ticks = 5000;
static bool load_ok = true, picker_open, picker_ended, automatic, cancelled;
static int exports, imports;
static unsigned capabilities = SAVES_CAN_AUTO | SAVES_CAN_FOLDER;
static PickResult ended;
static char place[600], shown[64][160];
static int shown_count;

Uint32 SDL_GetTicks(void) { return ticks; }
const Scene *scene_current(void) { return current; }
void scene_set(const Scene *scene) {
	if (current && current->leave) current->leave();
	current = scene;
	if (scene && scene->enter) scene->enter();
}
void platform_persist(void) { mirror_note(); }
void platform_own_taps(bool on) { (void)on; }
void platform_on_back(bool (*back)(void)) { (void)back; }
void platform_second_screen(SecondScreen draw, SecondChanged changed) { (void)draw; (void)changed; }
bool platform_tap(int *x, int *y) { (void)x; (void)y; return false; }
bool platform_dropped(char *path, size_t n) { (void)path; (void)n; return false; }
void touch_release(void) { }
void save_init(void) { }
void rivals_load(void) { }
void audio_sfx(Sfx effect) { (void)effect; }
bool load_run(void) { return load_ok; }
const NetAreaDef *net_area_def(int area) {
	static const NetAreaDef def = { .name = "ACDC Area" };
	(void)area;
	return &def;
}
const char *guardian_area_original_name(int area) { (void)area; return "ACDC Area"; }

void pixfont_draw(int x, int y, const char *text, SDL_Color color, SDL_Color shade, int align, int scale) {
	(void)x; (void)y; (void)color; (void)shade; (void)align; (void)scale;
	if (shown_count < 64) snprintf(shown[shown_count++], sizeof shown[0], "%s", text);
}
int pixfont_width(const char *text, int scale) { return (int)strlen(text) * 5 * scale; }
int pixfont_wrap(const char *text, int w, int scale, char lines[][96], int most) {
	int n = 0, per = w / (5 * scale);
	for (const char *p = text; *p && n < most; ++n) {
		snprintf(lines[n], 96, "%.*s", per, p);
		p += strlen(lines[n]);
	}
	return n;
}
void fill_rect(int x, int y, int w, int h, SDL_Color color) { (void)x; (void)y; (void)w; (void)h; (void)color; }
void fill_rects(const SDL_Rect *r, int n, SDL_Color c) { (void)r; (void)n; (void)c; }
SDL_Rect look_box(int x, int y, int w, int h, SDL_Color fill) { (void)fill; return (SDL_Rect){ x + 3, y + 3, w - 6, h - 6 }; }
int look_title_width(const char *s) { return (int)strlen(s) * 7; }
void look_title(int x, int y, const char *s, SDL_Color c) { (void)x; (void)y; (void)s; (void)c; }
void look_stripes(int sx, int y, int right, int h) { (void)sx; (void)y; (void)right; (void)h; }

/* the pickers: a place folder of plain files, choosers that end when told */
unsigned pick_saves_capabilities(void) { return capabilities; }
bool pick_saves_auto_enabled(void) { return automatic && (capabilities & SAVES_CAN_AUTO); }
void pick_saves_auto(bool on) { automatic = on; }
bool pick_busy(void) { return false; }
bool pick_saves_busy(void) { return picker_open; }
void pick_saves_cancel(void) { cancelled = true; picker_open = false; }
bool pick_saves_import(void) { ++imports; picker_open = true; return true; }
bool pick_saves_export(const char *from) {
	++exports;
	char to[800];
	snprintf(to, sizeof to, "%s/%s", place, BACKUP_NAME);
	size_t n = 0;
	uint8_t *b = backup_read_file(from, &n);
	bool ok = b && backup_write_file(to, b, n);
	free(b);
	picker_open = true;
	memset(&ended, 0, sizeof ended);
	ended.status = ok ? 1 : -2;
	snprintf(ended.text, sizeof ended.text, "%s", to);
	picker_ended = true;
	return true;
}
bool pick_saves_choose_folder(void) { picker_open = true; return true; }
bool pick_saves_done(PickResult *r) {
	if (!picker_ended) return false;
	*r = ended;
	picker_ended = picker_open = false;
	return true;
}
bool pick_saves_place(char *name, size_t n) { snprintf(name, n, "%s", place); return true; }
bool pick_saves_scope(char *out, size_t n) { snprintf(out, n, "%s", place); return true; }
int pick_saves_read(uint8_t **bytes, size_t *n) {
	char path[800];
	snprintf(path, sizeof path, "%s/%s", place, BACKUP_NAME);
	if (access(path, F_OK)) return 0;
	*bytes = backup_read_file(path, n);
	return *bytes ? 1 : -1;
}
bool pick_saves_stat(uint64_t *size, int64_t *mtime) { (void)size; (void)mtime; return false; }
bool pick_saves_put(const uint8_t *bytes, size_t n) {
	char path[800];
	snprintf(path, sizeof path, "%s/%s", place, BACKUP_NAME);
	return backup_write_file(path, bytes, n);
}

static void remove_tree(const char *path) {
	DIR *dir = opendir(path);
	if (!dir) { remove(path); return; }
	for (struct dirent *e; (e = readdir(dir));) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
		char child[1400];
		snprintf(child, sizeof child, "%s/%s", path, e->d_name);
		remove_tree(child);
	}
	closedir(dir);
	cw_rmdir(path);
}

static bool blob(const char *path, uint32_t magic, const void *value, uint32_t size) {
	uint8_t *bytes = malloc(12 + size);
	if (!bytes) return false;
	backup_put32(bytes, magic);
	backup_put32(bytes + 4, size);
	backup_put32(bytes + 8, backup_fnv(value, size, 2166136261u));
	memcpy(bytes + 12, value, size);
	bool ok = backup_write_file(path, bytes, size + 12);
	free(bytes);
	return ok;
}

/* invented saves in `dir`: `runs` runs, best `best`, a run on layer `depth` (0: none) */
static bool progress(const char *dir, int runs, int best, int depth) {
	char path[700];
	snprintf(path, sizeof path, "%s/savedata", dir);
	remove_tree(path);
	cw_mkdir(path);
	Profile p = { .runs = runs, .best_depth = best, .music_volume = 2, .sfx_volume = 3 };
	snprintf(path, sizeof path, "%s/savedata/profile.sav", dir);
	bool ok = blob(path, PROFILE_MAGIC, &p, sizeof p);
	if (!depth) return ok;
	Run r = { .active = true, .seed = 7, .depth = depth };
	snprintf(path, sizeof path, "%s/savedata/run.sav", dir);
	ok = ok && blob(path, RUN_MAGIC, &r, sizeof r);
	snprintf(path, sizeof path, "%s/savedata/run.state", dir);
	static const uint8_t state[] = { 1, 7, 3, 9 };
	return ok && backup_write_file(path, state, sizeof state);
}

/* another device's saves as a .cwsave at `path` */
static bool file_of(const char *base, const char *path, int runs, int best, int depth) {
	char other[700];
	snprintf(other, sizeof other, "%s/other", base);
	remove_tree(other);
	cw_mkdir(other);
	size_t n = 0;
	uint8_t *b = progress(other, runs, best, depth) ? backup_pack(other, 1700000000u, &n) : NULL;
	bool ok = b && backup_write_file(path, b, n);
	free(b);
	remove_tree(other);
	return ok;
}

static void press(uint32_t button) {
	P.pressed = P.repeat = button;
	scene_saves.update();
	P.pressed = P.repeat = 0;
}

static bool shows(int action);

/* the cursor put on `action`, shown, then A */
static bool choose(int action) {
	if (!shows(action)) return false;
	SV.focus = action;
	press(BTN_A);
	return true;
}

static bool shows(int action) {
	int list[SAVES_ACTIONS], n = saves_actions(list);
	for (int i = 0; i < n; ++i) if (list[i] == action) return true;
	return false;
}

static void open_home(void) {
	current = &scene_title;
	CHECK(saves_open(), "saves: opens over the title");
	press(0);
}

static bool drawn(const char *text) {
	shown_count = 0;
	Saves before = SV;
	saves_draw();
	CHECK(!memcmp(&before, &SV, sizeof SV), "saves: drawing changes nothing");
	for (int i = 0; i < shown_count; ++i) if (strstr(shown[i], text)) return true;
	return false;
}

static int local_runs(void) {
	BackupInfo info;
	return backup_local_info(g_data_dir, &info) ? info.runs : -1;
}

static void test_home(void) {
	capabilities = 0;
	open_home();
	CHECK(current == &scene_saves && SV.view == SAVES_HOME && SV.focus == SAVES_EXPORT, "saves: home, the cursor on EXPORT");
	press(BTN_RIGHT);
	CHECK(SV.focus == SAVES_IMPORT, "saves: right of EXPORT is IMPORT");
	press(BTN_LEFT);
	CHECK(shows(SAVES_EXPORT) && shows(SAVES_IMPORT) && !shows(SAVES_UNDO) && !shows(SAVES_AUTO) && !shows(SAVES_FOLDER),
		"saves: the browser's home shows only what it can do");
	capabilities = SAVES_CAN_AUTO | SAVES_CAN_FOLDER;
	CHECK(shows(SAVES_AUTO) && shows(SAVES_FOLDER), "saves: the copy's row and FOLDER where the system has them");
	CHECK(drawn("Layer 4, Act 2") && drawn("3 runs, best Layer 9") && !drawn("0 runs"), "saves: the panel names the run and the record");
	press(BTN_B);
	CHECK(current == &scene_title && title_menu, "saves: B goes back to the title's menu");
	current = &scene_emu;
	CHECK(!saves_open() && current == &scene_emu, "saves: never opened in a run");
}

static void test_rows(void) {
	char rows[4][64];
	BackupInfo i = { .runs = 0, .best = 0, .run_depth = 2, .run_act = 1 };
	int n = saves_rows(&i, false, false, rows);
	bool zero = false;
	for (int k = 0; k < n; ++k) zero |= strstr(rows[k], "0 runs") || strstr(rows[k], "best 0");
	CHECK(n >= 1 && !strcmp(rows[0], "Layer 2, Act 1") && !zero, "saves: no '0 runs' beside a Layer 2 run");
	BackupInfo none = { 0 };
	CHECK(saves_rows(&none, false, false, rows) == 1 && !strcmp(rows[0], "No runs yet"), "saves: no runs yet");
	CHECK(saves_rows(&i, true, true, rows) == 1 && !strcmp(rows[0], "Damaged"), "saves: a damaged file's column says only so");
	BackupInfo one = { .runs = 1, .best = 3 };
	saves_rows(&one, false, false, rows);
	CHECK(!strcmp(rows[0], "1 run, best Layer 3"), "saves: one run, not '1 runs' (%s)", rows[0]);
}

static void test_import_undo(const char *base) {
	char file[700];
	snprintf(file, sizeof file, "%s/in.cwsave", base);
	CHECK(file_of(base, file, 12, 20, 2), "saves: a file with more progress");
	open_home();
	saves_import(file);
	press(0);
	CHECK(SV.view == SAVES_COMPARE && SV.focus == SAVES_TAKE && shows(SAVES_KEEP) && shows(SAVES_TAKE),
		"saves: compared, the cursor on the side with more progress (the file's)");
	CHECK(drawn("THIS DEVICE") && drawn("THE FILE") && drawn("USE THE FILE'S"), "saves: both sides and the answers shown");
	CHECK(strstr(SV.note, "replaces your Layer 4 run") != NULL, "saves: the note says what is replaced: %s", SV.note);
	press(BTN_A);
	CHECK(SV.view == SAVES_RESULT && local_runs() == 12 && strstr(SV.note, "UNDO IMPORT brings back your Layer 4 run") &&
		shows(SAVES_UNDO) && shows(SAVES_CONTINUE), "saves: imported, the result offers CONTINUE and UNDO IMPORT: %s", SV.note);
	char now[700], old[700];
	snprintf(now, sizeof now, "%s/savedata/profile.sav", g_data_dir);
	snprintf(old, sizeof old, "%s/savedata.old/profile.sav", g_data_dir);
	size_t on = 0;
	uint8_t *before = backup_read_file(old, &on);
	CHECK(choose(SAVES_UNDO) && SV.view == SAVES_RESULT && SV.undone && local_runs() == 3 && !strcmp(saves_label(SAVES_UNDO), "UNDO"),
		"saves: UNDO IMPORT at once, no preview; the result offers UNDO");
	size_t nn = 0;
	uint8_t *after = backup_read_file(now, &nn);
	CHECK(before && after && on == nn && !memcmp(before, after, on), "saves: Undo brings the saves back byte for byte");
	free(before);
	free(after);
	CHECK(choose(SAVES_UNDO) && local_runs() == 12 && !SV.undone, "saves: UNDO again puts the import back");
	press(BTN_B);
	CHECK(SV.view == SAVES_HOME && shows(SAVES_UNDO), "saves: home offers UNDO IMPORT after an import");
	CHECK(choose(SAVES_UNDO) && local_runs() == 3, "saves: UNDO IMPORT from home");
	press(BTN_B);

	/* a second import: the saves Undo held kept as a file */
	CHECK(file_of(base, file, 30, 30, 0), "saves: another file");
	saves_import(file);
	press(0);
	press(BTN_A);
	char replaced[700];
	snprintf(replaced, sizeof replaced, "%s/saves-replaced-1.cwsave", g_data_dir);
	CHECK(local_runs() == 30 && access(replaced, F_OK) == 0, "saves: a second import keeps the replaced Undo as saves-replaced-1.cwsave");
	press(BTN_B);
}

static void test_keep_and_refuse(const char *base) {
	char file[700];
	snprintf(file, sizeof file, "%s/less.cwsave", base);
	CHECK(file_of(base, file, 1, 2, 0), "saves: a file with less progress");
	int runs = local_runs();
	open_home();
	saves_import(file);
	press(0);
	CHECK(SV.view == SAVES_COMPARE && SV.focus == SAVES_KEEP, "saves: the cursor on this device's, more progress");
	press(BTN_A);
	CHECK(SV.view == SAVES_HOME && local_runs() == runs, "saves: KEEP THESE keeps this device's");
	uint8_t junk[64] = "CWSAVE1\n not a carrier";
	snprintf(file, sizeof file, "%s/junk.cwsave", base);
	CHECK(backup_write_file(file, junk, sizeof junk), "saves: a damaged file");
	saves_import(file);
	press(0);
	const char *kept = strstr(SV.note, "Your saves are kept.");
	CHECK(SV.view == SAVES_COMPARE && !shows(SAVES_TAKE) && shows(SAVES_DONE) && kept && !strstr(kept + 1, "Your saves are kept.") &&
		strstr(SV.note, "Not a saves file, or damaged."), "saves: refused with one reason, DONE only: %s", SV.note);
	CHECK(drawn("Damaged"), "saves: the damaged column says so");
	press(BTN_A);
	CHECK(SV.view == SAVES_HOME && local_runs() == runs, "saves: the refusal changed nothing");
}

static void test_place_file(const char *base) {
	automatic = true;
	char file[700];
	snprintf(file, sizeof file, "%s/%s", place, BACKUP_NAME);
	CHECK(file_of(base, file, 50, 50, 0), "saves: another device's file in the copy's place");
	mirror_rescan();
	current = &scene_title;
	saves_tick();
	press(0);
	CHECK(current == &scene_saves && SV.view == SAVES_COMPARE && SV.from_place && strstr(SV.note, "becomes this device's"),
		"saves: the title opens the place's file's comparison, KEEP THESE named plainly: %s", SV.note);
	choose(SAVES_KEEP);
	char receipts[700];
	snprintf(receipts, sizeof receipts, "%s/saves-receipts.txt", g_data_dir);
	size_t n = 0;
	uint8_t *r = backup_read_file(receipts, &n);
	CHECK(r && strstr((const char *)r, "folder ") != NULL, "saves: the answer kept in saves-receipts.txt");
	free(r);
	mirror_rescan();
	current = &scene_title;
	saves_tick();
	CHECK(current == &scene_title, "saves: an answered file does not open again");
	automatic = false;
}

static void test_export(void) {
	open_home();
	int before = exports;
	CHECK(choose(SAVES_EXPORT) && exports == before + 1, "saves: EXPORT hands the file to the picker");
	press(0);
	char out[700], staged[700];
	snprintf(out, sizeof out, "%s/%s", place, BACKUP_NAME);
	snprintf(staged, sizeof staged, "%s/saves-export.tmp", g_data_dir);
	CHECK(SV.view == SAVES_RESULT && strstr(SV.note, "Exported to") && access(staged, F_OK) != 0,
		"saves: the export's result, its staged file gone: %s", SV.note);
	/* (the export's bytes are this device's own where they turn up) */
	size_t n = 0;
	uint8_t *b = backup_read_file(out, &n);
	automatic = true;
	mirror_rescan();
	CHECK(b && !mirror_incoming(), "saves: a manual export writes the receipt: its file is no incoming saves");
	automatic = false;
	free(b);
	press(BTN_B);
	press(BTN_B);
}

static void test_busy(void) {
	open_home();
	CHECK(choose(SAVES_IMPORT) && SV.busy && picker_open, "saves: IMPORT opens the picker");
	CHECK(!strcmp(SV.said, saves_waiting(SAVES_IMPORT)), "saves: home's line says what the picker waits for: \"%s\"", SV.said);
	press(BTN_A);
	CHECK(SV.busy, "saves: nothing chosen while the picker is open");
	press(BTN_B);
	CHECK(cancelled && !picker_open, "saves: B lets go of a picker's wait (the browser's page)");
	memset(&ended, 0, sizeof ended);
	ended.status = -1;
	picker_ended = true;
	press(0);
	CHECK(!SV.busy && SV.view == SAVES_HOME, "saves: a cancelled picker leaves home as it was");
	CHECK(!strcmp(SV.said, saves_cancelled(SAVES_IMPORT)), "saves: ... and says so: \"%s\"", SV.said);
	press(BTN_B);
}

/* This device's saves there but unreadable: said so, never "no saves" */
static void test_unreadable(const char *base) {
	char file[700], dir[700], path[800];
	snprintf(file, sizeof file, "%s/in.cwsave", base);
	CHECK(file_of(base, file, 2, 3, 0), "saves: a file for the unreadable device");
	snprintf(g_data_dir, sizeof g_data_dir, "%.400s/broken", base);
	snprintf(dir, sizeof dir, "%s", g_data_dir);
	snprintf(path, sizeof path, "%s/savedata", dir);
	cw_mkdir(dir);
	cw_mkdir(path);
	snprintf(path, sizeof path, "%s/savedata/profile.sav", dir);
	CHECK(cw_mkdir(path) == 0, "saves: a profile that cannot be read");
	mirror_note();
	open_home();
	saves_import(file);
	press(0);
	CHECK(SV.unreadable && strstr(SV.note, "cannot be read") && !strcmp(saves_label(SAVES_TAKE), "USE THE FILE'S") && drawn("Damaged"),
		"saves: unreadable saves are said so, not called none: %s", SV.note);
	press(BTN_B);
	press(BTN_B);
}

int main(void) {
	char base[128];
	snprintf(base, sizeof base, "build/host/test-saves-%d", (int)getpid());
	remove_tree(base);
	cw_mkdir(base);
	snprintf(g_data_dir, sizeof g_data_dir, "%s/device", base);
	snprintf(place, sizeof place, "%s/place", base);
	cw_mkdir(g_data_dir);
	cw_mkdir(place);
	P.w = 240;
	P.h = 160;
	R.data = (uint8_t *)base;   /* (a ROM read: CONTINUE offered, no byte of it used) */
	bool made = progress(g_data_dir, 3, 9, 4);
	CHECK(made, "saves: invented local saves");
	if (made) {
		test_rows();
		test_home();
		test_import_undo(base);
		test_keep_and_refuse(base);
		test_place_file(base);
		test_export();
		test_busy();
		test_unreadable(base);
	}
	free(SV.bytes);
	remove_tree(base);
	if (failures) return 1;
	printf("saves: three views, hidden buttons, the comparison's answers, Undo at once, the place's file kept by an answer\n");
	return 0;
}
