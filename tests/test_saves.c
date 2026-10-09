/* The production SAVES scene with invented progress and runtime adapters.
 * Exercise decisions through update/taps, with the real portable-save and
 * automatic-copy modules; no ROM, file dialog, or graphics device is used. */
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "audio.h"
#include "backup_internal.h"
#include "compat.h"
#include "director.h"
#include "gfx.h"
#include "guardians.h"
#include "mirror.h"
#include "pick.h"
#include "pixfont.h"
#include "platform.h"
#include "rivals.h"
#include "rom.h"
#include "save.h"
#include "save_format.h"
#include "saves.h"
#include "saves_layout.h"
#include "saves_state.h"
#include "second_frame.h"
#include "touch.h"

Platform P;
Rom R;
XRom XR[XROM_COUNT];
char g_data_dir[512];
bool emu_resume_requested;
const Scene scene_title = { .name = "title" };
const Scene scene_intro = { .name = "intro" };
const Scene scene_emu = { .name = "emu" };
const Scene scene_launcher = { .name = "launcher" };

static const Scene *current = &scene_title;
static Uint32 ticks = 5000;
static bool tap, rom_picker, picker_result, automatic, fail_copy, load_ok = true, scope_available = true;
static int tap_x, tap_y, picker, manual_exports, imports, folder_picks, copies, suspends;
static unsigned capabilities = SAVES_CAN_AUTO | SAVES_CAN_FOLDER;
static PickResult result;
static char destination[700], discover[1100], dropped[1100];
static char shown[512][160];
static int shown_x[512], shown_y[512], shown_count;

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
bool platform_tap(int *x, int *y) {
	if (!tap) return false;
	*x = tap_x;
	*y = tap_y;
	tap = false;
	return true;
}
bool platform_dropped(char *path, size_t n) {
	if (!dropped[0]) return false;
	snprintf(path, n, "%s", dropped);
	dropped[0] = 0;
	return true;
}
void platform_safe_edges(int *top, int *left, int *bottom, int *right) { *top = *left = *bottom = *right = 0; }
void touch_release(void) { }
bool director_suspend(void) { ++suspends; return true; }
void save_init(void) { }
void rivals_load(void) { }
void audio_sfx(Sfx effect) { (void)effect; }
bool peek_run(Run *checkpoint) {
	char path[600];
	snprintf(path, sizeof path, "%s/savedata/run.sav", g_data_dir);
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	bool ok = b && n == sizeof *checkpoint + 12;
	if (ok) memcpy(checkpoint, b + 12, sizeof *checkpoint);
	free(b);
	return ok;
}
bool save_exists(void) { Run checkpoint; return peek_run(&checkpoint) && checkpoint.active; }
bool load_run(void) { return load_ok; }

const NetAreaDef *net_area_def(int area) {
	static const NetAreaDef def = { .name = "RoboDog Comp" };
	(void)area;
	return &def;
}
const char *guardian_area_original_name(int area) { (void)area; return "RoboDog Comp"; }

/* Use the real font measurements/wrapping, recording its draw arguments. */
void __wrap_pixfont_draw(int x, int y, const char *text, SDL_Color color, SDL_Color shade, int align, int scale);
void __wrap_pixfont_draw(int x, int y, const char *text, SDL_Color color, SDL_Color shade, int align, int scale) {
	(void)color; (void)shade; (void)align; (void)scale;
	assert(shown_count < (int)(sizeof shown / sizeof *shown));
	snprintf(shown[shown_count], sizeof shown[0], "%s", text);
	shown_x[shown_count] = x;
	shown_y[shown_count++] = y;
}
void fill_rect(int x, int y, int w, int h, SDL_Color color) { (void)x; (void)y; (void)w; (void)h; (void)color; }
void fill_rects(const SDL_Rect *rects, int n, SDL_Color color) { (void)rects; (void)n; (void)color; }
void second_stripes(int x, int y) { (void)x; (void)y; }

