/* The running game in the window: the core's frame in the 240x160 view,
 * its sound on the audio device and the port's buttons as GBA keys. */
#include "audio.h"
#include "boot.h"
#include "cinema.h"
#include "autopilot.h"
#include "debug.h"
#include "devtools.h"
#include "pet.h"
#include "pet_text.h"
#include "director.h"
#include "npc.h"
#include "scripts.h"
#include "encounter.h"
#include "emu.h"
#include "events.h"
#include "gamecall.h"
#include "idle.h"
#include "game.h"
#include "gfx.h"
#include "platform.h"
#include "rom.h"
#include "tour.h"

static SDL_Texture *tex;
bool emu_resume_requested;
bool emu_start_in_town;

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
	/* (the layer's map on the second screen, the 3DS's bottom one) */
	platform_second_screen(director_draw_second_screen);
	/* (making a run's net takes a while on a slow machine: twenty seconds
	 * of black on a 3DS read as a hang) */
	platform_begin_frame();
	fill_rect(0, 0, P.w, P.h, BLACK);
	text_draw(P.w / 2, P.h / 2 - 8, "Building the net...", WHITE, TEXT_CENTER);
	platform_present_now();
	if (!emu_init(R.data, ROM_SIZE)) { fprintf(stderr, "the GBA core did not start (too little memory?)\n"); return; }
	gamecall_install();
	idle_install();
	npc_reach_install();
	chat_marks_install();
	pet_install();
	pet_text_install();
	cinema_reset();
	if (emu_resume_requested) {
		emu_resume_requested = false;
		emu_encounters_install();
		events_install();
		director_resume();
	} else {
		emu_boot();
		emu_encounters_install();
		events_install();
		if (emu_start_in_town) director_start_run();
		else director_start_layer();
		emu_start_in_town = false;
	}
	audio_external(emu_audio_read);
}

static void leave(void) {
	audio_external(NULL);
	platform_second_screen(NULL);
}

static uint32_t frame_keys(void) {
	return director_keys(pet_keys(devtools_keys(autopilot_on() ? autopilot_keys() : keys_from_buttons())));
}

/* What follows a frame of the game: the run's logic on it. */
static void after_frame(void) {
	director_see();
	director_update();
	pet_update();
	devtools_update();
	tour_update();
	cinema_update();
	emu_debug_frame();
}

static void update(void) {
	emu_drawing = false;
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

static void draw(void) {
	emu_drawing = true;   /* (until the next update: the second screen's too) */
	fill_rect(0, 0, P.w, P.h, BLACK);
	if (!emu_ready()) return;
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
#ifdef __3DS__
	for (int i = 0; i < EMU_W * EMU_H; ++i) px[i] = 0xFF000000u | (v[i] & 0xFF00u) | (v[i] & 0xFFu) << 16 | (v[i] >> 16 & 0xFFu);
#else
	for (int i = 0; i < EMU_W * EMU_H; ++i) px[i] = v[i] | 0xFF000000u;
#endif
	SDL_UpdateTexture(tex, NULL, px, EMU_W * 4);
	int dx, dy;
	cinema_offset(&dx, &dy);
	SDL_Rect dst = { P.core_x + dx, P.core_y + dy, EMU_W, EMU_H };
	SDL_RenderCopy(P.renderer, tex, NULL, &dst);
	if (!revealed) { fill_rect(P.core_x, P.core_y, EMU_W, EMU_H, BLACK); return; }
	if (revealed < REVEAL_FRAMES) { P.fx_fade = REVEAL_FRAMES - revealed; P.fx_fade_color = BLACK; }
	cinema_draw();
	director_draw_map();
	director_draw_duel();
	director_draw_tent();
	devtools_draw();
}

const Scene scene_emu = { "emu", enter, update, draw, leave };
