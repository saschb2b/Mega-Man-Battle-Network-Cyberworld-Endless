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

char g_data_dir[512] = ".";

/* The desktop builds (host, linux) open a window and keep their files in
 * the user's data folder; the handheld port fills the screen and keeps them
 * beside itself (its launcher passes --data-dir and --rom-dir). */
#ifdef CW_DESKTOP
#define DESKTOP true
#else
#define DESKTOP false
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
	char lines[12][64];
	int n = wrap_lines(error_msg, 54, lines, 12);
	minifont_draw_centered(P.w / 2, 24, "CYBERWORLD ENDLESS", rgba(120, 200, 248, 255), 2);
	for (int i = 0; i < n; ++i) minifont_draw_centered(P.w / 2, 56 + i * 8, lines[i], WHITE, 1);
	minifont_draw_centered(P.w / 2, P.h - 20, "START OR B: QUIT", rgba(160, 170, 200, 255), 1);
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

#if !defined(__EMSCRIPTEN__) && !defined(_WIN32)
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
#if !defined(__EMSCRIPTEN__) && !defined(_WIN32)
	if (remote_in >= 0) remote_tick();
#endif
	script_actions();
	if (script_pos >= script_len) { platform_inject(0); return; }
	platform_inject(script[script_pos].buttons);
	if (--script_left <= 0 && ++script_pos < script_len) script_left = script[script_pos].frames;
}

typedef struct { uint64_t frame; char path[256]; } Shot;
static Shot shots[64];
static int shot_count;
static uint64_t range_a = 1, range_b = 0;   /* --shot-range A:B:PREFIX */
static char range_prefix[200];

