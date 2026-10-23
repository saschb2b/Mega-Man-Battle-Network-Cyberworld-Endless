/* A liberation's battle on BN6's own battle loop (liberation_battle.h).
 *
 * BN5 counts a liberation's turns in BattleState +7, one more as each
 * Custom screen opens, and BN6 keeps that count (BN6_BATTLE_SCREENS) and
 * BN5's banners in its battle messages, though not BN5's rules: so four
 * hooks answer its loop as BN5's loop decides (docs/ROM_DATA.md, BN6's
 * battle loop and banners). As each turn's fight starts, BN5's banner for
 * it (0x08007954: TURN n START! below the third, FINAL TURN! from it). In
 * the fight, where the final turn's gauge is full, BN5's time up: BN6's
 * own draw (the battle stopped, its banner, the battle over with result 3:
 * no reward, HP as it stands), its banner TIME UP! and then, the draw run
 * again, LIBERATE FAILED! (BN5's 0x08007C24). At a win, LIBERATE SUCCESS!,
 * or 1 TURN LIBERATE! where the first Custom screen was the only one
 * (0x08007A50). BN5 asks the time up as its Custom screen is asked for
 * (0x08008BD2); BN6's opens on L or R alone, so here the gauge's filling
 * is the end, or a player could hold the final turn forever. The
 * Darkloid's battle counts no turns, and from its eleventh every hit does
 * double, as BN5's liberation battles do (0x0800C7CC): a hook on BN6's
 * object_subtractHP while it is armed.
 *
 * The hooks only read what the director armed between frames, and the
 * game's own state; the draw's second banner is theirs alone to count. */
#include "liberation_battle.h"

#include <stdint.h>
#include <stdio.h>

#include "bn6.h"
#include "bn6_battle_loop.h"
#include "bn6_forms.h"
#include "director.h"
#include "emu.h"
#include "hook.h"
#include "liberation.h"
#include "liberation_battle_words.h"
#include "liberation_state.h"
#include "pet_look.h"
#include "platform.h"

/* What the director armed: the battle's turns (0: none to count), and
 * whether it is a liberation's at all */
static struct {
	volatile bool on;
	volatile int turns;
} arm;
static int draw_pass;          /* (the hooks') the draw's banner shown: 0 TIME UP!, 1 LIBERATE FAILED! */
static int turn_seen;          /* (the director's) the turn the battle is in */
static bool doubling;          /* the late double's hook in */

