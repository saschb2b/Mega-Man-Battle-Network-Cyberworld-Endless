/* --dev rollbench (rollbench.h). Every number is the wall clock's around
 * the work alone (SDL's performance counter), mean and worst. */
#include "rollbench.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include <mgba/core/core.h>

#include "arena_boot.h"
#include "arena_pair.h"
#include "bn6.h"
#include "emu.h"
#include "gfx.h"
#include "platform.h"
#include "protocol.h"
#include "rom.h"

#define ONE_FRAMES   300    /* the game's core alone: from power-on, through its start */
#define PAIR_FRAMES  600    /* the pair: ten seconds of the battle */
#define DEPTH        8      /* the rollback timed and checked: eight frames back */
#define CHECK_EVERY  50     /* frames between two of them */
#define SLICE        20     /* frames a tick, so the screen shows the way */
#define BOOT_SLICE   64
#define FRAME_MS     (1000.0 / 60.0)
#define HEADROOM     0.25   /* of a frame, kept for the rest: drawing, the picture's present, the system */
#define LINES        16

typedef struct { double sum, worst; unsigned n; } Stat;

typedef enum { ONE, BOOT, PAIR, DONE } Stage;

static struct {
	Stage stage;
	unsigned frame;
	uint8_t *state;
	size_t one_size;
	Stat one_save, one_load, one_step;
	ArenaBoot *boot;
	ArenaPair *pair;
	ArenaPairSnap *ring[DEPTH], *straight, *again;
	uint16_t keys[DEPTH][ARENA_LOOKAHEAD][2];
	Stat save, load, step, back;
	unsigned checks, agreed, game;
	char lines[LINES][64];
	int nlines;
	const char *why;
} B;

static uint64_t now(void) { return SDL_GetPerformanceCounter(); }

static void count(Stat *s, uint64_t from) {
	double ms = (double)(now() - from) * 1000.0 / (double)SDL_GetPerformanceFrequency();
	s->sum += ms;
	if (ms > s->worst) s->worst = ms;
	++s->n;
}

static double mean(const Stat *s) { return s->n ? s->sum / s->n : 0; }

/* a line said: into the output and onto the screen (the screen's short form, where given) */
static void say(const char *screen, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void say(const char *screen, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	fputs("rollbench: ", stdout);
	vprintf(fmt, ap);
	fputc('\n', stdout);
	va_end(ap);
	fflush(stdout);
	if (screen && B.nlines < LINES) snprintf(B.lines[B.nlines++], sizeof B.lines[0], "%s", screen);
}

/* ... onto the screen alone */
static void show(const char *screen) {
	if (B.nlines < LINES) snprintf(B.lines[B.nlines++], sizeof B.lines[0], "%s", screen);
}

/* ---- the game's core alone (every build) ---- */

static void one_frames(unsigned n) {
	for (unsigned i = 0; i < n && B.frame < ONE_FRAMES; ++i, ++B.frame) {
		/* (START and A now and then: through the game's start to its menus) */
		uint32_t keys = B.frame % 40 < 2 ? KEY_START : B.frame % 40 == 20 ? KEY_A : 0;
		uint64_t t = now();
		if (!emu_state_save(B.state)) { B.why = "the game's core gave no state"; return; }
		count(&B.one_save, t);
		t = now();
		emu_frame(keys);
		emu_sync();
		count(&B.one_step, t);
		/* (a rollback of one: the state put back, the frame run again) */
		t = now();
		if (!emu_state_load(B.state)) { B.why = "the game's core took no state"; return; }
		count(&B.one_load, t);
		emu_frame(keys);
		emu_sync();
	}
}

static void one_said(void) {
	char s[64];
	snprintf(s, sizeof s, "1 CORE %zuKB", B.one_size / 1024);
	say(s, "the game's core alone (from power-on, not a battle): its state %zu bytes", B.one_size);
	snprintf(s, sizeof s, " SAVE %.2f/%.2f LOAD %.2f/%.2f", mean(&B.one_save), B.one_save.worst, mean(&B.one_load), B.one_load.worst);
	say(s, "  save %.3f ms (worst %.3f), load %.3f ms (worst %.3f), a frame %.3f ms (worst %.3f), over %u frames",
		mean(&B.one_save), B.one_save.worst, mean(&B.one_load), B.one_load.worst, mean(&B.one_step), B.one_step.worst, B.one_save.n);
	snprintf(s, sizeof s, " FRAME %.2f/%.2f MS", mean(&B.one_step), B.one_step.worst);
	show(s);
}

/* ---- the pair (CW_ARENA) ---- */

/* each side's keys for a frame: at the Custom screen the cursor to OK and
 * OK (the --dev arena peer's script); in the battle a step, a shot or the
 * buster a few frames at a time, from a number of the frame and side */
static uint16_t script(unsigned side, uint32_t frame, unsigned phase) {
	if (phase == BN6_PHASE_CUSTOM) {
		unsigned t = frame % 160;
		if (t >= 40 && t < 100) return (t - 40) % 12 < 4 ? KEY_RIGHT : 0;
		return t >= 100 && t < 104 ? KEY_A : 0;
	}
	static const uint16_t moves[8] = { 0, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_A, KEY_B, KEY_B };
	uint32_t x = (frame / 8) * 2654435761u ^ (side + 1) * 40503u;
	x ^= x >> 13;
	return moves[(x * 2246822519u) >> 29];
}

static void pair_keys(uint32_t frame, uint16_t keys[ARENA_LOOKAHEAD][2]) {
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *c = arena_pair_core(B.pair, side);
		unsigned phase = c->rawRead8(c, BN6_BATTLE_PHASE, -1);
		for (unsigned k = 0; k < ARENA_LOOKAHEAD; ++k) keys[k][side] = script(side, frame + k, phase);
	}
}

