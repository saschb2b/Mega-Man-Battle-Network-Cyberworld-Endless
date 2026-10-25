/* A liberation's battle on BN6's own battle loop (liberation_battle.h).
 *
 * BN5 counts a liberation's turns in BattleState +7, one more as each
 * Custom screen opens, and BN6 keeps that count (BN6_BATTLE_SCREENS) and
 * BN5's banners in its battle messages, though not BN5's rules: so four
 * hooks answer its loop as BN5's loop decides (docs/ROM_DATA.md, BN6's
 * battle loop and banners). As each turn's fight starts, BN5's banner for
 * it (0x08007954: TURN n START! below the third, FINAL TURN! from it). In
 * the fight, BN5 asks for the Custom screen as the gauge fills, no key
 * pressed (0x08008BD2): below the third turn it opens, by BN6's own way
 * to it from L or R; in the third, BN5's time up: BN6's own draw (the
 * battle stopped, its banner, the battle over with result 3: no reward,
 * HP as it stands), its banner TIME UP! and then, the draw run again,
 * LIBERATE FAILED! (BN5's 0x08007C24). At a win, LIBERATE SUCCESS!, or 1
 * TURN LIBERATE! where the first Custom screen was the only one
 * (0x08007A50). Every liberation battle runs so, the Darkloid's too: BN5's
 * battle reads nothing of its board but a unit's HP (bn5_liberation.h).
 *
 * The hooks only read what the director armed between frames, and the
 * game's own state; the draw's second banner is theirs alone to count. */
#include "liberation_battle.h"

#include <stdint.h>
#include <stdio.h>

#include "bn6.h"
#include "bn6_battle_loop.h"
#include "bn6_forms.h"
#include "bn6_panels.h"
#include "debug.h"
#include "director.h"
#include "emu.h"
#include "hook.h"
#include "liberation.h"
#include "liberation_battle_words.h"
#include "liberation_field.h"
#include "liberation_state.h"
#include "liberation_team.h"
#include "pet_look.h"
#include "platform.h"

/* What the director armed: the battle's turns (0: none to count), and
 * whether it is a liberation's at all */
static struct {
	volatile bool on;
	volatile int turns;
	volatile int soul;     /* a teammate's battle: his Soul (7-12), 0 MegaMan's own */
	volatile int cross;    /* ... or his Cross, BN6's form (1-5, a Cross Navi's) */
} arm;
static volatile bool down;     /* (the hooks') the teammate's deleting blow came */
static int draw_pass;          /* (the hooks') the draw's banner shown: 0 TIME UP!, 1 LIBERATE FAILED! */
static int turn_seen;          /* (the director's) the turn the battle is in */
/* (the director's) the foe's HP the board keeps, as BN5 gives its battle's
 * marked enemy the unit's (0 none), given him once he is in play; and his
 * HP as the battle last showed it, -1 unseen */
static int foe_hp, foe_left;
static bool foe_given;
static int hp_seen;            /* ... and MegaMan's HP in it, as last seen (BN6's return to the map works his NaviStats afresh) */
static bool soul_guarded;      /* the teammate's guard's hook in (soul_guard) */


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

/* The fight's frame, nothing decided yet: BN5's ask as the gauge fills
 * (both sides standing, the battle not held): the Custom screen below the
 * final turn; in it, the time is up, and the loop is set to its draw; a
 * teammate's deleting blow come, straight to LIBERATE FAILED! */
