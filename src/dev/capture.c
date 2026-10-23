/* Headless runs' input and pictures (build.py shot, tools/play.py):
 * scripted buttons, fingers and keys, the random-input bot, remote play's
 * batches over a pipe, and the canvas, the screen and the second screen
 * saved at given frames. */
#include "capture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(__EMSCRIPTEN__) && !defined(_WIN32) && !defined(__3DS__)
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "debug.h"
#include "devtools.h"
#include "director.h"
#include "director_hold.h"
#include "flags.h"
#include "game.h"
#include "netpass_screen.h"
#include "online_chat.h"
#include "padmap.h"
#include "pads.h"
#include "platform.h"
#include "screen_name.h"
#include "soulproxy.h"

/* ---- scripted input and captures for headless tests ---- */
/* A step holds buttons for some frames; one of no frames takes a picture
 * or writes the state instead (remote play). */
typedef struct { int frames; uint32_t buttons; uint64_t pad; int key; char shot[160], second[160], state[160], dump[160], foes[64], status[48], xrecord[32], chip[24], trade[40], reconnect[16]; int place[3], flags[3], stap[3], cross, folder, netpass, emote; unsigned hold; bool placed, flagged, battle, sig, held, onehit; } InputStep;
static InputStep script[1024];
static int script_len, script_pos, script_left;

/* "UP+A": the GBA's buttons injected as held; "pad.x", "pad.-lefty" a
 * virtual controller's (--pad), and "key.K" a key, through SDL as a
 * player's are (the controls screen and the maps in tests) */
static void parse_buttons(const char *s, InputStep *step) {
	static const struct { const char *n; uint32_t b; } names[] = {
		{ "UP", BTN_UP }, { "DOWN", BTN_DOWN }, { "LEFT", BTN_LEFT }, { "RIGHT", BTN_RIGHT },
		{ "A", BTN_A }, { "B", BTN_B }, { "L", BTN_L }, { "R", BTN_R }, { "START", BTN_START }, { "SELECT", BTN_SELECT },
	};
	char buf[128];
	snprintf(buf, sizeof buf, "%s", s);
	char *save = NULL;
	for (char *t = strtok_r(buf, "+", &save); t; t = strtok_r(NULL, "+", &save)) {
		int in = !SDL_strncasecmp(t, "pad.", 4) ? padmap_input(t + 4) : PAD_NONE;
		/* (a raw joystick's: --pad raw, "pad.b9", "pad.h0.1") */
		if (in == PAD_NONE && !SDL_strncasecmp(t, "pad.", 4)) in = padmap_input_of(PAD_FAMILY_RAW, t + 4);
		if (in != PAD_NONE) step->pad |= (uint64_t)1 << in;
		if (!SDL_strncasecmp(t, "key.", 4)) step->key = SDL_GetScancodeFromName(t + 4);
		for (size_t i = 0; i < sizeof names / sizeof *names; ++i)
			if (!strcmp(t, names[i].n)) step->buttons |= names[i].b;
	}
}

/* a dev step that takes its words whole ("foes SPEC", "status SPEC",
 * "xrecord ADDR", "chip ID CODE [N]", "cross F", "folder K", "onehit"), into its step;
 * false where `tok` is none */
static bool text_step(const char *tok, InputStep *s) {
	if (!strcmp(tok, "onehit")) s->onehit = true;
	else if (!strncmp(tok, "foes ", 5)) snprintf(s->foes, sizeof s->foes, "%s", tok + 5);
	else if (!strncmp(tok, "status ", 7)) snprintf(s->status, sizeof s->status, "%s", tok + 7);
	else if (!strncmp(tok, "xrecord ", 8)) snprintf(s->xrecord, sizeof s->xrecord, "%s", tok + 8);
	else if (!strncmp(tok, "chip ", 5)) snprintf(s->chip, sizeof s->chip, "%s", tok + 5);
	else if (!strncmp(tok, "cross ", 6)) s->cross = (int)strtol(tok + 6, NULL, 0);
	else if (!strncmp(tok, "folder ", 7)) s->folder = (int)strtol(tok + 7, NULL, 0);
	/* (the NetPass opened over the game, or its stamp moment: netpass_screen.h) */
	else if (!strcmp(tok, "netpass")) s->netpass = 1;
	else if (!strcmp(tok, "netpass stamp")) s->netpass = 2;
	/* (Trade Central's screens in a run: trade_preview.h) */
	else if (!strncmp(tok, "trade ", 6)) snprintf(s->trade, sizeof s->trade, "%s", tok + 6);
	/* (the wait for a dropped link over the game: reconnect_preview.c) */
	else if (!strncmp(tok, "reconnect ", 10)) snprintf(s->reconnect, sizeof s->reconnect, "%s", tok + 10);
	/* (an emote sent in the Net Square, as the strip sends it: online_chat_emote) */
	else if (!strncmp(tok, "emote ", 6)) s->emote = 1 + (int)strtol(tok + 6, NULL, 0);
	/* (a tap on the second screen at its pixel, --second-size's or the 3DS's: the Square's palette) */
	else if (!strncmp(tok, "stap ", 5)) s->stap[2] = sscanf(tok + 5, "%d %d", &s->stap[0], &s->stap[1]) == 2;
	else return false;
	return true;
}

