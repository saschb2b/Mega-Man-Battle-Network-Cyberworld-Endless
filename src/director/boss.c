/* A guardian from beginning to end (docs/game/bosses/), after the way Hades
 * stages its bosses: MegaMan steps into the arena and the music falls
 * away; the guardian logs in over the boss prelude, its name and epithet
 * on a title card, and speaks, remembering how their last battle went. The
 * battle follows without a question. Deleted, it says a last word and logs
 * out; its Guardian Data materializes where it stood, and only once
 * MegaMan takes it does the exit pad appear, with the area's music back. */
#include "boss.h"

#include <stdio.h>

#include "bn6_fields.h"
#include "boss_grand.h"
#include "bracket.h"
#include "cinema.h"
#include "debug.h"
#include "emu.h"
#include "encounter.h"
#include "flags.h"
#include "gfx.h"
#include "gamecall.h"
#include "guardians.h"
#include "loot.h"
#include "net_arena.h"
#include "netmap.h"
#include "pacing.h"
#include "platform.h"
#include "powers.h"
#include "rivals.h"
#include "save.h"
#include "story_words.h"
#include "xguardian.h"

#define WALK_NEAR   52   /* world units from the guardian MegaMan walks up to, where he has no place beside him */
#define WALK_FRAMES 100  /* his steps up to a guardian, at most */
#define STAND_NEAR  4    /* world units from his place beside the guardian he stops at */

enum {
	B_NONE,      /* no guardian on this layer */
	B_WAIT,      /* waiting in the arena */
	B_ENTER,     /* bars in, prelude, the guardian logs in */
	B_BRACKET,   /* a tournament's: its bracket shown on BN4's screen, off the map */
	B_ROUND_IN,  /* ... back on the map: the next round's Navi, the final's log-in */
	B_TITLE,     /* its title card */
	B_TALK,      /* its words before the battle */
	B_FIGHT,     /* the battle is on */
	B_PRIZE,     /* a tournament's round won: its Navi's chip */
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
	Encounter fought;        /* his battle, as it was forced (the run log's), */
	int fought_hp;           /* ... and the HP he spawns with */
	bool soul_new;           /* BN5's guardian beaten, his Soul new to the run: his data's words told it (souls.h) */
	/* how his battle stands, its last frame his record's (rivals.h):
	 * MegaMan's form while he stands, and each side's HP and max */
	int form, hp, max, g_hp, g_max;
	/* the tournament's bracket in his place (docs/game/tournament.md): the
	 * round under way (0, 1, 2 the final, 3 won), its Navi and version, and
	 * the frame its bracket was opened at */
	int round, foe, foe_version;
	uint32_t opened;
	uint32_t enter[4];  /* ... where the map entered again puts MegaMan, before it was made his place: put back after */
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
	const Guardian *gd = guardian(B.foe);
	static const char *const suffix[3] = { "", " EX", " SP" }, *const older[4] = { "", " V2", " V3", " SP" };
	char name[40], top[48];
	int v = B.foe_version;
	/* (BN5's by its own versions' names) */
	if (guardian_older(B.foe)) snprintf(name, sizeof name, "%s%s", gd->name, older[v < 0 || v > 3 ? 0 : v]);
	else snprintf(name, sizeof name, "%s%s", gd->name, suffix[v < 0 || v > 2 ? 0 : v]);
	snprintf(top, sizeof top, "Guardian of %s", guardian_area_in_text(layer.biome, run.side_kind));
	/* (a tournament's round over his name: docs/game/tournament.md, The card) */
	if (B.g.bracket) snprintf(top, sizeof top, "%s", bracket_round_label(B.round));
	cinema_title(top, name, gd->epithet, rgba(gd->r, gd->g, gd->b, 255), 170);
}

/* The bracket's round `round` set to be fought: its Navi and his version */
static void round_set(int round) {
	B.round = round;
	B.foe = B.g.bracket && round < 3 ? B.g.rounds.navi[round] : B.g.navi;
	B.foe_version = B.g.bracket && round < 3 ? B.g.rounds.version[round] : B.g.version;
}

/* BN4's bracket screen, moved into BN6, with the rounds MegaMan has won
 * settled (xbracket.h): off the map until its fade back */
static void bracket_show(int won) {
	XbrEntrant e[BN4_ENTRANTS];
	bracket_entrants(&B.g.rounds, won, e);
	cinema_letterbox(false);
	B.opened = P.frame;
	/* (the map entered again after it brings MegaMan back where he stands,
	 * not to the layer's arrival: measured, he came back at its start) */
	static const uint32_t enter[4] = { BN6_ENTER_X, BN6_ENTER_Y, BN6_ENTER_Z, BN6_ENTER_FACING };
	const uint32_t here[4] = { emu_read32(BN6_PLAYER_X), emu_read32(BN6_PLAYER_Y), emu_read32(BN6_PLAYER_Z), emu_read8(BN6_PLAYER_FACING) };
	for (int i = 0; i < 4; ++i) {
		B.enter[i] = emu_read32(enter[i]);
		emu_write32(enter[i], here[i]);
	}
	bool shown = xbracket_open(bracket_tournament((run.depth - 1) % CYCLE_LAYERS / 3), e);
	if (emu_debug_on()) fprintf(stderr, "bracket: %d rounds won, %s at frame %u\n", won, shown ? "shown" : "not shown", (unsigned)P.frame);
}

