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
#include "fast_forward_game.h"
#include "game.h"
#include "game_hooks.h"
#include "gfx.h"
#include "online_visit.h"
#include "platform.h"
#include "rom.h"
#include "second.h"
#include "tour.h"
#include "xinstall.h"
#ifdef __3DS__
#include "start_3ds.h"
#endif

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
	 * copy's patches, the same at every boot; docs/multirom/) */
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
	director_pick_shut();
	online_visit_shutdown();
	audio_external(NULL);
	platform_second_screen(NULL, NULL);
}

static uint32_t frame_keys(void) {
	uint32_t keys = autopilot_on() ? autopilot_keys() : keys_from_buttons();
	/* (in the Net Square the run's own keys rest: docs/online/) */
	if (online_visit_away()) return online_visit_keys(keys);
	/* (the Battle Lab's folder in BN6's own editor, docs/game/battle-lab.md: its keys alone, no screen of ours over it) */
	if (director_lab_editing()) return director_lab_edit_keys(keys);
	return director_keys(pet_keys(devtools_keys(online_visit_keys(keys))));
}

/* What follows a frame of the game: the run's logic on it. */
static void after_frame(void) {
	online_visit_update();
	if (online_visit_away()) { second_update(); emu_debug_frame(); return; }
	director_see();
	director_update();
	director_pick_watch();
	director_lab_watch();
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

#define LAB_LOAD_SPEED 24   /* the game's frames to one shown while a set's field comes up: about 200 of BN6's in a sixth of a second */
static int frames_speed(void) { return director_lab_loading() ? LAB_LOAD_SPEED : dev.speed; }
/* whether the i-th of the game's frames to the one shown runs: a speed's count of them (the Lab's ending as its field
 * is up), or fast forward's */
static bool frame_goes_on(int i, int speed) { return speed > 1 ? i < speed && !director_lab_shown() : fast_forward_goes_on(i); }

static void update(void) {
	emu_drawing = false;
	if (soft_reset()) return;
	/* (a NetBattle from the Net Square: the game's core holds still under it) */
	if (online_visit_held()) { online_visit_held_frame(keys_from_buttons()); return; }
	/* (the link to the Square dropped: the game's core holds still under the wait) */
	if (online_visit_waiting()) { online_visit_wait_frame(); return; }
	/* (a reward pick up, docs/game/reward-pick.md: the game's core holds still under it, its keys the screen's) */
	if (director_pick_shown()) {
		director_pick_keys(autopilot_on() ? autopilot_keys() : keys_from_buttons());
		return;
	}
	/* (the Battle Lab's builder up, docs/game/battle-lab.md: the same) */
	if (director_lab_shown()) {
		director_lab_keys(keys_from_buttons());
		return;
	}
	/* (the PET's Jukebox or Library up: the game's core, its music with it, holds still under it) */
	if (pet_holds()) {
		/* (the Library's chip under the cursor on the second screen: read again while the game holds) */
		if (pet_second_follows()) {
			second_update();
			platform_second_screen_draw();
		}
		return;
	}
	/* (the second screen first, from what the last update saw: the GBA's
	 * frame runs on beside it, where the reads below would wait for it) */
	platform_second_screen_draw();
	/* (several of the game's frames to one shown: the dev menu's speed, or
	 * fast forward where it may run, fast_forward_game.h) */
	/* (the Battle Lab's builder bringing a set's field up: BN6's battle opening run through at once, lab_battle.h) */
	int speed = frames_speed();
	bool dev_fast = speed > 1;
	/* the core on a thread of its own (emu.c): the frame done taken in,
	 * then the next begun, which runs while this one is drawn; keys, frames
	 * and the logic keep their order */
	if (emu_threaded()) {
		if (devtools_open()) { frame_keys(); return; }
		if (!dev_fast) fast_forward_update();
		for (int i = 0; frame_goes_on(i, speed); ++i) {
			if (started) after_frame();
			emu_frame_as(cinema_keys(frame_keys()), dev_fast ? 0 : fast_forward_how(i));
			started = true;
		}
		if (revealed || director_arrived()) ++revealed;
		return;
	}
	uint32_t keys = frame_keys();
	if (devtools_open()) return;   /* the game holds still under the dev menu */
	if (!dev_fast) fast_forward_update();
	for (int i = 0; frame_goes_on(i, speed); ++i) {
		/* (the way into BN6's folder editor is pressed a frame at a time, run through at once: its keys each frame) */
		if (i && director_lab_editing()) keys = frame_keys();
		emu_frame_as(cinema_keys(keys), dev_fast ? 0 : fast_forward_how(i));
		after_frame();
	}
	if (revealed || director_arrived()) ++revealed;
}

static void draw(void) {
	emu_drawing = true;   /* (until the next update: the second screen's too) */
	fill_rect(0, 0, P.w, P.h, BLACK);
	if (!emu_ready()) return;
	if (online_visit_held()) { online_visit_draw(); return; }
	int dx = 0, dy = 0;
	cinema_offset(&dx, &dy);
	emu_picture_draw(P.core_x + dx, P.core_y + dy);
	if (!revealed) { fill_rect(P.core_x, P.core_y, EMU_W, EMU_H, BLACK); return; }
	if (revealed < REVEAL_FRAMES) { P.fx_fade = REVEAL_FRAMES - revealed; P.fx_fade_color = BLACK; }
	if (online_visit_away()) { online_visit_draw(); return; }
	cinema_draw();
	director_draw_map();
	director_draw_counts();
	fast_forward_draw();
	director_draw_bug_note();
	director_draw_duel();
	director_draw_board();
	director_draw_tent();
	devtools_draw();
	online_visit_draw();
	pet_draw();
	director_draw_pick();
	director_draw_lab();
}

const Scene scene_emu = { "emu", enter, update, draw, leave };