unsigned pick_kinds(void) { return PICK_FILE; }
bool pick_busy(void) { return rom_picker; }
unsigned pick_saves_capabilities(void) { return capabilities; }
bool pick_saves_auto_enabled(void) { return automatic; }
void pick_saves_auto(bool on) { automatic = on; platform_persist(); }
bool pick_saves_place(char *path, size_t n) { snprintf(path, n, "%s", destination); return true; }
bool pick_saves_scope(char *out, size_t n) {
	if (!scope_available) { *out = 0; return false; }
	snprintf(out, n, "provider:%s", destination);
	return true;
}
bool pick_saves_busy(void) { return picker != 0; }
bool pick_saves_import(void) { ++imports; picker = SAVES_IMPORT + 1; return true; }
bool pick_saves_export(const char *path) { (void)path; ++manual_exports; picker = SAVES_EXPORT + 1; return true; }
bool pick_saves_choose_folder(void) { ++folder_picks; picker = SAVES_FOLDER + 1; return true; }
bool pick_saves_done(PickResult *out) {
	if (!picker_result) return false;
	*out = result;
	picker_result = false;
	picker = 0;
	return true;
}
bool pick_saves_find(const char *to) { (void)to; return false; }
bool pick_saves_discover(const char *to, uint32_t ignored, uint32_t exported) {
	(void)ignored; (void)exported;
	if (!discover[0]) return false;
	size_t n = 0;
	uint8_t *bytes = backup_read_file(discover, &n);
	bool ok = bytes && backup_write_file(to, bytes, n);
	free(bytes);
	discover[0] = 0;
	return ok;
}
bool pick_saves_put(const char *from) {
	++copies;
	if (fail_copy) return false;
	char to[800];
	snprintf(to, sizeof to, "%s/%s", destination, BACKUP_NAME);
	size_t n = 0;
	uint8_t *bytes = backup_read_file(from, &n);
	bool ok = bytes && backup_write_file(to, bytes, n);
	free(bytes);
	return ok;
}

static void remove_tree(const char *path) {
	DIR *dir = opendir(path);
	if (!dir) { remove(path); return; }
	for (struct dirent *entry; (entry = readdir(dir));) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
		char child[1400];
		snprintf(child, sizeof child, "%s/%s", path, entry->d_name);
		remove_tree(child);
	}
	closedir(dir);
	cw_rmdir(path);
}

static void blob(const char *path, uint32_t magic, const void *value, uint32_t size) {
	uint8_t *bytes = malloc(12 + size);
	assert(bytes);
	backup_put32(bytes, magic);
	backup_put32(bytes + 4, size);
	backup_put32(bytes + 8, backup_fnv(value, size, 2166136261u));
	memcpy(bytes + 12, value, size);
	assert(backup_write_file(path, bytes, size + 12));
	free(bytes);
}

static void progress(const char *dir, int runs, int depth) {
	char path[700];
	snprintf(path, sizeof path, "%s/savedata", dir);
	assert(!cw_mkdir(path));
	Profile permanent = { .runs = runs, .best_depth = 20, .music_volume = 2, .sfx_volume = 3 };
	snprintf(path, sizeof path, "%s/savedata/profile.sav", dir);
	blob(path, PROFILE_MAGIC, &permanent, sizeof permanent);
	Run checkpoint = { .active = true, .seed = 7, .depth = depth };
	snprintf(path, sizeof path, "%s/savedata/run.sav", dir);
	blob(path, RUN_MAGIC, &checkpoint, sizeof checkpoint);
	snprintf(path, sizeof path, "%s/savedata/run.state", dir);
	static const uint8_t state_bytes[] = { 1, 7, 3, 9 };
	assert(backup_write_file(path, state_bytes, sizeof state_bytes));
}

static uint32_t local_hash(void) {
	BackupInfo info;
	assert(backup_local_info(g_data_dir, &info));
	return info.hash;
}

static void draw(void) {
	shown_count = 0;
	Saves before = SV;
	saves_draw();
	assert(!memcmp(&before, &SV, sizeof SV));
}

static void press(uint32_t button) {
	P.pressed = P.repeat = button;
	scene_saves.update();
	P.pressed = P.repeat = 0;
}

static void tap_at(int x, int y) {
	tap_x = x; tap_y = y; tap = true;
	scene_saves.update();
	assert(!tap);
}

static void tap_button(const char *label) {
	draw();
	SavesLayout l = saves_layout(P.w, P.h, SV.view, SV.resume, SV.result_undo);
	for (int i = 0; i < shown_count; ++i) {
		if (strncmp(shown[i], label, strlen(label))) continue;
		SDL_Point point = { shown_x[i] + 2, shown_y[i] + 2 };
		if (SDL_PointInRect(&point, &l.back)) { tap_at(point.x, point.y); return; }
		for (int b = 0; b < l.count; ++b) {
			if (!SDL_PointInRect(&point, &l.buttons[b].rect)) continue;
			tap_at(point.x, point.y);
			return;
		}
	}
	fprintf(stderr, "Missing visible button: %s\n", label);
	abort();
}

