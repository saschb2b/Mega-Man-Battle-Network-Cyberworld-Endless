/* A liberation's team (liberation_team.h).
 *
 * BN5's board, played in romlab on mission 2 (MegaMan, Colonel and
 * KnightMan): the liberation state's +6 names the Navi in play, a team
 * Navi's unit +2 is 1 till he has acted this phase; R moves play to the
 * next Navi yet to act, the camera panning to him; a Navi's action (Pass,
 * after "End without doing anything?") moves it on by itself; the Navi in
 * play walks the board freely, through his teammates; the others' HP shows
 * under them, his own in the box at the top left. Here the Navi in play is
 * BN6's player object in his own overworld sprite (player_body.h), BN5's
 * own copied in for each teammate (xnavi_mate), and the others the layer's
 * objects (OBJ_UNIT, after the units), parked off the floor while in play
 * or out. */
#include "liberation_team.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5_liberation.h"
#include "bn6.h"
#include "bn6_expression.h"
#include "bn6_fields.h"
#include "debug.h"
#include "devtools.h"
#include "director_state.h"
#include "emu.h"
#include "guardians.h"
#include "liberation.h"
#include "liberation_battle.h"
#include "liberation_state.h"
#include "netmap.h"
#include "player_body.h"
#include "rom.h"
#include "run.h"
#include "souls.h"
#include "soulunite.h"
#include "text.h"
#include "xnavi.h"

#define MATE_FACE   3      /* the eighth the team faces as the board begins (MegaMan's, down-right) */
#define SPAWN_WAIT  2      /* frames the player's spawn takes in its new body, then his interaction let go */

/* BN5's teams by the stage an act stands for (docs/LIBERATION.md, What a
 * mission is made of): two at mission 1, three at 2-3, four at 4-5, five
 * on; and the mission whose HP its Navis have */
static const int room_of[6] = { 2, 2, 3, 4, 5, 5 };
static const int mission_of[6] = { 1, 1, 3, 5, 7, 9 };
static int bodies[LIB_TEAM];   /* each Navi's list-6 number (MegaMan's own), as the team was made */
static int saved_hp, saved_max;   /* MegaMan's while a teammate fights */

static int clamp_act(int act) { return act < 0 ? 0 : act > 5 ? 5 : act; }

/* A teammate's HP in BN5's mission `mission` (BN5_LIB_NAVI_HP: u16 his id,
 * then one a mission), 0 where it is not there */
static int bn5_hp(int navi, int mission) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	if (!x->data) return 0;
	const uint8_t *t = x->data + (BN5_LIB_NAVI_HP - 0x08000000u);
	for (int i = 0; i < SOULS; ++i, t += 20)
		if ((t[0] | t[1] << 8) == navi) return t[2 * mission] | t[2 * mission + 1] << 8;
	return 0;
}

void lib_team_make(LibTeam *t, int act) {
	memset(t, 0, sizeof *t);
	t->n = 1;
	t->navi[0] = MATE_MEGAMAN;
	bodies[0] = BN6_OW_MEGAMAN;
	if (!XR[XROM_BN5_COLONEL_US].data || !soulunite_ready()) return;
	int a = clamp_act(act), cand[SOULS], n = 0;
	for (int k = 0; k < SOULS; ++k)
		if (souls_held() >> k & 1) cand[n++] = MATE_COLONEL + k;
	/* (more held than BN5's team has room for: the layer's seed leaves some
	 * out, a different team a board) */
	uint32_t r = run.layer_seed ^ 0x7EA3u;
	while (n > room_of[a] - 1) {
		r = r * 1103515245u + 12345u;
		int drop = (int)(r >> 16) % n;
		memmove(cand + drop, cand + drop + 1, sizeof *cand * (size_t)(n - drop - 1));
		--n;
	}
	for (int i = 0; i < n; ++i) {
		int body = xnavi_mate(t->n - 1, guardian_older_sprite(GUARDIAN_OLDER_FIRST + cand[i] - MATE_COLONEL));
		int hp = bn5_hp(cand[i], mission_of[a]);
		if (body < 0 || hp <= 0) continue;
		bodies[t->n] = body;
		t->navi[t->n] = (uint8_t)cand[i];
		t->hp[t->n] = (int16_t)hp;
		++t->n;
	}
	if (emu_debug_on()) fprintf(stderr, "liberation team: %d Navis for act %d\n", t->n, a + 1);
}

