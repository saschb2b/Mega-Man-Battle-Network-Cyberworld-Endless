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
	(void)from;
	return false;
}
#else
typedef int pick_desktop_unused;
#endif