static void completed_press(int status, const char *text, uint32_t button) {
	memset(&result, 0, sizeof result);
	result.status = status;
	result.path = picker == SAVES_IMPORT + 1;
	snprintf(result.text, sizeof result.text, "%s", text);
	picker_result = true;
	press(button);
}

static void completed(int status, const char *text) { completed_press(status, text, 0); }

static void import_ready(const char *path) {
	saves_import(path);
	press(0);
}

static void home(void) {
	free(SV.bytes);
	memset(&SV, 0, sizeof SV);
	current = &scene_title;
	saves_open();
	assert(current == &scene_saves && SV.view == SAVES_HOME);
}

static void test_navigation(void) {
	const int dimensions[][2] = { { 240, 160 }, { 400, 240 } };
	for (size_t i = 0; i < sizeof dimensions / sizeof *dimensions; ++i) {
		P.w = dimensions[i][0]; P.h = dimensions[i][1];
		home();
		assert(SV.focus == SAVES_EXPORT);
		press(BTN_RIGHT); assert(SV.focus == SAVES_IMPORT);
		press(BTN_DOWN); assert(SV.focus == SAVES_DETAILS);
		press(BTN_LEFT); assert(SV.focus == SAVES_UNDO);
		press(BTN_DOWN); assert(SV.focus == SAVES_OPTIONS);
		press(BTN_UP); assert(SV.focus == SAVES_UNDO);
		press(BTN_RIGHT); assert(SV.focus == SAVES_DETAILS);
		press(BTN_UP); assert(SV.focus == SAVES_IMPORT);
		press(BTN_RIGHT); assert(SV.focus == SAVES_IMPORT);
	}
	P.w = 160; P.h = 240;
	home();
	press(BTN_RIGHT); assert(SV.focus == SAVES_EXPORT);
	press(BTN_DOWN); assert(SV.focus == SAVES_IMPORT);
	press(BTN_DOWN); assert(SV.focus == SAVES_UNDO);
	press(BTN_DOWN); assert(SV.focus == SAVES_DETAILS);
	press(BTN_DOWN); assert(SV.focus == SAVES_OPTIONS);
	press(BTN_UP); assert(SV.focus == SAVES_DETAILS);
	P.w = 240; P.h = 160;
}

static void test_disabled(void) {
	home();
	uint32_t hash = local_hash();
	assert(!SV.undo && saves_disabled(SAVES_UNDO));
	tap_button("Undo");
	assert(current == &scene_saves && SV.view == SAVES_HOME && local_hash() == hash);
	capabilities = 0;
	tap_button("Options");
	assert(SV.view == SAVES_TRANSFER && saves_disabled(SAVES_AUTO) && saves_disabled(SAVES_FOLDER));
	int folders = folder_picks;
	tap_button("Auto-export");
	assert(!automatic && SV.view == SAVES_TRANSFER);
	tap_button("Choose folder");
	assert(folder_picks == folders && !picker && SV.view == SAVES_TRANSFER && local_hash() == hash);
	press(BTN_B);
	assert(SV.view == SAVES_HOME);
	capabilities = SAVES_CAN_AUTO | SAVES_CAN_FOLDER;
}

static void test_export_result(void) {
	home();
	int count = manual_exports;
	tap_button("Export");
	assert(manual_exports == count + 1 && SV.busy && picker);
	press(BTN_B);
	assert(current == &scene_saves && SV.busy);
	completed_press(1, "/Downloads/cyberworld-endless.cwsave", BTN_START);
	assert(!SV.busy && SV.view == SAVES_RESULT && !SV.result_resume && SV.focus == SAVES_DONE);
	char message[sizeof SV.note];
	snprintf(message, sizeof message, "%s", SV.note);
	tap_at(P.w / 2, 22);
	assert(current == &scene_saves && SV.view == SAVES_RESULT && !strcmp(message, SV.note));
	tap_button("Back");
	assert(current == &scene_title);
	home();
	tap_button("Export");
	completed_press(-2, "This destination is read-only.", BTN_A);
	assert(SV.view == SAVES_RESULT && strstr(SV.note, "read-only"));
	press(BTN_B);
	assert(current == &scene_title);
}