bool lib_team_shrink(LibTeam *t) {
	if (t->n <= 1) return false;
	--t->n;
	return true;
}

void lib_team_objects(void) {
	for (int k = 0; k < LB.b.team.n && layer.nobj < MAX_OBJS; ++k) {
		NetObj *o = &layer.obj[layer.nobj++];
		memset(o, 0, sizeof *o);
		o->type = OBJ_UNIT;
		o->x = LB.b.mate[k].x;
		o->y = LB.b.mate[k].y;
		o->param = bodies[k] | MATE_FACE << 8 | 6 << 16 | (LIB_UNITS + k) << 20;
		o->prop = -1;
		LB.anim[LIB_UNITS + k] = MATE_FACE;
	}
}

void lib_team_begin(void) {
	LB.in_play = 0;
	LB.switch_t = 0;
	memset(LB.weak, 0, sizeof LB.weak);
	for (int k = 0; k < LB.b.team.n; ++k) {
		LB.mate[k].max = LB.mate[k].hp = LB.b.team.hp[k];
		/* (--dev matehp=N: a teammate's deletion tested) */
		if (k && dev.mate_hp > 0 && dev.mate_hp < LB.mate[k].max) LB.mate[k].hp = dev.mate_hp;
		LB.mate[k].sprite = bodies[k];
		LB.mate[k].face = MATE_FACE;
		LB.mate[k].acted = LB.mate[k].melody = false;
		netmap_world(LB.b.mate[k].x, LB.b.mate[k].y, &LB.mate[k].wx, &LB.mate[k].wy);
	}
	player_body(-1);
}

int lib_in_play(void) { return LB.in_play; }

const char *lib_mate_name(int k) {
	int navi = k > 0 && k < LB.b.team.n ? LB.b.team.navi[k] : MATE_MEGAMAN;
	return navi == MATE_MEGAMAN ? "MegaMan" : guardian(GUARDIAN_OLDER_FIRST + navi - MATE_COLONEL)->name;
}

int lib_mate_face(int k) { return k > 0 && k < LB.b.team.n ? LB.mate[k].sprite : FACE_MEGAMAN; }

int lib_mate_command(int k) { return lib_command_of(k >= 0 && k < LB.b.team.n ? LB.b.team.navi[k] : MATE_MEGAMAN); }

int lib_mate_hp(int k) { return k > 0 ? LB.mate[k].hp : emu_read16(BN6_NAVI_HP); }

int lib_mate_max(int k) { return k > 0 ? LB.mate[k].max : emu_read16(BN6_NAVI_MAX_HP); }

void lib_mate_hp_set(int k, int hp) {
	int max = lib_mate_max(k);
	hp = hp < 1 ? 1 : hp > max ? max : hp;
	if (k > 0) { LB.mate[k].hp = hp; return; }
	uint16_t v = (uint16_t)hp;
	emu_write(BN6_NAVI_HP, &v, 2);
}

void lib_mate_world(int k, int *wx, int *wy) {
	if (k == LB.in_play) { *wx = bn6_player_x(); *wy = bn6_player_y(); return; }
	*wx = LB.mate[k].wx;
	*wy = LB.mate[k].wy;
}

int lib_mate_nearest(int x, int y) {
	int best = -1, bd = 1 << 30;
	for (int k = 0; k < LB.b.team.n; ++k) {
		int d = abs(LB.b.mate[k].x - x) + abs(LB.b.mate[k].y - y);
		if (!LB.b.mate[k].out && d < bd) { bd = d; best = k; }
	}
	return best;
}

void lib_team_phase(void) {
	for (int k = 0; k < LB.b.team.n; ++k) LB.mate[k].acted = LB.mate[k].melody = false;
}

/* The next Navi yet to act after the one in play, in the team's order */
static int next_after(void) {
	for (int i = 1; i < LB.b.team.n; ++i) {
		int k = (LB.in_play + i) % LB.b.team.n;
		if (!LB.b.mate[k].out && !LB.mate[k].acted) return k;
	}
	return -1;
}

