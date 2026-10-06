/* The game's start and its frame loop: the command line, the platform
 * and the ROMs set up, the first scene, and each frame's scenes, input,
 * update, sound and drawing, at the GBA's pace. */
#include "game.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "audio.h"
#include "capture.h"
#include "controls.h"
#include "desktop.h"
#include "devtools.h"
#include "director.h"
#include "emu.h"
#include "gfx.h"
#include "guest.h"
#include "meta.h"
#include "net_layouts.h"
#include "platform.h"
#include "rom.h"
#include "run.h"
#include "save.h"
#include "start_3ds.h"
#include "startup.h"
#include "tools.h"
#include "touch.h"
#include "tour.h"

char g_data_dir[512] = ".";

static const Scene *current, *pending;

const Scene *scene_current(void) { return current; }

void scene_set(const Scene *s) { pending = s; }

static const Scene *scene_by_name(const char *n) {
	/* (the launcher is a start's own: --scene launcher opens it, then the title) */
	const Scene *all[] = { &scene_title, &scene_intro, &scene_gallery, &scene_emu };
	for (size_t i = 0; i < sizeof all / sizeof *all; ++i)
		if (!strcmp(all[i]->name, n)) return all[i];
	return NULL;
}

/* The first scene: the intro, then the title, at a plain start; a scene
 * asked for (--scene) and a headless run begin where they did, so scripted
 * captures and playtests keep their frames. */
static bool scene_given;