static void test_picker_cancel(void) {
	home();
	uint32_t hash = local_hash();
	tap_button("Import");
	assert(SV.busy);
	completed_press(-1, "", BTN_START);
	assert(!SV.busy && SV.view == SAVES_HOME && !saves_comparing() && local_hash() == hash);
	/* Cancellation does not show a result needing another dismissal. */
	press(BTN_B);
	assert(current == &scene_title);
}

static void test_import(const char *incoming) {
	home();
	uint32_t hash = local_hash();
	int count = imports;
	tap_button("Import");
	assert(imports == count + 1 && SV.busy);
	completed(1, incoming);
	assert(SV.view == SAVES_COMPARE && SV.focus == SAVES_KEEP && !SV.undo_preview && SV.status == BACKUP_OK);
	assert(SV.here.runs == 10 && SV.here.run_depth == 5 && SV.file.runs == 7 && SV.file.run_depth == 2);
	assert(local_hash() == hash);
	press(BTN_B);
	assert(SV.view == SAVES_HOME && local_hash() == hash);
	import_ready(incoming);
	assert(SV.view == SAVES_COMPARE && SV.focus == SAVES_KEEP);
	/* Blank space cannot select Import; the deliberate button can. */
	tap_at(P.w / 2, 22);
	assert(SV.view == SAVES_COMPARE && local_hash() == hash);
	press(BTN_RIGHT);
	assert(SV.focus == SAVES_TAKE);
	press(BTN_A);
	assert(SV.view == SAVES_RESULT && SV.result_undo && SV.here.runs == 7 && SV.here.run_depth == 2);
	BackupInfo previous;
	assert(backup_undo_info(g_data_dir, &previous) && previous.hash == hash && previous.runs == 10 && previous.run_depth == 5);
	assert(local_hash() == SV.here.hash && local_hash() != hash);
}

static void test_undo(void) {
	uint32_t imported = local_hash();
	assert(SV.view == SAVES_RESULT && SV.result_undo);
	tap_button("Undo");
	assert(SV.view == SAVES_COMPARE && SV.undo_preview && SV.focus == SAVES_KEEP);
	assert(SV.here.runs == 7 && SV.here.run_depth == 2 && SV.file.runs == 10 && SV.file.run_depth == 5);
	assert(local_hash() == imported);
	/* If the retained copy changes after preview, Restore must not silently
	 * bring back different progress from the one the player inspected. */
	char old_path[700];
	snprintf(old_path, sizeof old_path, "%s/savedata.old/profile.sav", g_data_dir);
	size_t old_n = 0;
	uint8_t *old_bytes = backup_read_file(old_path, &old_n);
	assert(old_bytes && old_n > 16);
	uint8_t *changed = malloc(old_n);
	assert(changed);
	memcpy(changed, old_bytes, old_n);
	backup_put32(changed + 12, 43);
	backup_put32(changed + 8, backup_fnv(changed + 12, old_n - 12, 2166136261u));
	assert(backup_write_file(old_path, changed, old_n));
	tap_button("Restore");
	assert(SV.view == SAVES_COMPARE && SV.status == BACKUP_IO && local_hash() == imported);
	assert(backup_write_file(old_path, old_bytes, old_n));
	free(old_bytes);
	free(changed);
	press(BTN_B);
	assert(local_hash() == imported && !saves_comparing());
	home();
	tap_button("Undo");
	assert(SV.undo_preview && SV.file.runs == 10 && local_hash() == imported);
	tap_button("Restore");
	assert(SV.view == SAVES_RESULT && SV.here.runs == 10 && SV.here.run_depth == 5 && local_hash() != imported);
	BackupInfo previous;
	assert(backup_undo_info(g_data_dir, &previous) && previous.hash == imported && previous.runs == 7);
	/* Continue is an explicit action after a successful restored run. */
	assert(SV.resume && !emu_resume_requested);
	tap_button("Continue");
	assert(current == &scene_emu && emu_resume_requested);
	emu_resume_requested = false;
}

