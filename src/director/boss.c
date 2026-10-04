/* A guardian from beginning to end (docs/BOSSES.md), after the way Hades
 * stages its bosses: MegaMan steps into the arena and the music falls
 * away; the guardian logs in over the boss prelude, its name and epithet
 * on a title card, and speaks, remembering how their last battle went. The
 * battle follows without a question. Deleted, it says a last word and logs
 * out; its Guardian Data materializes where it stood, and only once
 * MegaMan takes it does the exit pad appear, with the area's music back. */
#include "boss.h"

#include <stdio.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "cinema.h"
#include "debug.h"
#include "director.h"
#include "emu.h"
#include "encounter.h"
#include "flags.h"
#include "gfx.h"
#include "gamecall.h"
#include "guardians.h"
#include "loot.h"
#include "net.h"
#include "net_arena.h"
#include "netmap.h"
#include "powers.h"
#include "rivals.h"
#include "run.h"
#include "save.h"

#define WALK_NEAR 52   /* world units from the guardian MegaMan walks up to */

enum {
	B_NONE,      /* no guardian on this layer */
	B_WAIT,      /* waiting in the arena */
	B_ENTER,     /* bars in, prelude, the guardian logs in */
	B_TITLE,     /* its title card */
	B_TALK,      /* its words before the battle */
	B_FIGHT,     /* the battle is on */
	B_AFTER,     /* back on the map, the music hushed */
	B_LAST_WORD, /* its words, deleted */
	B_LOGOUT,    /* it fades out */
	B_REWARD,    /* its Guardian Data waits to be taken */
	B_OPEN,      /* the exit pad appears */
	B_DONE,
};

static struct {
	int state, t;
	bool chat_seen;
	bool checkpoint;      /* the guardian fell: the run wants saving past it */
	bool door, door_taken;   /* MegaMan stepped in: the run wants saving at the arena's door (once an approach) */
	uint32_t archive;
	GuardianStage g;
} B;

static void to(int state) {
	if (emu_debug_on()) fprintf(stderr, "guardian %d state %d -> %d\n", B.g.navi, B.state, state);
	B.state = state;
	B.t = 0;
	B.chat_seen = false;
}

static void run_script(int script) { game_call(BN6_CHAT_RUN_SCRIPT, B.archive, (uint32_t)script); }

/* A conversation started with run_script has been read to its end. */
static bool chat_done(void) {
	bool open = emu_read8(BN6_CHATBOX) != 0;
	if (open) B.chat_seen = true;
	return (B.chat_seen && !open) || B.t > 60 * 60;
}

/* MegaMan's grid panel. */
static bool megaman_panel(int *x, int *y) {
	int wx = bn6_player_x(), wy = bn6_player_y();
	return netmap_panel(wx, wy, x, y);
}

/* In the arena (or, without one, close to the guardian), or within
 * `margin` panels of it. */
static bool near_arena(int margin) {
	int x, y;
	if (!megaman_panel(&x, &y)) return false;
	if (layer.arena >= 0) {
		const Room *a = &layer.rooms[layer.arena];
		return x >= a->x - margin && x < a->x + a->w + margin && y >= a->y - margin && y < a->y + a->h + margin;
	}
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_BOSS) {
			int dx = x - (int)layer.obj[i].x, dy = y - (int)layer.obj[i].y;
			return dx * dx + dy * dy <= (2 + margin) * (2 + margin);
		}
	return false;
}

static bool entered(void) { return near_arena(0); }

/* No random battle on the step into the arena: the game rolls one as he
 * steps in, before the staging can hold it, and the staging waited on a
 * virus battle (issue #38; the walk's steps, issue #24). Nor on the way
 * there, the antechamber and its bridge: MegaMan's last words before the
 * arena are said as he steps into the antechamber ("Once we're in,
 * there's no running from a guardian!"), and a playtester met a random
 * battle right after them, on the bridge three panels short of the arena:
 * the pair that had taken him from 320 to 10 HP a layer before (session
 * 64). The antechamber, where its dealer stands, is the last place to get
 * ready. Held a panel out around the arena and on its way in while its
 * guardian waits, lifted again away from them. */
static void door_quiet(void) {
	static bool held;
	int x, y;
	bool near = near_arena(1) || (megaman_panel(&x, &y) && arena_approach(x, y));
	if (near) flag_set(BN6_FLAG_NO_ENCOUNTERS);
	else if (held) flag_clear(BN6_FLAG_NO_ENCOUNTERS);
	held = near;
}

/* MegaMan in the arena, no chat open and no card showing (a CONTINUE at
 * the arena's door shows the act's card first): the staging begins. */
static bool steps_in(void) {
	door_quiet();
	return !emu_read8(BN6_CHATBOX) && !cinema_busy() && entered();
}

