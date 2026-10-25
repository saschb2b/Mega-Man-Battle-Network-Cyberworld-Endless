/* The start's data folder and ROMs: where a desktop keeps its files, the
 * ROM looked for there, beside the binary and in ./rom, the ROMs found
 * said, and the player's files read from the folder. */
#include "startup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analytics_net.h"
#include "backup.h"
#include "capture.h"
#include "compat.h"
#include "desktop.h"
#include "display.h"
#include "devtools.h"
#include "fast_forward.h"
#include "feel_out.h"
#include "game.h"
#include "pads.h"
#include "platform.h"
#include "options_text.h"
#include "pick.h"
#include "rom.h"
#include "sound_filter.h"
#include "sound_heard.h"
#include "touch.h"
#ifdef CW_IOS
#include "ios.h"
#endif
#ifdef __ANDROID__
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>

static int output_pipe[2];

/* (Android throws an app's stdout and stderr away: a thread hands them on
 * to logcat a line at a time) */
static void *output_pump(void *arg) {
	(void)arg;
	FILE *in = fdopen(output_pipe[0], "r");
	char line[1024];
	while (in && fgets(line, sizeof line, in)) {
		size_t n = strlen(line);
		if (n && line[n - 1] == '\n') line[n - 1] = 0;
		__android_log_write(ANDROID_LOG_INFO, "Cyberworld", line);
	}
	return NULL;
}
#endif

void startup_output(void) {
#ifdef __ANDROID__
	pthread_t pump;
	if (pipe(output_pipe) != 0) return;
	setvbuf(stderr, NULL, _IOLBF, 0);
	dup2(output_pipe[1], 1);
	dup2(output_pipe[1], 2);
	if (pthread_create(&pump, NULL, output_pump, NULL) == 0) pthread_detach(pump);
#endif
}

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
bool desktop_rom_anywhere(char *msg, size_t msglen) {
	if (desktop_rom(msg, msglen)) return true;
	char dir[600];
	snprintf(dir, sizeof dir, "%s/rom", g_data_dir);
	return desktop_rom_elsewhere(dir, msg, msglen);
}
#endif

/* The ROMs read, said at the start: BN6's, and the other games' found
 * beside it (Android's log takes SDL_Log, not stdout). */
void say_roms(void) {
	printf("ROM: %s (%s)\n", R.layout->name, R.path);
	for (int i = 0; i < XROM_COUNT; ++i)
		if (XR[i].data) printf("%s found: %s (%s)\n", XR[i].layout->tag, XR[i].layout->name, XR[i].path);
#ifdef __ANDROID__
	SDL_Log("ROM: %s (%s)", R.layout->name, R.path);
	for (int i = 0; i < XROM_COUNT; ++i)
		if (XR[i].data) SDL_Log("%s found: %s (%s)", XR[i].layout->tag, XR[i].layout->name, XR[i].path);
#endif
}

/* The data folder, where none is given, and its rom/: a desktop's in the
 * user's data folders; iOS's the app's Documents, which Files shows as On
 * My iPhone › Cyberworld (its display name), the ROM's place, beside the
 * saves, which a player can copy off there; a handheld's the launcher's. */
void data_dir_setup(bool given) {
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
	/* (an import a power cut stopped put right before anything, the ROMs
	 * screen first, reads the saves: backup.h) */
	backup_recover(g_data_dir);
}

#ifdef CW_IOS
/* iOS's ROMs at the start: the copies in the app's rom/ (where a pick
 * lands) or at the top of its folder (where Files puts a file dropped on
 * it), BN5 read with BN6 from either; then the folder picked looked in for
 * what they lack (BN5 put there since, or BN6 where none is kept) */
static bool ios_rom_here(const char *dir, char *msg, size_t msglen) {
	char first[512];
	bool found = rom_find(dir, msg, msglen);
	snprintf(first, sizeof first, "%s", msg);
	if (!found) found = rom_find(g_data_dir, msg, msglen);
	if (found) {
		xrom_find(dir);
		xrom_find(g_data_dir);
		return true;
	}
	/* (a .gba that is not the right one says so; else where to put it) */
	if (strncmp(first, "Put your", 8)) snprintf(msg, msglen, "%s", first);
	return false;
}

static bool ios_rom_start(char *msg, size_t msglen) {
	char dir[600], said[1024];
	snprintf(dir, sizeof dir, "%s/rom", g_data_dir);
	bool found = ios_rom_here(dir, msg, msglen);
	unsigned want = (found ? 0 : IOS_ROM_BN6) | (XR[XROM_BN5_COLONEL_US].data ? 0 : IOS_ROM_BN5);
	if (want && ios_rom_folder_look(dir, want, said, sizeof said) > 0) {
		if (!found) found = ios_rom_here(dir, msg, msglen);
		else xrom_find(dir);
	}
	return found;
}
#endif

#ifdef __ANDROID__
/* Android's ROMs at the start: the copies in the app's ROM folder; the
 * folder the player chose in the launcher looked in again for what they
 * lack (a quick look: only a .gba not seen before is opened), so BN5 put
 * there since comes in by itself */
static bool android_rom_start(const char *rom_dir, char *msg, size_t msglen) {
	bool found = rom_find(rom_dir, msg, msglen);
	if (found && XR[XROM_BN5_COLONEL_US].data) return true;
	char said[1024];
	if (pick_look(said, sizeof said) > 0) {
		if (!found) found = rom_find(rom_dir, msg, msglen);
		else xrom_find(rom_dir);
	}
	return found;
}
#endif

