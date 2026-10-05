/* The screens before a game: the error a ROM that cannot be used shows,
 * and where a desktop or an iPhone has no ROM yet, the screen that says
 * where to put it and looks again. */

#include "scene_norom.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(CW_DESKTOP) && !defined(_WIN32)
#include <unistd.h>
#endif

#include "audio.h"
#include "game.h"
#include "gfx.h"
#include "minifont.h"
#include "platform.h"
#include "rom.h"
#include "save.h"
#include "startup.h"
#ifdef CW_IOS
#include "ios.h"
#endif

/* ---- error scene: shown when no usable ROM is present ---- */
static char error_msg[512];

void error_show(const char *msg) {
	snprintf(error_msg, sizeof error_msg, "%s", msg);
	scene_set(&scene_error);
}

/* Lines of `text` at most `cols` (under 64) characters wide, broken at
 * newlines, spaces, and after a slash, dot or hyphen (a long path breaks at
 * its folders, a Flatpak's at its app id's words); returns how many. */
static int wrap_lines(const char *text, int cols, char out[][64], int max) {
	int n = 0;
	const char *p = text;
	while (*p && n < max) {
		while (*p == ' ') ++p;
		if (!*p) break;
		int len = 0, cut = 0;
		while (p[len] && p[len] != '\n' && len < cols) ++len;
		if (!p[len] || p[len] == '\n') cut = len;
		else {
			for (int i = len; i > 0 && !cut; --i) if (p[i] == ' ' || strchr("/.-", p[i - 1])) cut = i;
			if (!cut) cut = len;
		}
		snprintf(out[n++], 64, "%.*s", cut, p);
		p += cut;
		if (*p == '\n') ++p;
	}
	return n;
}

/* The message, in the ROM's font when there is one, else in the engine's
 * own (a handheld without its ROM showed a row of bars). */
static void error_draw(void) {
	SDL_SetRenderDrawColor(P.renderer, 8, 16, 48, 255);
	SDL_RenderClear(P.renderer);
	if (R.data) {
		text_draw(P.w / 2, 40, "Cyberworld Endless", WHITE, TEXT_CENTER);
		text_draw(P.w / 2, 70, error_msg, WHITE, TEXT_CENTER);
		return;
	}
	/* (in the picture's place: a phone held upright has the canvas below
	 * it for its controls) */
	char lines[12][64];
	int n = wrap_lines(error_msg, 54, lines, 12), x = P.core_x + CORE_W / 2;
	minifont_draw_centered(x, P.core_y + 24, "CYBERWORLD ENDLESS", rgba(120, 200, 248, 255), 2);
	for (int i = 0; i < n; ++i) minifont_draw_centered(x, P.core_y + 56 + i * 8, lines[i], WHITE, 1);
	minifont_draw_centered(x, P.core_y + CORE_H - 20, "START OR B: QUIT", rgba(160, 170, 200, 255), 1);
}

static void error_update(void) {
	if (btn_pressed(BTN_START) || btn_pressed(BTN_B)) P.quit = true;
}

const Scene scene_error = { "error", NULL, error_update, error_draw, NULL };

#ifdef CW_DESKTOP
/* ---- no ROM on a desktop that can show no dialog: the game's own window
 * says where to put it, and looks again every three seconds and on A; once
 * it is there the game starts itself again (a Flatpak on the Steam Deck
 * has no dialog, and quit before its window opened) ---- */
char **g_argv;
static int norom_t, norom_looks;
static char norom_dir[600];

static void norom_update(void) {
	++norom_t;
	if (btn_pressed(BTN_B) || btn_pressed(BTN_START)) { P.quit = true; return; }
	if (!btn_pressed(BTN_A) && norom_t % 180) return;
	++norom_looks;
	char msg[512];
	if (!desktop_rom_anywhere(msg, sizeof msg)) return;
	/* (found: a fresh start sets everything up from it) */
	platform_shutdown();
	execv("/proc/self/exe", g_argv);
	perror("restart");
	exit(0);
}

