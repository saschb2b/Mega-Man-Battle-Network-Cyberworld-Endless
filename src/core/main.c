#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "gfx.h"
#include "platform.h"
#include "rom.h"
#include "save.h"
#include "run.h"
#include "battle.h"
#include "audio.h"

char g_data_dir[512] = ".";

static const Scene *current, *pending;

void scene_set(const Scene *s) { pending = s; }
const Scene *scene_current(void) { return current; }

static uint32_t rng_s = 0x9E3779B9u;
void rng_seed(uint32_t s) { rng_s = s ? s : 0x9E3779B9u; }
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

/* Plain fallback text for when the ROM font is unavailable. */
static void error_draw(void) {
	SDL_SetRenderDrawColor(P.renderer, 8, 16, 48, 255);
	SDL_RenderClear(P.renderer);
	if (R.data) {
		text_draw(P.w / 2, 40, "Cyberworld Endless", WHITE, TEXT_CENTER);
		text_draw(P.w / 2, 70, error_msg, WHITE, TEXT_CENTER);
		return;
	}
	/* Without a ROM there is no font: draw a simple bar pattern so the
	 * screen is not blank, and print the message to the log. */
	for (int i = 0; i < 6; ++i) fill_rect(P.w / 2 - 60 + i * 20, P.h / 2 - 4, 12, 8, rgba(80, 160, 255, 255));
}

static void error_update(void) {
	if (btn_pressed(BTN_START) || btn_pressed(BTN_B)) P.quit = true;
}

const Scene scene_error = { "error", NULL, error_update, error_draw, NULL };

/* ---- scripted input and captures for headless tests ---- */
typedef struct { int frames; uint32_t buttons; } InputStep;
static InputStep script[512];
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

