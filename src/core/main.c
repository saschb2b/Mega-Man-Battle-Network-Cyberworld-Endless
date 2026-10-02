#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#elif !defined(_WIN32)
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "compat.h"

#include "game.h"
#include "gfx.h"
#include "platform.h"
#include "rom.h"
#include "save.h"
#include "run.h"
#include "audio.h"
#include "atlas.h"
#include "pacing_report.h"
#include "devtools.h"
#include "tour.h"
#include "director.h"
#include "flags.h"
#include "net_layouts.h"
#include "desktop.h"
#include "minifont.h"
#include "meta.h"
#include "touch.h"
#include "emu.h"
#ifdef CW_IOS
#include "ios.h"
#endif
#ifdef __3DS__
/* (libctru's parts, not <3ds.h>: its Friends service has a Profile too) */
#include <3ds/types.h>
#include <3ds/os.h>
#include <3ds/services/soc.h>
#include <3ds/3dslink.h>
#include <3ds/env.h>
#include <3ds/svc.h>
#include <3ds/result.h>
#include <3ds/allocator/mappable.h>
#include <3ds/services/apt.h>
#include <3ds/services/ptmsysm.h>
#include <3ds/thread.h>
#include <malloc.h>

/* (the main thread's stack: libctru's 32 KB is tight for the game's
 * deepest calls, a layer's making; the browser build's is 1 MB too) */
u32 __stacksize__ = 1u << 20;

/* (whether the New 3DS's third core takes a thread of the game's: emu.c
 * runs the GBA core there) */
static void core2_probe(void *arg) { *(volatile bool *)arg = true; }

/* The app's memory, split before main in place of libctru's split (and
 * mGBA's fixed sizes): the heap takes all its area holds, 96 MB, the
 * linear heap (the screens' and the sound's buffers) what is left, at
 * least 8 MB. The Homebrew Launcher gives a New 3DS app 124 MB, a 3DS 64:
 * a heap of all but the linear heap's share passed the area on the one,
 * a fixed 36 MB was too small for the game's ROM copies on both. */
void __system_allocateHeaps(void);   /* (libctru's, replaced) */
void __system_allocateHeaps(void) {
	extern char *fake_heap_start, *fake_heap_end;
	extern u32 __ctru_heap, __ctru_linear_heap;
	/* (the sizes env.h reads in its own accessors, written here) */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
	extern u32 __ctru_heap_size, __ctru_linear_heap_size;
#pragma GCC diagnostic pop
	Handle limit = 0;
	s64 most = 0, used = 0;
	ResourceLimitType commit = RESLIMIT_COMMIT;
	if (R_FAILED(svcGetResourceLimit(&limit, CUR_PROCESS_HANDLE))) svcBreak(USERBREAK_PANIC);
	svcGetResourceLimitLimitValues(&most, limit, &commit, 1);
	svcGetResourceLimitCurrentValues(&used, limit, &commit, 1);
	svcCloseHandle(limit);
	u32 left = (u32)(most - used) & ~0xFFFu, linear = 8u << 20;
	if (left <= linear) svcBreak(USERBREAK_PANIC);
	u32 heap = left - linear;
	if (heap > OS_HEAP_AREA_END - OS_HEAP_AREA_BEGIN) heap = OS_HEAP_AREA_END - OS_HEAP_AREA_BEGIN;
	__ctru_heap_size = heap;
	__ctru_linear_heap_size = left - heap;
	if (R_FAILED(svcControlMemory(&__ctru_heap, OS_HEAP_AREA_BEGIN, 0, heap, MEMOP_ALLOC, MEMPERM_READ | MEMPERM_WRITE))
	    || R_FAILED(svcControlMemory(&__ctru_linear_heap, 0, 0, __ctru_linear_heap_size, MEMOP_ALLOC_LINEAR, MEMPERM_READ | MEMPERM_WRITE)))
		svcBreak(USERBREAK_PANIC);
	mappableInit(OS_MAP_AREA_BEGIN, OS_MAP_AREA_END);
	fake_heap_start = (char *)__ctru_heap;
	fake_heap_end = fake_heap_start + heap;
}
#endif

char g_data_dir[512] = ".";

/* The desktop builds (host, linux) open a window and keep their files in
 * the user's data folder; the handheld port fills the screen and keeps them
 * beside itself (its launcher passes --data-dir and --rom-dir). */
#ifdef CW_DESKTOP
#define DESKTOP true
#else
#define DESKTOP false
#endif
#ifdef CW_IOS
#define IOS true
#else
#define IOS false
#endif

/* mkdir -p */
static void make_dirs(const char *path) {
	char p[600];
	snprintf(p, sizeof p, "%s", path);
	for (char *c = p + 1; *c; ++c)
		if (*c == '/') { *c = 0; cw_mkdir(p); *c = '/'; }
	cw_mkdir(p);
}

/* $XDG_DATA_HOME/cyberworld-endless, or ~/.local/share/cyberworld-endless;
 * on Windows %LOCALAPPDATA%\cyberworld-endless, on macOS
 * ~/Library/Application Support/cyberworld-endless */
static void desktop_data_dir(char *out, size_t n) {
#ifdef _WIN32
	const char *local = getenv("LOCALAPPDATA");
	if (local && *local) snprintf(out, n, "%s/cyberworld-endless", local);
	else snprintf(out, n, ".");
	for (char *c = out; *c; ++c)
		if (*c == '\\') *c = '/';
#elif defined(__APPLE__)
	const char *home = getenv("HOME");
	if (home && *home) snprintf(out, n, "%s/Library/Application Support/cyberworld-endless", home);
	else snprintf(out, n, ".");
#else
	const char *xdg = getenv("XDG_DATA_HOME"), *home = getenv("HOME");
	if (xdg && *xdg == '/') snprintf(out, n, "%s/cyberworld-endless", xdg);
	else if (home && *home) snprintf(out, n, "%s/.local/share/cyberworld-endless", home);
	else snprintf(out, n, ".");
#endif
}

#ifndef __3DS__
/* The ROM: in the data folder's rom/, beside the binary, or in ./rom. */
static bool desktop_rom(char *msg, size_t msglen) {
	char dirs[3][600], exe[512];
	int n = 0;
	snprintf(dirs[n++], sizeof dirs[0], "%s/rom", g_data_dir);
	if (cw_exe_path(exe, sizeof exe)) {
		char *slash = strrchr(exe, '/');
		if (slash) { *slash = 0; snprintf(dirs[n++], sizeof dirs[0], "%s/rom", exe); }
	}
	snprintf(dirs[n++], sizeof dirs[0], "rom");
	char first[512] = "";
	for (int i = 0; i < n; ++i) {
		if (rom_find(dirs[i], msg, msglen)) return true;
		/* a .gba that is not the right one says so; else the data folder is the place */
		if (!first[0] || strncmp(msg, "Put your", 8)) snprintf(first, sizeof first, "%s", msg);
	}
	snprintf(msg, msglen, "%s", first);
	return false;
}
#endif