static void norom_draw(void) {
	SDL_SetRenderDrawColor(P.renderer, 8, 16, 48, 255);
	SDL_RenderClear(P.renderer);
	SDL_Color blue = rgba(120, 200, 248, 255), grey = rgba(160, 170, 200, 255);
	minifont_draw_centered(P.w / 2, 10, "CYBERWORLD ENDLESS", blue, 2);
	minifont_draw_centered(P.w / 2, 28, "NO ROM FOUND", WHITE, 2);
	static const char *const text[] = {
		"IT RUNS ON YOUR OWN COPY OF",
		"MEGA MAN BATTLE NETWORK 6: CYBEAST GREGAR (USA),",
		"AN UNZIPPED .GBA FILE.",
		"",
		"PUT IT IN YOUR DOWNLOADS FOLDER, IN EMULATION/ROMS/GBA",
		"(EMUDECK) OR RETRODECK/ROMS/GBA, OR IN THIS FOLDER:",
	};
	int y = 46;
	for (unsigned i = 0; i < sizeof text / sizeof *text; ++i, y += 8) minifont_draw_centered(P.w / 2, y, text[i], WHITE, 1);
	char lines[4][64];
	int n = wrap_lines(norom_dir, 56, lines, 4);
	for (int i = 0; i < n; ++i, y += 8) minifont_draw_centered(P.w / 2, y + 2, lines[i], blue, 1);
	char looked[64];
	snprintf(looked, sizeof looked, norom_looks ? "LOOKED AGAIN: NOT THERE YET" : "IT LOOKS AGAIN ON ITS OWN");
	minifont_draw_centered(P.w / 2, P.h - 26, looked, grey, 1);
	minifont_draw_centered(P.w / 2, P.h - 14, "A: LOOK NOW    B: QUIT", WHITE, 1);
}

static const Scene scene_norom = { "norom", NULL, norom_update, norom_draw, NULL };

void norom_show(void) {
	/* (the home folder as ~, a Flatpak's is long) */
	const char *home = getenv("HOME");
	size_t hl = home ? strlen(home) : 0;
	if (hl > 1 && !strncmp(g_data_dir, home, hl) && g_data_dir[hl] == '/') snprintf(norom_dir, sizeof norom_dir, "~%s/rom", g_data_dir + hl);
	else snprintf(norom_dir, sizeof norom_dir, "%s/rom", g_data_dir);
	scene_set(&scene_norom);
}
#endif

#ifdef CW_IOS
/* ---- no ROM on an iPhone or iPad: CHOOSE FOLDER opens Files' picker for
 * the folder the ROMs are in, CHOOSE FILES for the files themselves
 * (ios.m); BN6's and BN5's ROMs there are copied into the app's rom/, and
 * the folder is kept and looked in again at each start, and here every
 * three seconds. Or Files puts them in the app's own folder (On My iPhone ›
 * Cyberworld), looked in as often. Found, the game starts in place, as
 * nothing restarts an app on iOS. ---- */
static int norom_t, norom_focus;   /* (the button a controller's A presses: 0 the folder's) */
static bool norom_picking;
static char norom_msg[1024];

/* The ROM in the app's rom/ (where a pick lands), or at the top of its
 * folder (where Files puts a file dropped on it); BN5 is read with it from
 * either. */
static bool ios_rom_here(char *msg, size_t msglen) {
	char dir[600], first[512];
	snprintf(dir, sizeof dir, "%s/rom", g_data_dir);
	bool found = rom_find(dir, msg, msglen);
	snprintf(first, sizeof first, "%s", msg);
	if (!found) found = rom_find(g_data_dir, msg, msglen);
	if (found) {
		/* (beside BN6's it is read already; in the other folder too) */
		xrom_find(dir);
		xrom_find(g_data_dir);
		return true;
	}
	/* (a .gba that is not the right one says so; else where to put it) */
	if (strncmp(first, "Put your", 8)) snprintf(msg, msglen, "%s", first);
	return false;
}

/* At the start: the ROMs kept, then the folder picked looked in for what
 * they lack (BN5 put there since, or BN6 where none is kept). */
bool ios_rom_start(char *msg, size_t msglen) {
	char dir[600];
	snprintf(dir, sizeof dir, "%s/rom", g_data_dir);
	bool found = ios_rom_here(msg, msglen);
	unsigned want = 0;
	if (!found) want |= IOS_ROM_BN6;
	if (!XR[XROM_BN5_COLONEL_US].data) want |= IOS_ROM_BN5;
	if (want && ios_rom_folder_look(dir, want, norom_msg, sizeof norom_msg) > 0) {
		if (!found) found = ios_rom_here(msg, msglen);
		else xrom_find(dir);
	}
	return found;
}

