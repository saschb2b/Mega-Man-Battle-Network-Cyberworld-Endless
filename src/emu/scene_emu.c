/* The running game in the window: the core's frame in the 240x160 view,
 * its sound on the audio device and the port's buttons as GBA keys. */
#include "audio.h"
#include "boot.h"
#include "emu.h"
#include "game.h"
#include "gfx.h"
#include "platform.h"
#include "rom.h"

static SDL_Texture *tex;

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

static void enter(void) {
	if (!emu_init(R.data, ROM_SIZE)) return;
	emu_boot();
	audio_external(emu_audio_read);
}

static void leave(void) { audio_external(NULL); }

static void update(void) { emu_frame(keys_from_buttons()); }

static void draw(void) {
	fill_rect(0, 0, P.w, P.h, BLACK);
	if (!emu_ready()) return;
	if (!tex) {
		tex = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, EMU_W, EMU_H);
		SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
	}
	SDL_UpdateTexture(tex, NULL, emu_video(), EMU_W * 4);
	SDL_Rect dst = { P.core_x, P.core_y, EMU_W, EMU_H };
	SDL_RenderCopy(P.renderer, tex, NULL, &dst);
}

const Scene scene_emu = { "emu", enter, update, draw, leave };
