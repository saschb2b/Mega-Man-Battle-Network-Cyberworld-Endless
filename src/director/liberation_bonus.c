/* A BonusPanel as BN5's (liberation_bonus.h), traced in BN5's own board
 * (docs/ROM_DATA.md, BN5's Liberation Missions; bn5_liberation.h's
 * BN5_LIB_BONUS_*): its frames counted from the moment MegaMan's line of
 * it has closed, as BN5's routine counts them. */
#include "liberation_bonus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5_liberation.h"
#include "debug.h"
#include "director_state.h"
#include "game.h"
#include "liberation.h"
#include "liberation_pieces.h"
#include "liberation_state.h"
#include "liberation_words.h"
#include "emu.h"
#include "netmap.h"

/* The board's frames to BN5's flashes and band, from the line's close
 * (BN5's: the panel's dissolve 19, its shine 31, then the flashes) */
#define FLASH_1   50
#define FLASH_2   (FLASH_1 + 26)
#define FLASH_3   (FLASH_2 + 10)
#define OPEN      (FLASH_3 + 30)
/* ... and the roulette's from the band's first frame: wide at 3, the icon
 * in from BN5_LIB_BONUS_IN_AT + 3, A heard from LIVE */
#define WIDE      3
#define SLIDE_AT  (BN5_LIB_BONUS_IN_AT + WIDE)
#define LIVE      (SLIDE_AT + 8 + BN5_LIB_BONUS_LIVE_AT)
/* ... after A: held, out, shut */
#define OUT_AT    BN5_LIB_BONUS_HOLD
#define SHUT_AT   (OUT_AT + 8 + 3)
#define DONE_AT   (SHUT_AT + 2)
#define STRIKE_HOLD 60   /* frames the camera rests on what Major Hit strikes before it goes */
#define BACK_PHASES 2    /* (liberation.c's: a DarkHole's guardian back the Darkloids' phase after next) */

enum { PH_LEAD, PH_SPIN, PH_STOPPED, PH_AIM, PH_STRIKE };

static struct {
	int x, y;          /* the BonusPanel */
	int phase, t;      /* PH_*, frames in it */
	int slot, count;   /* the slot, frames on it */
	int target;        /* Major Hit's unit, -1 none */
} S;

static int clamp_op(int op) { return op > LIB_OP_MAX ? LIB_OP_MAX : op; }

static void say(const char *words, int after) {
	snprintf(LB.words, sizeof LB.words, "%s", words ? words : "");
	LB.after = after;
	LB.step = LIB_STEP_TALK;
}

void lib_bonus_begin(int x, int y) {
	memset(&S, 0, sizeof S);
	S.x = x; S.y = y;
	S.target = -1;
	LB.fx = x; LB.fy = y;
	/* (BN5's first slot: its random number's (& 15) % 5) */
	S.slot = (int)((rng_next() & 15) % LIB_BONUS_SLOTS);
	say(lib_bonus_found_words(), LIB_STEP_BONUS);
}

bool lib_bonus_spinning(void) { return LB.step == LIB_STEP_BONUS && S.phase == PH_SPIN && S.t - OPEN >= LIVE; }

/* The unit nearest MegaMan of guardians (`darkloid` false) or the
 * Darkloid, -1 none (BN5's 0x08054290: the least squared distance) */
static int nearest(bool darkloid) {
	int best = -1, bd = 1 << 30;
	for (int u = darkloid ? 0 : 1; u < (darkloid ? 1 : LB.b.nunits); ++u) {
		if (LB.b.unit[u].gone) continue;
		int dx = LB.b.unit[u].x - LB.mx, dy = LB.b.unit[u].y - LB.my, d = dx * dx + dy * dy;
		if (d < bd) { bd = d; best = u; }
	}
	return best;
}