#ifdef CW_DESKTOP
/* ... and, looking again, where front ends and downloads keep theirs */
static bool desktop_rom_anywhere(char *msg, size_t msglen) {
	if (desktop_rom(msg, msglen)) return true;
	char dir[600];
	snprintf(dir, sizeof dir, "%s/rom", g_data_dir);
	return desktop_rom_elsewhere(dir, msg, msglen);
}
#endif

static const Scene *current, *pending;

void scene_set(const Scene *s) { pending = s; }

static uint32_t rng_s = 0x9E3779B9u;
/* A seed goes through a mixer first: xorshift from seeds one apart gave
 * nearly the same first numbers, so runs started with consecutive seeds
 * (and their layers) came out alike. */
void rng_seed(uint32_t s) {
	s += 0x9E3779B9u;
	s ^= s >> 16; s *= 0x85EBCA6Bu;
	s ^= s >> 13; s *= 0xC2B2AE35u;
	s ^= s >> 16;
	rng_s = s ? s : 0x9E3779B9u;
}
void rng_restore(uint32_t s) { rng_s = s ? s : 0x9E3779B9u; }
uint32_t rng_state(void) { return rng_s; }
uint32_t rng_next(void) {
	uint32_t x = rng_s;
	x ^= x << 13; x ^= x >> 17; x ^= x << 5;
	return rng_s = x;
}
int rng_range(int lo, int hi) {
	if (hi <= lo) return lo;
	return lo + (int)(rng_next() % (uint32_t)(hi - lo + 1));
}

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
static char **g_argv;
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

static void norom_show(void) {
	/* (the home folder as ~, a Flatpak's is long) */
	const char *home = getenv("HOME");
	size_t hl = home ? strlen(home) : 0;
	if (hl > 1 && !strncmp(g_data_dir, home, hl) && g_data_dir[hl] == '/') snprintf(norom_dir, sizeof norom_dir, "~%s/rom", g_data_dir + hl);
	else snprintf(norom_dir, sizeof norom_dir, "%s/rom", g_data_dir);
	scene_set(&scene_norom);
}
#endif

#ifdef CW_IOS
/* ---- no ROM on an iPhone or iPad: A opens Files' picker (ios.m), which
 * copies the file picked into the app's rom/; or Files puts it in the
 * app's own folder (On My iPhone › Cyberworld), where the game
 * looks again every three seconds. Found, the game starts in place, as
 * nothing restarts an app on iOS. ---- */
static int norom_t, norom_looks;
static bool norom_picking;
static char norom_msg[512];

/* The ROM in the app's rom/ (where a pick lands), or at the top of its
 * folder (where Files puts a file dropped on it). */
static bool ios_rom_here(char *msg, size_t msglen) {
	char dir[600], first[512];
	snprintf(dir, sizeof dir, "%s/rom", g_data_dir);
	if (rom_find(dir, msg, msglen)) return true;
	snprintf(first, sizeof first, "%s", msg);
	if (rom_find(g_data_dir, msg, msglen)) return true;
	/* (a .gba that is not the right one says so; else where to put it) */
	if (strncmp(first, "Put your", 8)) snprintf(msg, msglen, "%s", first);
	return false;
}

/* The game from the ROM just found, as main goes on from one found at the start. */
static void ios_start(void) {
	if (!gfx_init()) { error_show("The ROM could not be decoded."); return; }
	printf("ROM: %s (%s)\n", R.layout->name, R.path);
	save_init();
	audio_init();
	scene_set(&scene_title);
}

/* The screen's lines and button, laid out in the middle of the canvas, the
 * whole screen and not the picture's 240 x 160 (whose 1x text was too small
 * to read on a phone), the same for its update and its drawing: the title
 * at 3x, the rest at 2x, and a button a thumb finds at once. */
#define NOROM_LINE 15   /* (a 2x line, 10 pixels, and room between) */
typedef struct {
	char body[6][64], hint[4][64], note[5][64];
	int nbody, nhint, nnote, y;
	SDL_Rect button;
} NoRom;

static void norom_layout(NoRom *n) {
	int cols = (P.w - 16) / 8;   /* (a 2x character is 8 pixels across) */
	if (cols > 60) cols = 60;
	n->nbody = wrap_lines("It runs on your own Mega Man Battle Network 6: Cybeast Gregar (USA), an unzipped .gba file.", cols, n->body, 6);
	n->nhint = wrap_lines("Or put it in Files, On My iPhone (or iPad), in the Cyberworld folder.", cols, n->hint, 4);
	/* (a pick that was not the ROM: what was found instead) */
	n->nnote = norom_looks && norom_msg[0] ? wrap_lines(norom_msg, cols, n->note, 5) : 0;
	int w = P.w - 24 < 220 ? P.w - 24 : 220, h = 30;
	int total = 15 + 16 + n->nbody * NOROM_LINE + 14 + h + 18 + n->nhint * NOROM_LINE + (n->nnote ? 12 + n->nnote * NOROM_LINE : 0);
	n->y = (P.h - total) / 2;
	n->button = (SDL_Rect){ (P.w - w) / 2, n->y + 15 + 16 + n->nbody * NOROM_LINE + 14, w, h };
}

static void norom_enter(void) { platform_own_taps(true); }
static void norom_leave(void) { platform_own_taps(false); }

static void norom_update(void) {
	++norom_t;
	int picked = ios_pick_result();
	if (picked) norom_picking = false;
	NoRom n;
	norom_layout(&n);
	SDL_Point tap;
	bool tapped = platform_tap(&tap.x, &tap.y) && SDL_PointInRect(&tap, &n.button);
	if (!norom_picking && (tapped || btn_pressed(BTN_A) || btn_pressed(BTN_START))) {
		char dir[600];
		snprintf(dir, sizeof dir, "%s/rom", g_data_dir);
		norom_picking = true;
		ios_pick_rom(P.window, dir);
	}
	if (picked <= 0 && norom_t % 180) return;
	if (picked > 0) ++norom_looks;
	if (ios_rom_here(norom_msg, sizeof norom_msg)) ios_start();
}