static bool pair_open(void) {
	for (unsigned i = 0; i < DEPTH; ++i)
		if (!(B.ring[i] = arena_pair_snap_new(B.pair))) return false;
	return (B.straight = arena_pair_snap_new(B.pair)) && (B.again = arena_pair_snap_new(B.pair));
}

/* a rollback of DEPTH frames, its time and whether the pair comes back to
 * the same state as run straight */
static void pair_rollback(void) {
	uint64_t straight = arena_pair_hash(B.pair), t = now();
	arena_pair_save(B.pair, B.straight);
	unsigned from = B.frame + 1 - DEPTH;
	if (!arena_pair_load(B.pair, B.ring[from % DEPTH])) { B.why = "the pair took no snapshot"; return; }
	for (unsigned f = from; f <= B.frame; ++f) {
		arena_pair_save(B.pair, B.ring[f % DEPTH]);
		if (!arena_pair_step(B.pair, (const uint16_t (*)[2])B.keys[f % DEPTH], true)) { B.why = "the cable stuck"; return; }
	}
	count(&B.back, t);
	++B.checks;
	if (arena_pair_hash(B.pair) == straight) ++B.agreed;
	arena_pair_save(B.pair, B.again);
	if (!arena_pair_snap_diff(B.straight, B.again, B.frame, true)) ++B.game;
}

static void pair_frames(unsigned n) {
	for (unsigned i = 0; i < n && B.frame < PAIR_FRAMES && !B.why; ++i, ++B.frame) {
		ArenaPairSnap *snap = B.ring[B.frame % DEPTH];
		uint16_t (*keys)[2] = B.keys[B.frame % DEPTH];
		pair_keys(B.frame, keys);
		uint64_t t = now();
		if (!arena_pair_save(B.pair, snap)) { B.why = "the pair gave no snapshot"; return; }
		count(&B.save, t);
		t = now();
		if (!arena_pair_step(B.pair, (const uint16_t (*)[2])keys, true)) { B.why = "the cable stuck"; return; }
		count(&B.step, t);
		/* (a rollback of one each frame: the snapshot put back, the frame run again) */
		t = now();
		if (!arena_pair_load(B.pair, snap)) { B.why = "the pair took no snapshot"; return; }
		count(&B.load, t);
		arena_pair_step(B.pair, (const uint16_t (*)[2])keys, true);
		if (B.frame >= DEPTH && B.frame % CHECK_EVERY == 0) pair_rollback();
	}
}

/* frames a tick holds after one load: each resimulated one saved, then the new one */
static int window(double budget, double load, double step, double save) {
	double per = step + save;
	if (per <= 0 || budget <= load + per) return 0;
	return (int)((budget - load) / per) - 1;
}