/* The slot stopped on, given as BN5 gives it; the Navi acts again */
static void give(void) {
	char words[160];
	switch (S.slot) {
	case LIB_BONUS_SHIELD:
		LB.shield = true;
		lib_sound(BN5_LIB_BONUS_GUARD);
		say(lib_bonus_words(S.slot, 0), LIB_STEP_MOVE);
		break;
	case LIB_BONUS_HP:
		lib_hp_set(lib_hp() + 100 * 10);
		if (LB.npops < LIB_POPS) LB.pop[LB.npops++] = (__typeof__(LB.pop[0])){ (int8_t)S.x, (int8_t)S.y, BN5_ITEM_HEART, 0 };
		say(lib_bonus_words(S.slot, 0), LIB_STEP_MOVE);
		break;
	case LIB_BONUS_OP:
		LB.op = clamp_op(LB.op + 5);
		if (LB.npops < LIB_POPS) LB.pop[LB.npops++] = (__typeof__(LB.pop[0])){ (int8_t)S.x, (int8_t)S.y, BN5_ITEM_ORDER_POINTS, 0 };
		snprintf(words, sizeof words, "%s", lib_bonus_words(S.slot, LB.op));
		say(words, LIB_STEP_MOVE);
		break;
	case LIB_BONUS_CHIP: {
		/* (one of the board's four, alike: BN5's weights) */
		int k = (int)(rng_next() % LIB_BONUS_CHIPS), script = D.objs.lib_bonus[k];
		if (script >= 0 && LB.nfinds < LIB_FINDS) LB.finds[LB.nfinds++] = script;
		if (LB.npops < LIB_POPS) LB.pop[LB.npops++] = (__typeof__(LB.pop[0])){ (int8_t)S.x, (int8_t)S.y, BN5_ITEM_CHIP, 0 };
		say(lib_bonus_words(S.slot, 0), LIB_STEP_MOVE);
		break;
	}
	default:
		/* (the nearest guardian; none, the Darkloid: BN5's words first) */
		S.target = nearest(false);
		if (S.target < 0) S.target = nearest(true);
		S.phase = PH_AIM;
		S.t = 0;
		say(lib_bonus_words(S.slot, 0), S.target < 0 ? LIB_STEP_MOVE : LIB_STEP_BONUS);
		break;
	}
	if (emu_debug_on()) fprintf(stderr, "liberation: BonusPanel %d,%d gives slot %d (target %d)\n", S.x, S.y, S.slot, S.target);
}

/* Major Hit: the camera on its target, held, the blow, then home */
static void strike(void) {
	int wx, wy;
	netmap_world(LB.b.unit[S.target].x, LB.b.unit[S.target].y, &wx, &wy);
	if (S.phase == PH_AIM) {
		if (lib_camera_at(wx, wy)) { S.phase = PH_STRIKE; S.t = 0; }
		return;
	}
	if (++S.t == STRIKE_HOLD / 2) {
		if (S.target > 0) {
			/* (deleted; a DarkHole still standing sends him back as ever) */
			LB.b.unit[S.target].gone = true;
			LB.back_in[S.target] = BACK_PHASES;
		} else
			LB.darkloid_cut += 1;   /* (a fifth of his max off: his board HP's, docs/LIBERATION.md) */
	}
	if (S.t < STRIKE_HOLD || !lib_camera_home()) return;
	LB.step = LIB_STEP_MOVE;
}

static void roulette(void) {
	int r = S.t - OPEN;
	/* (the slot one on every BN5_LIB_BONUS_FRAMES frames, from the band's second) */
	if (S.phase == PH_SPIN && r >= 2 && ++S.count >= BN5_LIB_BONUS_FRAMES) {
		S.count = 0;
		S.slot = (S.slot + 1) % LIB_BONUS_SLOTS;
		if (r >= SLIDE_AT + 7) lib_sound(BN5_LIB_BONUS_TICK);
	}
	if (r == SLIDE_AT) lib_sound(BN5_LIB_PHASE_SOUND);
}

