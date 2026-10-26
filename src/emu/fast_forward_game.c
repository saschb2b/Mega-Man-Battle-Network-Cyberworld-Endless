/* fast_forward_game.h. Fast forward played on BN6: where it may run is read
 * from the game before each frame of a batch, so a battle's first Custom
 * screen, BN6's PET or a shop stops the batch on the frame that opens it. */
#include "fast_forward_game.h"

#include <SDL.h>

#include "bn6.h"
#include "buttons.h"
#include "cinema.h"
#include "devtools.h"
#include "director.h"
#include "emu.h"
#include "fast_forward.h"
#include "gfx.h"
#include "online_visit.h"
#include "pet.h"
#include "pet_look.h"
#include "platform.h"

/* (a batch keeps to this much of a frame's 16.7 ms, the rest the
 * drawing's: a machine that cannot play 3x plays what it can) */
#define BUDGET_MS 10.0

static FastButton button;
static bool can;              /* it could run here: its button does something, the touch controls show it */
static bool running;          /* the update's batch runs fast: the mark shows */
static uint64_t updated_at;   /* (P.frame of the last batch: none this frame, no mark) */
static int planned;           /* the batch's frames at most */
static uint64_t begun, stamp; /* the batch's start, and its last frame's */
static double last_ms;        /* a frame's time, the batch's last */

static double ms_since(uint64_t t) { return (double)(SDL_GetPerformanceCounter() - t) * 1000.0 / (double)SDL_GetPerformanceFrequency(); }

static bool allowed(void) {
	FastScene s = {
		.run = director_on_layer() || director_in_town() || director_in_hp(),
		.apart = online_visit_active() || pet_holds() || devtools_open(),
	};
	if (s.run && !s.apart) {
		s.toolkit = emu_read8(emu_read32(BN6_TOOLKIT));
		s.sub = emu_read8(BN6_GAMESTATE);
		s.phase = emu_read8(BN6_BATTLE_PHASE);
		uint32_t settings = emu_read32(BN6_BATTLE_SETTINGS);
		s.mode = settings >= 0x08000000u && settings < 0x0A000000u ? emu_read8(settings + 3) : 0;
	}
	return fast_scene_allows(&s);
}

void fast_forward_update(void) {
	can = fast_speed() > FAST_OFF && allowed();
	/* (a press where it cannot run does nothing; one held into a fight is let go) */
	fast_button(&button, can && btn_held(BTN_FAST));
	running = button.on && can;
	planned = running ? fast_speed() : 1;
	updated_at = P.frame;
	begun = stamp = SDL_GetPerformanceCounter();
	last_ms = 0;
}

bool fast_forward_goes_on(int i) {
	if (i == 0) return true;
	last_ms = ms_since(stamp);
	stamp = SDL_GetPerformanceCounter();
	double budget = P.headless ? 1e9 : BUDGET_MS;
	if (!fast_batch_more(planned, i, ms_since(begun), last_ms, budget)) return false;
	/* (the frame before opened a fight or a menu: BN6's pace from here) */
	if (!allowed()) { running = can = false; return false; }
	return true;
}

bool fast_forward_running(void) { return running && updated_at == P.frame; }

/* (the touch controls draw after a frame or two of smooth motion's) */
bool fast_forward_available(void) { return can && updated_at + 2 >= P.frame; }

unsigned fast_forward_how(int i) {
	unsigned how = i > 0 ? EMU_FRAME_UNHEARD : 0;
	/* (another to follow, by the time the last took: this one not drawn) */
	double budget = P.headless ? 1e9 : BUDGET_MS;
	if (fast_batch_more(planned, i + 1, ms_since(begun), 2 * last_ms, budget)) how |= EMU_FRAME_UNSHOWN;
	return how;
}

/* ---- the mark: the PET's orange cursor twice, ▶▶, in the corner ---- */

#define MARK_GAP 6   /* (the second arrow over the first's point) */
#define MARK_W (LOOK_CURSOR_W + MARK_GAP + 2)

void fast_forward_draw(void) {
	if (!running || updated_at != P.frame) return;
	int under = director_counts_bottom();
	int x = P.core_x + 240 - MARK_W - 3, y = P.core_y + (under ? under + 2 : 3 + cinema_note_height());
	look_cursor(x, y);
	look_cursor(x + MARK_GAP, y);
}

void fast_forward_glyph(float cx, float cy, float h, float alpha) {
	int k = h >= 18 ? (int)(h / 9) : 1, w = (LOOK_CURSOR_W + MARK_GAP) * k;
	int x = (int)(cx - w / 2.f), y = (int)(cy - 9 * k / 2.f);
	uint8_t a = (uint8_t)(255 * (alpha < 0 ? 0 : alpha > 1 ? 1 : alpha));
	look_cursor_scaled(x, y, k, a);
	look_cursor_scaled(x + MARK_GAP * k, y, k, a);
}