static void script_tick(void) {
	if (bot_seed) { bot_tick(); return; }
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

void debug_rewards(int busting, RewardOption *opts, int *n) {
	(void)busting;
	opts[0] = (RewardOption){ 1, 'A', 0 };
	opts[1] = (RewardOption){ 72, '*', 0 };
	opts[2] = (RewardOption){ -1, 0, 800 };
	*n = 3;
}

static const Scene *scene_by_name(const char *n) {
	const Scene *all[] = { &scene_title, &scene_gallery, &scene_net, &scene_gameover, &scene_emu };
	for (size_t i = 0; i < sizeof all / sizeof *all; ++i)
		if (!strcmp(all[i]->name, n)) return all[i];
	return NULL;
}

int main(int argc, char **argv) {
	const char *rom_dir = "rom";
	const char *start_scene = "title";
	int force_w = 0, force_h = 0;
	bool headless = false;
	uint64_t max_frames = 0;
	uint32_t seed = 0;
	const char *render_spec = NULL;
	const char *battle_spec = NULL;
	const char *sheet_spec = NULL;
	for (int i = 1; i < argc; ++i) {
		const char *a = argv[i];
		const char *v = i + 1 < argc ? argv[i + 1] : NULL;
		if (!strcmp(a, "--headless")) headless = true;
		else if (!strcmp(a, "--rom-dir") && v) { rom_dir = v; ++i; }
		else if (!strcmp(a, "--data-dir") && v) { snprintf(g_data_dir, sizeof g_data_dir, "%s", v); ++i; }
		else if (!strcmp(a, "--size") && v) { sscanf(v, "%dx%d", &force_w, &force_h); ++i; }
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
		else if (!strcmp(a, "--battle") && v) { battle_spec = v; ++i; }
		else if (!strcmp(a, "--render-song") && v) { render_spec = v; ++i; }
		else if (!strcmp(a, "--sheet") && v) { sheet_spec = v; ++i; }
		else if (!strcmp(a, "--net-biome") && v) { extern int net_debug_biome; net_debug_biome = atoi(v); ++i; }
		else if (!strcmp(a, "--bot") && v) { bot_seed = (uint32_t)strtoul(v, NULL, 0) | 1; ++i; }
		else { fprintf(stderr, "unknown argument %s\n", a); return 2; }
	}
	setvbuf(stdout, NULL, _IOLBF, 0);
	if (headless && !force_w) { force_w = 1280; force_h = 960; }
	if (!platform_init(force_w, force_h, headless)) return 1;
	rng_seed(seed ? seed : (uint32_t)SDL_GetPerformanceCounter());

	char msg[512];
	if (!rom_find(rom_dir, msg, sizeof msg)) {
		fprintf(stderr, "%s\n", msg);
		error_show(msg);
	} else if (!gfx_init()) {
		error_show("The ROM could not be decoded.");
	} else {
		printf("ROM: %s (%s)\n", R.layout->name, R.path);
		save_init();
		if (render_spec) {
			int song = 0, secs = 20;
			char out[256] = "song.wav";
			sscanf(render_spec, "%i:%d:%255s", &song, &secs, out);
			bool ok = audio_render_wav(song, secs, out);
			printf("rendered song %d (%ds) to %s: %s\n", song, secs, out, ok ? "ok" : "failed");
			platform_shutdown();
			return ok ? 0 : 1;
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
		const Scene *s = scene_by_name(start_scene);
		if (battle_spec) {
			/* --battle v:FAMILY:VER,n:NAVI:VER,...  start a test battle directly
			 * (x:CROSS, f:CHIP:CODE, r empty folder, k foes at 1 HP, h:HP MegaMan's HP) */
			run_new(seed ? seed : 1);
			Encounter e = { 0 };
			e.biome = BIOME_CENTRAL;
			char *copy = strdup(battle_spec);
			char *save = NULL;
			for (char *tok = strtok_r(copy, ",", &save); tok && e.nfoes < 4; tok = strtok_r(NULL, ",", &save)) {
				char kind = 'v';
				int fam = 1, ver = 0;
				sscanf(tok, "%c:%i:%d", &kind, &fam, &ver);
				if (kind == 'x') { if (fam == 99) run.beast_out = true; else run.crosses |= 1u << fam; continue; }
				if (kind == 'f') { folder_add(fam, (char)('A' + ver)); continue; }
				if (kind == 'r') { run.folder_n = 0; continue; }
				if (kind == 'k') { extern bool battle_debug_one_hp; battle_debug_one_hp = true; continue; }
				if (kind == 'h') { run.hp = fam; continue; }
				if (kind == 'g') { extern int battle_debug_bg; battle_debug_bg = fam; continue; }
				Foe *f = &e.foes[e.nfoes];
				f->kind = kind == 'n' ? FOE_NAVI : FOE_VIRUS;
				f->family = fam;
				f->version = ver;
				f->col = kind == 'n' ? 4 : 3 + e.nfoes % 3;
				f->row = kind == 'n' ? 1 : e.nfoes % 3;
				e.boss |= kind == 'n';
				e.nfoes++;
			}
			free(copy);
			extern void debug_rewards(int, RewardOption *, int *);
			battle_set_rewards(debug_rewards);
			battle_begin(&e, NULL);
			s = NULL;
		}
		if (!battle_spec) {
			if (s == &scene_net) { run_new(seed ? seed : 1); net_reset(); }
			if (s == &scene_emu) run_new(seed ? seed : 1);
			scene_set(s ? s : &scene_title);
		}
	}

	uint64_t last = SDL_GetPerformanceCounter();
	const double freq = (double)SDL_GetPerformanceFrequency();
	double acc = 0;
	while (!P.quit) {
		if (pending) {
			if (current && current->leave) current->leave();
			current = pending;
			pending = NULL;
			if (current->enter) current->enter();
		}
		if (!headless || (getenv("CYBERWORLD_AUDIO_DUMP") && !audio_offline())) {
			uint64_t now = SDL_GetPerformanceCounter();
			acc += (now - last) / freq;
			last = now;
			if (acc < 1.0 / 60.0 - 0.002) { SDL_Delay(1); continue; }
			acc -= 1.0 / 60.0;
			if (acc > 0.1) acc = 0;
		}
		script_tick();
		platform_poll();
		if (current && current->update) current->update();
		audio_frame();
		platform_begin_frame();
		if (current && current->draw) current->draw();
		platform_apply_effects();
		for (int i = 0; i < shot_count; ++i)
			if (shots[i].frame == P.frame) platform_save_canvas(shots[i].path);
		if (P.frame >= range_a && P.frame <= range_b) {
			char path[256];
			snprintf(path, sizeof path, "%s%05llu.bmp", range_prefix, (unsigned long long)P.frame);
			platform_save_canvas(path);
		}
		platform_end_frame();
		if (max_frames && P.frame >= max_frames) break;
	}
	platform_shutdown();
	return 0;
}