void lib_bonus_update(void) {
	if (S.phase == PH_AIM || S.phase == PH_STRIKE) { if (S.target >= 0) strike(); else LB.step = LIB_STEP_MOVE; return; }
	if (!lib_quiet() && S.phase == PH_LEAD && S.t == 0) return;
	/* (without BN5's ROM, nothing of its roulette to show: its first slot given) */
	if (S.phase == PH_LEAD && S.t == 0 && !lib_pieces_on()) {
		LB.b.kind[S.y][S.x] = LIB_CLEAN;
		give();
		return;
	}
	if (S.phase == PH_LEAD) {
		/* (the panel freed as BN5's dissolves, then its flashes) */
		if (S.t == 0) { LB.b.kind[S.y][S.x] = LIB_CLEAN; lib_look_freed(S.x, S.y); }
		if (S.t == FLASH_1 || S.t == FLASH_2 || S.t == FLASH_3) lib_sound(BN5_LIB_BONUS_FLASH);
		if (S.t + 1 == OPEN) S.phase = PH_SPIN;
		++S.t;
		return;
	}
	if (S.phase == PH_STOPPED) { if (++S.t >= DONE_AT) give(); return; }
	++S.t;
	roulette();
}

void lib_bonus_keys(uint32_t pressed) {
	if (!(pressed & KEY_A) || !lib_bonus_spinning()) return;
	lib_sound(BN5_LIB_BONUS_STOP);
	S.phase = PH_STOPPED;
	S.t = -1;   /* (0 this frame: BN5's routine changes as A is heard) */
}

/* ---- what it shows ---- */

static int whiteness(int t) {
	static const int at[3] = { FLASH_1, FLASH_2, FLASH_3 };
	for (int i = 0; i < 3; ++i) {
		int d = t - at[i];
		if (d >= 0 && d < 2) return 16;
		if (d >= 2 && d < 9) return 16 - 2 * (d - 1);
	}
	return 0;
}

/* The icon's place (BN5's +1): in from the right, held, out to the left */
static int icon_at(int r, bool stopped) {
	if (!stopped) return r < SLIDE_AT ? BN5_LIB_BONUS_FAR : r - SLIDE_AT >= 7 ? 0 : BN5_LIB_BONUS_FAR - BN5_LIB_BONUS_STEP * (r - SLIDE_AT + 1);
	int k = r - OUT_AT;
	if (k < 0) return 0;
	int o = -BN5_LIB_BONUS_STEP * (k + 1);
	return o < -BN5_LIB_BONUS_FAR ? -BN5_LIB_BONUS_FAR : o;
}

void lib_bonus_view(LibBonusView *v) {
	memset(v, 0, sizeof *v);
	v->words = -1;
	if (LB.step != LIB_STEP_BONUS || S.phase > PH_STOPPED) return;
	if (S.phase == PH_LEAD) { v->white = whiteness(S.t); return; }
	bool stopped = S.phase == PH_STOPPED;
	int r = stopped ? S.t : S.t - OPEN, mid = BN5_LIB_BAND_TOP + BN5_LIB_BAND_ROWS / 2;
	/* (the band in WIN1: a row, 17, then whole; shutting through 16) */
	v->band_top = BN5_LIB_BAND_TOP;
	v->band_rows = BN5_LIB_BAND_ROWS;
	if (!stopped && r < 2) { v->band_top = mid; v->band_rows = 1; }
	else if (!stopped && r == 2) { v->band_top = mid - 8; v->band_rows = 17; }
	else if (stopped && r == SHUT_AT) { v->band_top = mid - 8; v->band_rows = 16; }
	else if (stopped && r > SHUT_AT) { v->band_rows = 0; return; }
	int at = icon_at(r, stopped);
	v->icon = at > -BN5_LIB_BONUS_FAR;
	v->slot = S.slot;
	v->icon_x = (at + 15) * 8;
	if (!stopped && r >= LIVE) v->words = BN5_LIB_BONUS_PRESS;
	if (stopped && r < OUT_AT) v->words = BN5_LIB_BONUS_NAMES + S.slot;
}