#ifndef __3DS__
/* The ROM at the start (the 3DS has its own places, main): --rom-dir's,
 * else the desktop's, iOS's app folder, or ./rom on a handheld. */
bool start_rom(const char *rom_dir, char *msg, size_t msglen) {
#ifdef CW_IOS
	if (!rom_dir) return ios_rom_start(msg, msglen);
#endif
#ifdef __ANDROID__
	if (rom_dir) return android_rom_start(rom_dir, msg, msglen);
#endif
	return rom_dir || !DESKTOP ? rom_find(rom_dir ? rom_dir : "rom", msg, msglen) : desktop_rom(msg, msglen);
}
#endif

bool startup_fullscreen(bool big) {
	int kept = display_fullscreen_kept(g_data_dir);
	return kept >= 0 ? kept == 1 : big;
}

/* settings.ini in the data folder, made with the defaults where it is
 * missing: the display's lines (display.h), the sound, the music and
 * the effects heard (sound_heard.h), fast forward's speed (fast_forward.h),
 * the controller's vibration and trigger effects (feel_out.h), the frame
 * log */
static void settings_load(const char *path) {
	FILE *f = fopen(path, "r");
	if (!f) {
		f = fopen(path, "w");
		if (!f) return;
		fprintf(f, "# Cyberworld Endless: settings. Delete this file for the defaults.\n%ssmooth_motion = off\n%sscreen = auto\n%ssound = gba\n"
			"%smusic = on\n%seffects = on\n%sfast_forward = %s\n%svibration = %s\n%strigger_effects = on\n",
			options_smooth_note(), options_screen_note(), options_sound_note(), options_music_note(), options_effects_note(),
			options_fast_note(), fast_speed_word(FAST_DEFAULT), options_vibration_note(), feel_level_word(FEEL_DEFAULT), options_triggers_note());
		fclose(f);
		platform_persist();
		return;
	}
	char line[256];
	while (fgets(line, sizeof line, f)) {
		char key[64], val[64];
		if (line[0] == '#' || sscanf(line, " %63[a-z_] = %63s", key, val) != 2 || display_setting(key, val)) continue;
		if (!strcmp(key, "sound")) sound_filter_set(!strcmp(val, "soft") ? SOUND_SOFT : SOUND_GBA);
		bool on = strcmp(val, "off") && strcmp(val, "no") && strcmp(val, "0");
		if (!strcmp(key, "music")) sound_heard_set(on, sound_heard_effects());
		if (!strcmp(key, "effects")) sound_heard_set(sound_heard_music(), on);
		if (!strcmp(key, "fast_forward")) fast_speed_set(fast_speed_read(val));
		if (!strcmp(key, "vibration")) feel_set_level(feel_level_read(val));
		if (!strcmp(key, "trigger_effects")) feel_set_triggers(on);
		/* (not written by default: for a report of a machine's pacing) */
		if (!strcmp(key, "frame_log")) platform_frame_log = !strcmp(val, "on") || !strcmp(val, "yes") || !strcmp(val, "1");
	}
	fclose(f);
	/* (the screen setting read after the window opened; logged where it
	 * changed the scale, as the display line before it was printed with
	 * the default's) */
	if (P.renderer && !P.headless) platform_relayout();
}

void player_files(bool headless, int smooth_arg, int sound_arg, int music_arg, int effects_arg, const char *screen_arg, bool plain) {
	char path[600];
	const char *pad_kind = capture_pad_kind();
	if (pad_kind && !pads_virtual(pad_kind)) fprintf(stderr, "--pad %s: no such virtual controller here\n", pad_kind);
	if (!headless || pad_kind) {
		snprintf(path, sizeof path, "%s/pad.ini", g_data_dir);
		pads_load(path);
	}
	if (!headless) {
		snprintf(path, sizeof path, "%s/keys.ini", g_data_dir);
		platform_load_keys(path);
		snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
		settings_load(path);
		snprintf(path, sizeof path, "%s/touch.ini", g_data_dir);
		touch_load(path);
		if (smooth_arg >= 0) P.blend = smooth_arg;
	}
	if (sound_arg >= 0) sound_filter_set((SoundFilter)sound_arg);
	if (music_arg >= 0) sound_heard_set(music_arg, sound_heard_effects());
	if (effects_arg >= 0) sound_heard_set(sound_heard_music(), effects_arg);
	if (screen_arg) display_scale_use(display_scale_read(screen_arg));
	/* (the vibration as settings.ini has it, or its default where an older
	 * one has no line: SDL asked for a Bluetooth pad's rumble either way) */
	feel_set_level(feel_level());
	feel_trace(dev.feel);
	/* (the anonymous statistics' answer, in settings.ini, and the start's
	 * view where it is yes: analytics.h) */
	analytics_net_start(headless, plain);
	/* (BN5's boot state, which its battles on a second core kept beside the
	 * saves until issue #82 brought every battle into BN6's engine: gone,
	 * once, from a data folder that has it; docs/MULTIROM.md) */
	snprintf(path, sizeof path, "%s/guest-bn5-1.state", g_data_dir);
	if (remove(path) == 0) {
		printf("startup: the older net's boot state, unused now, removed\n");
		platform_persist();
	}
}
