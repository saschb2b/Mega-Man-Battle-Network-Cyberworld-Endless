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
#include "boss_grand.h"
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
#include "net_arena.h"
#include "netmap.h"
#include "platform.h"
#include "powers.h"
#include "rivals.h"
#include "save.h"

#define WALK_NEAR   52   /* world units from the guardian MegaMan walks up to, where he has no place beside him */
#define WALK_FRAMES 100  /* his steps up to a guardian, at most */
#define STAND_NEAR  4    /* world units from his place beside the guardian he stops at */

enum {
	B_NONE,      /* no guardian on this layer */
	B_WAIT,      /* waiting in the arena */
	B_ENTER,     /* bars in, prelude, the guardian logs in */
	B_TITLE,     /* its title card */
	B_TALK,      /* its words before the battle */
	B_FIGHT,     /* the battle is on */
	B_AFTER,     /* back on the map, the music hushed */
	B_LAST_WORD, /* its words, deleted */
	B_LOGOUT,    /* it logs out */
	B_REWARD,    /* its Guardian Data waits to be taken */
	B_OPEN,      /* the exit pad appears */
	B_DONE,
};

/* A guardian's log-out to his data: BN6's 17 frames, then a beat */
#define LOGOUT_BEAT 45

static struct {
	int state, t;
	bool chat_seen;
	bool checkpoint;      /* the guardian fell: the run wants saving past it */
	bool door, door_taken;   /* MegaMan stepped in: the run wants saving at the arena's door (once an approach) */
	bool there;              /* ... and has stepped up to the guardian */
	int leg;                 /* ... the eighth his steps' leg keeps (walk_toward), -1 none yet */
	uint32_t archive;
	GuardianStage g;
} B;