static void test_continue_failure(const char *incoming) {
	home();
	tap_button("Import");
	completed(1, incoming);
	tap_button("Use file");
	assert(SV.view == SAVES_RESULT && SV.result_resume && SV.resume);
	uint32_t hash = local_hash();
	load_ok = false;
	tap_button("Continue");
	assert(current == &scene_saves && SV.view == SAVES_RESULT && !emu_resume_requested && strstr(SV.note, "could not be loaded"));
	assert(local_hash() == hash);
	load_ok = true;
	press(BTN_B);
	assert(current == &scene_title);
	home();
	tap_button("Undo");
	tap_button("Restore");
	assert(SV.view == SAVES_RESULT && SV.here.runs == 10 && SV.result_resume);
	press(BTN_B);
}

static void test_refused(const char *base) {
	char damaged[700];
	snprintf(damaged, sizeof damaged, "%s/damaged.cwsave", base);
	static const uint8_t bad[] = { 0, 1, 2, 3 };
	assert(backup_write_file(damaged, bad, sizeof bad));
	home();
	uint32_t hash = local_hash();
	import_ready(damaged);
	assert(SV.view == SAVES_COMPARE && SV.status == BACKUP_DAMAGED && saves_disabled(SAVES_TAKE));
	SavesLayout layout = saves_layout(P.w, P.h, SV.view, SV.resume, SV.result_undo);
	for (int i = 0; i < layout.count; ++i) {
		if (layout.buttons[i].action != SAVES_TAKE) continue;
		SDL_Rect r = layout.buttons[i].rect;
		tap_at(r.x + r.w / 2, r.y + r.h / 2);
	}
	assert(SV.view == SAVES_COMPARE && local_hash() == hash);
	saves_answer(true);
	assert(SV.view == SAVES_COMPARE && local_hash() == hash);
	press(BTN_B);
	assert(!saves_comparing() && local_hash() == hash);
}

static void collect_text(char *out, size_t n) {
	draw();
	size_t at = strlen(out);
	SDL_Rect content = saves_layout(P.w, P.h, SV.view, SV.resume, SV.result_undo).content;
	for (int i = 0; i < shown_count; ++i) {
		if (shown_x[i] != content.x || shown_y[i] < content.y || shown_y[i] >= content.y + content.h) continue;
		for (const char *p = shown[i]; *p && at + 1 < n; ++p)
			if (*p != ' ' && *p != '\n') out[at++] = *p;
	}
	out[at] = 0;
}

static void read_details(char *text, size_t n) {
	*text = 0;
	for (int i = 0; i < 40; ++i) {
		collect_text(text, n);
		if (saves_disabled(SAVES_NEXT)) return;
		int previous = SV.details_page;
		saves_activate(SAVES_NEXT);
		assert(SV.details_page > previous);
	}
	abort();
}

static void test_details(const char *incoming) {
	home();
	import_ready(incoming);
	assert(SV.view == SAVES_COMPARE);
	snprintf(SV.file.version, sizeof SV.file.version, "0.0.1+git12345.aabbccddeeff00112233445566778899.long-build-test");
	snprintf(SV.file.device, sizeof SV.file.device, "Retroid-Pocket-Flip-2-with-a-long-device-name-12345678901234567");
	char version[sizeof SV.file.version], device[sizeof SV.file.device];
	snprintf(version, sizeof version, "%s", SV.file.version);
	snprintf(device, sizeof device, "%s", SV.file.device);
	memset(SV.path, 'i', sizeof SV.path - 1);
	SV.path[sizeof SV.path - 1] = 0;
	memcpy(SV.path + sizeof SV.path - 15, "/TAIL-LAST.txt", 14);
	uint32_t hash = local_hash();
	SV.focus = SAVES_TAKE;
	saves_activate(SAVES_FILE_DETAILS);
	assert(SV.view == SAVES_DETAIL && SV.details_file && SV.details_back == SAVES_COMPARE);
	char all[20000] = { 0 };
	read_details(all, sizeof all);
	assert(strstr(all, version) && strstr(all, device) && strstr(all, SV.path));
	assert(local_hash() == hash);
	press(BTN_B);
	assert(SV.view == SAVES_COMPARE && SV.focus == SAVES_TAKE && !SV.undo_preview && local_hash() == hash);
	saves_activate(SAVES_HERE_DETAILS);
	assert(SV.view == SAVES_DETAIL && !SV.details_file);
	memset(SV.place, 'z', sizeof SV.place - 1);
	SV.place[sizeof SV.place - 1] = 0;
	read_details(all, sizeof all);
	assert(strstr(all, SV.place));
	tap_button("Back");
	assert(SV.view == SAVES_COMPARE && SV.focus == SAVES_TAKE && local_hash() == hash);
	press(BTN_B);
}