static void norom_draw(void) {
	SDL_SetRenderDrawColor(P.renderer, 8, 16, 48, 255);
	SDL_RenderClear(P.renderer);
	NoRom n;
	norom_layout(&n);
	SDL_Color blue = rgba(120, 200, 248, 255), grey = rgba(160, 170, 200, 255), gold = rgba(255, 214, 16, 255);
	int x = P.w / 2, y = n.y;
	minifont_draw_centered(x, y, "CYBERWORLD ENDLESS", blue, 3);
	y += 15 + 16;
	for (int i = 0; i < n.nbody; ++i, y += NOROM_LINE) minifont_draw_centered(x, y, n.body[i], WHITE, 2);
	/* the button: gold edged, the PET's navy inside */
	SDL_Rect b = n.button;
	fill_rect(b.x, b.y, b.w, b.h, gold);
	fill_rect(b.x + 2, b.y + 2, b.w - 4, b.h - 4, rgba(16, 54, 74, 255));
	minifont_draw_centered(x, b.y + (b.h - 15) / 2, norom_picking ? "OPENING FILES" : "CHOOSE ROM", WHITE, 3);
	y = b.y + b.h + 18;
	for (int i = 0; i < n.nhint; ++i, y += NOROM_LINE) minifont_draw_centered(x, y, n.hint[i], grey, 2);
	y += 12;
	for (int i = 0; i < n.nnote; ++i, y += NOROM_LINE) minifont_draw_centered(x, y, n.note[i], rgba(247, 165, 0, 255), 2);
}

static const Scene scene_norom = { "norom", norom_enter, norom_update, norom_draw, norom_leave };

#endif

/* The data folder, where none is given, and its rom/: a desktop's in the
 * user's data folders; iOS's the app's Documents, which Files shows as On
 * My iPhone › Cyberworld (its display name), the ROM's place, beside the
 * saves, which a player can copy off there; a handheld's the launcher's. */
static void data_dir_setup(bool given) {
	if (DESKTOP && !given) desktop_data_dir(g_data_dir, sizeof g_data_dir);
#ifdef CW_IOS
	if (!given) {
		const char *home = getenv("HOME");
		snprintf(g_data_dir, sizeof g_data_dir, "%s/Documents", home && *home ? home : ".");
	}
#endif
	if (DESKTOP || IOS) {
		char rom[600];
		snprintf(rom, sizeof rom, "%s/rom", g_data_dir);
		make_dirs(rom);
	}
}

#ifndef __3DS__
/* The ROM at the start (the 3DS has its own places, main): --rom-dir's,
 * else the desktop's, iOS's app folder, or ./rom on a handheld. */
static bool start_rom(const char *rom_dir, char *msg, size_t msglen) {
#ifdef CW_IOS
	if (!rom_dir) return ios_rom_here(msg, msglen);
#endif
	return rom_dir || !DESKTOP ? rom_find(rom_dir ? rom_dir : "rom", msg, msglen) : desktop_rom(msg, msglen);
}
#endif

/* No ROM at the start: on iOS the screen that asks for it (but for a ROM
 * given by --rom-dir that is not one); elsewhere the plain error. */
static void rom_missing(const char *rom_dir, bool norom_scene, const char *msg) {
#ifdef CW_IOS
	if (!rom_dir || norom_scene) { scene_set(&scene_norom); return; }
#else
	(void)rom_dir; (void)norom_scene;
#endif
	error_show(msg);
}

/* ---- scripted input and captures for headless tests ---- */
/* A step holds buttons for some frames; one of no frames takes a picture
 * or writes the state instead (remote play). */
typedef struct { int frames; uint32_t buttons; char shot[160], state[160]; int place[3], flags[3]; bool placed, flagged; } InputStep;
static InputStep script[1024];
static int script_len, script_pos, script_left;

static uint32_t parse_buttons(const char *s) {
	static const struct { const char *n; uint32_t b; } names[] = {
		{ "UP", BTN_UP }, { "DOWN", BTN_DOWN }, { "LEFT", BTN_LEFT }, { "RIGHT", BTN_RIGHT },
		{ "A", BTN_A }, { "B", BTN_B }, { "L", BTN_L }, { "R", BTN_R }, { "START", BTN_START }, { "SELECT", BTN_SELECT },
	};
	uint32_t bits = 0;
	char buf[128];
	snprintf(buf, sizeof buf, "%s", s);
	char *save = NULL;
	for (char *t = strtok_r(buf, "+", &save); t; t = strtok_r(NULL, "+", &save))
		for (size_t i = 0; i < sizeof names / sizeof *names; ++i)
			if (!strcmp(t, names[i].n)) bits |= names[i].b;
	return bits;
}

/* "30:,2:A,10:,2:RIGHT" -> steps of (frames, buttons). */
static void parse_script(const char *spec) {
	char *copy = strdup(spec);
	for (char *tok = strtok(copy, ","); tok && script_len < 512; tok = strtok(NULL, ",")) {
		char *colon = strchr(tok, ':');
		memset(&script[script_len], 0, sizeof script[script_len]);
		script[script_len].frames = atoi(tok);
		script[script_len].buttons = colon ? parse_buttons(colon + 1) : 0;
		++script_len;
	}
	free(copy);
	script_left = script_len ? script[0].frames : 0;
}

/* --taps "FRAME:X,Y[>X2,Y2];...": a finger at screen pixel (X, Y) at
 * FRAME, held six frames, or dragged to (X2, Y2) over twenty, then lifted
 * (the touch controls, their menu and its editor in headless tests) */
typedef struct { int frame, x0, y0, x1, y1; bool drag; } Tap;
static Tap taps[32];
static int ntaps;

static void parse_taps(const char *spec) {
	char *copy = strdup(spec), *save = NULL;
	for (char *t = strtok_r(copy, ";", &save); t && ntaps < 32; t = strtok_r(NULL, ";", &save)) {
		Tap *p = &taps[ntaps];
		int n = sscanf(t, "%d:%d,%d>%d,%d", &p->frame, &p->x0, &p->y0, &p->x1, &p->y1);
		if (n < 3) continue;
		p->drag = n == 5;
		if (!p->drag) { p->x1 = p->x0; p->y1 = p->y0; }
		++ntaps;
	}
	free(copy);
}

static void taps_tick(void) {
	for (int i = 0; i < ntaps; ++i) {
		const Tap *t = &taps[i];
		int len = t->drag ? 20 : 6, f = (int)P.frame - t->frame;
		if (f < 0 || f > len) continue;
		uint32_t type = f == 0 ? SDL_FINGERDOWN : f == len ? SDL_FINGERUP : SDL_FINGERMOTION;
		touch_finger(type, 900 + i, t->x0 + (t->x1 - t->x0) * (float)f / len, t->y0 + (t->y1 - t->y0) * (float)f / len);
	}
}

static uint32_t bot_seed;
static uint32_t bot_buttons;