/* The pad keys that walk MegaMan towards world (x, y), 0 once he is within
 * `near` units: UP moves him +X -Y, RIGHT +X +Y, DOWN -X +Y, LEFT -X -Y. */
static uint32_t walk_toward(int x, int y, int near) {
	static const struct { int x, y; uint32_t k; } dirs[8] = {
		{ 7, -7, KEY_UP }, { 10, 0, KEY_UP | KEY_RIGHT }, { 7, 7, KEY_RIGHT }, { 0, 10, KEY_DOWN | KEY_RIGHT },
		{ -7, 7, KEY_DOWN }, { -10, 0, KEY_DOWN | KEY_LEFT }, { -7, -7, KEY_LEFT }, { 0, -10, KEY_UP | KEY_LEFT },
	};
	int dx = x - (bn6_player_x()), dy = y - (bn6_player_y());
	if (dx * dx + dy * dy <= near * near) return 0;
	uint32_t k = 0;
	long best = -1000000;
	for (int i = 0; i < 8; ++i) {
		long d = (long)dirs[i].x * dx + (long)dirs[i].y * dy;
		if (d > best) { best = d; k = dirs[i].k; }
	}
	return k;
}

static void title_card(void) {
	const Guardian *gd = guardian(B.g.navi);
	static const char *const suffix[3] = { "", " EX", " SP" }, *const older[4] = { "", " V2", " V3", " SP" };
	char name[40], top[48];
	/* (BN5's by its own versions' names) */
	if (guardian_older(B.g.navi)) snprintf(name, sizeof name, "%s%s", gd->name, older[B.g.version < 0 || B.g.version > 3 ? 0 : B.g.version]);
	else snprintf(name, sizeof name, "%s%s", gd->name, suffix[B.g.version < 0 || B.g.version > 2 ? 0 : B.g.version]);
	snprintf(top, sizeof top, "Guardian of %s", guardian_area_in_text(layer.biome, run.side_kind));
	cinema_title(top, name, gd->epithet, rgba(gd->r, gd->g, gd->b, 255), 170);
}

void boss_begin_layer(uint32_t archive, const GuardianStage *g) {
	B.archive = archive;
	B.g = *g;
	B.door = B.door_taken = false;
	to(g->navi ? B_WAIT : B_NONE);
	cinema_input(CINEMA_FREE);
	cinema_letterbox(false);
}

/* MegaMan's walk to the arena's middle, with no random battle from its
 * steps: one rolled as a player ran in, and it was fought under the
 * staging's bars (issue #24); the game lifts the flag as it enters the
 * next map. */
static void enter_walk(void) {
	flag_set(BN6_FLAG_NO_ENCOUNTERS);
	cinema_walk(B.t < 60 ? walk_toward(B.g.x, B.g.y, WALK_NEAR) : 0);
}

/* The guardian waits: MegaMan steps in, the run is saved at the arena's
 * door, and the staging begins on the next frame. */
static void wait_update(void) {
	if (!entered()) B.door_taken = false;   /* (out of the arena: the next step in is a new approach) */
	if (!steps_in()) return;
	/* the run saved at the arena's door first, on the last frame he is
	 * free (the owner's: bigger RPGs save before a boss), so a quit in the
	 * fight goes on from here, not from the layer's start */
	if (!B.door_taken) { B.door = B.door_taken = true; return; }
	/* the arena closes around MegaMan, who steps up to its middle */
	cinema_input(CINEMA_WALK);
	cinema_letterbox(true);
	run_script(B.g.hush);
	rival_met(B.g.navi);
	to(B_ENTER);
}

/* No question after his words: the battle begins, the player's again. */
static void fight_begin(void) {
	cinema_input(CINEMA_FREE);
	cinema_letterbox(false);
	/* (BN5's in BN5's engine, on the guest core: docs/BOSSES.md, BN5's
	 * Navis; its end comes back through director_guest_done) */
	if (guardian_older(B.g.navi) && director_guest_guardian(B.g.navi, B.g.version)) return;
	/* (else BN6's battle: the area's own guardian where the guest cannot
	 * fight his) */
	Encounter e = make_boss(run.depth, run.biome, guardian_older(B.g.navi) ? run.boss_order[run.biome] : B.g.navi);
	/* (no running from a guardian, as from BN6's story bosses: a
	 * playtester ran from CircusMan at 5 HP, healed beside the arena and
	 * came back to a fresh fight) */
	e.held = true;
	emu_battle_force(&e);
}

