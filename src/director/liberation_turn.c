/* A unit's turn in the Darkloids' phase as BN5 plays it (liberation.h;
 * docs/LIBERATION.md, The Darkloids' phase as BN5 shows it), traced in
 * romlab on BN5's mission 1: the camera's pan to the unit and its rest;
 * a guardian that moves warps there, all its steps at once (a puff and a
 * beam's lines where it stands, the camera's pan to the panel it takes,
 * the lines and the puff there); a unit with a Navi in its reach lights
 * it red, blinking twice, speaks its line, strikes in its own animation
 * (BigBrute's breath of fire over its Navi), and its harm shows over him
 * in BN5's yellow digits as his HP drops. Each goes for the Navi of the
 * team nearest it (BN5's text: closer enemies first); BigBrute hits every
 * Navi in its reach, the others the one they went for. The frames are BN5's, counted
 * from the moment each step begins (bn5_liberation.h). */
#include "liberation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5_liberation.h"
#include "bn6_fields.h"
#include "debug.h"
#include "director_state.h"
#include "guardians.h"
#include "liberation_state.h"
#include "liberation_team.h"
#include "liberation_words.h"
#include "netmap.h"
#include "talk.h"
#include "text.h"

/* The steps of a turn */
enum { T_PAN, T_REST, T_OUT, T_TRAVEL, T_IN, T_SETTLE, T_REACH, T_LINE, T_STRIKE };

static int line_at;      /* the frame the unit's line began */

#define WARP_OUT_FRAMES  5    /* BN5's puff 2 frames, its lines 3 */
#define WARP_LAND        4    /* frames between the camera's arrival and the warp in */
#define WARP_IN_FRAMES   5    /* the lines 2, the puff 2, the guardian with it 1 */
#define SETTLE_FRAMES    22   /* frames from the warp's end to the reach lit */
#define FLAME_BURN       6    /* the flame's rise (its animation 0, 3 frames of 2), */
#define FLAME_FALL       24   /* ... its burning till here, */
#define FLAME_END        27   /* ... its fall till here */

/* A strike's frames from its line's chat box closing: the attacker in its
 * strike, the blow (its sound, the effect over MegaMan), its harm's digits
 * and his HP dropped, the reach's red put out and the turn over. BN5's: a
 * BigBrute's (romlab, mission 1, MegaMan beside it) and the Darkloid's
 * (BlizzardMan's, his Snowball falling on MegaMan) */
typedef struct { int pose, pose_end, blow, harm, end; } Strike;
static const Strike guardian_strike = { 12, 27, 30, 60, 123 };
static const Strike darkloid_strike = { 19, 52, 52, 73, 127 };

/* The Navi unit u goes for: the nearest on the board */
static int target(int u) { return lib_mate_nearest(LB.b.unit[u].x, LB.b.unit[u].y); }
static int dist_to(int u, int k) { return abs(LB.b.unit[u].x - LB.b.mate[k].x) + abs(LB.b.unit[u].y - LB.b.mate[k].y); }
static int dist(int u) { int k = target(u); return k < 0 ? 1 << 20 : dist_to(u, k); }

static int reach_of(int u) { return u == 0 ? lib_darkloid_reach(LB.b.act) : lib_guard_rule(LB.b.unit[u].kind)->reach; }

/* The eighth from world (wx, wy) towards the Navi unit u goes for on the
 * screen: up-right, down-right, down-left or up-left (a world step is
 * x + y across and (y - x) / 2 down) */
static int face_target(int u, int wx, int wy) {
	int k = target(u), tx = bn6_player_x(), ty = bn6_player_y();
	if (k >= 0) lib_mate_world(k, &tx, &ty);
	int dx = tx + ty - wx - wy, dy = ty - tx - wy + wx;
	return dy >= 0 ? (dx >= 0 ? 3 : 5) : (dx >= 0 ? 1 : 7);
}

/* Unit u's animation: standing or striking, facing `face`; its own where
 * the board's sprite is BN5's guardian's or a Darkloid's BN6 Navi in list
 * 6 (his pose his strike), else as it was installed */
static void pose(int u, int face, bool strike) {
	int a = -1;
	if (u > 0) a = lib_look_unit_anim(LB.b.unit[u].kind, face, strike);
	else {
		NpcBody body = guardian_body(LB.darkloid, face);
		if (body.list == 6) a = strike && body.pose >= 0 ? body.pose : body.anim;
	}
	if (a >= 0) LB.anim[u] = (uint8_t)a;
}

