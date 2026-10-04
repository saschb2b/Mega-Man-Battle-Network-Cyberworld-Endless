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
#include "guest.h"
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
		director_before_boot();
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

/* The keys a guest battle gets (guest.h): the player's, or the autopilot's,
 * which reads its battle as it reads BN6's (autopilot.c) */
/* An L or R pressed before the Custom gauge filled, kept two and a half
 * seconds and given as it fills (let go a frame first, so BN5 sees a
 * press), again every 20 frames until the screen opens, as BN6's battles
 * keep one (director.c): a playtester's R pressed a little early did
 * nothing in BN5's battles, twice (session 65). */
static uint32_t guest_custom_keep(uint32_t keys) {
	static int kept, step;
	static uint32_t was;
	uint32_t lr = keys & (KEY_L | KEY_R), pressed = lr & ~was;
	was = lr;
	int gauge = guest_custom_gauge();
	if (gauge < 0) { kept = step = 0; return keys; }
	if (pressed) { kept = gauge >= 0x4000 ? 0 : 150; step = 0; return keys; }
	if (kept <= 0) return keys;
	--kept;
	if (gauge < 0x4000) return keys;
	int s = step++ % 20;
	return s == 0 ? keys & ~(KEY_L | KEY_R) : s == 1 ? keys | KEY_R : keys;
}

static uint32_t guest_keys(void) {
	return autopilot_on() ? autopilot_guest_keys() : guest_custom_keep(keys_from_buttons());
}

/* A flash as the guest's battle takes the screen, frames left (the switch
 * of engines, framed as BN6's own battles open with one; issue #65) */
#define GUEST_FLASH 20
static int guest_flash, guest_wait;   /* (and frames the guest has run before its battle shows: BN6's frame fades to white) */

/* The guest's last frame plain white (a few pixels across it) */
static bool guest_white(void) {
	const uint32_t *v = guest_video();
	if (!v) return false;
	static const int at[6][2] = { { 8, 8 }, { 120, 20 }, { 232, 40 }, { 60, 80 }, { 180, 120 }, { 120, 152 } };
	for (int k = 0; k < 6; ++k) {
		uint32_t c = v[at[k][1] * EMU_W + at[k][0]];
		if ((c & 0xF0) < 0xF0 || (c >> 8 & 0xF0) < 0xF0 || (c >> 16 & 0xF0) < 0xF0) return false;
	}
	return true;
}

/* A battle on the guest core: its frames in BN6's place, BN6's core
 * waiting, and its result into the run as it ends */
static bool guest_update(void) {
	static bool was;
	bool shown = guest_on_screen();
	if (shown && !was) guest_flash = GUEST_FLASH;
	else if (guest_flash > 0) --guest_flash;
	guest_wait = guest_active() && !shown ? guest_wait + 1 : 0;
	was = shown;
	if (!guest_active()) return false;
	uint32_t keys = guest_keys();
	/* (while its battle opens on a plain white screen, or before it shows:
	 * four of its frames a frame, unheard; BN5's opening held the white two
	 * and a half seconds, which a playtester read as a hang, session 65) */
	if (!shown || guest_white())
		for (int i = 0; i < 4 * dev.speed && guest_active() && (!guest_on_screen() || guest_white()); ++i) guest_frame_quiet(keys);
	else
		for (int i = 0; i < dev.speed && guest_active(); ++i) guest_frame(keys);
	GuestResult r;
	if (guest_take_result(&r)) director_guest_done(&r);
	return true;
}

static void update(void) {
	emu_drawing = false;
	if (guest_update()) return;
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
			/* (a guest battle begun from it: BN6's next frame waits for its
			 * end, as on the main thread, where it had run a frame further) */
			if (guest_active()) { started = false; break; }
			emu_frame(cinema_keys(frame_keys()));
			started = true;
		}
		if (revealed || director_arrived()) ++revealed;
		return;
	}
	uint32_t keys = frame_keys();
	if (devtools_open()) return;   /* the game holds still under the dev menu */
	/* (fast-forwarded: several game frames to one shown; none past a guest
	 * battle begun from one) */
	for (int i = 0; i < dev.speed && !guest_active(); ++i) {
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
	/* (the guest's battle once it is on its screen: before, BN6's last
	 * frame, where its opening drew the room its boot left it in) */
	bool guest = guest_on_screen();
	const uint32_t *v = guest ? guest_video() : emu_video();
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
	/* (another game's battle: its own screen, none of the layer's marks) */
	if (guest) {
		if (guest_flash > 0) fill_rect(P.core_x, P.core_y, EMU_W, EMU_H, rgba(255, 255, 255, (Uint8)(guest_flash * 255 / GUEST_FLASH)));
		return;
	}
	if (revealed < REVEAL_FRAMES) { P.fx_fade = REVEAL_FRAMES - revealed; P.fx_fade_color = BLACK; }
	cinema_draw();
	director_draw_map();
	if (guest_wait > 0) {
		fill_rect(P.core_x, P.core_y, EMU_W, EMU_H, rgba(255, 255, 255, (Uint8)(guest_wait >= GUEST_FLASH ? 255 : guest_wait * 255 / GUEST_FLASH)));
		return;
	}
	director_draw_counts();
	director_draw_bug_note();
	director_draw_duel();
	director_draw_tent();
	devtools_draw();
}

const Scene scene_emu = { "emu", enter, update, draw, leave };