static void manifest_found(const char *name, const uint8_t *data, uint32_t size, void *user) {
	(void)size;
	if (!strcmp(name, BACKUP_MANIFEST)) *(const uint8_t **)user = data;
}

static void future_version(const char *incoming, const char *path, const char *version) {
	size_t n = 0;
	uint8_t *bytes = backup_read_file(incoming, &n);
	const uint8_t *manifest = NULL;
	assert(bytes && backup_walk(bytes, n, manifest_found, &manifest) && manifest);
	size_t offset = (size_t)(manifest - bytes), start = offset + strlen("format=2\nversion="), end = start;
	while (end < n && bytes[end] != '\n') ++end;
	assert(end < n && strlen(version) >= end - start);
	size_t extra = strlen(version) - (end - start), changed_n = n + extra;
	uint8_t *changed = malloc(changed_n);
	assert(changed);
	memcpy(changed, bytes, start);
	memcpy(changed + start, version, strlen(version));
	memcpy(changed + start + strlen(version), bytes + end, n - end);
	/* A dev build has no release version to order, so the future carrier
	 * format makes the refusal deterministic for local and release tests. */
	changed[offset + 7] = '3';
	backup_put32(changed + offset - 4, backup_get32(bytes + offset - 4) + (uint32_t)extra);
	backup_put32(changed + changed_n - 4, backup_fnv(changed, changed_n - 4, 2166136261u));
	BackupInfo info;
	assert(backup_info(changed, changed_n, &info) && info.status == BACKUP_NEWER_FORMAT);
	assert(backup_write_file(path, changed, changed_n));
	free(changed);
	free(bytes);
}

static void test_complete_messages(const char *incoming, const char *base) {
	char path[1024], future[700], all[20000];
	memset(path, 'i', sizeof path - 1);
	path[sizeof path - 1] = 0;
	memcpy(path + sizeof path - 15, "/EXPORT-TAIL.x", 14);
	static const char version[] = "9.9.9+git12345.aabbccddeeff00112233445566778899.long-build-test";
	snprintf(future, sizeof future, "%s/future.cwsave", base);
	future_version(incoming, future, version);
	const int dimensions[][2] = { { 240, 160 }, { 160, 240 } };
	for (size_t i = 0; i < sizeof dimensions / sizeof *dimensions; ++i) {
		P.w = dimensions[i][0]; P.h = dimensions[i][1];
		home();
		uint32_t hash = local_hash();
		tap_button("Export");
		completed(1, path);
		assert(SV.view == SAVES_RESULT && SV.resume && strstr(SV.note, path));
		tap_button("Details");
		assert(SV.view == SAVES_DETAIL && SV.details_back == SAVES_RESULT);
		int focus = SV.details_focus;
		read_details(all, sizeof all);
		assert(strstr(all, path) && local_hash() == hash);
		press(BTN_B);
		assert(SV.view == SAVES_RESULT && SV.focus == focus);
		press(BTN_B);
		assert(current == &scene_title);
		import_ready(future);
		assert(SV.status == BACKUP_NEWER_FORMAT && saves_disabled(SAVES_TAKE));
		tap_button("File details");
		read_details(all, sizeof all);
		assert(strstr(all, version) && strstr(all, "updatethisdevicefirst.Yoursavesarekept.") && local_hash() == hash);
		press(BTN_B);
		assert(SV.view == SAVES_COMPARE && saves_disabled(SAVES_TAKE));
		press(BTN_B);
	}
	P.w = 240; P.h = 160;
	R.data = NULL;
	home();
	tap_button("Export");
	completed(1, path);
	assert(SV.view == SAVES_RESULT && !SV.resume && SV.focus == SAVES_DONE);
	tap_button("Done");
	assert(current == &scene_saves && SV.view == SAVES_HOME && !emu_resume_requested);
	R.data = (uint8_t *)&P;
}