/* whether a step is one of the dev steps (no buttons, no frames of its own) */
static bool dev_step(const InputStep *s) {
	return s->placed || s->flagged || s->battle || s->foes[0] || s->status[0] || s->xrecord[0] || s->chip[0] || s->cross || s->folder || s->sig || s->held ||
		s->onehit || s->netpass || s->trade[0] || s->reconnect[0] || s->emote || s->stap[2];
}

/* "30:,2:A,10:,2:RIGHT" -> steps of (frames, buttons). */
static void parse_script(const char *spec) {
	char *copy = strdup(spec);
	for (char *tok = strtok(copy, ","); tok && script_len < 512; tok = strtok(NULL, ",")) {
		char *colon = strchr(tok, ':');
		memset(&script[script_len], 0, sizeof script[script_len]);
		InputStep *s = &script[script_len];
		s->frames = atoi(tok);
		/* (remote play's dev steps too, as "0:place X Y FACE", "0:flags
		 * FROM TO 1|0", "0:battle", "0:foes 200@52/01@43", "0:xrecord
		 * 811DD90", "0:status freeze 90", "0:cross 8" and "0:sig": a scripted
		 * capture of a set piece puts MegaMan at it, of a battle starts the
		 * layer's next, one of the given foes or one of BN5's records in BN6's
		 * engine, of a status gives the battle's enemies one, of a cross
		 * commits the next Cross chosen as that form, of a layer's signature
		 * puts him in it; "300:battle" waits its 300 frames first) */
		if (colon && !strncmp(colon + 1, "place ", 6)) s->placed = sscanf(colon + 7, "%d %d %d", &s->place[0], &s->place[1], &s->place[2]) == 3;
		else if (colon && !strncmp(colon + 1, "flags ", 6)) s->flagged = sscanf(colon + 7, "%i %i %i", &s->flags[0], &s->flags[1], &s->flags[2]) == 3;
		else if (colon && !strcmp(colon + 1, "battle")) s->battle = true;
		else if (colon && text_step(colon + 1, s)) {}
		else if (colon && !strcmp(colon + 1, "sig")) s->sig = true;
		else if (colon && !strncmp(colon + 1, "stuck ", 6)) s->held = (s->hold = held_parse(colon + 7)) != 0;
		else if (colon) parse_buttons(colon + 1, s);
		if (dev_step(s) && s->frames > 0 && script_len + 1 < 512) {
			script[script_len + 1] = *s;
			script[script_len + 1].frames = 0;
			*s = (InputStep){ .frames = s->frames };
			++script_len;
		}
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

void capture_taps(void) {
	for (int i = 0; i < ntaps; ++i) {
		const Tap *t = &taps[i];
		int len = t->drag ? 20 : 6, f = (int)P.frame - t->frame;
		if (f < 0 || f > len) continue;
		uint32_t type = f == 0 ? SDL_FINGERDOWN : f == len ? SDL_FINGERUP : SDL_FINGERMOTION;
		platform_finger(type, 900 + i, t->x0 + (t->x1 - t->x0) * (float)f / len, t->y0 + (t->y1 - t->y0) * (float)f / len);
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
	fprintf(f, "frame %llu\nscene %s\n", (unsigned long long)P.frame, scene_current() ? scene_current()->name : "none");
	fprintf(f, "screen %s\n", screen_name());
	if (scene_current() == &scene_emu) director_describe(f);
	fclose(f);
}

/* A step's dev steps on the game: a battle, foes, a record of BN5's, a
 * status, a chip, the form the next Cross chosen takes (soulproxy.h), the
 * signature, the run saved with MegaMan held (a CONTINUE's recovery, issue
 * #23), its enemies held at 1 HP */
static void emu_steps(const InputStep *s) {
	if (s->battle) director_dev_battle();
	if (s->foes[0]) director_dev_foes(s->foes);
	if (s->xrecord[0]) director_dev_xrecord(s->xrecord);
	if (s->status[0]) director_dev_status(s->status);
	if (s->chip[0]) director_dev_chip(s->chip);
	if (s->folder) director_dev_folder(s->folder);
	if (s->cross) soulproxy_cross(s->cross);
	if (s->sig) director_dev_signature();
	if (s->held) director_dev_hold(s->hold);
	/* (every enemy held at 1 HP from now on, as --dev onehit: a long fight's end) */
	if (s->onehit) devtools_parse("onehit");
	if (s->netpass == 1) netpass_screen_open();
	if (s->netpass == 2) netpass_stamp_begin();
	if (s->trade[0]) trade_preview(s->trade);
	if (s->reconnect[0]) reconnect_preview(s->reconnect);
	if (s->emote) online_chat_emote(s->emote - 1);
	if (s->stap[2]) platform_second_tap_at(s->stap[0], s->stap[1], 0, 0);
}

/* The steps of no frames at the script's position: pictures, states. */
static void script_actions(void) {
	while (script_pos < script_len && script[script_pos].frames == 0) {
		InputStep *s = &script[script_pos];
		if (s->shot[0]) platform_save_canvas(s->shot);
		if (s->second[0]) platform_save_second_screen(s->second);
		if (s->state[0]) write_state(s->state);
		if (s->dump[0]) emu_debug_dump(s->dump);
		if (s->placed) director_dev_place(s->place[0], s->place[1], s->place[2]);
		if (scene_current() == &scene_emu) emu_steps(s);
		/* (event flags FROM..TO set, then as they were: finding what a flag does) */
		if (s->flagged && scene_current() == &scene_emu) {
			static bool was[FLAG_COUNT];
			for (int f = s->flags[0] < 0 ? 0 : s->flags[0]; f <= s->flags[1] && f < FLAG_COUNT; ++f)
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

/* One line: "N BUTTONS" holds them N frames, "shot PATH", "second PATH"
 * (the second screen's picture), "state PATH",
 * "dump PREFIX" (the video memory, emu_debug_dump), "place X Y FACING", "flags FROM TO 1" (set; 0: back as they were),
 * "battle" (the layer's next random battle, director_dev_battle), "foes
 * SPEC" (a battle of these foes, director_dev_foes), "xrecord ADDR" (a
 * record of BN5's in BN6's engine, director_dev_xrecord), "status SPEC" (the
 * battle's enemies given a status, director_dev_status), "chip ID CODE
 * [N]" (a chip in the folder's first entries and the pack,
 * director_dev_chip), "cross F" (the next Cross chosen committed as form
 * F, soulproxy_cross), "sig"
 * (MegaMan in the layer's signature, director_dev_signature), "stuck MASK"
 * (the run saved with BN6 holding MegaMan so, director_dev_hold), "quit";
 * items apart by ';'. */
static void remote_parse(char *line) {
	script_len = script_pos = 0;
	for (char *tok = strtok(line, ";\n"); tok && script_len < (int)(sizeof script / sizeof *script); tok = strtok(NULL, ";\n")) {
		while (*tok == ' ') ++tok;
		InputStep *s = &script[script_len];
		memset(s, 0, sizeof *s);
		if (!strncmp(tok, "shot ", 5)) snprintf(s->shot, sizeof s->shot, "%s", tok + 5);
		else if (!strncmp(tok, "second ", 7)) snprintf(s->second, sizeof s->second, "%s", tok + 7);
		else if (!strncmp(tok, "state ", 6)) snprintf(s->state, sizeof s->state, "%s", tok + 6);
		else if (!strncmp(tok, "dump ", 5)) snprintf(s->dump, sizeof s->dump, "%s", tok + 5);
		else if (!strncmp(tok, "place ", 6)) s->placed = sscanf(tok + 6, "%d %d %d", &s->place[0], &s->place[1], &s->place[2]) == 3;
		else if (!strncmp(tok, "flags ", 6)) s->flagged = sscanf(tok + 6, "%i %i %i", &s->flags[0], &s->flags[1], &s->flags[2]) == 3;
		else if (!strcmp(tok, "battle")) s->battle = true;
		else if (text_step(tok, s)) {}
		else if (!strcmp(tok, "sig")) s->sig = true;
		else if (!strncmp(tok, "stuck ", 6)) s->held = (s->hold = held_parse(tok + 6)) != 0;
		else if (!strncmp(tok, "quit", 4)) { P.quit = true; return; }
		else {
			char buttons[128] = "";
			if (sscanf(tok, "%d %127s", &s->frames, buttons) < 1 || s->frames <= 0) continue;
			parse_buttons(buttons, s);
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

/* The script's key, held as SDL's events say a player's is */
static void script_key(int key) {
	static int held;
	if (key == held) return;
	SDL_Event e;
	memset(&e, 0, sizeof e);
	for (int k = 0; k < 2; ++k) {
		int sc = k ? key : held;
		if (!sc) continue;
		e.type = k ? SDL_KEYDOWN : SDL_KEYUP;
		e.key.state = k ? SDL_PRESSED : SDL_RELEASED;
		e.key.keysym.scancode = (SDL_Scancode)sc;
		e.key.keysym.sym = SDL_GetKeyFromScancode((SDL_Scancode)sc);
		SDL_PushEvent(&e);
	}
	held = key;
}

void capture_input(void) {
	if (bot_seed) { bot_tick(); return; }
#if !defined(__EMSCRIPTEN__) && !defined(_WIN32) && !defined(__3DS__)
	if (remote_in >= 0) remote_tick();
#endif
	script_actions();
	bool done = script_pos >= script_len;
	pads_virtual_hold(done ? 0 : script[script_pos].pad);
	script_key(done ? 0 : script[script_pos].key);
	if (done) { platform_inject(0); return; }
	platform_inject(script[script_pos].buttons);
	if (--script_left <= 0 && ++script_pos < script_len) script_left = script[script_pos].frames;
}

typedef struct { uint64_t frame; char path[256]; } Shot;
static Shot shots[64], screen_shots[16], second_shots[16];
static int shot_count, screen_shot_count, second_shot_count;
/* --shot-range A:B:PREFIX: frames A to B, each into PREFIX's file of its
 * number; --second-shot-range the second screen's alike (a clip of both) */
typedef struct { uint64_t a, b; char prefix[200]; } Range;
static Range range = { 1, 0, "" }, second_range = { 1, 0, "" };

static void parse_range(const char *spec, Range *r) {
	unsigned long long ra = 0, rb = 0;
	if (sscanf(spec, "%llu:%llu:%199s", &ra, &rb, r->prefix) == 3) { r->a = ra; r->b = rb; }
}

/* the range's file for this frame, false outside it */
static bool range_path(const Range *r, char *path, size_t n) {
	if (P.frame < r->a || P.frame > r->b) return false;
	snprintf(path, n, "%s%05llu.bmp", r->prefix, (unsigned long long)P.frame);
	return true;
}

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

/* --pad KIND: a virtual controller (pads_virtual), and pad.ini in the
 * data folder read and written as a player's, headless too */
static const char *pad_kind;

/* --shot FRAME:PATH,... saves the canvas; --screen-shot the screen as the
 * player sees it, the touch controls on it; --second-shot the second
 * screen, the 3DS's bottom one (on any target, for a check), at
 * --second-size's WxH (an Android display's); --pad */
static bool test_option(const char *a, const char *v) {
	if (!strcmp(a, "--pad")) pad_kind = v;
	else if (!strcmp(a, "--shot")) parse_shots(v, shots, &shot_count, 64);
	else if (!strcmp(a, "--screen-shot")) parse_shots(v, screen_shots, &screen_shot_count, 16);
	else if (!strcmp(a, "--second-shot")) parse_shots(v, second_shots, &second_shot_count, 16);
	else if (!strcmp(a, "--second-size")) {
		int w = SECOND_W, h = SECOND_H;
		sscanf(v, "%dx%d", &w, &h);
		platform_second_shot_size(w, h);
	} else return false;
	return true;
}

#if !defined(__EMSCRIPTEN__) && !defined(_WIN32) && !defined(__3DS__)
static const char *remote_dir;   /* --remote: remote play's pipes' folder */
#endif

bool capture_option(const char *a, const char *v) {
	if (!strcmp(a, "--input")) parse_script(v);
	else if (!strcmp(a, "--taps")) parse_taps(v);
	else if (!strcmp(a, "--shot-range")) parse_range(v, &range);
	else if (!strcmp(a, "--second-shot-range")) parse_range(v, &second_range);
#if !defined(__EMSCRIPTEN__) && !defined(_WIN32) && !defined(__3DS__)
	else if (!strcmp(a, "--remote")) remote_dir = v;
#endif
	else if (!strcmp(a, "--bot")) bot_seed = (uint32_t)strtoul(v, NULL, 0) | 1;
	else return test_option(a, v);
	return true;
}

bool capture_start(void) {
#if !defined(__EMSCRIPTEN__) && !defined(_WIN32) && !defined(__3DS__)
	if (remote_dir && !remote_open(remote_dir)) { fprintf(stderr, "--remote: cannot open the pipes in %s\n", remote_dir); return false; }
#endif
	return true;
}

void capture_shots(void) {
	for (int i = 0; i < shot_count; ++i)
		if (shots[i].frame == P.frame) platform_save_canvas(shots[i].path);
	for (int i = 0; i < screen_shot_count; ++i)
		if (screen_shots[i].frame == P.frame) platform_shot_screen(screen_shots[i].path);
	for (int i = 0; i < second_shot_count; ++i)
		if (second_shots[i].frame == P.frame) platform_save_second_screen(second_shots[i].path);
	char path[256];
	if (range_path(&range, path, sizeof path)) platform_save_canvas(path);
	if (range_path(&second_range, path, sizeof path)) platform_save_second_screen(path);
}

const char *capture_pad_kind(void) { return pad_kind; }