static void fx(int sprite, int anim, int wx, int wy) {
	LB.effect.sprite = sprite;
	LB.effect.anim = anim;
	LB.effect.x = wx;
	LB.effect.y = wy;
	++LB.effect.n;
}

/* The panels in unit u's reach (every one of the board's, MegaMan's too) */
static void reach_mark(int u) {
	memset(LB.reach, 0, sizeof LB.reach);
	int r = reach_of(u), ux = LB.b.unit[u].x, uy = LB.b.unit[u].y;
	for (int y = uy - r; y <= uy + r; ++y)
		for (int x = ux - r; x <= ux + r; ++x)
			if (x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && abs(x - ux) + abs(y - uy) <= r && LB.b.kind[y][x] != LIB_VOID) LB.reach[y][x] = 1;
}

static void step(int s) {
	if (emu_debug_on() && s > T_REST) fprintf(stderr, "liberation turn: unit %d step %d at frame %d\n", LB.enemy_unit, s, D.frame);
	LB.enemy_steps = s;
	LB.enemy_t = 0;
}

/* After its rest: a guardian's move (its steps toward the Navi it goes
 * for, BN5's warp shown after), then its strike where he is in its reach */
static void decide(int u) {
	if (u > 0) {
		const LibGuardRule *r = lib_guard_rule(LB.b.unit[u].kind);
		int steps = 0, k = target(u);
		while (k >= 0 && dist_to(u, k) > r->reach && steps < r->move && lib_guardian_step(&LB.b, u, LB.b.mate[k].x, LB.b.mate[k].y)) ++steps;
		if (steps) {
			/* (its object parked now shows gone in the next frame, the
			 * effect with it) */
			LB.hidden[u] = true;
			step(T_OUT);
			return;
		}
	}
	step(dist(u) <= reach_of(u) ? T_REACH : -1);
	if (LB.enemy_steps == T_REACH) reach_mark(u);
}

/* The reach lit as BN5 lights it: 9 frames, 7 dark, 9, 7 dark, then lit
 * till the strike is over; its sound as each light comes */
static void reach_blink(int t) {
	int period = BN5_LIB_REACH_ON + BN5_LIB_REACH_OFF;
	LB.reach_lit = t >= 2 * period || t % period < BN5_LIB_REACH_ON;
	if (t % period == 0 && t <= 2 * period) lib_sound(BN5_LIB_REACH_SOUND);
}

/* The Navis a unit's blow lands on: every one in its reach where it hits
 * them all (BigBrute), else the one it went for; bit k Navi k. Those it
 * harms: none under a BonusPanel's shield, nor KnightMan and those beside
 * him (his StoneBody) */
static unsigned victims(int u) {
	int r = reach_of(u), k0 = target(u);
	bool all = u > 0 && lib_guard_rule(LB.b.unit[u].kind)->all;
	unsigned hit = 0;
	for (int k = 0; k < LB.b.team.n; ++k)
		if (!LB.b.mate[k].out && dist_to(u, k) <= r && (all || k == k0)) hit |= 1u << k;
	return hit;
}

static unsigned harmed(unsigned hit) {
	unsigned h = 0;
	for (int k = 0; k < LB.b.team.n; ++k)
		if ((hit >> k & 1) && !LB.shield && LB.b.team.navi[k] != MATE_KNIGHTMAN && !lib_mate_shielded(&LB.b, k)) h |= 1u << k;
	return h;
}

/* The strike after its line, frame t from the chat box's closing */
static bool strike(int u, int t) {
	const Strike *k = u == 0 ? &darkloid_strike : &guardian_strike;
	int face = face_target(u, LB.ux[u], LB.uy[u]), wx = bn6_player_x(), wy = bn6_player_y(), at = target(u);
	unsigned hit = victims(u), hurt = harmed(hit);
	if (at >= 0) lib_mate_world(at, &wx, &wy);
	if (t == k->pose) pose(u, face, true);
	if (t == k->pose_end) pose(u, face, false);
	if (t == k->blow) {
		lib_sound(hurt ? BN5_LIB_HIT_SOUND : BN5_LIB_NO_HIT_SOUND);
		/* (BigBrute's BeastBreath over the Navi it went for; the others'
		 * strikes are their own animations alone, their effects not traced) */
		if (u > 0 && LB.b.unit[u].kind == UNIT_BRUTE) fx(BN5_LIB_FLAME_SPRITE, 0, wx, wy);
	}
	if (LB.effect.sprite == BN5_LIB_FLAME_SPRITE) {
		if (t == k->blow + FLAME_BURN) fx(BN5_LIB_FLAME_SPRITE, 1, wx, wy);
		if (t == k->blow + FLAME_FALL) fx(BN5_LIB_FLAME_SPRITE, 2, wx, wy);
		if (t == k->blow + FLAME_END) LB.effect.sprite = -1;
	}
	if (t == k->harm && hurt) {
		int harm = lib_harm(u == 0 ? UNIT_DARKLOID : LB.b.unit[u].kind, LB.b.act);
		for (int m = 0; m < LB.b.team.n; ++m)
			if (hurt >> m & 1) lib_mate_hp_set(m, lib_mate_hp(m) - harm);
		LB.harm = harm;
		LB.harm_mates = (uint8_t)hurt;
		LB.harm_t = 0;
	}
	if (t < k->end) return false;
	LB.harm = 0;
	LB.harm_mates = 0;
	LB.reach_lit = false;
	return true;
}