static void test_input_guards(const char *incoming) {
	const uint32_t buttons[] = { BTN_A, BTN_START, 0 };
	for (size_t i = 0; i < sizeof buttons / sizeof *buttons; ++i) {
		home();
		uint32_t hash = local_hash();
		tap_button("Import");
		if (!buttons[i]) {
			SavesLayout l = saves_layout(P.w, P.h, SAVES_COMPARE, false, false);
			tap_x = l.buttons[0].rect.x + 2;
			tap_y = l.buttons[0].rect.y + 2;
			tap = true;
		}
		completed_press(1, incoming, buttons[i]);
		assert(SV.view == SAVES_COMPARE && SV.focus == SAVES_KEEP && SV.bytes && !tap && local_hash() == hash);
		/* A deliberate press in the following frame still works. */
		press(BTN_A);
		assert(SV.view == SAVES_HOME && !SV.bytes && local_hash() == hash);
	}
	home();
	uint32_t hash = local_hash();
	snprintf(dropped, sizeof dropped, "%s", incoming);
	saves_tick();
	press(BTN_START);
	assert(SV.view == SAVES_COMPARE && SV.bytes && local_hash() == hash);
	press(BTN_A);
	assert(SV.view == SAVES_HOME && local_hash() == hash);
}

static void test_options_copy(const char *incoming) {
	home();
	tap_button("Options");
	assert(SV.view == SAVES_TRANSFER && !automatic);
	tap_button("Auto-export");
	assert(automatic && SV.view == SAVES_TRANSFER && !SV.note[0]);
	fail_copy = true;
	ticks += 4000;
	mirror_tick();
	MirrorStatus status;
	mirror_status(&status);
	assert(status.state == MIRROR_FAILED);
	/* A vanished provider can retain its display name while its stable
	 * identity is inaccessible. A failed Retry must still be visible. */
	scope_available = false;
	tap_button("Retry");
	ticks += 4000;
	mirror_tick();
	mirror_status(&status);
	assert(status.state == MIRROR_FAILED && !status.copied_at);
	scope_available = true;
	int count = copies;
	fail_copy = false;
	tap_button("Retry");
	assert(SV.view == SAVES_TRANSFER);
	ticks += 4000;
	mirror_tick();
	mirror_status(&status);
	assert(copies == count + 1 && status.state == MIRROR_COPIED && status.copied_at && !strcmp(status.destination, destination));
	/* A retry still discovers and holds a different incoming file first. */
	snprintf(discover, sizeof discover, "%s", incoming);
	count = copies;
	tap_button("Retry");
	ticks += 4000;
	mirror_tick();
	mirror_status(&status);
	assert(copies == count && status.state == MIRROR_INCOMING);
	saves_tick();
	press(BTN_START);
	assert(SV.view == SAVES_COMPARE && SV.focus == SAVES_KEEP && SV.here.runs == 10 && SV.file.runs == 7);
	press(BTN_B);
	assert(SV.view == SAVES_HOME);
	tap_button("Options");
	tap_button("Auto-export");
	assert(!automatic && SV.view == SAVES_TRANSFER);
	press(BTN_B);
}

int main(void) {
	char base[] = "build/host/test-saves-XXXXXX", remote[600], incoming[700];
	assert(mkdtemp(base));
	snprintf(g_data_dir, sizeof g_data_dir, "%s/local", base);
	snprintf(remote, sizeof remote, "%s/remote", base);
	snprintf(destination, sizeof destination, "%s/exports", base);
	assert(!cw_mkdir(g_data_dir) && !cw_mkdir(remote) && !cw_mkdir(destination));
	progress(g_data_dir, 10, 5);
	progress(remote, 7, 2);
	size_t n = 0;
	uint8_t *bytes = backup_pack(remote, 1700000000, &n);
	snprintf(incoming, sizeof incoming, "%s/incoming.cwsave", base);
	assert(bytes && backup_write_file(incoming, bytes, n));
	free(bytes);
	P.w = 240; P.h = 160;
	R.data = (uint8_t *)&P;
	test_navigation();
	test_disabled();
	test_export_result();
	test_picker_cancel();
	test_import(incoming);
	test_undo();
	test_continue_failure(incoming);
	test_refused(base);
	test_details(incoming);
	P.w = 160; P.h = 240;
	test_details(incoming);
	P.w = 240; P.h = 160;
	test_complete_messages(incoming, base);
	test_input_guards(incoming);
	test_options_copy(incoming);
	current = &scene_emu;
	saves_open();
	assert(suspends == 1 && current == &scene_saves && SV.back == &scene_title);
	free(SV.bytes);
	SV.bytes = NULL;
	remove_tree(base);
	puts("saves: production decisions, pointer controls, details and copy retry passed");
	return 0;
}
