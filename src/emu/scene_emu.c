/* The running game in the window: the core's frame in the 240x160 view,
 * its sound on the audio device and the port's buttons as GBA keys. */
#include <SDL.h>

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

/* (a part of the frame timed for the frame log, where it is on: platform.h, frame_log_part) */
#define TIMED(name, call) do { \
		uint64_t t0_ = frame_part_begin(); \
		const char *was_ = emu_draw_part; \
		emu_draw_part = name; \
		call; \
		emu_draw_part = was_; \
		frame_part_end(name, t0_); \
	} while (0)


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
	music_carry_drop();
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
	/* (the dev menu's keys first: its A is never a dive's A on a diver) */
	keys = director_keys(pet_keys(online_visit_keys(devtools_keys(keys))));
	director_before_keys(keys);
	return keys;
}

/* What follows a frame of the game: the run's logic on it. */
static void after_frame(void) {
	online_visit_update();
	if (online_visit_away()) { second_update(); emu_debug_frame(); return; }
	director_see();
	/* (a pick's choice starts its script before the director's own talk
	 * can take the free chat box: MegaMan's words on the battle's older
	 * Net virus came before "MegaMan got: DiveMan", session 78) */
	director_pick_watch();
	director_update();
	director_lab_watch();
	director_hold_watch();
	director_settle_watch();
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
/* The game left for the run's title: a Net Square visit's connection
 * closed, the director stopped, the checkpoint as it stands. */
static void to_title(void) {
	online_visit_shutdown();
	emu_sync();
	director_stop();
	cinema_reset();
	started = false;
	P.quit_prompt = 0;
	scene_set(&scene_title);
}

static bool soft_reset(void) {
	uint32_t chord = KEY_A | KEY_B | KEY_SELECT | KEY_START;
	static bool from_square;
	if ((keys_from_buttons() & chord) != chord) { from_square = false; return false; }
	/* (in the Net Square: back to the gate's checkpoint, as Leave; the
	 * chord still held after it is no reset, and BN6 never sees it, whose
	 * own soft reset it is) */
	if (from_square) return true;
	if (online_visit_away()) { online_visit_leave(VISIT_END_LEFT); from_square = true; return true; }
	to_title();
	return true;
}

#define SETTLE_MOST 600   /* frames run on at most: a won battle's RESULT paged and the way to the map took about 200 (session 76) */

/* The quit's save waits for MegaMan free; on a won battle's way back to
 * the map (its RESULT, the fade, the map's entry) the game is run on to
 * him free first, unseen and unheard, so the battle and its reward are
 * kept with the run (director_settle.c). On the core's own thread, the
 * frame begun last is ended and its logic run first, and the next update
 * begins afresh. */
void emu_keep_run(void) {
	if (!director_can_suspend() && director_settles()) {
		emu_sync();
		if (emu_threaded() && started) after_frame();
		started = false;
		director_settling(true);
		int n = 0;
		for (; n < SETTLE_MOST && director_settles() && !director_can_suspend(); ++n) {
			emu_frame_as(director_settle_keys(n), EMU_FRAME_UNSHOWN | EMU_FRAME_UNHEARD);
			emu_sync();
			after_frame();
		}
		director_settling(false);
		printf("quit: the game run on %d frames after the battle, MegaMan %s\n", n, director_can_suspend() ? "free" : "still held");
	}
	director_suspend();
}

/* Options' Return to title (options.c; docs/engine/settings-screens.md,
 * Return to title): offered wherever the game is the scene but in the Net
 * Square, whose own Leave is the way out of it. */
bool emu_title_offered(void) {
	return scene_current() == &scene_emu && !online_visit_away() && !online_visit_held() && !online_visit_waiting();
}

/* ... where the run goes on from, the quit prompt's own line */
const char *emu_title_where(void) { return director_on_layer() ? director_quit_where() : NULL; }

/* ... and its Yes: the run kept as a quit keeps it, where the soft reset
 * leaves the last checkpoint, then the title on its menu, CONTINUE and
 * SAVES' key a press away */
void emu_to_title(void) {
	emu_keep_run();
	title_menu = true;
	to_title();
}

#define LAB_LOAD_SPEED 24   /* the game's frames to one shown while a set's field comes up: about 200 of BN6's in a sixth of a second */
static int frames_speed(void) { return director_lab_loading() ? LAB_LOAD_SPEED : dev.speed; }
/* whether the i-th of the game's frames to the one shown runs: a speed's count of them (the Lab's ending as its field
 * is up), or fast forward's */
static bool frame_goes_on(int i, int speed) { return speed > 1 ? i < speed && !director_lab_shown() : fast_forward_goes_on(i); }

/* the game held under a screen of the engine's this frame, and the scene's
 * own Options and the touch controls' menu (main.c): its music carried on
 * under it (audio.h, music_carry_held), where no screen plays its own (the
 * Jukebox) and no other core sounds (a NetBattle from the Net Square) */
static void held(void) {
	if (!pet_plays_music() && !online_visit_held() && !online_visit_waiting()) music_carry_held();
}

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
		held();
		return;
	}
	/* (the Battle Lab's builder up, docs/game/battle-lab.md: the same) */
	if (director_lab_shown()) {
		director_lab_keys(keys_from_buttons());
		held();
		return;
	}
	/* (the PET's Jukebox or Library up: the game's core, its music with it, holds still under it) */
	if (pet_holds()) {
		held();
		/* (the Library's chip under the cursor on the second screen: read again while the game holds) */
		if (pet_second_follows()) {
			second_update();
			platform_second_screen_draw();
		}
		return;
	}
	/* (the second screen first, from what the last update saw: the GBA's
	 * frame runs on beside it, where the reads below would wait for it) */
	TIMED("second screen", platform_second_screen_draw());
	/* (several of the game's frames to one shown: the dev menu's speed, or
	 * fast forward where it may run, fast_forward_game.h) */
	/* (the Battle Lab's builder bringing a set's field up: BN6's battle opening run through at once, lab_battle.h) */
	int speed = frames_speed();
	bool dev_fast = speed > 1;
	/* the core on a thread of its own (emu.c): the frame done taken in,
	 * then the next begun, which runs while this one is drawn; keys, frames
	 * and the logic keep their order */
	if (emu_threaded()) {
		if (devtools_open()) { frame_keys(); held(); return; }
		music_carry_resume();
		if (!dev_fast) fast_forward_update();
		for (int i = 0; frame_goes_on(i, speed); ++i) {
			if (started) TIMED("director", after_frame());
			emu_frame_as(cinema_keys(frame_keys()), dev_fast ? 0 : fast_forward_how(i));
			started = true;
		}
		if (revealed || director_arrived()) ++revealed;
		return;
	}
	uint32_t keys = frame_keys();
	if (devtools_open()) { held(); return; }   /* the game holds still under the dev menu */
	music_carry_resume();
	if (!dev_fast) fast_forward_update();
	for (int i = 0; frame_goes_on(i, speed); ++i) {
		/* (the way into BN6's folder editor is pressed a frame at a time, run through at once: its keys each frame) */
		if (i && director_lab_editing()) keys = frame_keys();
		emu_frame_as(cinema_keys(keys), dev_fast ? 0 : fast_forward_how(i));
		TIMED("director", after_frame());
	}
	if (revealed || director_arrived()) ++revealed;
}