/* Random-input bot for soak tests: never pauses, mashes everything else. */
static void bot_tick(void) {
	if (P.frame % 6 == 0) {
		bot_seed = bot_seed * 1103515245u + 12345u;
		uint32_t r = bot_seed >> 8;
		bot_buttons = 0;
		static const uint32_t dirs[8] = { BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_UP | BTN_RIGHT, BTN_DOWN | BTN_LEFT, 0, BTN_RIGHT };
		bot_buttons |= dirs[r & 7];
		if ((r >> 3) % 3 == 0) bot_buttons |= BTN_A;
		if ((r >> 5) % 4 == 0) bot_buttons |= BTN_B;
		if ((r >> 7) % 9 == 0) bot_buttons |= BTN_L;
	}
	platform_inject(bot_buttons);
}

/* What a player sees, in words (remote play's state file). */
static void write_state(const char *path) {
	FILE *f = fopen(path, "w");
	if (!f) return;
	fprintf(f, "frame %llu\nscene %s\n", (unsigned long long)P.frame, current ? current->name : "none");
	if (current == &scene_emu) director_describe(f);
	fclose(f);
}

/* The steps of no frames at the script's position: pictures, states. */
static void script_actions(void) {
	while (script_pos < script_len && script[script_pos].frames == 0) {
		InputStep *s = &script[script_pos];
		if (s->shot[0]) platform_save_canvas(s->shot);
		if (s->state[0]) write_state(s->state);
		if (s->placed) director_dev_place(s->place[0], s->place[1], s->place[2]);
		/* (event flags FROM..TO set, then as they were: finding what a flag does) */
		if (s->flagged && current == &scene_emu) {
			static bool was[0x2000];
			for (int f = s->flags[0] < 0 ? 0 : s->flags[0]; f <= s->flags[1] && f < 0x2000; ++f)
				if (s->flags[2]) { was[f] = flag_get(f); flag_set(f); }
				else if (was[f]) flag_set(f);
				else flag_clear(f);
		}
		if (++script_pos < script_len) script_left = script[script_pos].frames;
	}
}

#if !defined(__EMSCRIPTEN__) && !defined(_WIN32) && !defined(__3DS__)
/* ---- remote play (tools/play.py): the game waits for batches of steps on
 * DIR/in and answers each on DIR/out once its frames have run ---- */
static int remote_in = -1, remote_out = -1;
static bool remote_answer;   /* a batch has run: answer before the next */

static bool remote_open(const char *dir) {
	char in[600], out[600];
	snprintf(in, sizeof in, "%s/in", dir);
	snprintf(out, sizeof out, "%s/out", dir);
	mkfifo(in, 0600);
	mkfifo(out, 0600);
	/* (both ends read-write: no end of file between the client's calls) */
	remote_in = open(in, O_RDWR);
	remote_out = open(out, O_RDWR);
	return remote_in >= 0 && remote_out >= 0;
}

/* One line: "N BUTTONS" holds them N frames, "shot PATH", "state PATH",
 * "place X Y FACING", "flags FROM TO 1" (set; 0: back as they were), "quit"; items apart by ';'. */
static void remote_parse(char *line) {
	script_len = script_pos = 0;
	for (char *tok = strtok(line, ";\n"); tok && script_len < (int)(sizeof script / sizeof *script); tok = strtok(NULL, ";\n")) {
		while (*tok == ' ') ++tok;
		InputStep *s = &script[script_len];
		memset(s, 0, sizeof *s);
		if (!strncmp(tok, "shot ", 5)) snprintf(s->shot, sizeof s->shot, "%s", tok + 5);
		else if (!strncmp(tok, "state ", 6)) snprintf(s->state, sizeof s->state, "%s", tok + 6);
		else if (!strncmp(tok, "place ", 6)) s->placed = sscanf(tok + 6, "%d %d %d", &s->place[0], &s->place[1], &s->place[2]) == 3;
		else if (!strncmp(tok, "flags ", 6)) s->flagged = sscanf(tok + 6, "%i %i %i", &s->flags[0], &s->flags[1], &s->flags[2]) == 3;
		else if (!strncmp(tok, "quit", 4)) { P.quit = true; return; }
		else {
			char buttons[128] = "";
			if (sscanf(tok, "%d %127s", &s->frames, buttons) < 1 || s->frames <= 0) continue;
			s->buttons = parse_buttons(buttons);
		}
		++script_len;
	}
	script_left = script_len ? script[0].frames : 0;
}

/* Between batches: the answer to the last, then the next (blocking). */
static void remote_tick(void) {
	for (;;) {
		script_actions();
		if (script_pos < script_len || P.quit) return;
		if (remote_answer) {
			char ok[64];
			int n = snprintf(ok, sizeof ok, "ok %llu\n", (unsigned long long)P.frame);
			if (write(remote_out, ok, (size_t)n) < 0) { P.quit = true; return; }
			remote_answer = false;
		}
		static char line[1 << 17];   /* (a sheet of pictures names every one) */
		int n = 0;
		while (n < (int)sizeof line - 1) {
			char c;
			if (read(remote_in, &c, 1) != 1) { P.quit = true; return; }
			if (c == '\n') break;
			line[n++] = c;
		}
		line[n] = 0;
		remote_parse(line);
		remote_answer = true;
	}
}
#endif

static void script_tick(void) {
	if (bot_seed) { bot_tick(); return; }
#if !defined(__EMSCRIPTEN__) && !defined(_WIN32) && !defined(__3DS__)
	if (remote_in >= 0) remote_tick();
#endif
	script_actions();
	if (script_pos >= script_len) { platform_inject(0); return; }
	platform_inject(script[script_pos].buttons);
	if (--script_left <= 0 && ++script_pos < script_len) script_left = script[script_pos].frames;
}

typedef struct { uint64_t frame; char path[256]; } Shot;
static Shot shots[64], screen_shots[16], second_shots[16];
static int shot_count, screen_shot_count, second_shot_count;
static uint64_t range_a = 1, range_b = 0;   /* --shot-range A:B:PREFIX */
static char range_prefix[200];

static void parse_shots(const char *spec, Shot *into, int *count, int most) {
	char *copy = strdup(spec);
	for (char *tok = strtok(copy, ","); tok && *count < most; tok = strtok(NULL, ",")) {
		char *colon = strchr(tok, ':');
		if (!colon) continue;
		into[*count].frame = strtoull(tok, NULL, 10);
		snprintf(into[*count].path, sizeof into[*count].path, "%s", colon + 1);
		++*count;
	}
	free(copy);
}

/* --shot FRAME:PATH,... saves the canvas; --screen-shot the screen as the
 * player sees it, the touch controls on it; --second-shot the second
 * screen, the 3DS's bottom one (on any target, for a check) */