static void pair_said(void) {
	size_t core = arena_pair_snap_core_bytes(B.ring[0]), all = arena_pair_snap_bytes(B.ring[0]);
	char s[64];
	snprintf(s, sizeof s, "PAIR SNAP %zuKB (+%zuB)", all / 1024, all - 2 * core);
	say(s, "the pair (BN6's rules, a Practice NetBattle, %u frames of scripted keys): a core's state %zu bytes, the snapshot %zu "
		"(%zu of the cable's and the pair's own)", B.step.n, core, all, all - 2 * core);
	snprintf(s, sizeof s, " SAVE %.2f/%.2f LOAD %.2f/%.2f", mean(&B.save), B.save.worst, mean(&B.load), B.load.worst);
	say(s, "  save %.3f ms (worst %.3f), load %.3f ms (worst %.3f), a pair frame %.3f ms (worst %.3f)", mean(&B.save), B.save.worst,
		mean(&B.load), B.load.worst, mean(&B.step), B.step.worst);
	snprintf(s, sizeof s, " FRAME %.2f/%.2f BACK%u %.1f/%.1f", mean(&B.step), B.step.worst, DEPTH, mean(&B.back), B.back.worst);
	say(s, "  a rollback of %u frames %.3f ms (worst %.3f); %u checked: the game's state the same in %u, the whole state (the "
		"sound's too) in %u", DEPTH, mean(&B.back), B.back.worst, B.checks, B.game, B.agreed);
	snprintf(s, sizeof s, " CHECKS %u: GAME %u SAME, ALL %u", B.checks, B.game, B.agreed);
	show(s);
	double budget = FRAME_MS * (1.0 - HEADROOM);
	int w = window(budget, mean(&B.load), mean(&B.step), mean(&B.save));
	int worst = window(budget, B.load.worst, B.step.worst, B.save.worst);
	snprintf(s, sizeof s, "WINDOW %d (WORST %d) FRAMES", w, worst);
	say(s, "a tick's budget %.1f ms (%.0f%% of a frame kept for the rest): a window of %d frames at the means, %d at the worst",
		budget, HEADROOM * 100, w, worst);
	/* (at delay D the pair's frame needs the peer's keys 3 frames on, so 3 - D of them are guessed even with
	 * no latency: a window W covers a one way of W + D - 3 frames) */
	for (int d = 1; d <= 3; ++d) {
		int frames = w + d - 3;
		snprintf(s, sizeof s, " DELAY %d: ONE WAY %d F %.0f MS", d, frames, frames * FRAME_MS);
		say(s, "  delay %d: covers a one way of %d frames (%.0f ms; %d at the worst)", d, frames, frames * FRAME_MS, worst + d - 3);
	}
}

/* ---- the scene ---- */

static void enter(void) {
	memset(&B, 0, sizeof B);
	say("ROLLBENCH", "issue #128's measurement, on this machine");
	if (!emu_init(R.data, ROM_SIZE) || !(B.one_size = emu_state_size()) || !(B.state = malloc(B.one_size)))
		B.why = "the game's core did not start";
}

static void finish(void) {
	if (B.why) say(B.why, "stopped: %s", B.why);
	B.stage = DONE;
	if (P.headless) P.quit = true;
}

static void update(void) {
	if (B.stage == DONE) {
		if (btn_pressed(BTN_A) || btn_pressed(BTN_B)) P.quit = true;
		return;
	}
	if (B.why) { finish(); return; }
	if (B.stage == ONE) {
		one_frames(SLICE);
		if (B.why || B.frame < ONE_FRAMES) return;
		one_said();
		if (!CW_ARENA) {
			say("NO ARENA IN THIS BUILD", "the Arena is not in this build (CW_ARENA): the game's core alone");
			finish();
			return;
		}
		const OnlineLoadout *loadout[2] = { NULL, NULL };
		B.boot = arena_boot_new(R.data, ROM_SIZE, loadout, ONLINE_FORMAT_BN6);
		if (!B.boot) B.why = "no memory for the pair";
		B.stage = BOOT;
		B.frame = 0;
		return;
	}
	if (B.stage == BOOT) {
		ArenaBootStage s = arena_boot_run(B.boot, BOOT_SLICE);
		if (s == ARENA_BOOT_FAILED) B.why = arena_boot_error(B.boot);
		if (s != ARENA_BOOT_READY) return;
		B.pair = arena_boot_take(B.boot);
		arena_boot_free(B.boot);
		B.boot = NULL;
		if (!pair_open()) B.why = "no memory for the snapshots";
		B.stage = PAIR;
		return;
	}
	pair_frames(SLICE);
	if (B.why || B.frame < PAIR_FRAMES) return;
	pair_said();
	finish();
}

static void draw(void) {
	fill_rect(0, 0, P.w, P.h, rgba(0, 0, 0, 255));
	int x = P.core_x + 4, y = P.core_y + 4;
	for (int i = 0; i < B.nlines; ++i) text_draw(x, y + 9 * i, B.lines[i], i ? WHITE : rgba(170, 200, 255, 255), TEXT_LEFT);
	if (B.stage != DONE) {
		unsigned total = B.stage == ONE ? ONE_FRAMES : B.stage == PAIR ? PAIR_FRAMES : 0;
		if (total) text_drawf(x, P.core_y + ARENA_H - 12, WHITE, TEXT_LEFT, "%s %u/%u", B.stage == ONE ? "CORE" : "PAIR", B.frame, total);
	}
}

static void leave(void) {
	arena_boot_free(B.boot);
	for (unsigned i = 0; i < DEPTH; ++i) arena_pair_snap_free(B.ring[i]);
	arena_pair_snap_free(B.straight);
	arena_pair_snap_free(B.again);
	arena_pair_close(B.pair);
	free(B.state);
	memset(&B, 0, sizeof B);
}

const Scene scene_rollbench = { "rollbench", enter, update, draw, leave };
