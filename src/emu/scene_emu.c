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
#include "guest.h"
#include "guest_wait.h"
#include "game.h"
#include "game_hooks.h"
#include "gfx.h"
#include "platform.h"
#include "rom.h"
#include "save.h"
#include "second.h"
#include "tour.h"

static SDL_Texture *tex;
bool emu_resume_requested;
bool emu_start_in_town;
bool emu_start_at_home;

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
	game_hooks_install();
	cinema_reset();
	if (emu_resume_requested) {
		emu_resume_requested = false;
		game_hooks_after_boot();
		director_resume();
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
	audio_external(NULL);
	platform_second_screen(NULL, NULL);
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
	second_update();
	emu_debug_frame();
}

/* The keys a guest battle gets (guest.h): the player's, or the autopilot's,
 * which reads its battle as it reads BN6's (autopilot.c) */
/* An L or R pressed before the Custom gauge filled, kept two and a half
 * seconds and given as it fills (let go a frame first, so BN5 sees a
 * press), again every 20 frames until the screen opens, as BN6's battles
 * keep one (director_keys.c custom_buffer): a playtester's R pressed a little early did
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
	/* (where the battle waits for the guest's boot, the older net's wait
	 * on the white: guest_wait.c) */
	guest_wait_update(guest_boot_waiting(), guest_wait >= GUEST_FLASH);
	GuestResult r;
	/* (one that waited for a boot that failed ended unfought, as the boot
	 * did, between the frames: guest_tick) */
	if (!guest_active() && guest_take_result(&r)) director_guest_done(&r);
	if (!guest_active()) return false;
	/* (the dev menu over it too: it holds still while the menu is open) */
	uint32_t keys = devtools_keys(guest_keys());
	if (devtools_open()) return true;
	/* (its boot first, where the battle waits for it: it runs in the
	 * background, guest_tick, which begins the battle at its end) */
	if (guest_boot_waiting()) return true;
	/* (while its battle opens on a plain white screen, or before it shows:
	 * four of its frames a frame, unheard; BN5's opening held the white two
	 * and a half seconds, which a playtester read as a hang, session 65) */
	if (!shown || guest_white())
		for (int i = 0; i < 4 * dev.speed && guest_active() && (!guest_on_screen() || guest_white()); ++i) guest_frame_quiet(keys);
	else
		for (int i = 0; i < dev.speed && guest_active(); ++i) {
			guest_frame(keys);
			devtools_guest_update();
		}
	if (guest_take_result(&r)) director_guest_done(&r);
	return true;
}

static void update(void) {
	emu_drawing = false;
	/* (the run's time, its battles in the older net's engine too: the
	 * statistics' minutes, save.h) */
	profile_played_frame();
	/* (an older net's battle: BN6's frame waits, the second screen still
	 * follows) */
	if (guest_update()) {
		second_update();
		return;
	}
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

/* BN6's frame as the switch to a guest battle takes it, `t` frames in: in
 * blocks growing wider than tall as it fades to white, as BN6's own battle
 * switch draws the map (captured in a BN6 battle: 16 pixels wide at most) */
static void switch_blocks(uint32_t *px, int t) {
	int mx = 1 + t * 15 / GUEST_FLASH, my = 1 + t * 3 / GUEST_FLASH;
	for (int y0 = 0; y0 < EMU_H; y0 += my)
		for (int x0 = 0; x0 < EMU_W; x0 += mx) {
			uint32_t c = px[y0 * EMU_W + x0];
			for (int y = y0; y < y0 + my && y < EMU_H; ++y)
				for (int x = x0; x < x0 + mx && x < EMU_W; ++x) px[y * EMU_W + x] = c;
		}
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
	bool box = false;
	int hour = guest ? 0 : director_town_tint(&box);
	for (int i = 0; i < EMU_W * EMU_H; ++i) {
		uint32_t c = hour ? tinted(v[i], hour, box && i / EMU_W >= TINT_BOX_TOP) : v[i];
#ifdef __3DS__
		px[i] = 0xFF000000u | (c & 0xFF00u) | (c & 0xFFu) << 16 | (c >> 16 & 0xFFu);
#else
		px[i] = c | 0xFF000000u;
#endif
	}
	if (!guest && guest_wait > 0 && guest_wait < GUEST_FLASH) switch_blocks(px, guest_wait);
	SDL_UpdateTexture(tex, NULL, px, EMU_W * 4);
	int dx = 0, dy = 0;
	if (!guest) cinema_offset(&dx, &dy);   /* (a shake under way as it began holds still while it runs) */
	SDL_Rect dst = { P.core_x + dx, P.core_y + dy, EMU_W, EMU_H };
	SDL_RenderCopy(P.renderer, tex, NULL, &dst);
	if (!revealed) { fill_rect(P.core_x, P.core_y, EMU_W, EMU_H, BLACK); return; }
	/* (another game's battle: its own screen, none of the layer's marks;
	 * the dev menu's alone) */
	if (guest) {
		if (guest_flash > 0) fill_rect(P.core_x, P.core_y, EMU_W, EMU_H, rgba(255, 255, 255, (Uint8)(guest_flash * 255 / GUEST_FLASH)));
		guest_wait_draw(P.core_x, P.core_y);   /* (closing over its battle's own white, where one waited) */
		devtools_draw();
		return;
	}
	if (revealed < REVEAL_FRAMES) { P.fx_fade = REVEAL_FRAMES - revealed; P.fx_fade_color = BLACK; }
	cinema_draw();
	director_draw_map();
	if (guest_wait > 0) {
		fill_rect(P.core_x, P.core_y, EMU_W, EMU_H, rgba(255, 255, 255, (Uint8)(guest_wait >= GUEST_FLASH ? 255 : guest_wait * 255 / GUEST_FLASH)));
		guest_wait_draw(P.core_x, P.core_y);
		return;
	}
	director_draw_counts();
	director_draw_bug_note();
	director_draw_duel();
	director_draw_tent();
	devtools_draw();
}

const Scene scene_emu = { "emu", enter, update, draw, leave };