int lib_team_next(void) {
	LB.mate[LB.in_play].acted = true;
	return next_after();
}

int lib_team_other(void) { return next_after(); }

/* Play to Navi k at once: the one in play left standing where he is, k's
 * body the player's where k stands */
static void swap_to(int k) {
	int old = LB.in_play;
	LB.mate[old].wx = bn6_player_x();
	LB.mate[old].wy = bn6_player_y();
	LB.mate[old].face = emu_read8(BN6_PLAYER_FACING) & 7;
	LB.anim[LIB_UNITS + old] = (uint8_t)LB.mate[old].face;
	LB.in_play = k;
	emu_write32(BN6_PLAYER_X, (uint32_t)LB.mate[k].wx << 16);
	emu_write32(BN6_PLAYER_Y, (uint32_t)LB.mate[k].wy << 16);
	emu_write32(BN6_PLAYER_NEXT_X, (uint32_t)LB.mate[k].wx << 16);
	emu_write32(BN6_PLAYER_NEXT_Y, (uint32_t)LB.mate[k].wy << 16);
	emu_write8(BN6_PLAYER_FACING, (uint8_t)LB.mate[k].face);
	emu_write8(BN6_PLAYER_ANIM, (uint8_t)LB.mate[k].face);
	player_body(k ? LB.mate[k].sprite : -1);
	LB.mx = LB.b.mate[k].x;
	LB.my = LB.b.mate[k].y;
	if (emu_debug_on()) fprintf(stderr, "liberation team: %s in play at %d,%d\n", lib_mate_name(k), LB.mx, LB.my);
}

bool lib_team_switch_frame(int k) {
	if (k < 0 || k >= LB.b.team.n) return true;
	if (!LB.switch_t) {
		if (k == LB.in_play) return lib_camera_home();
		if (!lib_camera_at(LB.mate[k].wx, LB.mate[k].wy)) return false;
		swap_to(k);
		LB.switch_t = 1;
		return false;
	}
	if (LB.switch_t <= SPAWN_WAIT) {
		/* (its spawn locks his interaction, as a map's arrival does: let go) */
		if (++LB.switch_t > SPAWN_WAIT) emu_write8(BN6_PLAYER_LOCKED, 0);
		return false;
	}
	if (!lib_camera_home()) return false;
	LB.switch_t = 0;
	return true;
}

void lib_team_end(void) { player_body(-1); }

void lib_team_resume(void) {
	player_body(LB.in_play ? LB.mate[LB.in_play].sprite : -1);
	LB.mx = LB.b.mate[LB.in_play].x;
	LB.my = LB.b.mate[LB.in_play].y;
	LB.switch_t = 0;
}

void lib_team_fight_begin(void) {
	int k = LB.in_play;
	LB.mate_down = false;
	if (!k) return;
	saved_hp = emu_read16(BN6_NAVI_HP);
	saved_max = emu_read16(BN6_NAVI_MAX_HP);
	uint16_t v[2] = { (uint16_t)LB.mate[k].hp, (uint16_t)LB.mate[k].max };
	emu_write(BN6_NAVI_HP, v, sizeof v);
	lib_battle_soul(LB.b.team.navi[k]);
}

bool lib_team_fight_end(void) {
	int k = LB.in_play;
	if (!k) return false;
	/* (his HP as the battle left it: BN6's return to the map works
	 * MegaMan's NaviStats afresh, his max among them) */
	bool down = lib_battle_down();
	int hp = lib_battle_hp() ? lib_battle_hp() : emu_read16(BN6_NAVI_HP);
	uint16_t v[2] = { (uint16_t)saved_hp, (uint16_t)saved_max };
	emu_write(BN6_NAVI_HP, v, sizeof v);
	LB.mate[k].hp = down ? 0 : hp < 1 ? 1 : hp;
	if (down) LB.b.mate[k].out = true;
	if (emu_debug_on()) fprintf(stderr, "liberation team: %s's battle over, HP %d%s\n", lib_mate_name(k), LB.mate[k].hp, down ? ", deleted" : "");
	return down;
}
