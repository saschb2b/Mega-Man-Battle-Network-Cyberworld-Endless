/* pick.h on the desktops (Linux, macOS, Windows): the desktop's own file
 * dialog (desktop.c's zenity, kdialog or macOS's open panel, desktop_win.c's
 * Windows dialog) for one ROM, on a thread of its own, so the game's window
 * goes on drawing and answering while it is open (a window that does not
 * is marked "not responding"). No folder: BN5 beside a BN6 chosen comes
 * with it (rom.c's xrom_find_beside). Steam's big screen, where a pad
 * cannot answer a dialog, has none: the ROM is looked for where it can be
 * put (Downloads, EmuDeck's and RetroDECK's folders). */
#include "pick.h"

#ifdef CW_DESKTOP
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "desktop.h"
#include "backup.h"
#include "game.h"
#include "pick_saves.h"

static SDL_Thread *thread;
static SDL_atomic_t state;   /* 0 open, 1 a file chosen, -1 none */
static char chosen[1024];
static int chosen_slot;

static int choose(void *user) {
	(void)user;
	bool ok = desktop_choose_rom(chosen_slot, chosen, sizeof chosen);
	SDL_AtomicSet(&state, ok && chosen[0] ? 1 : -1);
	return 0;
}

unsigned pick_kinds(void) {
	static int can = -1;
	if (can < 0) can = !desktop_big_screen() && desktop_can_choose();
	return can ? PICK_FILE : 0;
}

bool pick_open(int kind, int slot) {
	if (thread || !(kind & (int)pick_kinds())) return false;
	chosen_slot = slot;
	chosen[0] = 0;
	SDL_AtomicSet(&state, 0);
	thread = SDL_CreateThread(choose, "rom-chooser", NULL);
	return thread != NULL;
}

bool pick_busy(void) { return thread != NULL; }

bool pick_done(PickResult *r) {
	if (!thread || !SDL_AtomicGet(&state)) return false;
	SDL_WaitThread(thread, NULL);
	thread = NULL;
	memset(r, 0, sizeof *r);
	r->status = SDL_AtomicGet(&state);
	r->path = true;
	snprintf(r->text, sizeof r->text, "%s", chosen);
	return true;
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
	char folder[1024], to[1100];
	if (!pick_saves_place(folder, sizeof folder)) return false;
	snprintf(to, sizeof to, "%s/%s", folder, BACKUP_NAME);
	return saves_copy(from, to);
}

bool pick_saves_get(const char *to) {
	char folder[1024], from[1100];
	if (!pick_saves_place(folder, sizeof folder)) return false;
	snprintf(from, sizeof from, "%s/%s", folder, BACKUP_NAME);
	return saves_stage(from, to);
}

static SDL_Thread *save_thread;
static SDL_atomic_t save_state;
static int save_kind;
static char save_from[1024], save_chosen[1024];

static void keep_parent(const char *path) {
	char folder[1024];
	snprintf(folder, sizeof folder, "%s", path);
	char *slash = strrchr(folder, '/');
	if (slash && slash != folder) { *slash = 0; saves_folder_keep(folder); }
}

static int choose_saves(void *user) {
	(void)user;
	char folder[1024];
	bool have = pick_saves_place(folder, sizeof folder);
	bool dialog = !desktop_big_screen() && desktop_can_choose();
	int status = -1;
	if (dialog) {
		if (desktop_choose_saves(save_kind, have ? folder : g_data_dir, save_chosen, sizeof save_chosen)) status = 1;
	} else if (save_kind == 0) {
		snprintf(save_chosen, sizeof save_chosen, "%s/saves-found.cwsave", g_data_dir);
		status = pick_saves_find(save_chosen) ? 1 : -2;
	} else if (save_kind == 1 && have) {
		snprintf(save_chosen, sizeof save_chosen, "%.*s/%s", (int)sizeof save_chosen - 30, folder, BACKUP_NAME);
		status = 1;
	}
	if (status == 1 && save_kind == 1) {
		status = saves_copy(save_from, save_chosen) ? 1 : -2;
		if (status == 1 && dialog) desktop_saves_reveal(save_chosen);
	}
	if (status == -2) snprintf(save_chosen, sizeof save_chosen, "%s", save_kind == 0 ? "No saves file found in Downloads or the data folder." : "The saves file could not be exported.");
	SDL_AtomicSet(&save_state, status);
	return 0;
}

static bool saves_dialog_open(int kind, const char *from) {
	if (thread || save_thread || (kind == 2 && (desktop_big_screen() || !desktop_can_choose()))) return false;
	save_kind = kind;
	snprintf(save_from, sizeof save_from, "%s", from ? from : "");
	save_chosen[0] = 0;
	SDL_AtomicSet(&save_state, 0);
	save_thread = SDL_CreateThread(choose_saves, "saves-chooser", NULL);
	return save_thread != NULL;
}

bool pick_saves_import(void) { return saves_dialog_open(0, NULL); }
bool pick_saves_export(const char *from) { return saves_dialog_open(1, from); }
bool pick_saves_choose_folder(void) { return saves_dialog_open(2, NULL); }
bool pick_saves_busy(void) { return save_thread != NULL; }

bool pick_saves_done(PickResult *r) {
	if (!save_thread || !SDL_AtomicGet(&save_state)) return false;
	SDL_WaitThread(save_thread, NULL);
	save_thread = NULL;
	memset(r, 0, sizeof *r);
	r->status = SDL_AtomicGet(&save_state);
	r->path = save_kind == 0;
	snprintf(r->text, sizeof r->text, "%s", save_chosen);
	/* Persist on the game's thread: platform_persist also notes the next
	 * auto-export, whose state the worker must not change. */
	if (r->status == 1 && save_kind == 1) keep_parent(save_chosen);
	else if (r->status == 1 && save_kind == 2 && !saves_folder_keep(save_chosen)) {
		r->status = -2;
		snprintf(r->text, sizeof r->text, "The transfer folder could not be remembered.");
	}
	return true;
}
#else
typedef int pick_desktop_unused;
#endif