void boss_update(void) {
	++B.t;
	switch (B.state) {
	case B_WAIT: wait_update(); break;
	case B_ENTER:
		enter_walk();
		if (B.t == 60) cinema_input(CINEMA_HOLD);
		if (B.t == 30) run_script(B.g.prelude);
		if (B.t == 50) { flag_set(LAYER_BOSS_APPEAR_FLAG); cinema_shake(16, 3); }
		if (B.t == 80) { title_card(); to(B_TITLE); }
		break;
	case B_TITLE:
		if (cinema_busy()) break;
		/* the bars make way for the chat box */
		cinema_letterbox(false);
		cinema_input(CINEMA_TALK);
		run_script(B.g.intro);
		to(B_TALK);
		break;
	case B_TALK:
		if (!chat_done()) break;
		fight_begin();
		to(B_FIGHT);
		break;
	case B_FIGHT:
	case B_NONE:
	case B_DONE:
		break;
	case B_AFTER:
		if (B.t == 1) { cinema_input(CINEMA_HOLD); cinema_letterbox(true); run_script(B.g.hush); }
		if (B.t < 40) break;
		cinema_letterbox(false);
		cinema_input(CINEMA_TALK);
		run_script(B.g.defeat);
		to(B_LAST_WORD);
		break;
	case B_LAST_WORD:
		if (!chat_done()) break;
		cinema_input(CINEMA_HOLD);
		cinema_letterbox(true);
		flag_set(LAYER_BOSS_GONE_FLAG);
		cinema_flash(20);
		to(B_LOGOUT);
		break;
	case B_LOGOUT:
		if (B.t < 60) break;
		/* what it leaves behind; MegaMan goes to take it */
		flag_set(LAYER_REWARD_FLAG);
		cinema_letterbox(false);
		cinema_input(CINEMA_FREE);
		run_script(B.g.theme);
		/* (a checkpoint here, which boss_resume takes up with the Guardian
		 * Data waiting: a playtester quit in its talk after deleting
		 * HeatMan, and his CONTINUE began the layer, and HeatMan, again) */
		B.checkpoint = true;
		to(B_REWARD);
		break;
	case B_REWARD:
		if (!flag_get(LAYER_REWARD_TAKEN_FLAG) || emu_read8(BN6_CHATBOX)) break;
		/* (the Guardian Data's draft has told the board's rules once) */
		if (!profile.navicust_taught && run.side_kind == LAYER_NORMAL) {
			profile.navicust_taught = 1;
			profile_save();
		}
		flag_set(LAYER_EXIT_OPEN_FLAG);
		cinema_shake(12, 2);
		to(B_OPEN);
		break;
	case B_OPEN:
		if (B.t > 20) to(B_DONE);
		break;
	}
}

bool boss_fighting(void) { return B.state == B_FIGHT; }

void boss_battle_over(bool won) {
	if (B.state != B_FIGHT) return;
	if (!won) {
		/* not deleted but not won either: it waits to fight again */
		to(B_WAIT);
		return;
	}
	rival_result(B.g.navi, RIVAL_MEGAMAN_WON);
	run.bosses_beaten++;
	powers_after_boss(B.g.navi, run.biome);
	if (run.side_kind == LAYER_SECRET) run.secret_cleared = true;
	to(B_AFTER);
}

void boss_lost(void) {
	if (B.state == B_FIGHT) rival_result(B.g.navi, RIVAL_NAVI_WON);
}

bool boss_exit_open(void) { return B.state == B_NONE || B.state >= B_OPEN; }

bool boss_beaten(void) { return B.state >= B_AFTER; }

bool boss_cinematic(void) { return B.state >= B_ENTER && B.state <= B_LOGOUT && B.state != B_FIGHT; }

bool boss_done(void) { return B.state >= B_OPEN; }

bool boss_idle(void) { return B.state == B_NONE || B.state == B_WAIT || B.state >= B_OPEN; }

bool boss_take_checkpoint(void) {
	bool due = B.checkpoint;
	B.checkpoint = false;
	return due;
}

bool boss_take_door(void) {
	bool due = B.door;
	B.door = false;
	return due;
}

void boss_resume(void) {
	/* (a run saved mid-layer: the guardian's flags are in the state; saved
	 * at the arena's door, he stands in it, and the staging begins) */
	if (B.state != B_WAIT) return;
	B.door_taken = entered();
	if (flag_get(LAYER_EXIT_OPEN_FLAG)) to(B_DONE);
	else if (flag_get(LAYER_REWARD_FLAG) && !flag_get(LAYER_REWARD_TAKEN_FLAG)) to(B_REWARD);
}

bool boss_goal(int *x, int *y, bool *talk) {
	if (B.state == B_NONE || B.state >= B_OPEN) return false;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_BOSS) {
			*x = (int)layer.obj[i].x;
			*y = (int)layer.obj[i].y;
			*talk = B.state == B_REWARD;
			return true;
		}
	return false;
}