static const int dirs[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

/* ---- the hooks ---- */

/* Each turn's fight starts: BN5's banner for it */
static HookAct fight_banner(HookRegs *r, void *user) {
	(void)user;
	if (!arm.on || !arm.turns) return HOOK_CONTINUE;
	int turn = hook_read8(BN6_BATTLE_SCREENS);
	bool final = turn >= arm.turns;
	r->r[0] = final ? BN6_MSG_FINAL_TURN : BN6_MSG_TURN_START;
	r->r[1] = final ? 0 : (uint32_t)turn;
	r->r[12] = BN6_FIGHT_BANNER_SHOW | 1;
	return HOOK_JUMP;
}

/* The fight's frame, nothing decided yet: the final turn's gauge full, its
 * time is up, and the loop is set to its draw */
static HookAct fight_ask(HookRegs *r, void *user) {
	(void)user;
	if (!arm.on || !arm.turns || r->r[0] != 0xFF) return HOOK_CONTINUE;
	if (hook_read8(BN6_BATTLE_SCREENS) < arm.turns || hook_read16(BN6_CUSTOM_GAUGE) < BN6_GAUGE_FULL) return HOOK_CONTINUE;
	hook_write32(BN6_LOOP, BN6_LOOP_DRAW);
	draw_pass = 0;
	r->r[12] = BN6_FIGHT_RETURN | 1;
	return HOOK_JUMP;
}

/* The win's banner */
static HookAct win_banner(HookRegs *r, void *user) {
	(void)user;
	if (!arm.on || !arm.turns) return HOOK_CONTINUE;
	bool first = hook_read8(BN6_BATTLE_SCREENS) <= 1;
	r->r[6] = first ? BN6_MSG_TURN_LIBERATE : BN6_MSG_LIB_SUCCESS;
	r->r[1] = 1;
	return HOOK_CONTINUE;
}

/* The draw's banner: TIME UP!, then LIBERATE FAILED! */
static HookAct draw_banner(HookRegs *r, void *user) {
	(void)user;
	if (!arm.on || !arm.turns) return HOOK_CONTINUE;
	r->r[0] = draw_pass ? BN6_MSG_LIB_FAILED : BN6_MSG_TIME_UP;
	r->r[1] = 0;
	r->r[12] = BN6_DRAW_BANNER_SHOW | 1;
	return HOOK_JUMP;
}

/* ... the first gone, the draw begun again for the second */
static HookAct draw_shown(HookRegs *r, void *user) {
	(void)user;
	if (!arm.on || !arm.turns || draw_pass) return HOOK_CONTINUE;
	draw_pass = 1;
	hook_write8(BN6_LOOP_STEP, 0);
	r->r[12] = BN6_DRAW_RETURN | 1;
	return HOOK_JUMP;
}

/* object_subtractHP: from the Darkloid battle's eleventh turn, double */
static HookAct late_double(HookRegs *r, void *user) {
	(void)user;
	if (arm.on && !arm.turns && r->r[0] && hook_read8(BN6_BATTLE_SCREENS) >= LIB_DOUBLE_TURN) r->r[0] *= 2;
	return HOOK_CONTINUE;
}

void lib_battle_install(void) {
	static bool done;
	if (done) return;
	done = true;
	bool ok = emu_hook(BN6_FIGHT_BANNER, fight_banner, NULL) && emu_hook(BN6_FIGHT_ASK, fight_ask, NULL) &&
		emu_hook(BN6_WIN_BANNER, win_banner, NULL) && emu_hook(BN6_DRAW_BANNER, draw_banner, NULL) &&
		emu_hook(BN6_DRAW_SHOWN, draw_shown, NULL);
	if (!ok) fprintf(stderr, "liberation: the battle loop's hooks did not fit\n");
}

/* ---- the director's ---- */

void lib_battle_arm(int turns) {
	arm.turns = turns;
	arm.on = true;
	turn_seen = 0;
	lib_cue_battle(turns > 0);   /* (won in the first turn: BN5's jingle, liberation_cues.c) */
	/* (the Darkloid's: the late double's hook while it runs; the duel's
	 * watch holds the same instruction, never on a dark layer) */
	if (!turns && !doubling) doubling = emu_hook(BN6_SUBTRACT_HP, late_double, NULL);
}

void lib_battle_disarm(void) {
	arm.on = false;
	arm.turns = 0;
	lib_cue_battle(false);
	if (doubling) emu_unhook(BN6_SUBTRACT_HP);
	doubling = false;
}

void lib_battle_watch(void) {
	if (!arm.on) return;
	int phase = emu_read8(BN6_BATTLE_PHASE);
	if (emu_read8(BN6_BATTLE_MODE) == BN6_BATTLE_ON && (phase == BN6_PHASE_CUSTOM || phase == BN6_PHASE_FIGHT))
		turn_seen = emu_read8(BN6_BATTLE_SCREENS);
}

int lib_battle_end(int *turn) {
	int result = emu_read8(BN6_BATTLE_RESULT), out = result == 1 ? LIB_OUT_WON : result == 2 ? LIB_OUT_LOST : LIB_OUT_TIME_UP;
	if (turn) *turn = turn_seen;
	lib_battle_disarm();
	return out;
}

/* ---- the stakes ---- */

bool lib_swept(int x, int y) {
	if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || lib_unit_at(x, y) >= 0) return false;
	int k = LB.b.kind[y][x];
	return k == LIB_DARK || k == LIB_ITEM || (k == LIB_RING && !lib_holes_left(&LB.b));
}

static int swept_around(int x, int y) {
	int n = 0;
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx) n += (dx || dy) && lib_swept(x + dx, y + dy);
	return n;
}

bool lib_battle_stakes(int x, int y, int cmd, LibStakes *s) {
	*s = (LibStakes){ 0 };
	if (x < 0 || y < 0 || cmd == LIB_CMD_PASS) return false;
	int u = lib_unit_at(x, y);
	if (u == 0) return !lib_holes_left(&LB.b);   /* (the Darkloid's: no turns to count) */
	s->turns = LIB_TURNS;
	if (u > 0) {
		s->unit = true;
		s->freed = 1;
		s->one_turn = swept_around(x, y);
		return true;
	}
	if (!lib_liberable(&LB.b, x, y)) return false;
	if (LB.b.kind[y][x] == LIB_HOLE) {
		s->hole = true;
		s->freed = 1 + swept_around(x, y);
		return true;
	}
	s->freed = 1;
	if (cmd == LIB_CMD_LONGSWRD) {
		int x2 = x + dirs[LB.face][0], y2 = y + dirs[LB.face][1];
		s->freed += lib_liberable(&LB.b, x2, y2) && lib_unit_at(x2, y2) < 0 && LB.b.kind[y2][x2] != LIB_HOLE;
	}
	s->one_turn = swept_around(x, y);
	return true;
}

void lib_battle_draw_stakes(int right, int y) {
	if (LB.step != LIB_STEP_MENU || !LB.can[LB.menu]) return;
	LibStakes s;
	int fx, fy;
	lib_faced(&fx, &fy);
	if (!lib_battle_stakes(fx, fy, LB.menu, &s)) return;
	char a[32], b[32];
	lib_stakes_words(&s, a, sizeof a, b, sizeof b);
	int w = look_caps_width(a), wb = look_caps_width(b);
	if (wb > w) w = wb;
	w += 8;
	int h = b[0] ? 19 : 11;
	SDL_Rect f = look_box(P.core_x + right - w, P.core_y + y, w, h, LOOK_NAVY);
	look_caps(f.x + 4, f.y + 3, a, LOOK_TEXT, LOOK_SHADE);
	if (b[0]) look_caps(f.x + 4, f.y + 11, b, LOOK_GOLD, LOOK_SHADE);
}