static bool shot_option(const char *a, const char *v) {
	if (!strcmp(a, "--shot")) parse_shots(v, shots, &shot_count, 64);
	else if (!strcmp(a, "--screen-shot")) parse_shots(v, screen_shots, &screen_shot_count, 16);
	else if (!strcmp(a, "--second-shot")) parse_shots(v, second_shots, &second_shot_count, 16);
	else return false;
	return true;
}

static const Scene *scene_by_name(const char *n) {
	const Scene *all[] = { &scene_title, &scene_gallery, &scene_emu };
	for (size_t i = 0; i < sizeof all / sizeof *all; ++i)
		if (!strcmp(all[i]->name, n)) return all[i];
	return NULL;
}

/* ---- the frame loop ---- */

static struct {
	bool headless;
	uint64_t max_frames;
	uint64_t last;
	double acc;
} loop;

/* After one Escape: a strip over the picture until the second quits. */
static void quit_prompt_draw(void) {
	if (P.quit_prompt <= 0 || !R.data) return;
	/* (a run is saved from its first layer on: where MegaMan stands when he
	 * is free to move, else at the layer's start) */
	bool saved = director_on_layer();
	int y = P.core_y + CORE_H - 30;
	fill_rect(0, y, P.w, saved ? 26 : 14, rgba(0, 0, 0, 200));
#ifdef __ANDROID__
	const char *again = P.quit_pad ? "Hold SELECT+START again to quit" : "Press Back again to quit";
#else
	const char *again = P.quit_pad ? "Hold SELECT+START again to quit" : "Press Esc again to quit";
#endif
	text_draw(P.w / 2, y + 2, again, WHITE, TEXT_CENTER);
	if (saved) text_draw(P.w / 2, y + 14, director_can_suspend() ? "Your run is saved right here" : director_saved_where(),
		rgba(170, 200, 255, 255), TEXT_CENTER);
}

/* One game frame: scenes, input, update, sound, drawing. False once the
 * frame budget of a headless run is spent. */
static bool game_frame(void) {
	if (pending) {
		if (current && current->leave) current->leave();
		current = pending;
		pending = NULL;
		if (current->enter) current->enter();
	}
	uint64_t t0 = SDL_GetPerformanceCounter();
	script_tick();
	platform_poll();
	taps_tick();
	/* (the touch controls' menu pauses the game under it) */
	if (current && current->update && !touch_paused()) current->update();
	audio_frame();
	uint64_t t1 = SDL_GetPerformanceCounter();
	platform_begin_frame();
	if (current && current->draw) current->draw();
	platform_apply_effects();
	quit_prompt_draw();
	if (devtools_shot[0]) { platform_save_canvas(devtools_shot); devtools_shot[0] = 0; }
	for (int i = 0; i < shot_count; ++i)
		if (shots[i].frame == P.frame) platform_save_canvas(shots[i].path);
	for (int i = 0; i < screen_shot_count; ++i)
		if (screen_shots[i].frame == P.frame) platform_shot_screen(screen_shots[i].path);
	for (int i = 0; i < second_shot_count; ++i)
		if (second_shots[i].frame == P.frame) platform_save_second_screen(second_shots[i].path);
	if (P.frame >= range_a && P.frame <= range_b) {
		char path[256];
		snprintf(path, sizeof path, "%s%05llu.bmp", range_prefix, (unsigned long long)P.frame);
		platform_save_canvas(path);
	}
	platform_frame_parts(t1 - t0, SDL_GetPerformanceCounter() - t1);
	platform_end_frame();
	return !(loop.max_frames && P.frame >= loop.max_frames);
}

/* Seconds since the last call, from the performance counter. */
static double elapsed(void) {
	uint64_t now = SDL_GetPerformanceCounter();
	double dt = (double)(now - loop.last) / (double)SDL_GetPerformanceFrequency();
	loop.last = now;
	return dt;
}

/* Smooth motion (settings.ini): at each refresh of the display, the game
 * frames due (two at most), not shown, then the last two mixed by how far
 * the next one is due. */
static bool blend_frames(void) {
	if (loop.acc > 0.1) loop.acc = 1.0 / 60.0;
	for (int n = 0; n < 2 && loop.acc >= 1.0 / 60.0; ++n) {
		loop.acc -= 1.0 / 60.0;
		P.skip_present = true;
		bool go = game_frame();
		P.skip_present = false;
		if (!go || P.quit) return false;
	}
	platform_present_blend(loop.acc * 60.0);
	return true;
}

#ifndef __EMSCRIPTEN__
/* The native loop: 60 game frames a second (as fast as it can headless). */
static bool step(void) {
#ifdef CW_IOS
	/* (sent to the background: the run kept where MegaMan stands, as a
	 * quit keeps it, for iOS may end the app there unasked; and nothing
	 * drawn, which iOS refuses an app it cannot see, till it comes back) */
	if (P.background) {
		static bool kept;
		if (!kept && current == &scene_emu) director_suspend();
		kept = true;
		SDL_Delay(50);
		platform_poll();
		if (!P.background) { kept = false; loop.last = SDL_GetPerformanceCounter(); loop.acc = 0; }
		return true;
	}
#endif
	if (!loop.headless && P.blend) {
		loop.acc += elapsed();
		uint64_t t0 = SDL_GetPerformanceCounter();
		if (!blend_frames()) return false;
		/* (a display that does not hold a present to its refresh: a
		 * millisecond at least between them) */
		if ((SDL_GetPerformanceCounter() - t0) * 1000 < SDL_GetPerformanceFrequency()) SDL_Delay(1);
		return true;
	}
	if (!loop.headless || (getenv("CYBERWORLD_AUDIO_DUMP") && !audio_offline())) {
		loop.acc += elapsed();
		if (loop.acc < 1.0 / 60.0 - 0.002) { SDL_Delay(1); return true; }
		loop.acc -= 1.0 / 60.0;
		if (loop.acc > 0.1) loop.acc = 0;
		/* a frame behind (a phone slower than a frame, a display's refresh
		 * missed, a 50 Hz display): one more game frame first, not shown,
		 * so the game keeps the GBA's pace, as the browser's loop does (a
		 * player's phone felt slow; each missed refresh had cost a frame) */
		/* (the GBA core on a thread of its own sets the pace: a frame
		 * played unshown would cost it a whole frame, drawn aside) */
		if (loop.acc >= 1.0 / 60.0 - 0.002 && !emu_threaded()) {
			loop.acc -= 1.0 / 60.0;
			P.skip_present = true;
			bool go = game_frame();
			P.skip_present = false;
			if (!go) return false;
		}
	}
	return game_frame();
}
#else
/* The browser's frame (60, 120 or 144 a second): as many game frames as
 * 1/60 s steps have passed, at most two, so the game keeps the GBA's pace. */