static const Scene *first_scene(const Scene *s, bool headless) {
	if (!s) s = &scene_title;
	return s == &scene_title && !scene_given && !headless ? &scene_intro : s;
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

/* The frame's spare time for BN5's boot where it runs a slice a frame
 * (guest_tick: the browser's), in milliseconds: the frame's work and the
 * slice within FRAME_WORK_MS of its 1/60 s together, the rest left to the
 * display and the page (8 ms slices at the title and 12 ms behind a waiting
 * battle's note kept the page's frames at 16.7 ms, docs/MULTIROM.md); more
 * while a battle waits for it, the frame having nothing else to do; none
 * in a frame played unshown */
#define FRAME_WORK_MS 10
#define FRAME_WAIT_MS 12
static int boot_spare_ms(uint64_t t0) {
	if (P.skip_present) return 0;
	int used = (int)((SDL_GetPerformanceCounter() - t0) * 1000 / SDL_GetPerformanceFrequency()), most = guest_boot_waiting() ? FRAME_WAIT_MS : FRAME_WORK_MS;
	return used < most ? most - used : 0;
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
	capture_input();
	platform_poll();
	capture_taps();
	/* (the touch controls' menu and the controls screen pause the game
	 * under them) */
	if (controls_shown()) controls_update();
	else if (current && current->update && !touch_paused()) current->update();
	/* (BN5's boot begun as the game starts, at its boot screen or title,
	 * as early as it can be: guest.h) */
	if (current != &scene_emu) guest_warm();
	audio_frame();
	uint64_t t1 = SDL_GetPerformanceCounter();
	platform_begin_frame();
	if (current && current->draw) current->draw();
	platform_apply_effects();
	controls_draw();
	quit_prompt_draw();
	if (devtools_shot[0]) { platform_save_canvas(devtools_shot); devtools_shot[0] = 0; }
	capture_shots();
	platform_frame_parts(t1 - t0, SDL_GetPerformanceCounter() - t1);
	/* (and BN5's boot in the frame's spare time where it runs a slice a
	 * frame; where it has a thread, its end taken in) */
	guest_tick(boot_spare_ms(t0));
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
	/* (the page's menu open: nothing runs, and the time it held is not caught up) */
	if (P.paused) { loop.acc = 0; return; }
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
/* The game's end: BN5's boot stopped where its thread still runs, and
 * BN6's core's thread (emu.c) */
static void cores_quit(void) {
	guest_quit();
	emu_quit();
}

/* SIGTERM or SIGINT (a launcher closing the port, a terminal's Ctrl+C):
 * the loop ends at a frame's end and the run is kept */
static volatile sig_atomic_t quit_signal;
#ifndef __3DS__
static void on_quit_signal(int sig) { (void)sig; quit_signal = 1; }
#endif
#endif

/* What the command line asks for (main) */
typedef struct {
	const char *rom_dir;
	bool fullscreen, data_dir_given, screen_given, headless;
	const char *start_scene;
	int run_depth, guardian_navi;
	int smooth_arg;   /* --smooth-motion on|off, over settings.ini */
	/* --setup NET,FOLDER,THREAT,HELPERS[,CROSS] (short or endless, then
	 * numbers; CROSS the navi whose Cross the run brings, 1-5):
	 * the run's setup as the setup screen chooses it; NEW GAME's (the
	 * short net) for --scene town, the endless net otherwise */
	const char *setup_spec;
	int marks_spec;
	int force_w, force_h;
	uint64_t max_frames;
	uint32_t seed;
	/* --launcher auto, open or off (LAUNCHER_*): the ROMs' screen before
	 * the game (src/launcher/); -1 the platform's own way (main) */
	int launcher;
	/* a test's or a developer's start (a scene, a seed, --dev, a capture's
	 * options): no player's, which the anonymous statistics count */
	bool dev;
} Options;

/* (the start's options, for the launcher's PLAY: game_begin) */
static Options opts;

/* Each group of the command line's options takes argument `a` (its value
 * `v`, NULL after the last) where it is one of its own: the arguments it
 * took, else 0. Where the game runs, and for how long: */
static int start_option(const char *a, const char *v, Options *o) {
	if (!strcmp(a, "--headless")) { o->headless = true; return 1; }
	if (!v) return 0;
	if (!strcmp(a, "--rom-dir")) o->rom_dir = v;
	else if (!strcmp(a, "--data-dir")) { snprintf(g_data_dir, sizeof g_data_dir, "%s", v); o->data_dir_given = true; }
	else if (!strcmp(a, "--frames")) o->max_frames = strtoull(v, NULL, 10);
	else return 0;
	return 2;
}

/* ... the screen and the controls: */
static int screen_option(const char *a, const char *v, Options *o) {
	if (!strcmp(a, "--fullscreen") || !strcmp(a, "--window")) { o->fullscreen = a[2] == 'f'; o->screen_given = true; return 1; }
	/* the touch controls from the start (the browser on a phone) */
	if (!strcmp(a, "--touch")) { touch_always(); return 1; }
	/* (the frame log without an environment: the 3DS over 3dslink) */
	if (!strcmp(a, "--frame-log")) { platform_frame_log = true; return 1; }
	if (!v) return 0;
	if (!strcmp(a, "--size")) sscanf(v, "%dx%d", &o->force_w, &o->force_h);
	else if (!strcmp(a, "--smooth-motion")) o->smooth_arg = !strcmp(v, "on");
	/* (the screen's density for the touch controls, in dots per inch) */
	else if (!strcmp(a, "--dpi")) platform_set_dpi((float)atof(v));
	else return 0;
	return 2;
}

/* ... the first scene and the run it starts on: */
static int run_option(const char *a, const char *v, Options *o) {
	if (!v) return 0;
	if (!strcmp(a, "--scene")) { o->start_scene = v; scene_given = true; }
	else if (!strcmp(a, "--seed")) o->seed = (uint32_t)strtoul(v, NULL, 0);
	else if (!strcmp(a, "--run-depth")) o->run_depth = atoi(v);
	else if (!strcmp(a, "--net-biome")) director_net_biome_arg(v);
	else if (!strcmp(a, "--guardian")) o->guardian_navi = atoi(v);
	else if (!strcmp(a, "--setup")) o->setup_spec = v;
	/* --marks HEX: the title's marks as if earned, for a capture */
	else if (!strcmp(a, "--marks")) o->marks_spec = (int)strtol(v, NULL, 16);
	else if (!strcmp(a, "--talk")) director_dev_talks = v;
	else if (!strcmp(a, "--net-layout")) layout_forced = atoi(v);
	else if (!strcmp(a, "--dev")) devtools_parse(v);
	else if (!strcmp(a, "--tour")) { tour_parse(v); o->start_scene = "emu"; }
	else if (!strcmp(a, "--launcher")) o->launcher = !strcmp(v, "open") ? LAUNCHER_OPEN : !strcmp(v, "off") ? LAUNCHER_OFF : LAUNCHER_AUTO;
	else return 0;
	return 2;
}

/* The command line into `o` (headless runs' own options, capture.c, and
 * the dev tools', tools.c); -1 to go on, else the exit code: an unknown
 * argument's, or the Steam shortcut's command's */
static int parse_args(int argc, char **argv, Options *o) {
	for (int i = 1; i < argc; ++i) {
		const char *a = argv[i];
		const char *v = i + 1 < argc ? argv[i + 1] : NULL;
#ifdef CW_DESKTOP
		/* (linux/steam/add-to-steam.py, which the Linux builds carry) */
		if (!strcmp(a, "--add-to-steam") || !strcmp(a, "--remove-from-steam")) return desktop_steam_command(a[2] == 'r');
#endif
		int took = start_option(a, v, o);
		if (!took) took = screen_option(a, v, o);
		if (!took && (took = run_option(a, v, o)) != 0 && strcmp(a, "--launcher")) o->dev = true;
		if (!took && v && (capture_option(a, v) || tools_option(a, v))) { took = 2; o->dev = true; }
		if (!took) {
			fprintf(stderr, "unknown argument %s\n", a);
			return 2;
		}
		i += took - 1;
	}
	return -1;
}

/* The ROM at the start, `msg` where there is none */
static bool rom_at_start(const Options *o, char *msg, size_t n) {
#ifdef __3DS__
	return start_3ds_rom(o->rom_dir, msg, n);
#else
	return start_rom(o->rom_dir, msg, n);
#endif
}

#ifdef CW_DESKTOP
/* A desktop's start, before its window: the big screen filled, the menu
 * entry and Steam's offered, and the ROM looked for where front ends and
 * downloads keep theirs (a copy kept); where there is none, the launcher
 * asks for it in the game's own window */
static void desktop_start(Options *o, bool *rom_ok, char *msg, size_t n) {
	bool big = !o->headless && desktop_big_screen();
	if (!o->screen_given && big) o->fullscreen = true;
	/* the desktop's dialogs come before the window, which would be marked
	 * "not responding" while they wait (and Steam, not a menu entry, starts
	 * the game on its big screen) */
	if (!o->headless && !big) desktop_menu_entry(g_data_dir);
	if (!o->headless && !big) desktop_steam_offer(g_data_dir);
	if (!*rom_ok && !o->headless && !o->rom_dir) {
		char dir[600];
		snprintf(dir, sizeof dir, "%s/rom", g_data_dir);
		/* (where a Steam Deck keeps its ROMs: its Gaming Mode shows no file
		 * chooser a pad can work) */
		*rom_ok = desktop_rom_elsewhere(dir, msg, n);
	}
}
#endif

/* The first scene, and the run it starts on: --scene's ("town" a new run
 * from the town as NEW GAME starts one, "setup" the title's setup,
 * "summary" a made-up run's), --setup's run, --run-depth, --guardian */
static void start_scene(const Options *o) {
	audio_init();
	/* "town": a new run from the town, as NEW GAME starts one */
	bool town = !strcmp(o->start_scene, "town");
	/* "home": a run at home before the act at --run-depth (4), as an act's
	 * exit takes it (docs/HOME.md) */
	bool home = !strcmp(o->start_scene, "home");
	const Scene *s = town || home ? &scene_emu : scene_by_name(o->start_scene);
	int net = town ? RUN_SHORT : RUN_ENDLESS, folder = 0, threat = 0, helpers = 0, cross = 0;
	if (o->setup_spec) {
		net = !strncmp(o->setup_spec, "short", 5) ? RUN_SHORT : RUN_ENDLESS;
		const char *c = strchr(o->setup_spec, ',');
		if (c) sscanf(c + 1, "%d,%d,%d,%d", &folder, &threat, &helpers, &cross);
	}
	/* "setup": the title with the setup after NEW GAME open */
	if (!strcmp(o->start_scene, "setup")) { title_setup = true; s = &scene_title; }
	/* "summary": the title's summary of a made-up run lost at --run-depth,
	 * or won with --setup short at layer 10 (11 on threat 10; its
	 * unlocks said, and saved in --data-dir) */
	if (!strcmp(o->start_scene, "summary")) {
		run_new(o->seed ? o->seed : 1);
		run_setup(net, folder, threat, helpers, cross);
		run.depth = o->run_depth > 0 ? o->run_depth : 12;
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
		run_new(o->seed ? o->seed : 1);
		run_setup(net, folder, threat, helpers, cross);
		if (o->run_depth > 0) run.depth = o->run_depth;
		else if (home) run.depth = 4;
		run.clock = (uint8_t)dev.clock;   /* (--dev clock=N, docs/HOME.md) */
		jobs_dev(&run.job, run.seed, run.depth, home, dev.job, dev.job_state);   /* (--dev job=K,jobstate=S) */
		/* (the area its act's in the run too, whose draws read it, and
		 * every area's guardian one navi: a scripted capture keeps its
		 * run as the areas' draw changes) */
		int p = (run.depth - 1) % CYCLE_LAYERS;
		if (director_debug_biome >= 0 && director_debug_biome < BIOME_COUNT && p < 18) run.biome_order[p / 3] = (uint8_t)director_debug_biome;
		if (o->guardian_navi > 0) run_debug_guardian(o->guardian_navi);
	}
	/* (later NEW GAMEs take the next seeds, so a session replays) */
	title_seed = !o->seed ? 0 : !s || s == &scene_title ? o->seed : o->seed + 1;
	if (town) emu_start_in_town = true;
	if (home) emu_start_at_home = true;
	scene_set(first_scene(s, o->headless));
}

/* The game from the ROM read: its font, the saves, the dev tools' veteran
 * and marks; false where it cannot be decoded (the error says so) */
static bool game_ready(const Options *o) {
	if (!gfx_init()) {
		error_show("The ROM could not be decoded.");
		return false;
	}
	say_roms();
	save_init();
	devtools_veteran();
	if (o->marks_spec >= 0) profile.marks = (uint16_t)o->marks_spec;
	return true;
}

/* PLAY on the launcher at the start: the game, as a start with its ROM
 * goes on (no dev tool: the launcher shows at a plain start alone) */
static void game_begin(void) {
	if (game_ready(&opts)) start_scene(&opts);
}

/* The game on the ROM found (`rom_ok`, else `msg` says why): the launcher
 * where it is wanted (src/launcher/), the error, a dev tool, or the first
 * scene; -1 to go on, else the exit code of a tool that ran */
static int game_start(const Options *o, bool rom_ok, const char *msg) {
	opts = *o;
	if (launcher_start(o->launcher, o->rom_dir, game_begin)) return -1;
	if (!rom_ok) {
		fprintf(stderr, "%s\n", msg);
		error_show(msg);
		return -1;
	}
	if (!game_ready(o)) return -1;
	int r = tools_run();
	if (r >= 0) {
		platform_shutdown();
		return r;
	}
	start_scene(o);
	return -1;
}

/* The frame loop, to its end: quit signals heard, remote play's pipes
 * opened, and the run kept where MegaMan stands; the exit code */
static int frame_loop(const Options *o) {
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
#endif
	if (!capture_start()) return 1;
	loop.headless = o->headless;
	loop.max_frames = o->max_frames;
	loop.last = SDL_GetPerformanceCounter();
#ifdef __EMSCRIPTEN__
	/* the browser calls in once per display frame */
	emscripten_set_main_loop(web_frame, 0, 1);
#else
	while (!P.quit && !quit_signal && step()) {}
	/* a run on a layer is kept where MegaMan stands */
	if (current == &scene_emu) director_suspend();
	cores_quit();
#endif
	platform_shutdown();
	return 0;
}

/* The launcher at a start where none was asked for: a desktop's and an
 * iPhone's own ROM places (not a --rom-dir's: the browser's page, a
 * PortMaster handheld's launcher), never headless; Android's app asks with
 * --launcher. "--scene launcher" opens it (a capture). */
static int launcher_mode(const Options *o) {
	if (o->start_scene && !strcmp(o->start_scene, "launcher")) return LAUNCHER_OPEN;
	if (o->launcher >= 0) return o->headless ? LAUNCHER_OFF : o->launcher;
	return (DESKTOP || IOS) && !o->rom_dir && !o->headless ? LAUNCHER_AUTO : LAUNCHER_OFF;
}

int main(int argc, char **argv) {
	Options o = { .fullscreen = !DESKTOP, .start_scene = "title", .smooth_arg = -1, .marks_spec = -1, .launcher = -1 };
	int code = parse_args(argc, argv, &o);
	if (code >= 0) return code;
	setvbuf(stdout, NULL, _IOLBF, 0);
#ifdef __3DS__
	start_3ds(o.data_dir_given);
#endif
	if (o.headless && !o.force_w) { o.force_w = 1280; o.force_h = 960; }
	data_dir_setup(o.data_dir_given);
	char msg[512];
	bool rom_ok = rom_at_start(&o, msg, sizeof msg);
#ifdef CW_DESKTOP
	desktop_start(&o, &rom_ok, msg, sizeof msg);
#endif
	o.launcher = launcher_mode(&o);
	if (!platform_init(o.force_w, o.force_h, o.headless, o.fullscreen)) return 1;
	player_files(o.headless, o.smooth_arg, !o.dev);
	rng_seed(o.seed ? o.seed : (uint32_t)SDL_GetPerformanceCounter());
	code = game_start(&o, rom_ok, msg);
	return code >= 0 ? code : frame_loop(&o);
}
