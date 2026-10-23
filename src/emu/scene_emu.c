/* The running game in the window: the core's frame in the 240x160 view,
 * its sound on the audio device and the port's buttons as GBA keys. */
#include "audio.h"
#include "boot.h"
#include "cinema.h"
#include "autopilot.h"
#include "debug.h"
#include "devtools.h"
#include "pet.h"
#include "director.h"
#include "emu.h"
#include "game.h"
#include "game_hooks.h"
#include "gfx.h"
#include "online_visit.h"
#include "platform.h"
#include "rom.h"
#include "save.h"
#include "second.h"
#include "tour.h"
#include "xinstall.h"
#ifdef __3DS__
#include "start_3ds.h"
#endif

static SDL_Texture *tex;
bool emu_resume_requested;
bool emu_start_in_town;
bool emu_start_at_home;

bool emu_saved_home(uint32_t seed, int depth) { return director_saved_home(seed, depth); }

static uint32_t keys_from_buttons(void) {
	static const struct { int btn; uint32_t key; } map[] = {
		{ BTN_A, KEY_A }, { BTN_B, KEY_B }, { BTN_SELECT, KEY_SELECT }, { BTN_START, KEY_START },
		{ BTN_RIGHT, KEY_RIGHT }, { BTN_LEFT, KEY_LEFT }, { BTN_UP, KEY_UP }, { BTN_DOWN, KEY_DOWN },
		{ BTN_R, KEY_R }, { BTN_L, KEY_L },
	};
	uint32_t k = 0;
	for (size_t i = 0; i < sizeof map / sizeof *map; ++i)
		if (btn_held(map[i].btn)) k |= map[i].key;
	return k;
}

/* The picture stays black until the run's first map has loaded (the boot
 * state's own screen shows meanwhile: its backdrop is bright green), then
 * fades in. */
#define REVEAL_FRAMES 16
static int revealed;   /* frames since the first map showed, 0 not yet */
static bool started;   /* (the core on its own thread: a frame begun since enter) */

static void enter(void) {
	revealed = 0;
	started = false;
	/* (the second screen, issue #72: the 3DS's bottom one, a display
	 * beside an Android handheld's) */
	platform_second_screen(second_draw, second_changed);
	/* (making a run's net takes a while on a slow machine: twenty seconds
	 * of black on a 3DS read as a hang) */
	platform_begin_frame();
	fill_rect(0, 0, P.w, P.h, BLACK);
	text_draw(P.w / 2, P.h / 2 - 8, "Building the net...", WHITE, TEXT_CENTER);
	platform_present_now();
	if (!emu_init(R.data, ROM_SIZE)) { fprintf(stderr, "the GBA core did not start (too little memory?)\n"); return; }
	/* (other games' virus families in BN6's battle engine first: the ROM
	 * copy's patches, the same at every boot; docs/MULTIROM.md) */
	xinstall_core(dev.xcontrol ? XVIRUS_CONTROL : dev.nofamilies ? XVIRUS_NONE : XVIRUS_FAMILIES,
		dev.xnavicontrol ? XNAVI_CONTROL : dev.nonavis ? XNAVI_NONE : XNAVI_BN5);
#ifdef __3DS__
	/* (what the core's copy and the installs left of the heap, BN5's or
	 * not: 3ds/README.md, Testing on a 3DS) */
	char heap[120];
	start_3ds_heap(heap, sizeof heap);
	printf("3ds: the core started, the installs done; %s\n", heap);
#endif
	game_hooks_install();
	cinema_reset();
	if (emu_resume_requested) {
		emu_resume_requested = false;
		game_hooks_after_boot();
		if (director_resume()) online_visit_resumed();
	} else {
		director_before_boot();
		emu_boot();
		game_hooks_after_boot();
		if (emu_start_in_town) director_start_run();
		else director_start_layer();
		if (emu_start_at_home) director_dev_home();
		emu_start_in_town = emu_start_at_home = false;
	}
	audio_external(emu_audio_read);
}

static void leave(void) {
	pet_leave();
	online_visit_shutdown();
	audio_external(NULL);
	platform_second_screen(NULL, NULL);
}

static uint32_t frame_keys(void) {
	uint32_t keys = autopilot_on() ? autopilot_keys() : keys_from_buttons();
	/* (in the Net Square the run's own keys rest: docs/ONLINE.md) */
	if (online_visit_away()) return online_visit_keys(keys);
	return director_keys(pet_keys(devtools_keys(online_visit_keys(keys))));
}

/* What follows a frame of the game: the run's logic on it. */
static void after_frame(void) {
	online_visit_update();
	if (online_visit_away()) { second_update(); emu_debug_frame(); return; }
	director_see();
	director_update();
	director_hold_watch();
	pet_update();
	devtools_update();
	tour_update();
	cinema_update();
	second_update();
	emu_debug_frame();
}

/* The GBA's reset chord belongs to the run's title (issue #113): checked
 * on the player's mapped buttons before the PET, the map and the cinema
 * take keys, and before the core sees them. The last checkpoint stays as
 * it was, as a quit in a battle leaves it: CONTINUE takes the run up
 * through the director, where BN6's own title would start on the map
 * tables the run has taken over. */
static bool soft_reset(void) {
	uint32_t chord = KEY_A | KEY_B | KEY_SELECT | KEY_START;
	static bool from_square;
	if ((keys_from_buttons() & chord) != chord) { from_square = false; return false; }
	/* (in the Net Square: back to the gate's checkpoint, as Leave; the
	 * chord still held after it is no reset, and BN6 never sees it, whose
	 * own soft reset it is) */
	if (from_square) return true;
	if (online_visit_away()) { online_visit_leave(VISIT_END_LEFT); from_square = true; return true; }
	online_visit_shutdown();
	emu_sync();
	director_stop();
	cinema_reset();
	started = false;
	P.quit_prompt = 0;
	scene_set(&scene_title);
	return true;
}