/* The game from the ROM just found, as main goes on from one found at the start. */
static void ios_start(void) {
	if (!gfx_init()) { error_show("The ROM could not be decoded."); return; }
	say_roms();
	save_init();
	audio_init();
	ios_rom_note(P.window);
	scene_set(&scene_intro);
}

/* The screen's words: BN6 needed, BN5 optional with what it lends (the
 * title note's words), what the folder is for, and the other ways */
#define NOROM_SIX "It runs on your own Mega Man Battle Network 6: Cybeast Gregar (USA), an unzipped .gba file."
#define NOROM_FIVE "Optional beside it: Mega Man Battle Network 5: Team Colonel (USA), whose net then joins ours."
#define NOROM_HOW "Looked in again at each start: BN5 can come later."
#define NOROM_HINT "Or put them in Files: On My iPhone (or iPad), Cyberworld. Only .gba files are opened, only these two ROMs copied."

/* The screen's lines and buttons, laid out in the middle of the canvas, the
 * whole screen and not the picture's 240 x 160 (whose 1x text was too small
 * to read on a phone), the same for its update and its drawing: the title
 * at 3x, the rest at 2x, and buttons a thumb finds at once; where that
 * does not fit (a phone on its side), the title and the folder's button at
 * 2x and the rest at 1x, then without the hint, then without the line
 * under the folder's button. */
typedef struct {
	char six[6][64], five[6][64], how[4][64], hint[6][64], note[12][64];
	int nsix, nfive, nhow, nhint, nnote, s, line;
	int y_title, y_six, y_five, y_how, y_hint, y_note;
	SDL_Rect folder, files;
} NoRom;

/* The layout at text scale s, from `top`, with `parts` (bit 0 the hint,
 * bit 1 the folder's line); its height. */
static int norom_fit(NoRom *n, int s, int parts, int top) {
	int cols = (P.w - 16) / (4 * s), gap = 3 * s, w = s > 1 ? 220 : 130, y = top;
	if (cols > 60) cols = 60;
	if (w > P.w - 24) w = P.w - 24;
	n->s = s;
	n->line = s > 1 ? 15 : 8;
	n->nsix = wrap_lines(NOROM_SIX, cols, n->six, 6);
	n->nfive = wrap_lines(NOROM_FIVE, cols, n->five, 6);
	n->nhow = parts & 2 ? wrap_lines(NOROM_HOW, cols, n->how, 4) : 0;
	n->nhint = parts & 1 ? wrap_lines(NOROM_HINT, cols, n->hint, 6) : 0;
	n->nnote = norom_msg[0] ? wrap_lines(norom_msg, cols, n->note, 12) : 0;
	n->y_title = y;
	y += 5 * (s + 1) + 8 * s;
	n->y_six = y;
	y += n->nsix * n->line + gap;
	n->y_five = y;
	y += n->nfive * n->line + 4 * gap;
	n->folder = (SDL_Rect){ (P.w - w) / 2, y, w, 10 * s + 10 };
	y += n->folder.h + gap;
	n->y_how = y;
	y += n->nhow * n->line + 2 * gap;
	n->files = (SDL_Rect){ (P.w - w) / 2, y, w, 10 * s + 4 };
	y += n->files.h + 3 * gap;
	n->y_hint = y;
	y += n->nhint * n->line;
	n->y_note = y + (n->nhint ? 3 * gap : 0);
	if (n->nnote) y = n->y_note + n->nnote * n->line;
	return y - top;
}

static void norom_layout(NoRom *n) {
	static const int tries[][2] = { { 2, 3 }, { 1, 3 }, { 1, 2 }, { 1, 0 } };
	int k = 0;
	while (k < 3 && norom_fit(n, tries[k][0], tries[k][1], 0) > P.h - 8) ++k;
	int top = (P.h - norom_fit(n, tries[k][0], tries[k][1], 0)) / 2;
	norom_fit(n, tries[k][0], tries[k][1], top < 4 ? 4 : top);
}

static void norom_enter(void) { platform_own_taps(true); }
static void norom_leave(void) { platform_own_taps(false); }

