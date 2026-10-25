/* A unit's turn in the Darkloids' phase as BN5 plays it (liberation.h;
 * docs/LIBERATION.md, The Darkloids' phase as BN5 shows it), traced in
 * romlab on BN5's missions 1, 2 and 4: the camera's pan to the unit and
 * its rest; a guardian that moves warps there, all its steps at once (a
 * puff and a beam's lines where it stands, the camera's pan to the panel
 * it takes, the lines and the puff there); a unit with MegaMan in its
 * reach lights it red, blinking twice, speaks its line, strikes in its own
 * animation (BigBrute's breath of fire over MegaMan, Bladia's DarkSlash
 * and the Darkloid's blow BN5's burst, TinHawk's HawkAttack a warp to him
 * and back), and its harm shows over him in BN5's yellow digits as his HP
 * drops. After every unit's turn, a DarkHole's deleted guardian rises on
 * it and warps beside it as BN5 brings him back. The frames are BN5's,
 * counted from the moment each step begins (bn5_liberation.h). */
#include "liberation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5_liberation.h"
#include "bn6_fields.h"
#include "cinema.h"
#include "debug.h"
#include "director_state.h"
#include "guardians.h"
#include "liberation_state.h"
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
 * strike, the blow (its sound, its effect over MegaMan: BN5's list 7),
 * its harm's digits and his HP dropped, the reach's red put out and the
 * turn over, and its cry (-1 none). BN5's: a BigBrute's (romlab, mission
 * 1, MegaMan beside it: its BeastBreath), a Bladia's (mission 4, Colonel
 * beside it: its DarkSlash, BN5's blow, BN5_LIB_BLOW_EVENT) and the
 * Darkloid's (BlizzardMan's pose; BN5's blow, as its Darkloids 0x15 and
 * 0x16 strike, where BN6's Navi has no Snowball) */
typedef struct { int pose, pose_end, blow, harm, end, fx, cry; } Strike;
static const Strike brute_strike = { 12, 27, 30, 60, 123, BN5_LIB_FLAME_SPRITE, -1 };
static const Strike bladia_strike = { 22, 69, 34, 35, 123, BN5_LIB_BLOW_SPRITE, 33 };
static const Strike darkloid_strike = { 19, 52, 52, 53, 127, BN5_LIB_BLOW_SPRITE, -1 };
#define BLOW_FRAMES 13   /* BN5's burst over its Navi */

/* TinHawk's HawkAttack (BN5_LIB_HAWK_ATTACK; romlab, mission 2, MegaMan
 * two from it), from its line's box closing: its warp out where it stands,
 * the camera to MegaMan, its warp in beside him already in its strike (by
 * BN5_LIB_HAWK_PLACES), shown four frames on, the blow four after that and
 * its cry five after; it warps back with the harm's digits, the camera to
 * its panel, its warp in there, and the red goes out */
enum { HAWK_OUT = 22, HAWK_IN = 97, HAWK_BLOW = 105, HAWK_CRY = 110, HAWK_LEAVE = 140, HAWK_HOME = 216, HAWK_END = 284 };

static int dist(int u) { return abs(LB.b.unit[u].x - LB.mx) + abs(LB.b.unit[u].y - LB.my); }

static int reach_of(int u) { return u == 0 ? lib_darkloid_reach(LB.b.act) : lib_guard_rule(LB.b.unit[u].kind)->reach; }

/* The eighth from world (wx, wy) towards MegaMan on the screen: up-right,
 * down-right, down-left or up-left (a world step is x + y across and
 * (y - x) / 2 down) */
static int face_megaman(int wx, int wy) {
	int dx = bn6_player_x() + bn6_player_y() - wx - wy, dy = bn6_player_y() - bn6_player_x() - wy + wx;
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

static void fx_up(int sprite, int anim, int wx, int wy, int up) {
	LB.effect.sprite = sprite;
	LB.effect.anim = anim;
	LB.effect.x = wx;
	LB.effect.y = wy;
	LB.effect.up = up;
	++LB.effect.n;
}

static void fx(int sprite, int anim, int wx, int wy) { fx_up(sprite, anim, wx, wy, 0); }

/* A blow on MegaMan: its sound, BN5's burst over him (16 up, the event's
 * place) with the board shaken, or BigBrute's flame */
static void blow(int sprite) {
	int wx = bn6_player_x(), wy = bn6_player_y();
	lib_sound(LB.shield ? BN5_LIB_NO_HIT_SOUND : BN5_LIB_HIT_SOUND);
	if (sprite == BN5_LIB_FLAME_SPRITE) fx(sprite, 0, wx, wy);
	if (sprite != BN5_LIB_BLOW_SPRITE) return;
	fx_up(sprite, 0, wx, wy, BN5_LIB_BLOW_UP);
	cinema_shake(BN5_LIB_BLOW_SHAKE, BN5_LIB_BLOW_JOLT);
}

/* ... its harm on him, in BN5's digits */
static void harm_on(int u) {
	if (LB.shield) return;
	int harm = lib_harm(u == 0 ? UNIT_DARKLOID : LB.b.unit[u].kind, LB.b.act);
	lib_hp_set(lib_hp() - harm);
	LB.harm = harm;
	LB.harm_t = 0;
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

/* After its rest: a guardian's move (its steps toward MegaMan, BN5's
 * warp shown after), then its strike where he is in its reach */
static void decide(int u) {
	if (u > 0) {
		const LibGuardRule *r = lib_guard_rule(LB.b.unit[u].kind);
		int steps = 0;
		while (dist(u) > r->reach && steps < r->move && lib_guardian_step(&LB.b, u, LB.mx, LB.my)) ++steps;
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

/* A frame of the camera's pan to world (wx, wy) before frame `due`: the
 * turn held at it till the camera is there */
static bool pan_by(int wx, int wy, int t, int due) {
	if (lib_camera_at(wx, wy) || t < due - 1) return true;
	LB.enemy_t = t;
	return false;
}

/* ... its warps: out where it stands and from beside MegaMan, in beside
 * him and at its panel, each shown four frames into its warp in */
static void hawk_warps(int u, int t) {
	if (t == HAWK_OUT - 1 || t == HAWK_LEAVE - 1) LB.hidden[u] = true;
	if (t == HAWK_OUT || t == HAWK_LEAVE) { fx(BN5_LIB_WARP_SPRITE, BN5_LIB_WARP_OUT, LB.ux[u], LB.uy[u]); lib_sound(BN5_LIB_WARP_SOUND); }
	if (t == HAWK_IN || t == HAWK_HOME) { fx(BN5_LIB_WARP_SPRITE, BN5_LIB_WARP_IN, LB.ux[u], LB.uy[u]); lib_sound(BN5_LIB_WARP_SOUND); }
	if (t == HAWK_IN + BN5_LIB_WARP_SHOWS - 1 || t == HAWK_HOME + BN5_LIB_WARP_SHOWS - 1) LB.hidden[u] = false;
	if (t == HAWK_IN + BN5_LIB_WARP_SHOWS || t == HAWK_HOME + BN5_LIB_WARP_SHOWS) lib_sound(BN5_LIB_APPEAR_SOUND);
	if (t == HAWK_OUT + WARP_OUT_FRAMES || t == HAWK_IN + WARP_IN_FRAMES || t == HAWK_HOME + WARP_IN_FRAMES) LB.effect.sprite = -1;
}

/* TinHawk's HawkAttack, frame t from its line's box closing */
static bool hawk_strike(int u, int t) {
	int wx = bn6_player_x(), wy = bn6_player_y(), hx, hy;
	netmap_world(LB.b.unit[u].x, LB.b.unit[u].y, &hx, &hy);
	if (t > HAWK_OUT + WARP_OUT_FRAMES && t < HAWK_IN && !pan_by(wx, wy, t, HAWK_IN)) return false;
	if (t == HAWK_IN) {
		/* (beside him, up-left or down-right of him on the screen: BN5's
		 * RNG's pick, ours the phase's) */
		bool right = (LB.phase + u) & 1;
		LB.ux[u] = wx + (right ? BN5_LIB_HAWK_NEAR : 0);
		LB.uy[u] = wy - (right ? 0 : BN5_LIB_HAWK_NEAR);
		pose(u, face_megaman(LB.ux[u], LB.uy[u]), true);
	}
	hawk_warps(u, t);
	if (t == HAWK_BLOW) blow(BN5_LIB_BLOW_SPRITE);
	if (t == HAWK_BLOW + BLOW_FRAMES) LB.effect.sprite = -1;
	if (t == HAWK_BLOW + 1) harm_on(u);
	if (t == HAWK_CRY) lib_sound(BN5_LIB_SLASH_SOUND);
	if (t == HAWK_LEAVE) LB.harm = 0;
	if (t == HAWK_LEAVE + WARP_OUT_FRAMES) {
		LB.effect.sprite = -1;
		LB.ux[u] = hx;
		LB.uy[u] = hy;
		pose(u, LIB_FACE_REST, false);
	}
	if (t > HAWK_LEAVE + WARP_OUT_FRAMES && t < HAWK_HOME && !pan_by(hx, hy, t, HAWK_HOME)) return false;
	if (t < HAWK_END) return false;
	LB.reach_lit = false;
	return true;
}

/* The strike after its line, frame t from the chat box's closing */
static bool strike(int u, int t) {
	int kind = u == 0 ? UNIT_DARKLOID : LB.b.unit[u].kind;
	if (kind == UNIT_HAWK) return hawk_strike(u, t);
	const Strike *k = kind == UNIT_DARKLOID ? &darkloid_strike : kind == UNIT_BLADIA ? &bladia_strike : &brute_strike;
	int face = face_megaman(LB.ux[u], LB.uy[u]), wx = bn6_player_x(), wy = bn6_player_y();
	if (t == k->pose) pose(u, face, true);
	if (t == k->pose_end) pose(u, face, false);
	if (t == k->blow) blow(k->fx);
	if (t == k->cry) lib_sound(BN5_LIB_SLASH_SOUND);
	if (LB.effect.sprite == BN5_LIB_FLAME_SPRITE) {
		if (t == k->blow + FLAME_BURN) fx(BN5_LIB_FLAME_SPRITE, 1, wx, wy);
		if (t == k->blow + FLAME_FALL) fx(BN5_LIB_FLAME_SPRITE, 2, wx, wy);
		if (t == k->blow + FLAME_END) LB.effect.sprite = -1;
	}
	if (LB.effect.sprite == BN5_LIB_BLOW_SPRITE && t == k->blow + BLOW_FRAMES) LB.effect.sprite = -1;
	if (t == k->harm) harm_on(u);
	if (t < k->end) return false;
	LB.harm = 0;
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

/* ---- a deleted guardian's return ---- */

/* As BN5 brings him back (BN5_LIB_RETURN; romlab, mission 2, a BigBrute
 * deleted by a poke beside its hole): for each DarkHole in its order whose
 * guardian comes back (lib_guardian_return), the camera to the hole; 20
 * frames on, he rises on it in BN5's OBJ mosaic, fading in as it sharpens
 * over 16 frames, with sound 0x94; 40 frames after the rise began he warps
 * out of the hole, 22 later in on his panel, shown four frames on; 20
 * frames, the next hole */
enum { R_FIND, R_PAN, R_SHOW };
#define RISE_AT     BN5_LIB_RETURN_WAIT
#define RETURN_OUT  (RISE_AT + 40)
#define RETURN_IN   (RETURN_OUT + 22)
#define RETURN_END  (RETURN_IN + WARP_IN_FRAMES + BN5_LIB_RETURN_WAIT)

static void return_find(void) {
	for (; LB.ret_hole < LB.b.nholes; ++LB.ret_hole) {
		int x, y, u = lib_guardian_return(&LB.b, LB.ret_hole, LB.mx, LB.my, &x, &y);
		if (u < 0) continue;
		/* (his panel the board's from now; unseen on the hole till he rises) */
		LB.b.unit[u].x = (uint8_t)x;
		LB.b.unit[u].y = (uint8_t)y;
		LB.b.unit[u].gone = false;
		LB.hidden[u] = true;
		pose(u, LIB_FACE_REST, false);
		netmap_world(LB.b.hole_x[LB.ret_hole], LB.b.hole_y[LB.ret_hole], &LB.ux[u], &LB.uy[u]);
		LB.ret_unit = u;
		LB.ret_step = R_PAN;
		LB.ret_t = 0;
		if (emu_debug_on()) fprintf(stderr, "liberation return: unit %d at hole %d, to %d,%d at frame %d\n", u, LB.ret_hole, x, y, D.frame);
		return;
	}
}

static void return_frame(int u, int t) {
	if (t == RISE_AT - 1) LB.hidden[u] = false;
	if (t == RISE_AT) { lib_sound(BN5_LIB_APPEAR_SOUND); LB.rise_unit = u; }
	if (t >= RISE_AT && t < RISE_AT + BN5_LIB_RISE_FRAMES) LB.rise_mosaic = BN5_LIB_RISE_FRAMES - 1 - (t - RISE_AT);
	if (t == RISE_AT + BN5_LIB_RISE_FRAMES) { LB.rise_mosaic = 0; LB.rise_unit = -1; }
	if (t == RETURN_OUT - 1) LB.hidden[u] = true;
	if (t == RETURN_OUT) { fx(BN5_LIB_WARP_SPRITE, BN5_LIB_WARP_OUT, LB.ux[u], LB.uy[u]); lib_sound(BN5_LIB_WARP_SOUND); }
	if (t == RETURN_OUT + WARP_OUT_FRAMES) { LB.effect.sprite = -1; netmap_world(LB.b.unit[u].x, LB.b.unit[u].y, &LB.ux[u], &LB.uy[u]); }
	if (t == RETURN_IN) { fx(BN5_LIB_WARP_SPRITE, BN5_LIB_WARP_IN, LB.ux[u], LB.uy[u]); lib_sound(BN5_LIB_WARP_SOUND); }
	if (t == RETURN_IN + BN5_LIB_WARP_SHOWS - 1) LB.hidden[u] = false;
	if (t == RETURN_IN + BN5_LIB_WARP_SHOWS) lib_sound(BN5_LIB_APPEAR_SOUND);
	if (t == RETURN_IN + WARP_IN_FRAMES) LB.effect.sprite = -1;
}

bool lib_return(void) {
	if (LB.ret_step == R_FIND) return_find();
	if (LB.ret_step == R_FIND) { LB.ret_hole = 0; return true; }
	int u = LB.ret_unit, hx, hy;
	netmap_world(LB.b.hole_x[LB.ret_hole], LB.b.hole_y[LB.ret_hole], &hx, &hy);
	if (LB.ret_step == R_PAN) {
		if (lib_camera_at(hx, hy)) LB.ret_step = R_SHOW;
		return false;
	}
	int t = LB.ret_t++;
	return_frame(u, t);
	if (t < RETURN_END) return false;
	LB.ret_unit = -1;
	LB.ret_step = R_FIND;
	++LB.ret_hole;
	return false;
}