static void update(void) {
	emu_drawing = false;
	if (soft_reset()) return;
	/* (a NetBattle from the Net Square: the game's core holds still under it) */
	if (online_visit_held()) { online_visit_held_frame(keys_from_buttons()); return; }
	/* (the link to the Square dropped: the game's core holds still under the wait) */
	if (online_visit_waiting()) { online_visit_wait_frame(); return; }
	/* (the PET's Jukebox up: the game's core, its music with it, holds still under it) */
	if (pet_holds()) return;
	/* (the run's time: the statistics' minutes, save.h) */
	if (!online_visit_away()) profile_played_frame();
	/* (the second screen first, from what the last update saw: the GBA's
	 * frame runs on beside it, where the reads below would wait for it) */
	platform_second_screen_draw();
	/* the core on a thread of its own (emu.c): the frame done taken in,
	 * then the next begun, which runs while this one is drawn; keys, frames
	 * and the logic keep their order */
	if (emu_threaded()) {
		if (devtools_open()) { frame_keys(); return; }
		for (int i = 0; i < dev.speed; ++i) {
			if (started) after_frame();
			emu_frame(cinema_keys(frame_keys()));
			started = true;
		}
		if (revealed || director_arrived()) ++revealed;
		return;
	}
	uint32_t keys = frame_keys();
	if (devtools_open()) return;   /* the game holds still under the dev menu */
	/* (fast-forwarded: several game frames to one shown) */
	for (int i = 0; i < dev.speed; ++i) {
		emu_frame(cinema_keys(keys));
		after_frame();
	}
	if (revealed || director_arrived()) ++revealed;
}

/* The town's hour on its picture (docs/HOME.md, piece 8): each colour
 * channel of the map's layers and of the sprites over it times the hour's
 * (in 256ths: afternoon a touch warm, evening orange, night a dim blue).
 * mGBA's top byte names a pixel's layer: a background's has 0x08 and its
 * index at bit 4 (BG0, the chat's text, kept), a sprite's none; with a chat
 * box open the sprites in its rows (its frame and face) keep theirs too. */
#define TINT_BOX_TOP 96
static uint32_t tinted(uint32_t c, int hour, bool box_row) {
	static const uint16_t by[4][3] = { { 256, 256, 256 }, { 262, 246, 222 }, { 270, 196, 150 }, { 104, 124, 184 } };
	uint32_t flags = c >> 24;
	if ((flags & 0x08) ? (flags >> 4 & 3) == 0 : box_row) return c;
	uint32_t r = (c & 0xFF) * by[hour][0] >> 8, g = (c >> 8 & 0xFF) * by[hour][1] >> 8, b = (c >> 16 & 0xFF) * by[hour][2] >> 8;
	return (c & 0xFF000000u) | (b > 255 ? 255 : b) << 16 | (g > 255 ? 255 : g) << 8 | (r > 255 ? 255 : r);
}

static void draw(void) {
	emu_drawing = true;   /* (until the next update: the second screen's too) */
	fill_rect(0, 0, P.w, P.h, BLACK);
	if (!emu_ready()) return;
	if (online_visit_held()) { online_visit_draw(); return; }
	/* (on the 3DS in the canvas's order, so the copy onto it converts
	 * nothing: mGBA's red and blue swapped here, where a computer's GPU
	 * converts them) */
#ifdef __3DS__
	const Uint32 format = SDL_PIXELFORMAT_ARGB8888;
#else
	const Uint32 format = SDL_PIXELFORMAT_ABGR8888;
#endif
	if (!tex) {
		tex = SDL_CreateTexture(P.renderer, format, SDL_TEXTUREACCESS_STREAMING, EMU_W, EMU_H);
		SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
	}
	/* mGBA keeps layer flags in the top byte; GL renderers read it as alpha */
	static uint32_t px[EMU_W * EMU_H];
	const uint32_t *v = emu_video();
	bool box = false;
	int hour = online_visit_away() ? 0 : director_town_tint(&box);
	for (int i = 0; i < EMU_W * EMU_H; ++i) {
		uint32_t c = hour ? tinted(v[i], hour, box && i / EMU_W >= TINT_BOX_TOP) : v[i];
#ifdef __3DS__
		px[i] = 0xFF000000u | (c & 0xFF00u) | (c & 0xFFu) << 16 | (c >> 16 & 0xFFu);
#else
		px[i] = c | 0xFF000000u;
#endif
	}
	SDL_UpdateTexture(tex, NULL, px, EMU_W * 4);
	int dx = 0, dy = 0;
	cinema_offset(&dx, &dy);
	SDL_Rect dst = { P.core_x + dx, P.core_y + dy, EMU_W, EMU_H };
	SDL_RenderCopy(P.renderer, tex, NULL, &dst);
	if (!revealed) { fill_rect(P.core_x, P.core_y, EMU_W, EMU_H, BLACK); return; }
	if (revealed < REVEAL_FRAMES) { P.fx_fade = REVEAL_FRAMES - revealed; P.fx_fade_color = BLACK; }
	if (online_visit_away()) { online_visit_draw(); return; }
	cinema_draw();
	director_draw_map();
	director_draw_counts();
	director_draw_bug_note();
	director_draw_duel();
	director_draw_tent();
	devtools_draw();
	online_visit_draw();
	pet_draw();
}

const Scene scene_emu = { "emu", enter, update, draw, leave };