static void norom_update(void) {
	++norom_t;
	char dir[600], said[1024] = "";
	snprintf(dir, sizeof dir, "%s/rom", g_data_dir);
	int picked = ios_pick_result(said, sizeof said);
	if (picked) norom_picking = false;
	if (picked > 0) snprintf(norom_msg, sizeof norom_msg, "%s", said);
	NoRom n;
	norom_layout(&n);
	SDL_Point tap;
	int press = -1;
	if (platform_tap(&tap.x, &tap.y)) {
		if (SDL_PointInRect(&tap, &n.folder)) press = IOS_PICK_FOLDER;
		else if (SDL_PointInRect(&tap, &n.files)) press = IOS_PICK_FILES;
	}
	if (btn_pressed(BTN_UP) || btn_pressed(BTN_DOWN)) norom_focus ^= 1;
	if (btn_pressed(BTN_A) || btn_pressed(BTN_START)) press = norom_focus ? IOS_PICK_FILES : IOS_PICK_FOLDER;
	if (!norom_picking && press >= 0) {
		norom_picking = true;
		norom_focus = press == IOS_PICK_FILES;
		ios_pick_roms(P.window, dir, press);
	}
	if (picked <= 0 && norom_t % 180) return;
	/* (every three seconds the folder picked is looked in again: a ROM put
	 * there meanwhile, or come down from iCloud) */
	if (picked <= 0 && !norom_picking) ios_rom_folder_look(dir, IOS_ROM_BN6 | IOS_ROM_BN5, norom_msg, sizeof norom_msg);
	char found[512];
	if (ios_rom_here(found, sizeof found)) { ios_start(); return; }
	/* (a .gba in the app's own folder that is not the right one, where no
	 * look had more to say) */
	if (!norom_msg[0] && strncmp(found, "Put your", 8)) snprintf(norom_msg, sizeof norom_msg, "%s", found);
}

/* A button: gold edged where a controller's A would press it, the PET's navy inside */
static void norom_button(SDL_Rect b, const char *label, int scale, bool focus) {
	fill_rect(b.x, b.y, b.w, b.h, focus ? rgba(255, 214, 16, 255) : rgba(110, 130, 170, 255));
	fill_rect(b.x + 2, b.y + 2, b.w - 4, b.h - 4, rgba(16, 54, 74, 255));
	minifont_draw_centered(b.x + b.w / 2, b.y + (b.h - 5 * scale) / 2, label, WHITE, scale);
}

static void norom_draw(void) {
	SDL_SetRenderDrawColor(P.renderer, 8, 16, 48, 255);
	SDL_RenderClear(P.renderer);
	NoRom n;
	norom_layout(&n);
	SDL_Color blue = rgba(120, 200, 248, 255), soft = rgba(170, 200, 230, 255), grey = rgba(160, 170, 200, 255);
	int x = P.w / 2, s = n.s;
	minifont_draw_centered(x, n.y_title, "CYBERWORLD ENDLESS", blue, s + 1);
	for (int i = 0; i < n.nsix; ++i) minifont_draw_centered(x, n.y_six + i * n.line, n.six[i], WHITE, s);
	for (int i = 0; i < n.nfive; ++i) minifont_draw_centered(x, n.y_five + i * n.line, n.five[i], soft, s);
	norom_button(n.folder, norom_picking && !norom_focus ? "OPENING FILES" : "CHOOSE FOLDER", s + 1, !norom_focus);
	for (int i = 0; i < n.nhow; ++i) minifont_draw_centered(x, n.y_how + i * n.line, n.how[i], grey, s);
	norom_button(n.files, norom_picking && norom_focus ? "OPENING FILES" : "CHOOSE FILES", s, norom_focus);
	for (int i = 0; i < n.nhint; ++i) minifont_draw_centered(x, n.y_hint + i * n.line, n.hint[i], grey, s);
	for (int i = 0; i < n.nnote; ++i) minifont_draw_centered(x, n.y_note + i * n.line, n.note[i], rgba(247, 165, 0, 255), s);
}

static const Scene scene_norom = { "norom", norom_enter, norom_update, norom_draw, norom_leave };

#endif

/* No ROM at the start: on iOS the screen that asks for it (but for a ROM
 * given by --rom-dir that is not one); elsewhere the plain error. */
void rom_missing(const char *rom_dir, bool norom_scene, const char *msg) {
#ifdef CW_IOS
	if (!rom_dir || norom_scene) { scene_set(&scene_norom); return; }
#else
	(void)rom_dir; (void)norom_scene;
#endif
	error_show(msg);
}