/* A guardian's warp, a frame of it: out where it stood, the camera's pan
 * to the panel it takes, in there */
static void warp_frame(int u, int t) {
	int wx, wy;
	switch (LB.enemy_steps) {
	case T_OUT:
		if (t == 0) {
			fx(BN5_LIB_WARP_SPRITE, BN5_LIB_WARP_OUT, LB.ux[u], LB.uy[u]);
			lib_sound(BN5_LIB_WARP_SOUND);
		}
		if (t >= WARP_OUT_FRAMES) { LB.effect.sprite = -1; step(T_TRAVEL); }
		return;
	case T_TRAVEL:
		netmap_world(LB.b.unit[u].x, LB.b.unit[u].y, &wx, &wy);
		if (!lib_camera_at(wx, wy)) { LB.enemy_t = 0; return; }
		if (t < WARP_LAND) return;
		LB.ux[u] = wx;
		LB.uy[u] = wy;
		fx(BN5_LIB_WARP_SPRITE, BN5_LIB_WARP_IN, wx, wy);
		lib_sound(BN5_LIB_WARP_SOUND);
		step(T_IN);
		return;
	default:
		/* (its object put back a frame before BN5's shows it) */
		if (t == BN5_LIB_WARP_SHOWS - 1) LB.hidden[u] = false;
		if (t == BN5_LIB_WARP_SHOWS) lib_sound(BN5_LIB_APPEAR_SOUND);
		if (t >= WARP_IN_FRAMES) { LB.effect.sprite = -1; step(T_SETTLE); }
		return;
	}
}

/* Its reach lit, then its line once nothing else shows: the attack's
 * name, the Darkloid's with his mugshot */
static void reach_frame(int u, int t) {
	reach_blink(t);
	if (t < BN5_LIB_REACH_LINE) return;
	const char *line = lib_strike_words(u == 0 ? UNIT_DARKLOID : LB.b.unit[u].kind, guardian(LB.darkloid)->name);
	if (!lib_quiet() || !talk_start(line, u == 0 ? guardian_face(LB.darkloid) : FACE_NONE)) { LB.enemy_t = t; return; }
	line_at = D.frame;
	step(T_LINE);
}

static bool turn_frame(int u) {
	int t = LB.enemy_t++;
	switch (LB.enemy_steps) {
	case T_PAN:
		/* (BN5's camera to the unit, then its rest there) */
		if (lib_camera_at(LB.ux[u], LB.uy[u])) step(T_REST);
		return false;
	case T_REST:
		if (lib_look_on() && t < BN5_LIB_UNIT_HOLD) return false;
		decide(u);
		return LB.enemy_steps < 0;
	case T_OUT: case T_TRAVEL: case T_IN:
		warp_frame(u, t);
		return false;
	case T_SETTLE:
		if (t < SETTLE_FRAMES) return false;
		step(dist(u) <= reach_of(u) ? T_REACH : -1);
		if (LB.enemy_steps == T_REACH) reach_mark(u);
		return LB.enemy_steps < 0;
	case T_REACH:
		reach_frame(u, t);
		return false;
	case T_LINE:
		/* (the strike once its box has closed: the phase steps only while
		 * nothing shows, liberation.c) */
		if (D.frame - line_at > 2) step(T_STRIKE);
		return false;
	case T_STRIKE:
		return strike(u, t);
	default:
		return true;
	}
}

bool lib_turn(int u) {
	if (u < 0 || u >= LB.b.nunits || LB.b.unit[u].gone) return true;
	if (LB.harm) ++LB.harm_t;
	if (!turn_frame(u)) return false;
	/* (over: the unit stands as it rests, the next begins with its pan) */
	pose(u, LIB_FACE_REST, false);
	LB.effect.sprite = -1;
	LB.reach_lit = false;
	LB.harm = 0;
	step(T_PAN);
	return true;
}