static void parse_shots(const char *spec) {
	char *copy = strdup(spec);
	for (char *tok = strtok(copy, ","); tok && shot_count < 64; tok = strtok(NULL, ",")) {
		char *colon = strchr(tok, ':');
		if (!colon) continue;
		shots[shot_count].frame = strtoull(tok, NULL, 10);
		snprintf(shots[shot_count].path, sizeof shots[shot_count].path, "%s", colon + 1);
		++shot_count;
	}
	free(copy);
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
	if (saved) text_draw(P.w / 2, y + 14, director_can_suspend() ? "Your run is saved right here" : "Run saved at layer start",
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
	script_tick();
	platform_poll();
	if (current && current->update) current->update();
	audio_frame();
	platform_begin_frame();
	if (current && current->draw) current->draw();
	platform_apply_effects();
	quit_prompt_draw();
	touch_draw();
	if (devtools_shot[0]) { platform_save_canvas(devtools_shot); devtools_shot[0] = 0; }
	for (int i = 0; i < shot_count; ++i)
		if (shots[i].frame == P.frame) platform_save_canvas(shots[i].path);
	if (P.frame >= range_a && P.frame <= range_b) {
		char path[256];
		snprintf(path, sizeof path, "%s%05llu.bmp", range_prefix, (unsigned long long)P.frame);
		platform_save_canvas(path);
	}
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

#ifndef __EMSCRIPTEN__
/* The native loop: 60 game frames a second (as fast as it can headless). */
static bool step(void) {
	if (!loop.headless || (getenv("CYBERWORLD_AUDIO_DUMP") && !audio_offline())) {
		loop.acc += elapsed();
		if (loop.acc < 1.0 / 60.0 - 0.002) { SDL_Delay(1); return true; }
		loop.acc -= 1.0 / 60.0;
		if (loop.acc > 0.1) loop.acc = 0;
	}
	return game_frame();
}
#else
/* The browser's frame (60, 120 or 144 a second): as many game frames as
 * 1/60 s steps have passed, at most two, so the game keeps the GBA's pace. */
static void web_frame(void) {
	loop.acc += elapsed();
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
static void on_quit_signal(int sig) { (void)sig; quit_signal = 1; }
#endif

int main(int argc, char **argv) {
#ifdef CW_DESKTOP
	g_argv = argv;
#endif
	const char *rom_dir = NULL;
	bool fullscreen = !DESKTOP, data_dir_given = false, screen_given = false;
	const char *start_scene = "title";
	int run_depth = 0, guardian_navi = 0;
	/* --setup NET,FOLDER,THREAT,HELPERS (short or endless, then numbers):
	 * the run's setup as the setup screen chooses it; NEW GAME's (the
	 * short net) for --scene town, the endless net otherwise */
	const char *setup_spec = NULL;
	int marks_spec = -1;
	int force_w = 0, force_h = 0;
	bool headless = false;
#if !defined(__EMSCRIPTEN__) && !defined(_WIN32)
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
		else if (!strcmp(a, "--input") && v) { parse_script(v); ++i; }
		else if (!strcmp(a, "--shot") && v) { parse_shots(v); ++i; }
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
		else if (!strcmp(a, "--net-biome") && v) { director_debug_biome = atoi(v); ++i; }
		else if (!strcmp(a, "--guardian") && v) { guardian_navi = atoi(v); ++i; }
		else if (!strcmp(a, "--setup") && v) { setup_spec = v; ++i; }
		/* --marks HEX: the title's marks as if earned, for a capture */
		else if (!strcmp(a, "--marks") && v) { marks_spec = (int)strtol(v, NULL, 16); ++i; }
		else if (!strcmp(a, "--talk") && v) { director_dev_talks = v; ++i; }
#if !defined(__EMSCRIPTEN__) && !defined(_WIN32)
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
	if (headless && !force_w) { force_w = 1280; force_h = 960; }
	if (DESKTOP && !data_dir_given) desktop_data_dir(g_data_dir, sizeof g_data_dir);
	if (DESKTOP) { char rom[600]; snprintf(rom, sizeof rom, "%s/rom", g_data_dir); make_dirs(rom); }
	char msg[512];
	bool rom_ok = rom_dir || !DESKTOP ? rom_find(rom_dir ? rom_dir : "rom", msg, sizeof msg) : desktop_rom(msg, sizeof msg);
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
	}
	rng_seed(seed ? seed : (uint32_t)SDL_GetPerformanceCounter());

	if (!rom_ok) {
		fprintf(stderr, "%s\n", msg);
#ifdef CW_DESKTOP
		/* (a ROM given by --rom-dir that is not one keeps the plain error) */
		if ((!headless && !rom_dir) || norom_scene) norom_show();
		else
#endif
		error_show(msg);
	} else if (!gfx_init()) {
		error_show("The ROM could not be decoded.");
	} else {
		printf("ROM: %s (%s)\n", R.layout->name, R.path);
		save_init();
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
		int net = town ? RUN_SHORT : RUN_ENDLESS, folder = 0, threat = 0, helpers = 0;
		if (setup_spec) {
			net = !strncmp(setup_spec, "short", 5) ? RUN_SHORT : RUN_ENDLESS;
			const char *c = strchr(setup_spec, ',');
			if (c) sscanf(c + 1, "%d,%d,%d", &folder, &threat, &helpers);
		}
		/* "setup": the title with the setup after NEW GAME open */
		if (!strcmp(start_scene, "setup")) { title_setup = true; s = &scene_title; }
		/* "summary": the title's summary of a made-up run lost at --run-depth,
		 * or won with --setup short at layer 10 (its unlocks said, and saved
		 * in --data-dir) */
		if (!strcmp(start_scene, "summary")) {
			run_new(seed ? seed : 1);
			run_setup(net, folder, threat, helpers);
			run.depth = run_depth > 0 ? run_depth : 12;
			run.viruses_deleted = run.depth * 6;
			run.bosses_beaten = run.depth / 3;
			title_new_best = run.depth > profile.best_depth;
			title_won = run_short_nest(run.depth);
			if (title_won) {
				snprintf(title_cause, sizeof title_cause, "on layer %d", run.depth);
				meta_run_over(true);
			} else snprintf(title_cause, sizeof title_cause, "by HeatMan in the Graveyard");
			title_summary = true;
			s = &scene_title;
		}
		if (s == &scene_emu) {
			run_new(seed ? seed : 1);
			run_setup(net, folder, threat, helpers);
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
#elif !defined(__EMSCRIPTEN__)
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
#endif
	platform_shutdown();
	return 0;
}