static void to(int state) {
	if (emu_debug_on()) fprintf(stderr, "guardian %d state %d -> %d at frame %u\n", B.g.navi, B.state, state, (unsigned)P.frame);
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
 * `near` units: UP moves him +X -Y, RIGHT +X +Y, DOWN -X +Y, LEFT -X -Y. In
 * straight legs, as BN6's cutscenes walk him: a leg keeps its eighth while
 * (x, y) still lies ahead along it, and only then turns, and one ended
 * within twice `near` is there (an eighth picked afresh each frame swung
 * him between two neighbours, left and right quickly: the owner, on the
 * Nova). */
static uint32_t walk_toward(int x, int y, int near) {
	static const struct { int x, y; uint32_t k; } dirs[8] = {
		{ 7, -7, KEY_UP }, { 10, 0, KEY_UP | KEY_RIGHT }, { 7, 7, KEY_RIGHT }, { 0, 10, KEY_DOWN | KEY_RIGHT },
		{ -7, 7, KEY_DOWN }, { -10, 0, KEY_DOWN | KEY_LEFT }, { -7, -7, KEY_LEFT }, { 0, -10, KEY_UP | KEY_LEFT },
	};
	int dx = x - (bn6_player_x()), dy = y - (bn6_player_y());
	long left = (long)dx * dx + (long)dy * dy;
	if (left <= (long)near * near) return 0;
	if (B.leg >= 0 && (long)dirs[B.leg].x * dx + (long)dirs[B.leg].y * dy > 0) return dirs[B.leg].k;
	if (B.leg >= 0 && left <= 4L * near * near) return 0;
	long best = -1000000;
	for (int i = 0; i < 8; ++i) {
		long d = (long)dirs[i].x * dx + (long)dirs[i].y * dy;
		if (d > best) { best = d; B.leg = i; }
	}
	return dirs[B.leg].k;
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
	/* (a super boss's staging: boss_grand.c) */
	if (super_boss(g->navi)) grand_begin(archive, g);
	if (emu_debug_on() && g->navi)
		fprintf(stderr, "guardian %d at %d %d, arena entered along %d, MegaMan meets him at %d %d facing %d\n", g->navi, g->x, g->y,
			layer.arena_dir, g->stand_x, g->stand_y, g->stand_face);
	B.door = B.door_taken = false;
	to(g->navi ? B_WAIT : B_NONE);
	cinema_input(CINEMA_FREE);
	cinema_letterbox(false);
}

/* MegaMan turned to face eighth `face`, as BN6's cutscenes turn him (his
 * facing and the animation he stands in) */
static void face_to(int face) {
	emu_write8(BN6_PLAYER_FACING, (uint8_t)face);
	emu_write8(BN6_PLAYER_ANIM, (uint8_t)face);
}

/* MegaMan's steps up to the guardian, the staging's own as BN6's
 * cutscenes walk him (cs_move_player_in_facing_direction: as if the
 * player walked him, the pad the staging's while his is held): to his
 * place beside the guardian (guardian_stand), where the chat box under
 * them covers neither, then a turn to face him; straight at him to `near`
 * units where the layer gives no place. Whether he is there, at the
 * latest after `frames`. No random battle from its steps: one rolled as a
 * player ran in was fought under the staging's bars (issue #24); the
 * game lifts the flag as it enters the next map. */
static bool enter_walk(int frames, int near) {
	flag_set(BN6_FLAG_NO_ENCOUNTERS);
	if (B.there) return true;
	bool beside = B.g.stand_face >= 0;
	uint32_t keys = B.t > frames ? 0 : beside ? walk_toward(B.g.stand_x, B.g.stand_y, STAND_NEAR) : walk_toward(B.g.x, B.g.y, near);
	cinema_walk(keys);
	if (keys) return false;
	B.there = true;
	if (beside) face_to(B.g.stand_face);
	return true;
}

/* The guardian waits: MegaMan steps in, the run is saved at the arena's
 * door, and the staging begins on the next frame. */
static void wait_update(void) {
	if (!entered()) B.door_taken = false;   /* (out of the arena: the next step in is a new approach) */
	/* (a super boss's approach: the theme fading near his arena, the
	 * floor shaking before the Cybeast) */
	if (super_boss(B.g.navi)) {
		int x, y;
		grand_approach(near_arena(0) || (megaman_panel(&x, &y) && arena_approach(x, y)));
	}
	if (!steps_in()) return;
	/* the run saved at the arena's door first, on the last frame he is
	 * free (the owner's: bigger RPGs save before a boss), so a quit in the
	 * fight goes on from here, not from the layer's start */
	if (!B.door_taken) { B.door = B.door_taken = true; return; }
	/* the arena closes around MegaMan, who steps up to the guardian */
	cinema_input(CINEMA_WALK);
	cinema_letterbox(true);
	run_script(B.g.hush);
	rival_met(B.g.navi);
	B.there = false;
	B.leg = -1;
	to(B_ENTER);
}

/* What the Guardian Data, taken, has told once: the draft the board's rules,
 * BN5's guardian's what a Soul does (docs/META.md, Souls) */
static void reward_taught(void) {
	bool soul = guardian_older(B.g.navi) && !profile.soul_taught, board = !profile.navicust_taught && run.side_kind == LAYER_NORMAL;
	if (soul) profile.soul_taught = 1;
	if (board) profile.navicust_taught = 1;
	if (soul || board) profile_save();
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
	/* (a tenth more HP each notch of the Net's clock, docs/HOME.md) */
	emu_battle_clock(navi_hp(e.foes[0].family, e.foes[0].version));
	emu_battle_force(&e);
}

/* The entrance: MegaMan steps up beside the guardian, the prelude, the
 * guardian's log-in with a shake, his card once MegaMan is there; a super
 * boss's is boss_grand.c's, longer */
static void enter_update(void) {
	bool grand = super_boss(B.g.navi);
	bool there = enter_walk(grand ? GRAND_WALK_FRAMES : WALK_FRAMES, grand ? GRAND_WALK_NEAR : WALK_NEAR);
	if (there && cinema_input_mode() == CINEMA_WALK) cinema_input(CINEMA_HOLD);
	if (grand) {
		if (grand_enter(B.t, there)) to(B_TITLE);
		return;
	}
	if (B.t == 30) run_script(B.g.prelude);
	if (B.t == 50) { flag_set(LAYER_BOSS_APPEAR_FLAG); cinema_shake(16, 3); }
	if (B.t >= 80 && there) { title_card(); to(B_TITLE); }
}

/* Back on the map after the battle: bars and silence, then his last word.
 * The game starts the map's theme a frame after the battle, over a hush
 * run before it: the hush runs while a song plays (BN6_MUSIC_STATUS read
 * each frame after a guardian's battle). */
static bool music_on(void) { return !(emu_read32(BN6_MUSIC_STATUS) & BN6_MUSIC_STOPPED); }

static void after_update(void) {
	if (B.t == 1) {
		cinema_input(CINEMA_HOLD);
		cinema_letterbox(true);
	}
	if (B.t < 40) {
		if (music_on()) run_script(B.g.hush);
		return;
	}
	cinema_letterbox(false);
	cinema_input(CINEMA_TALK);
	run_script(B.g.defeat);
	to(B_LAST_WORD);
}

/* His last word read: a guardian logs out, BN6's beam beside him (a
 * super boss's fall boss_grand.c's); no flash, which BN6 never gives a
 * Navi's log-out */
static void last_word_done(void) {
	cinema_input(CINEMA_HOLD);
	cinema_letterbox(true);
	if (!super_boss(B.g.navi)) flag_set(LAYER_BOSS_GONE_FLAG);
	to(B_LOGOUT);
}

/* He logs out; a beat after, what he leaves behind shows, and MegaMan goes
 * to take it, the area's theme back (after the Cybeast the Net stays
 * quiet) */
static void logout_update(void) {
	bool grand = super_boss(B.g.navi);
	if (grand ? !grand_fall(B.t) : B.t < LOGOUT_BEAT) return;
	flag_set(LAYER_REWARD_FLAG);
	cinema_letterbox(false);
	cinema_input(CINEMA_FREE);
	if (!grand || grand_theme_after()) run_script(B.g.theme);
	/* (a checkpoint here, which boss_resume takes up with the Guardian
	 * Data waiting: a playtester quit in its talk after deleting HeatMan,
	 * and his CONTINUE began the layer, and HeatMan, again) */
	B.checkpoint = true;
	to(B_REWARD);
}

void boss_update(void) {
	++B.t;
	switch (B.state) {
	case B_WAIT: wait_update(); break;
	case B_ENTER:
		enter_update();
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
	case B_AFTER: after_update(); break;
	case B_LAST_WORD:
		if (chat_done()) last_word_done();
		break;
	case B_LOGOUT:
		logout_update();
		break;
	case B_REWARD:
		if (!flag_get(LAYER_REWARD_TAKEN_FLAG) || emu_read8(BN6_CHATBOX)) break;
		reward_taught();
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
	powers_after_boss(B.g.navi);
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
