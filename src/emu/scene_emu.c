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

static void enter(void) {
	revealed = 0;
	if (!emu_init(R.data, ROM_SIZE)) return;
	npc_reach_install();
	chat_marks_install();
	pet_install();
	pet_text_install();
	cinema_reset();
	if (emu_resume_requested) {
		emu_resume_requested = false;
		emu_encounters_install();
		director_resume();
	} else {
		emu_boot();
		emu_encounters_install();
		if (emu_start_in_town) director_start_run();
		else director_start_layer();
		emu_start_in_town = false;
	}
	audio_external(emu_audio_read);
}

static void leave(void) { audio_external(NULL); }

static void update(void) {
	uint32_t keys = director_keys(pet_keys(devtools_keys(autopilot_on() ? autopilot_keys() : keys_from_buttons())));
	if (devtools_open() || pet_link_is_open()) return;   /* the game holds still under the dev menu and the SciLab link */
	/* (fast-forwarded: several game frames to one shown) */
	for (int i = 0; i < dev.speed; ++i) {
		emu_frame(cinema_keys(keys));
		director_update();
		pet_update();
		devtools_update();
		tour_update();
		cinema_update();
		emu_debug_frame();
	}
	if (revealed || director_arrived()) ++revealed;
}

static void draw(void) {
	fill_rect(0, 0, P.w, P.h, BLACK);
	if (!emu_ready()) return;
	if (!tex) {
		tex = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, EMU_W, EMU_H);
		SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
	}
	/* mGBA keeps layer flags in the top byte; GL renderers read it as alpha */
	static uint32_t px[EMU_W * EMU_H];
	const uint32_t *v = emu_video();
	for (int i = 0; i < EMU_W * EMU_H; ++i) px[i] = v[i] | 0xFF000000u;
	SDL_UpdateTexture(tex, NULL, px, EMU_W * 4);
	int dx, dy;
	cinema_offset(&dx, &dy);
	SDL_Rect dst = { P.core_x + dx, P.core_y + dy, EMU_W, EMU_H };
	SDL_RenderCopy(P.renderer, tex, NULL, &dst);
	if (!revealed) { fill_rect(P.core_x, P.core_y, EMU_W, EMU_H, BLACK); return; }
	if (revealed < REVEAL_FRAMES) { P.fx_fade = REVEAL_FRAMES - revealed; P.fx_fade_color = BLACK; }
	cinema_draw();
	director_draw_map();
	pet_link_draw();
	devtools_draw();
}

const Scene scene_emu = { "emu", enter, update, draw, leave };