/* Whether the bracket has come and gone: frames passed off the map since
 * it opened (this runs on the map alone), then a beat back on it; or it
 * never showed */
static bool bracket_back(void) {
	uint32_t away = P.frame - B.opened - (uint32_t)B.t;
	return (away > 30 && B.t > 20) || B.t > 240;
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
	round_set(g->bracket ? 0 : 2);
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
	/* (a quit anywhere in a bracket comes back to its door: round 1 again,
	 * with the HP walked in with, docs/game/tournament.md, The arena's door) */
	round_set(B.g.bracket ? 0 : 2);
	rival_met(B.foe);
	B.there = false;
	B.leg = -1;
	to(B_ENTER);
}

/* What the Guardian Data, taken, has told once: the draft the board's rules,
 * BN5's guardian's what a Soul does (docs/game/meta/souls.md) */
static void reward_taught(void) {
	bool older = B.soul_new, soul = older && !profile.soul_taught, form = older && !profile.soul_form_taught,
		board = !profile.navicust_taught && run.side_kind == LAYER_NORMAL;
	if (soul) profile.soul_taught = 1;
	if (form) profile.soul_form_taught = 1;
	if (board) profile.navicust_taught = 1;
	if (soul || form || board) profile_save();
}

/* No question after his words: the battle begins, the player's again. */
static void fight_begin(void) {
	cinema_input(CINEMA_FREE);
	cinema_letterbox(false);
	/* (BN5's in BN6's engine as BN6's are, his moved id at the version his
	 * act's band takes: docs/game/bosses/bn5-navis.md) */
	bool older = guardian_older(B.foe), round = B.g.bracket && B.round < 2;
	Encounter e = older ? xguardian_encounter(B.foe, B.foe_version) : make_boss(run.depth, run.biome, B.foe);
	/* (a bracket's round at its own version: run_bracket_draw) */
	if (round && !older) e.foes[0].version = B.foe_version;
	/* (no running from a guardian, as from BN6's story bosses: a
	 * playtester ran from CircusMan at 5 HP, healed beside the arena and
	 * came back to a fresh fight) */
	e.held = true;
	/* (a tenth more HP each notch of the Net's clock, docs/world/home/: on his
	 * own HP, or on BN5's held to the act's band, as he spawns) */
	int own = navi_hp(e.foes[0].family, e.foes[0].version);
	if (older) own = xguardian_hp(B.foe, B.foe_version);
	int fought = older ? xguardian_hp_fought(B.foe, B.foe_version, run.depth) : own;
	if (round && older) fought = B.g.rounds.hp[B.round];
	/* (a special route's last boss at the endless Nest: the Cybeast's HP) */
	if (run.biome == BIOME_NEST && run.side_kind == LAYER_NORMAL) fought = run_nest_hp(run.depth, B.g.navi, fought);
	B.fought_hp = pacing_clock_hp(fought, run.clock);
	emu_battle_hp(own, B.fought_hp);
	if (emu_debug_on()) fprintf(stderr, "guardian %d: enemy id %#x, version %d, HP %d fought at %d%s\n", B.foe, e.foes[0].id, e.foes[0].version, own,
		B.fought_hp, B.g.bracket ? B.round == 0 ? " (round 1)" : B.round == 1 ? " (round 2)" : " (the final)" : "");
	B.fought = e;
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
	/* (a tournament: its bracket first, the final's Navi logging in after
	 * the rounds) */
	if (B.g.bracket) {
		if (B.t >= 40 && there) { bracket_show(0); to(B_BRACKET); }
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

/* Back on the map from the bracket: a round's Navi straight to his card,
 * the final's logging in as a guardian's does */
static void round_in_update(void) {
	if (B.t == 1) { cinema_letterbox(true); cinema_input(CINEMA_HOLD); }
	/* (the map's theme the map entered again started, hushed) */
	if (B.t < 10 && music_on()) run_script(B.g.hush);
	if (B.round < 2) {
		if (B.t >= 30) { title_card(); to(B_TITLE); }
		return;
	}
	if (B.t == 10) run_script(B.g.prelude);
	if (B.t == 30) { flag_set(LAYER_BOSS_APPEAR_FLAG); cinema_shake(16, 3); }
	if (B.t >= 60) { title_card(); to(B_TITLE); }
}

/* A round won, back on the map: its Navi's chip, then the bracket again */
static void prize_update(void) {
	if (B.t == 1) { cinema_letterbox(true); cinema_input(CINEMA_HOLD); }
	if (B.t < 30) {
		if (music_on()) run_script(B.g.hush);
		return;
	}
	int script = B.g.round_prize[B.round];
	if (B.t == 30 && script >= 0) { cinema_letterbox(false); cinema_input(CINEMA_TALK); run_script(script); }
	if (script >= 0 && !chat_done()) return;
	cinema_input(CINEMA_HOLD);
	round_set(B.round + 1);
	rival_met(B.foe);
	bracket_show(B.round);
	to(B_BRACKET);
}

void boss_update(void) {
	++B.t;
	switch (B.state) {
	case B_WAIT: wait_update(); break;
	case B_ENTER:
		enter_update();
		break;
	case B_BRACKET:
		if (!bracket_back()) break;
		emu_write32(BN6_ENTER_X, B.enter[0]);
		emu_write32(BN6_ENTER_Y, B.enter[1]);
		emu_write32(BN6_ENTER_Z, B.enter[2]);
		emu_write32(BN6_ENTER_FACING, B.enter[3]);
		to(B.round > 2 ? B_AFTER : B_ROUND_IN);
		break;
	case B_ROUND_IN: round_in_update(); break;
	case B_PRIZE: prize_update(); break;
	case B_TITLE:
		if (cinema_busy()) break;
		/* (a bracket's round: no words, the battle at once) */
		if (B.g.bracket && B.round < 2) {
			cinema_letterbox(false);
			fight_begin();
			B.form = B.hp = B.max = B.g_hp = B.g_max = 0;
			to(B_FIGHT);
			break;
		}
		/* the bars make way for the chat box */
		cinema_letterbox(false);
		cinema_input(CINEMA_TALK);
		run_script(B.g.intro);
		to(B_TALK);
		break;
	case B_TALK:
		if (!chat_done()) break;
		fight_begin();
		B.form = B.hp = B.max = B.g_hp = B.g_max = 0;
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

void boss_battle_frame(void) {
	if (B.state != B_FIGHT) return;
	bool mine = false;
	for (uint32_t k = 0; k < BN6_T1_COUNT; ++k) {
		uint32_t o = BN6_T1_OBJECTS + k * BN6_T1_SIZE;
		if (!(emu_read8(o + BN6_T1_IN_PLAY) & 1)) continue;
		int side = emu_read8(o + BN6_T1_ALLIANCE), hp = emu_read16(o + BN6_T1_HP), max = emu_read16(o + BN6_T1_MAX_HP);
		/* (MegaMan the first in play on his side, as encounter.c takes
		 * him; the guardian the enemy with the most MaxHP, his soldiers
		 * and summons fewer) */
		if (side == 0 && !mine) {
			mine = true;
			B.hp = hp;
			B.max = max;
			if (hp > 0) B.form = emu_read8(BN6_BATTLE_FORM);
		} else if (side == 1 && max >= B.g_max) {
			B.g_hp = hp;
			B.g_max = max;
		}
	}
}

const Encounter *boss_battle(int *hp) {
	*hp = B.fought_hp;
	return &B.fought;
}

void boss_battle_over(bool won) {
	if (B.state != B_FIGHT) return;
	if (!won) {
		/* not deleted but not won either: it waits to fight again */
		to(B_WAIT);
		return;
	}
	rival_result(B.foe, RIVAL_MEGAMAN_WON, &(RivalEnd){ B.form, B.hp, B.max }, profile.runs + 1);
	/* (a bracket's round: its prize, then the next) */
	if (B.g.bracket && B.round < 2) {
		to(B_PRIZE);
		return;
	}
	run.bosses_beaten++;
	B.soul_new = powers_after_boss(B.g.navi);
	if (run.side_kind == LAYER_SECRET) run.secret_cleared = true;
	/* (its final: the bracket once more, MegaMan its winner, then his last
	 * word) */
	if (B.g.bracket) {
		round_set(3);
		bracket_show(3);
		to(B_BRACKET);
		return;
	}
	to(B_AFTER);
}

void boss_lost(void) {
	if (B.state == B_FIGHT) rival_result(B.foe, RIVAL_NAVI_WON, &(RivalEnd){ B.form, B.g_hp, B.g_max }, profile.runs + 1);
}

int boss_foe(void) { return B.foe; }

int boss_round(void) { return B.g.bracket ? B.round : -1; }

bool boss_exit_open(void) { return B.state == B_NONE || B.state >= B_OPEN; }

bool boss_beaten(void) { return B.state >= B_AFTER; }

bool boss_cinematic(void) { return B.state >= B_ENTER && B.state <= B_LOGOUT && B.state != B_FIGHT; }

bool boss_reward_waiting(void) { return B.state == B_REWARD; }

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
