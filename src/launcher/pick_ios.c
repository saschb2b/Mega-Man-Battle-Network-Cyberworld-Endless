/* pick.h on the iPhone and iPad: Files' pickers over the game (ios.m), the
 * folder the ROMs are in (kept as a bookmark, looked in again at each
 * start) or files, several at once; BN6's and BN5's ROMs, checked by
 * their SHA-1, copied into the app's rom/ folder; a .cwsave in the folder
 * copied to the app's found.cwsave, which the launcher offers back, and
 * the saves' copy written into the folder (mirror.h). */
#include "pick.h"

#ifdef CW_IOS
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "backup.h"
#include "ios.h"
#include "ios_saves.h"
#include "pick_saves.h"
#include "platform.h"
#include "rom.h"

static bool busy;

static void rom_dir(char *out, size_t n) { snprintf(out, n, "%s/rom", g_data_dir); }

unsigned pick_kinds(void) {
	backup_set_device(ios_saves_device());
	return PICK_FOLDER | PICK_FILES;
}

bool pick_open(int kind, int slot) {
	(void)slot;
	if (busy) return false;
	char dir[600];
	rom_dir(dir, sizeof dir);
	ios_pick_roms(P.window, dir, kind == PICK_FILES ? IOS_PICK_FILES : IOS_PICK_FOLDER);
	busy = true;
	return true;
}

bool pick_busy(void) { return busy; }

bool pick_done(PickResult *r) {
	char msg[1024];
	int k = busy ? ios_pick_result(msg, sizeof msg) : 0;
	if (!k) return false;
	busy = false;
	memset(r, 0, sizeof *r);
	r->status = k;
	r->saves = k > 0 && ios_pick_saves();
	snprintf(r->text, sizeof r->text, "%s", k > 0 ? msg : "");
	return true;
}

int pick_look(char *msg, size_t n) {
	unsigned want = (R.data ? 0 : IOS_ROM_BN6) | (XR[XROM_BN5_COLONEL_US].data ? 0 : IOS_ROM_BN5);
	if (!want) return 0;
	char dir[600];
	rom_dir(dir, sizeof dir);
	return ios_rom_folder_look(dir, want, msg, n);
}

bool pick_folder(char *name, size_t n) { return ios_rom_folder_name(name, n); }

bool pick_saves_put(const char *from) { return ios_saves_put(from); }

bool pick_saves_get(const char *to) { return ios_saves_get(to); }
bool saves_phone_place(char *out, size_t n) { return ios_saves_folder_name(out, n); }
bool saves_phone_scope(char *out, size_t n) { return ios_saves_folder_scope(out, n); }

static bool saves_busy;
static int saves_kind;

static bool saves_dialog_open(int kind, const char *from) {
	if (busy || saves_busy || !ios_saves_pick(P.window, g_data_dir, kind, from)) return false;
	saves_kind = kind;
	saves_busy = true;
	return true;
}

bool pick_saves_import(void) { return saves_dialog_open(0, NULL); }
bool pick_saves_export(const char *from) { return saves_dialog_open(1, from); }
bool pick_saves_choose_folder(void) { return saves_dialog_open(2, NULL); }
bool pick_saves_busy(void) { return saves_busy; }

bool pick_saves_done(PickResult *r) {
	char msg[1024];
	int status = saves_busy ? ios_saves_result(msg, sizeof msg) : 0;
	if (!status) return false;
	saves_busy = false;
	memset(r, 0, sizeof *r);
	r->status = status;
	r->path = saves_kind == 0;
	snprintf(r->text, sizeof r->text, "%s", msg);
	return true;
}
#else
typedef int pick_ios_unused;
#endif