static void web_frame(void) {
	loop.acc += elapsed();
	if (P.blend) {
		if (!blend_frames() || P.quit) { emscripten_cancel_main_loop(); platform_shutdown(); }
		return;
	}
	if (loop.acc > 0.1) loop.acc = 1.0 / 60.0;
	for (int n = 0; n < 2 && loop.acc >= 1.0 / 60.0 - 0.002; ++n) {
		loop.acc -= 1.0 / 60.0;
		if (!game_frame() || P.quit) { emscripten_cancel_main_loop(); platform_shutdown(); return; }
	}
}
#endif

#ifndef __EMSCRIPTEN__
/* SIGTERM or SIGINT (a launcher closing the port, a terminal's Ctrl+C):
 * the loop ends at a frame's end and the run is kept */
static volatile sig_atomic_t quit_signal;
#ifndef __3DS__
static void on_quit_signal(int sig) { (void)sig; quit_signal = 1; }
#endif
#endif

int main(int argc, char **argv) {
#ifdef CW_DESKTOP
	g_argv = argv;
#endif
	const char *rom_dir = NULL;
	bool fullscreen = !DESKTOP, data_dir_given = false, screen_given = false;
	const char *start_scene = "title";
	int run_depth = 0, guardian_navi = 0;
	int smooth_arg = -1;   /* --smooth-motion on|off, over settings.ini */
	/* --setup NET,FOLDER,THREAT,HELPERS[,CROSS] (short or endless, then
	 * numbers; CROSS the navi whose Cross the run brings, 1-5):
	 * the run's setup as the setup screen chooses it; NEW GAME's (the
	 * short net) for --scene town, the endless net otherwise */
	const char *setup_spec = NULL;
	int marks_spec = -1;
	int force_w = 0, force_h = 0;
	bool headless = false;
#if !defined(__EMSCRIPTEN__) && !defined(_WIN32) && !defined(__3DS__)
	const char *remote_dir = NULL;
#endif
	uint64_t max_frames = 0;
	uint32_t seed = 0;
	const char *render_spec = NULL;
	const char *sheet_spec = NULL;
	const char *atlas_spec = NULL;
	const char *pacing_spec = NULL;
	for (int i = 1; i < argc; ++i) {
		const char *a = argv[i];
		const char *v = i + 1 < argc ? argv[i + 1] : NULL;
#ifdef CW_DESKTOP
		/* (linux/steam/add-to-steam.py, which the Linux builds carry) */
		if (!strcmp(a, "--add-to-steam") || !strcmp(a, "--remove-from-steam")) return desktop_steam_command(a[2] == 'r');
#endif
		if (!strcmp(a, "--headless")) headless = true;
		else if (!strcmp(a, "--rom-dir") && v) { rom_dir = v; ++i; }
		else if (!strcmp(a, "--data-dir") && v) { snprintf(g_data_dir, sizeof g_data_dir, "%s", v); data_dir_given = true; ++i; }
		else if (!strcmp(a, "--fullscreen")) { fullscreen = true; screen_given = true; }
		else if (!strcmp(a, "--window")) { fullscreen = false; screen_given = true; }
		else if (!strcmp(a, "--size") && v) { sscanf(v, "%dx%d", &force_w, &force_h); ++i; }
		/* the touch controls from the start (the browser on a phone) */
		else if (!strcmp(a, "--touch")) touch_always();
		else if (!strcmp(a, "--frames") && v) { max_frames = strtoull(v, NULL, 10); ++i; }
		else if (!strcmp(a, "--smooth-motion") && v) { smooth_arg = !strcmp(v, "on"); ++i; }
		/* (the frame log without an environment: the 3DS over 3dslink) */
		else if (!strcmp(a, "--frame-log")) platform_frame_log = true;
		else if (!strcmp(a, "--input") && v) { parse_script(v); ++i; }
		else if (!strcmp(a, "--taps") && v) { parse_taps(v); ++i; }
		else if (v && shot_option(a, v)) ++i;
		/* (the screen's density for the touch controls, in dots per inch) */
		else if (!strcmp(a, "--dpi") && v) { platform_set_dpi((float)atof(v)); ++i; }
		else if (!strcmp(a, "--shot-range") && v) {
			unsigned long long ra = 0, rb = 0;
			if (sscanf(v, "%llu:%llu:%199s", &ra, &rb, range_prefix) == 3) { range_a = ra; range_b = rb; }
			++i;
		}
		else if (!strcmp(a, "--scene") && v) { start_scene = v; ++i; }
		else if (!strcmp(a, "--seed") && v) { seed = (uint32_t)strtoul(v, NULL, 0); ++i; }
		else if (!strcmp(a, "--render-song") && v) { render_spec = v; ++i; }
		else if (!strcmp(a, "--sheet") && v) { sheet_spec = v; ++i; }
		else if (!strcmp(a, "--run-depth") && v) { run_depth = atoi(v); ++i; }
		else if (!strcmp(a, "--net-biome") && v) { director_net_biome_arg(v); ++i; }
		else if (!strcmp(a, "--guardian") && v) { guardian_navi = atoi(v); ++i; }
		else if (!strcmp(a, "--setup") && v) { setup_spec = v; ++i; }
		/* --marks HEX: the title's marks as if earned, for a capture */
		else if (!strcmp(a, "--marks") && v) { marks_spec = (int)strtol(v, NULL, 16); ++i; }
		else if (!strcmp(a, "--talk") && v) { director_dev_talks = v; ++i; }
#if !defined(__EMSCRIPTEN__) && !defined(_WIN32) && !defined(__3DS__)
		else if (!strcmp(a, "--remote") && v) { remote_dir = v; ++i; }
#endif
		else if (!strcmp(a, "--net-layout") && v) { layout_forced = atoi(v); ++i; }
		else if (!strcmp(a, "--atlas") && v) { atlas_spec = v; ++i; }
		else if (!strcmp(a, "--pacing") && v) { pacing_spec = v; ++i; }
		else if (!strcmp(a, "--dev") && v) { devtools_parse(v); ++i; }
		else if (!strcmp(a, "--tour") && v) { tour_parse(v); start_scene = "emu"; ++i; }
		else if (!strcmp(a, "--bot") && v) { bot_seed = (uint32_t)strtoul(v, NULL, 0) | 1; ++i; }
		else { fprintf(stderr, "unknown argument %s\n", a); return 2; }
	}
	setvbuf(stdout, NULL, _IOLBF, 0);
#ifdef __3DS__
	/* the 3DS (issue #9, 3ds/): the New 3DS's faster clock and cache, the
	 * SD card's folder for the saves and the ROM, and stdout to the PC
	 * that sent the game over 3dslink. (Whether ptm:sysm, which sets the
	 * clock, answers is asked first for the log.) */
	Result sysm = ptmSysmInit();
	if (R_SUCCEEDED(sysm)) ptmSysmExit();
	osSetSpeedupEnable(true);
	volatile bool third = false;
	union { volatile bool *c; void *v; } arg = { &third };   /* (the thread writes it through a volatile) */
	Thread probe = threadCreate(core2_probe, arg.v, 0x1000, 0x30, 2, false);
	if (probe) { threadJoin(probe, U64_MAX); threadFree(probe); }
	bool n3ds = false;
	APT_CheckNew3DS(&n3ds);
	char mem[300];
	snprintf(mem, sizeof mem, "3ds: %s, %s, heap %lu KB, linear heap %lu KB; the speedup %s; the third core %s",
		n3ds ? "New 3DS" : "3DS", envIsHomebrew() ? "homebrew" : "title",
		(unsigned long)(envGetHeapSize() / 1024), (unsigned long)(envGetLinearHeapSize() / 1024),
		R_SUCCEEDED(sysm) ? "on" : "refused", third ? "free for the game" : probe ? "ran nothing" : "refused");
	if (__3dslink_host.s_addr) {
		u32 *soc = memalign(0x1000, 0x100000);
		if (soc && socInit(soc, 0x100000) == 0) link3dsStdio();
	}
	if (!data_dir_given) snprintf(g_data_dir, sizeof g_data_dir, "sdmc:/3ds/cyberworld-endless");
	static char rom_3ds[600];
	snprintf(rom_3ds, sizeof rom_3ds, "%s/rom", g_data_dir);
	mkdir("sdmc:/3ds", 0777);
	mkdir(g_data_dir, 0777);
	mkdir(rom_3ds, 0777);
	/* (started from the Homebrew Launcher: the output into log.txt beside
	 * the saves, as the handhelds' launcher keeps it) */
	if (!__3dslink_host.s_addr) {
		char log[600];
		snprintf(log, sizeof log, "%s/log.txt", g_data_dir);
		if (freopen(log, "w", stdout)) setvbuf(stdout, NULL, _IOLBF, 0);
		freopen(log, "a", stderr);
	}
	printf("%s\n", mem);
#endif
	if (headless && !force_w) { force_w = 1280; force_h = 960; }
	data_dir_setup(data_dir_given);
	char msg[512];
#ifdef __3DS__
	/* (the game's own rom folder, then where 3DS players keep GBA ROMs: a
	 * header check passes over the other games without reading them) */
	bool rom_ok = false;
	if (rom_dir) rom_ok = rom_find(rom_dir, msg, sizeof msg);
	else {
		const char *places[] = { rom_3ds, "sdmc:/roms/gba", "sdmc:/roms", "sdmc:/gba" };
		char first[512] = "", close[512] = "";
		for (size_t i = 0; !rom_ok && i < sizeof places / sizeof *places; ++i) {
			rom_ok = rom_find(places[i], msg, sizeof msg);
			if (!i) snprintf(first, sizeof first, "%s", msg);
			if (!rom_ok && rom_find_close && !close[0]) snprintf(close, sizeof close, "%s", msg);
		}
		/* (a near miss, the wrong version found, says more than where to put one) */
		if (!rom_ok) snprintf(msg, sizeof msg, "%s", close[0] ? close : first);
	}
#else
	bool rom_ok = start_rom(rom_dir, msg, sizeof msg);
#endif
	/* ("--scene norom": the screen a desktop without its ROM shows, for a
	 * capture) */
	bool norom_scene = start_scene && !strcmp(start_scene, "norom");
	if (norom_scene) rom_ok = false;
#ifdef CW_DESKTOP
	bool big = !headless && desktop_big_screen();
	if (!screen_given && big) fullscreen = true;
	/* the desktop's dialogs come before the window, which would be marked
	 * "not responding" while they wait (and Steam, not a menu entry, starts
	 * the game on its big screen) */
	if (!headless && !big) desktop_menu_entry(g_data_dir);
	if (!headless && !big) desktop_steam_offer(g_data_dir);
	if (!rom_ok && !headless && !rom_dir) {
		char dir[600];
		snprintf(dir, sizeof dir, "%s/rom", g_data_dir);
		/* first where a Steam Deck keeps its ROMs: its Gaming Mode shows no
		 * file chooser a pad can work */
		rom_ok = desktop_rom_elsewhere(dir, msg, sizeof msg);
	}
	/* the desktop's own dialog; none on the big screen, where a pad cannot
	 * answer one, and none in a Flatpak on Wayland: the game's window asks
	 * then (norom_show) */
	if (!rom_ok && !headless && !rom_dir && !big) {
		char dir[600];
		snprintf(dir, sizeof dir, "%s/rom", g_data_dir);
		fprintf(stderr, "%s\n", msg);
		int asked = desktop_rom_dialog(dir, desktop_rom_anywhere, msg, sizeof msg);
		if (asked == 0) return 1;
		rom_ok = asked > 0;
	}
#else
	(void)screen_given;   /* (the handheld fills its screen, the page its canvas) */
#endif
	if (!platform_init(force_w, force_h, headless, fullscreen)) return 1;
	if (!headless) {
		char keys[600];
		snprintf(keys, sizeof keys, "%s/keys.ini", g_data_dir);
		platform_load_keys(keys);
		snprintf(keys, sizeof keys, "%s/settings.ini", g_data_dir);
		platform_load_settings(keys);
		snprintf(keys, sizeof keys, "%s/touch.ini", g_data_dir);
		touch_load(keys);
		if (smooth_arg >= 0) P.blend = smooth_arg;
	}
	rng_seed(seed ? seed : (uint32_t)SDL_GetPerformanceCounter());

	if (!rom_ok) {
		fprintf(stderr, "%s\n", msg);
#ifdef CW_DESKTOP
		/* (a ROM given by --rom-dir that is not one keeps the plain error) */
		if ((!headless && !rom_dir) || norom_scene) norom_show();
		else
#endif
		rom_missing(rom_dir, norom_scene, msg);
	} else if (!gfx_init()) {
		error_show("The ROM could not be decoded.");
	} else {
		printf("ROM: %s (%s)\n", R.layout->name, R.path);
		save_init();
		devtools_veteran();
		if (marks_spec >= 0) profile.marks = (uint16_t)marks_spec;
		if (atlas_spec) {
			int r = atlas_run(atlas_spec);
			platform_shutdown();
			return r;
		}
		if (pacing_spec) {
			int r = pacing_report_run(pacing_spec);
			platform_shutdown();
			return r;
		}
		if (render_spec) {
			int song = 0, secs = 20;
			char out[256] = "song.wav";
			sscanf(render_spec, "%i:%d:%255s", &song, &secs, out);
			bool ok = audio_render_wav(song, secs, out);
			printf("rendered song %d (%ds) to %s: %s\n", song, secs, out, ok ? "ok" : "failed");
			platform_shutdown();
			return ok ? 0 : 1;
		}
		if (sheet_spec && sheet_spec[0] == '@') {
			/* --sheet @CAT:FIRST:COUNT:PATH  frame 0 of anim 0 of many sprites, 48x64 cells */
			int cat = 0, first = 0, count = 64;
			char out[256] = "sprites.bmp";
			sscanf(sheet_spec + 1, "%i:%i:%i:%255s", &cat, &first, &count, out);
			platform_begin_frame();
			fill_rect(0, 0, P.w, P.h, rgba(96, 96, 96, 255));
			/* 32 x 48 cells, or 96 x 128 when the canvas is wide enough to
			 * show the big overworld objects whole (--size 479xH) */
			int cw = P.w >= 384 ? 96 : 32, ch = P.w >= 384 ? 128 : 48, cols = P.w / cw;
			for (int i = 0; i < count; ++i) {
				int cx = (i % cols) * cw, cy = (i / cols) * ch;
				Sprite *spr = sprite_get(cat, first + i);
				fill_rect(cx + 1, cy + 1, cw - 2, ch - 2, rgba(40, 40, 60, 255));
				if (spr) sprite_draw_frame(spr, 0, 0, cx + cw / 2, cy + ch - 8, false, 0, 0);
				if (cw > 32 && R.data) text_drawf(cx + 2, cy + 1, WHITE, TEXT_LEFT, "%x", first + i);
			}
			platform_save_canvas(out);
			platform_shutdown();
			return 0;
		}
		if (sheet_spec) {
			/* --sheet CAT:IDX:ANIM[:PAL]:PATH  every frame of one animation, 64x64 cells */
			int cat = 0, idx = 0, anim = 0, pal = 0;
			char out[256] = "sheet.bmp";
			if (sscanf(sheet_spec, "%i:%i:%i:%i:%255s", &cat, &idx, &anim, &pal, out) < 5)
				sscanf(sheet_spec, "%i:%i:%i:%255s", &cat, &idx, &anim, out);
			Sprite *spr = sprite_get(cat, idx);
			int n = sprite_frame_count(spr, anim);
			platform_begin_frame();
			fill_rect(0, 0, P.w, P.h, rgba(96, 96, 96, 255));
			for (int f = 0; f < n && f < 12; ++f) {
				int cx = (f % 4) * 64, cy = (f / 4) * 64;
				fill_rect(cx + 1, cy + 1, 62, 62, rgba(40, 40, 60, 255));
				sprite_draw_frame(spr, anim, f, cx + 32, cy + 48, false, pal, 0);
			}
			platform_save_canvas(out);
			printf("sheet %d:%d:%d frames %d -> %s\n", cat, idx, anim, n, out);
			platform_shutdown();
			return 0;
		}
		audio_init();
		/* "town": a new run from the town, as NEW GAME starts one */
		bool town = !strcmp(start_scene, "town");
		const Scene *s = town ? &scene_emu : scene_by_name(start_scene);
		int net = town ? RUN_SHORT : RUN_ENDLESS, folder = 0, threat = 0, helpers = 0, cross = 0;
		if (setup_spec) {
			net = !strncmp(setup_spec, "short", 5) ? RUN_SHORT : RUN_ENDLESS;
			const char *c = strchr(setup_spec, ',');
			if (c) sscanf(c + 1, "%d,%d,%d,%d", &folder, &threat, &helpers, &cross);
		}
		/* "setup": the title with the setup after NEW GAME open */
		if (!strcmp(start_scene, "setup")) { title_setup = true; s = &scene_title; }
		/* "summary": the title's summary of a made-up run lost at --run-depth,
		 * or won with --setup short at layer 10 (11 on threat 10; its
		 * unlocks said, and saved in --data-dir) */
		if (!strcmp(start_scene, "summary")) {
			run_new(seed ? seed : 1);
			run_setup(net, folder, threat, helpers, cross);
			run.depth = run_depth > 0 ? run_depth : 12;
			run.viruses_deleted = run.depth * 6;
			run.bosses_beaten = run.depth / 3;
			title_new_best = run.depth > profile.best_depth;
			title_won = run_short_last(run.depth);
			if (title_won) {
				snprintf(title_cause, sizeof title_cause, "on layer %d", run.depth);
				meta_run_over(true);
			} else snprintf(title_cause, sizeof title_cause, "by HeatMan in the Graveyard");
			title_summary = true;
			s = &scene_title;
		}
		if (s == &scene_emu) {
			run_new(seed ? seed : 1);
			run_setup(net, folder, threat, helpers, cross);
			if (run_depth > 0) run.depth = run_depth;
			/* (the area its act's in the run too, whose draws read it, and
			 * every area's guardian one navi: a scripted capture keeps its
			 * run as the areas' draw changes) */
			int p = (run.depth - 1) % CYCLE_LAYERS;
			if (director_debug_biome >= 0 && director_debug_biome < BIOME_COUNT && p < 18) run.biome_order[p / 3] = (uint8_t)director_debug_biome;
			if (guardian_navi > 0) for (int b = 0; b < MAX_BIOMES; ++b) run.boss_order[b] = (uint8_t)guardian_navi;
		}
		/* (later NEW GAMEs take the next seeds, so a session replays) */
		title_seed = !seed ? 0 : !s || s == &scene_title ? seed : seed + 1;
		if (town) emu_start_in_town = true;
		scene_set(s ? s : &scene_title);
	}

#if defined(_WIN32)
	signal(SIGTERM, on_quit_signal);
	signal(SIGINT, on_quit_signal);
#elif !defined(__EMSCRIPTEN__) && !defined(__3DS__)
	/* (no SA_RESTART: a remote game waiting in read() wakes up to quit) */
	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = on_quit_signal;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	if (remote_dir && !remote_open(remote_dir)) { fprintf(stderr, "--remote: cannot open the pipes in %s\n", remote_dir); return 1; }
#endif
	loop.headless = headless;
	loop.max_frames = max_frames;
	loop.last = SDL_GetPerformanceCounter();
#ifdef __EMSCRIPTEN__
	/* the browser calls in once per display frame */
	emscripten_set_main_loop(web_frame, 0, 1);
#else
	while (!P.quit && !quit_signal && step()) {}
	/* a run on a layer is kept where MegaMan stands */
	if (current == &scene_emu) director_suspend();
	emu_quit();
#endif
	platform_shutdown();
	return 0;
}