/* The engine's own drawing over the game's picture, each part timed */
static void overlays_draw(void) {
	TIMED("cinema", cinema_draw());
	TIMED("map", director_draw_map());
	TIMED("counts", director_draw_counts());
	TIMED("fast forward", fast_forward_draw());
	TIMED("bug note", director_draw_bug_note());
	TIMED("duel", director_draw_duel());
	TIMED("board", director_draw_board());
	TIMED("tent", director_draw_tent());
	TIMED("dev tools", devtools_draw());
	TIMED("visit", online_visit_draw());
	TIMED("pet", pet_draw());
	TIMED("pick", director_draw_pick());
	TIMED("lab", director_draw_lab());
}

static void draw(void) {
	emu_drawing = true;   /* (until the next update: the second screen's too) */
	fill_rect(0, 0, P.w, P.h, BLACK);
	if (!emu_ready()) return;
	if (online_visit_held()) { online_visit_draw(); return; }
	int dx = 0, dy = 0;
	cinema_offset(&dx, &dy);
	TIMED("picture", emu_picture_draw(P.core_x + dx, P.core_y + dy));
	if (!revealed) { fill_rect(P.core_x, P.core_y, EMU_W, EMU_H, BLACK); return; }
	if (revealed < REVEAL_FRAMES) { P.fx_fade = REVEAL_FRAMES - revealed; P.fx_fade_color = BLACK; }
	if (online_visit_away()) { online_visit_draw(); return; }
	overlays_draw();
}

const Scene scene_emu = { "emu", enter, update, draw, leave, held };