static HookAct fight_ask(HookRegs *r, void *user) {
	(void)user;
	if (arm.on && down && r->r[0] == 0xFF && hook_read32(BN6_LOOP) != BN6_LOOP_DRAW) {
		hook_write32(BN6_LOOP, BN6_LOOP_DRAW);
		draw_pass = 1;
		r->r[12] = BN6_FIGHT_RETURN | 1;
		return HOOK_JUMP;
	}
	if (!arm.on || !arm.turns || r->r[0] != 0xFF) return HOOK_CONTINUE;
	unsigned flags = hook_read16(BN6_BATTLE_STATE + BN6_STATE_FLAGS);
	if (!(flags & BN6_FLAG_GAUGE_FULL) || (flags & BN6_FLAG_HELD) || !hook_read8(BN6_BATTLE_STATE + BN6_STATE_SIDE0_LEFT) ||
		!hook_read8(BN6_BATTLE_STATE + BN6_STATE_SIDE1_LEFT) || hook_read8(BN6_BATTLE_STATE + BN6_STATE_ENDING))
		return HOOK_CONTINUE;
	if (hook_read8(BN6_BATTLE_SCREENS) < arm.turns) {
		r->r[12] = BN6_FIGHT_CUSTOM | 1;
		return HOOK_JUMP;
	}
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
	if (!arm.on || (!arm.turns && !down)) return HOOK_CONTINUE;
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

/* object_subtractHP: in a teammate's battle, the blow that would delete
 * MegaMan in his Soul or Cross leaves him 1 HP, and the battle ends at the
 * fight's next frame */
static HookAct soul_guard(HookRegs *r, void *user) {
	(void)user;
	if (!arm.on || !r->r[0]) return HOOK_CONTINUE;
	if ((arm.soul || arm.cross) && r->r[5] == hook_read32(BN6_BATTLE_PLAYERS)) {
		uint32_t hp = hook_read16(r->r[5] + BN6_T1_HP);
		if (r->r[0] >= hp) { r->r[0] = hp > 1 ? hp - 1 : 0; down = true; }
	}
	return HOOK_CONTINUE;
}

/* OK's hand built: on a teammate's battle's first Custom screen, his Soul
 * committed as BN5's OK commits UNITE's (soulunite.c's emblem_commit): the
 * record the proxy form's, the Soul pending for soullife.c with turns that
 * never count down (0xFF), its used bit */
static HookAct soul_ok(HookRegs *r, void *user) {
	(void)r;
	(void)user;
	/* (a Cross Navi's: his Cross committed as CROSSSELECT's commit writes
	 * it, BN6_CROSS_COMMIT: the form, 0 for its chaos, its used bit; from
	 * then BN6's own rules, his weakness's hit breaking it) */
	if (arm.on && arm.cross && hook_read8(BN6_BATTLE_SCREENS) <= 1) {
		hook_write8(BN6_CROSS_RECORD, (uint8_t)arm.cross);
		hook_write8(BN6_CROSS_RECORD + BN6_RECORD_CHAOS, 0);
		hook_write32(BN6_CROSSES_USED, hook_read32(BN6_CROSSES_USED) | 1u << (arm.cross - 1));
		return HOOK_CONTINUE;
	}
	if (!arm.on || !arm.soul || hook_read8(BN6_BATTLE_SCREENS) > 1 || hook_read8(BN6_SOULS + BN6_SOUL_ID)) return HOOK_CONTINUE;
	hook_write8(BN6_CROSS_RECORD, BN6_FORM_PROXY);
	hook_write8(BN6_CROSS_RECORD + BN6_RECORD_CHAOS, 0);
	hook_write8(BN6_CROSS_RECORD + BN6_RECORD_TURNS, 0xFF);
	hook_write8(BN6_SOULS + BN6_SOUL_NEXT, (uint8_t)arm.soul);
	hook_write8(BN6_SOULS + BN6_SOUL_NEXT_CHAOS, 0);
	hook_write8(BN6_SOULS + BN6_SOUL_NEXT_TURNS, 0xFF);
	hook_write32(BN6_SOULS + BN6_SOUL_USED, hook_read32(BN6_SOULS + BN6_SOUL_USED) | 1u << arm.soul);
	return HOOK_CONTINUE;
}

void lib_battle_install(void) {
	static bool done;
	if (done) return;
	done = true;
	bool ok = emu_hook(BN6_FIGHT_BANNER, fight_banner, NULL) && emu_hook(BN6_FIGHT_ASK, fight_ask, NULL) &&
		emu_hook(BN6_WIN_BANNER, win_banner, NULL) && emu_hook(BN6_DRAW_BANNER, draw_banner, NULL) &&
		emu_hook(BN6_DRAW_SHOWN, draw_shown, NULL) && emu_hook(BN6_CUSTOM_OK_HAND, soul_ok, NULL);
	if (!ok) fprintf(stderr, "liberation: the battle loop's hooks did not fit\n");
}

/* ---- the director's ---- */

void lib_battle_arm(int turns) {
	arm.turns = turns;
	arm.on = true;
	turn_seen = hp_seen = 0;
	lib_cue_battle(turns > 0);   /* (won in the first turn: BN5's jingle, liberation_cues.c) */
	down = false;
	/* (a teammate's: the hook on the HP taken while it runs; the duel's
	 * watch holds the same instruction, never on a dark layer) */
	if ((arm.soul || arm.cross) && !soul_guarded) soul_guarded = emu_hook(BN6_SUBTRACT_HP, soul_guard, NULL);
}

int lib_battle_field(Encounter *e, int x, int y, bool darkloid) {
	int level = lib_field_level(x, y, LB.face);
	lib_field_apply(e, lib_field_sides(level, darkloid));
	return level;
}

void lib_battle_soul(int soul) { arm.soul = soul >= 7 && soul <= 12 ? soul : 0; arm.cross = 0; }

void lib_battle_cross(int cross) { arm.cross = cross >= 1 && cross <= 5 ? cross : 0; arm.soul = 0; }

bool lib_battle_soul_armed(void) { return arm.on && (arm.soul || arm.cross); }

bool lib_battle_down(void) { return down; }

int lib_battle_hp(void) { return hp_seen; }

void lib_battle_disarm(void) {
	arm.on = false;
	arm.turns = 0;
	arm.soul = arm.cross = 0;
	if (soul_guarded) emu_unhook(BN6_SUBTRACT_HP);
	soul_guarded = false;
	lib_cue_battle(false);
}

void lib_battle_foe(int hp) {
	foe_hp = hp;
	foe_left = -1;
	foe_given = false;
}

int lib_battle_foe_left(void) { return foe_left; }

/* The foe's object: the first of the battle's in play on the enemies' side */
static uint32_t foe_object(void) {
	for (uint32_t k = 0; k < BN6_T1_COUNT; ++k) {
		uint32_t o = BN6_T1_OBJECTS + k * BN6_T1_SIZE;
		if ((emu_read8(o + BN6_T1_IN_PLAY) & 1) && emu_read8(o + BN6_T1_ALLIANCE) == 1) return o;
	}
	return 0;
}

/* The foe's HP from the board's as he is first seen in play (his max his
 * own, as BN5's 0x08006862 gives the board's max with it), then kept as
 * the battle takes it, for the board (BN5's 0x08009050) */
static void foe_watch(void) {
	uint32_t o = foe_hp > 0 ? foe_object() : 0;
	if (!o) return;
	int hp = emu_read16(o + BN6_T1_HP);
	if (!foe_given) {
		foe_given = true;
		if (foe_hp < hp) {
			uint16_t v = (uint16_t)foe_hp;
			emu_write(o + BN6_T1_HP, &v, 2);
			if (emu_debug_on()) fprintf(stderr, "liberation: the foe's battle from the board's %d HP (his own %d)\n", foe_hp, hp);
			hp = foe_hp;
		}
	}
	foe_left = hp;
}

void lib_battle_watch(void) {
	if (!arm.on) return;
	int phase = emu_read8(BN6_BATTLE_PHASE);
	if (emu_read8(BN6_BATTLE_MODE) == BN6_BATTLE_ON && (phase == BN6_PHASE_CUSTOM || phase == BN6_PHASE_FIGHT)) {
		turn_seen = emu_read8(BN6_BATTLE_SCREENS);
		uint32_t me = emu_read32(BN6_BATTLE_PLAYERS);
		if (me) hp_seen = emu_read16(me + BN6_T1_HP);
	}
	foe_watch();
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
	return k == LIB_DARK || k == LIB_ITEM || k == LIB_TRAP || (k == LIB_RING && !lib_holes_left(&LB.b));
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
	if (u == 0 && lib_holes_left(&LB.b)) return false;
	s->turns = LIB_TURNS;
	s->surrounded = lib_field_sides(lib_field_level(x, y, LB.face), u == 0) == BN6_SIDES_SURROUNDED;
	if (u == 0) {
		s->unit = s->darkloid = true;
		return true;
	}
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
	/* (the command's shape, LongSwrd's, ScrenDiv's, TomahawkSwing's and the
	 * Cross Navis' own; a LifeMelody sung for the Navi in play, five ahead;
	 * the commands with no battle stake none) */
	int command = cmd == LIB_CMD_LONGSWRD ? lib_mate_command(lib_in_play()) : LB.mate[lib_in_play()].melody ? LIB_DO_MELODY : LIB_DO_NONE;
	if (cmd == LIB_CMD_LONGSWRD && !lib_command_battles(command)) return false;
	int xs[6], ys[6], n = command != LIB_DO_NONE ? lib_command_panels(&LB.b, command, x, y, LB.face, xs, ys) : 0;
	/* (PanelSearch's: its line's item and trap panels alone) */
	if (command == LIB_DO_SEARCH) s->freed = 0;
	for (int i = 0; i < n; ++i)
		s->freed += (command == LIB_DO_SEARCH || xs[i] != x || ys[i] != y) && lib_liberable(&LB.b, xs[i], ys[i]) && lib_unit_at(xs[i], ys[i]) < 0 &&
		            LB.b.kind[ys[i]][xs[i]] != LIB_HOLE;
	s->one_turn = swept_around(x, y);
	return true;
}

void lib_battle_draw_stakes(int right, int y) {
	if (LB.step != LIB_STEP_MENU || !LB.can[LB.menu]) return;
	LibStakes s;
	int fx, fy;
	lib_faced(&fx, &fy);
	if (!lib_battle_stakes(fx, fy, LB.menu, &s)) return;
	char a[32], b[32], c[32];
	lib_stakes_words(&s, a, sizeof a, b, sizeof b);
	lib_surrounded_words(&s, c, sizeof c);
	int w = look_caps_width(a), wb = look_caps_width(b), wc = look_caps_width(c);
	if (wb > w) w = wb;
	if (wc > w) w = wc;
	w += 11;   /* (the frame's 3 a side, the text 4 in) */
	int lines = 1 + !!b[0] + !!c[0], h = lines == 1 ? 11 : lines == 2 ? 19 : 30;
	/* (a third line grows it upward, clear of the window under it) */
	SDL_Rect f = look_box(P.core_x + right - w, P.core_y + y - (h > 19 ? h - 19 : 0), w, h, LOOK_NAVY);
	look_caps(f.x + 4, f.y + 3, a, LOOK_TEXT, LOOK_SHADE);
	if (b[0]) look_caps(f.x + 4, f.y + 11, b, LOOK_GOLD, LOOK_SHADE);
	if (c[0]) look_caps(f.x + 4, f.y + (b[0] ? 19 : 11), c, LOOK_TEXT, LOOK_SHADE);
}
