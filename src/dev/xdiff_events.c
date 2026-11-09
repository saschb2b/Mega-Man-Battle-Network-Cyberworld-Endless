/* xdiff_events.h: the differential's hooks, each game's at its own
 * addresses (bn5.h, bn6.h: the same engine), run on the core whose frame
 * runs (hook.h: per-core hooks). A hook reads where it was called from:
 * its caller (lr, or a spawner's own caller from the spawner's frame), then
 * the stack from there up, word by word, to the object runner's return
 * address (the call that runs each battle object): each word that is a
 * return address (after a BL, or after mov lr,pc; bx rN, as the runner and
 * the jump tables call) is a caller further out, the last the outermost,
 * the first in a family's code the family's. */
#include "xdiff_events.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "debug.h"
#include "emu.h"
#include "guest.h"
#include "rom.h"
#include "soulcode.h"
#include "xchips_bn5.h"
#include "xchips_code.h"
#include "xchips_moved.h"
#include "xchips_pas.h"
#include "xvirus.h"
#include "xvirus_bn5.h"
#include "xvirus_dispatch.h"
#include "xdiff_sprite.h"
#include "xnavi_bn5.h"
#include "xsong.h"
#include "xvirus_families.h"

#define EVENTS_MAX  8192    /* a log's events between two takes (a battle's opening, a frame of the fight) */
#define DRAWS_MAX   32768   /* the reference's draws kept for the replay: a battle's */
#define KEYS_MAX    64      /* the keys a frame's draws take */
#define CHAIN_WORDS 96      /* the stack words looked through for callers further out */
#define FAMILY_MAX  4096    /* the spec's families', the Navis' and the played chips' blocks of code (BN5's 25 families' 80 or so, and
                               their clones; the Navis', BN4's too; the chips' units'): of the ranges moved, past 1000, those a spec
                               watches; more refuse the log, never drop a block unwatched (xvirus_moved.h) */
#define SPAWNER_LR  0x18    /* a spawner's frame (push {r7,lr}; sub sp,#0x14) from its five words: its lr, its caller */
#define DISPATCH_FRAME 0x1C /* ... and above it, where the second-level dispatch's spawner called it, that one's (push {r0,lr}): its
                               game's type, its own caller (xvirus_dispatch.c) */
#define ROM_END     0x0A000000u
#define IWRAM_END   0x03008000u   /* (the stack's top lies below it: a scan stops there, short of IWRAM's mirrors) */

/* Each game's routines (bn5.h, bn6.h; BN4's xdiff_bn4_trace.c's, xd_log_bn4) */
typedef XdGame Game;

static const Game games[2] = {
	{ { BN5_RNG_DRAW, BN5_RNG_DRAW_POSITIVE, BN5_RNG_DRAW_SECOND, BN5_RNG_DRAW_SECOND_POSITIVE }, BN5_SPAWN_COMMON, BN5_SPAWN_COMMON_END,
		BN5_PLAY_SOUND, BN5_SPAWN_EFFECT, BN5_SET_ATTACK, BN5_RESERVE_PANEL_DONE, BN5_BLINK_FLASHING, BN5_SPRITE_LOAD, BN5_T1_OBJECTS,
		BN5_OBJECT_RUNNER_RET, BN5_OBJECT_RUNNING,
		{ BN5_RNG_PRIMARY, BN5_RNG_SECONDARY }, BN5_COLL_FLASH, 0, BN5_MOOD_GAIN, BN5_BATTLE_NAVI, BN5_BATTLE_NAVI_SIZE, BN5_MOOD_SET,
		BN5_MOOD_CALM },
	{ { BN6_RNG_DRAW, BN6_RNG_DRAW_POSITIVE, BN6_RNG_DRAW_SECOND, BN6_RNG_DRAW_SECOND_POSITIVE }, BN6_SPAWN_COMMON, BN6_SPAWN_COMMON_END,
		BN6_PLAY_SOUND, BN6_SPAWN_EFFECT, BN6_SET_ATTACK, BN6_RESERVE_PANEL_DONE, BN6_BLINK_FLASHING, BN6_SPRITE_LOAD, BN6_T1_OBJECTS,
		BN6_OBJECT_RUNNER_RET, BN6_OBJECT_RUNNING,
		{ BN6_RNG_PRIMARY, BN6_RNG_SECONDARY }, BN6_COLL_FLASH, 2, 0, BN6_BATTLE_NAVI, BN6_BATTLE_NAVI_SIZE, 0, 0 },
};
static const Game *game4;   /* (BN4's, xd_log_bn4) */

void xd_log_bn4(const XdGame *g) { game4 = g; }

/* a game's routines: 5's, 6's or 4's (NULL where BN4's were not given) */
static const Game *game_of(int game) { return game == 4 ? game4 : &games[game == 6]; }

/* A draw kept: the battle's timer, its key (a BN5 address), the word it was drawn from, which word; and whether it was a
 * draw of the family's made before the fight was seeded (xd_replay_setup: kept first, all before the fight's) */
typedef struct { uint32_t t, key, word, which; bool setup; } Draw;
/* a key's draws in the frame replayed so far */
typedef struct { uint32_t key, which; int n; } Keyed;

struct XdLog {
	const Game *g;
	int game;
	bool alone;   /* BN6's against BN6's (xd_control_frame): every place keyed as it is, the control's own code as its copy */
	bool navis;   /* a foe of the spec's a moved Navi: its code watched, BN6's own songs of a copied sound's number told apart */
	bool on, seeded, setup;   /* (setup: the battle being set up, its spawns and sprites watched: xd_setup) */
	uint32_t frame, t;
	XdEvent ev[EVENTS_MAX];
	int nev, dropped;
	XdEvent spawning;   /* a spawn begun, until its end names the object */
	bool pending;
	XvMoved fam[FAMILY_MAX];   /* the families' code: BN5's at from, BN6's copy at to */
	int nfam;
	Draw draws[DRAWS_MAX];     /* the reference's, kept */
	int ndraws;
	const XdLog *ref;          /* the subject's: the reference it replays */
	const XdTwin *twins;
	int ntwins;
	int cursor;                /* the reference's first draw of the timer replayed */
	int lag, lag_cursor;       /* (xd_replay_lag) the family's own draws given the reference's of `lag` fight frames before: its
	                              first draw of that timer */
	uint32_t keyed_t;
	Keyed keys[KEYS_MAX];
	int nkeys, replayed, unmatched, unmapped;
	bool setup_draws;            /* (xd_replay_setup) the family's draws before the fight is seeded kept (the reference's) or
	                                replayed (the subject's), the k-th of a key in the whole setup the reference's k-th */
	Keyed setup_keys[KEYS_MAX];  /* ... the subject's setup draws of each key met so far */
	int nsetup_keys;
	Keyed setup_skips[XD_SETUP_SKIPS];   /* (xd_replay_setup_skip) each key's reference's first n setup draws, without a twin */
	int nsetup_skips;
};

/* (made as a dev tool first asks for one: their megabyte or two stays out of every other run) */
static XdLog *logs[2];

/* game 5's log (also the control's runs' reference) or 6's, emptied; NULL where no memory is left */
static XdLog *log_of(int game) {
	XdLog **at = &logs[game == 6];
	if (!*at) *at = malloc(sizeof **at);
	if (*at) memset(*at, 0, sizeof **at);
	return *at;
}

/* ---- where an event was called from ---- */

/* an address of a family's code: its BN5 address (bit 0 clear); 0 none */
static uint32_t family_of(const XdLog *log, uint32_t a) {
	a &= ~1u;
	for (int i = 0; i < log->nfam; ++i) {
		uint32_t base = log->game == 6 ? log->fam[i].to : log->fam[i].from;
		if (a >= base && a < base + log->fam[i].len) return log->fam[i].from + (a - base);
	}
	return 0;
}

/* a word that is a return address: after a BL (bit 0 set), or after
 * mov lr,pc; bx rN (bit 0 clear) */
static bool return_address(uint32_t w) {
	if (w < 0x08000000u || w >= ROM_END) return false;
	if (w & 1) {
		uint32_t at = (w & ~1u) - 4;
		return (hook_read16(at) & 0xF800) == 0xF000 && (hook_read16(at + 2) & 0xF800) == 0xF800;
	}
	return (hook_read16(w - 2) & 0xFF87) == 0x4700 && hook_read16(w - 4) == 0x46FE;
}

static void callers(const XdLog *log, XdEvent *e, uint32_t ret, uint32_t sp) {
	e->caller = e->outer = ret;
	e->family = family_of(log, ret) ? ret : 0;
	for (uint32_t k = 0; k < CHAIN_WORDS && sp + 4 * k < IWRAM_END; ++k) {
		uint32_t w = hook_read32(sp + 4 * k);
		if (w == log->g->runner_ret) break;
		if (!return_address(w)) continue;
		e->outer = w;
		if (!e->family && family_of(log, w)) e->family = w;
	}
}

/* MegaMan, the first kind's first object, runs no family's or Navi's own code: an event while he runs that an engine
 * routine a Navi's copy holds led to (BN5's engine runs it for him too: the buster's arm, 0x080EBBA6) is not the family's */
static void megamans(const XdLog *log, XdEvent *e, uint32_t running_now) {
	if (running_now == log->g->t1) e->family = 0;
}

/* the object the runner runs now (0 none) */
static uint32_t running(const XdLog *log) {
	uint32_t node = hook_read32(log->g->running);
	return node ? node + BN6_OBJECT_NODE : 0;
}

static XdEvent *add(XdLog *log, int kind, int sub) {
	bool setup = !log->on && log->setup && (kind == XD_SPAWN || kind == XD_SPRITE);
	if (!log->on && !setup) return NULL;
	if (log->nev >= EVENTS_MAX) { ++log->dropped; return NULL; }
	XdEvent *e = &log->ev[log->nev++];
	*e = (XdEvent){ .kind = (uint8_t)kind, .sub = (uint8_t)sub, .flags = setup ? XD_SETUP : 0, .frame = log->frame, .t = log->t };
	return e;
}

/* ---- draws: kept (the reference's) or replayed (the subject's) ---- */

/* a draw's key: the family's code that led to it, else its caller, as BN5 has them (a BN6 routine's through its twin, and
 * the word BN5's draws there take into *which); 0 none */
static uint32_t key_of(const XdLog *log, const XdEvent *e, uint32_t *which) {
	if (e->family) return family_of(log, e->family);
	if (log->game != 6 || log->alone) return e->caller & ~1u;
	for (int i = 0; i < log->ntwins; ++i) {
		const XdTwin *w = &log->twins[i];
		if ((e->caller & ~1u) < w->bn6 || (e->caller & ~1u) >= w->bn6 + w->len) continue;
		if (w->word >= 0) *which = (uint32_t)w->word;
		return w->bn5 + ((e->caller & ~1u) - w->bn6);
	}
	return 0;
}

static void keep(XdLog *log, XdEvent *e, uint32_t which) {
	if (log->ndraws >= DRAWS_MAX) return;
	log->draws[log->ndraws++] = (Draw){ log->seeded ? log->t : 0, key_of(log, e, &which), e->a, which, !log->seeded };
	e->flags |= XD_KEPT;
	e->ref = (uint32_t)log->ndraws;
}

/* how many draws of this key the frame's replay has met before this one */
static int nth(XdLog *log, uint32_t key, uint32_t which) {
	const XdLog *ref = log->ref;
	if (log->keyed_t != log->t) {
		log->keyed_t = log->t;
		log->nkeys = 0;
		while (log->cursor < ref->ndraws && ref->draws[log->cursor].t < log->t) ++log->cursor;
		while (log->lag_cursor < ref->ndraws && ref->draws[log->lag_cursor].t + (uint32_t)log->lag < log->t) ++log->lag_cursor;
	}
	for (int i = 0; i < log->nkeys; ++i)
		if (log->keys[i].key == key && log->keys[i].which == which) return log->keys[i].n++;
	if (log->nkeys < KEYS_MAX) log->keys[log->nkeys++] = (Keyed){ key, which, 1 };
	return 0;
}

/* The reference's draw of this key in this frame, the k-th: its word
 * written into the RNG word the draw reads, before it reads it; or none,
 * drawn as it is and said so */
static void replay_setup(XdLog *log, XdEvent *e, uint32_t own);

static void replay(XdLog *log, XdEvent *e, uint32_t own) {
	uint32_t which = own, key = key_of(log, e, &which);
	if (!key) {
		e->flags |= XD_UNMAPPED;
		++log->unmapped;
		return;
	}
	const XdLog *ref = log->ref;
	/* (a draw of the family's that the lag puts before the reference's first seeded frame, t - lag below 0: one of its opening's,
	 * its setup draws, xd_replay_setup's way. BN4's Navi runner runs his states through the paused opening, BN6's from the
	 * fight on, its frame of the fight before its timer's first tick seeded: JunkMan's state 1 draws there) */
	if (log->setup_draws && e->family && log->t < (uint32_t)log->lag) {
		replay_setup(log, e, own);
		return;
	}
	int k = nth(log, key, which);
	/* (with a lag, every keyed draw, the family's own and its twins': the reference's of that many fight frames before,
	 * xd_replay_lag; the engine's per-frame steps have no key) */
	bool lagged = log->lag != 0;
	uint32_t at = lagged ? log->t - (uint32_t)log->lag : log->t;
	for (int i = lagged ? log->lag_cursor : log->cursor; i < ref->ndraws && ref->draws[i].t == at; ++i) {
		const Draw *d = &ref->draws[i];
		if (d->setup || d->key != key || d->which != which || k-- > 0) continue;
		hook_write32(log->g->rng[own], d->word);
		e->b = d->word;
		e->flags |= XD_REPLAYED;
		e->ref = (uint32_t)i + 1;
		++log->replayed;
		return;
	}
	e->flags |= XD_UNMATCHED;
	++log->unmatched;
}

/* (xd_replay_setup) A draw of the family's before the fight is seeded, the subject's: the reference's k-th setup draw of its
 * key, whatever the frame (each game's opening is its own length, and lag is of the fight's frames: no setup draw is one of
 * them); or none, drawn as it is and said so */
static void replay_setup(XdLog *log, XdEvent *e, uint32_t own) {
	uint32_t which = own, key = key_of(log, e, &which);
	int k = -1;
	for (int i = 0; i < log->nsetup_keys && k < 0; ++i)
		if (log->setup_keys[i].key == key && log->setup_keys[i].which == which) k = log->setup_keys[i].n++;
	if (k < 0 && log->nsetup_keys < KEYS_MAX) log->setup_keys[log->nsetup_keys++] = (Keyed){ key, which, 1 };
	/* (the opening's draws of a key paired from its end: the reference's first ones of it skipped, xd_replay_setup_skip) */
	int skip = 0;
	for (int i = 0; i < log->nsetup_skips; ++i)
		if (log->setup_skips[i].key == key) skip = log->setup_skips[i].n;
	for (int i = 0, n = (k < 0 ? 0 : k) + skip; i < log->ref->ndraws && log->ref->draws[i].setup; ++i) {
		const Draw *d = &log->ref->draws[i];
		if (d->key != key || d->which != which || n-- > 0) continue;
		hook_write32(log->g->rng[own], d->word);
		e->b = d->word;
		e->flags |= XD_REPLAYED;
		e->ref = (uint32_t)i + 1;
		++log->replayed;
		return;
	}
	e->flags |= XD_UNMATCHED;
	++log->unmatched;
}

static HookAct on_draw(HookRegs *r, void *user) {
	XdLog *log = user;
	int sub = 0;
	while (sub < 3 && log->g->draw[sub] != r->pc) ++sub;
	/* (before the fight is seeded, a draw of the family's kept or replayed where the spec's logs take them, xd_replay_setup:
	 * one of its battle's set-up, BATTLE_INIT's, has no event of its own) */
	bool setup = !log->seeded && log->setup_draws;
	XdEvent own = { .kind = XD_DRAW, .sub = (uint8_t)sub }, *e = add(log, XD_DRAW, sub);
	if (!e && !setup) return HOOK_CONTINUE;
	if (!e) e = &own;
	uint32_t which = (uint32_t)sub >> 1;
	callers(log, e, r->lr, r->sp);
	e->object = running(log);
	e->a = e->b = hook_read32(log->g->rng[which]);
	if (log->seeded && log->ref) replay(log, e, which);
	else if (log->seeded) keep(log, e, which);
	else if (setup && e->family && log->ref) replay_setup(log, e, which);
	else if (setup && e->family) keep(log, e, which);
	return HOOK_CONTINUE;
}

/* ---- spawns, sounds, effects, setAttack, a reservation, the blink ---- */

static HookAct on_spawn(HookRegs *r, void *user) {
	XdLog *log = user;
	if (!log->on && !log->setup) return HOOK_CONTINUE;
	XdEvent *e = &log->spawning;
	*e = (XdEvent){ .kind = XD_SPAWN, .sub = (uint8_t)(r->r[0] & 0xF), .frame = log->frame, .t = log->t };
	uint32_t lr = hook_read32(r->r[1] + SPAWNER_LR), above = r->r[1] + SPAWNER_LR + 4;
	e->a = hook_read8(r->r[1]);
	/* (called by the second-level dispatch's spawner for a moved family's code: its caller that code, its type its game's where
	 * the dispatch runs it, as BN5's spawn of it has them) */
	if (log->game == 6 && (lr & ~1u) >= XV_STUBS && (lr & ~1u) < XV_STUBS_END) {
		if ((int)e->a == xvirus_dispatch_type(e->sub)) e->a = hook_read8(r->r[1] + DISPATCH_FRAME);
		lr = hook_read32(r->r[1] + DISPATCH_FRAME + 4);
		above = r->r[1] + DISPATCH_FRAME + 8;
	}
	callers(log, e, lr, above);
	e->b = running(log);
	megamans(log, e, e->b);
	log->pending = true;
	return HOOK_CONTINUE;
}

static HookAct on_spawned(HookRegs *r, void *user) {
	XdLog *log = user;
	if (!log->pending) return HOOK_CONTINUE;
	log->pending = false;
	XdEvent *e = add(log, XD_SPAWN, 0);
	if (e) {
		uint16_t flags = e->flags;
		*e = log->spawning;
		e->flags = flags;
		e->object = r->r[5];
	}
	return HOOK_CONTINUE;
}

/* a call's event: its caller lr, or (where the routine has called on since
 * it began) the word its push left at sp */
static XdEvent *call(XdLog *log, const HookRegs *r, int kind, int sub, bool pushed) {
	XdEvent *e = add(log, kind, sub);
	if (e) {
		if (pushed) callers(log, e, hook_read32(r->sp), r->sp + 4);
		else callers(log, e, r->lr, r->sp);
		e->object = running(log);
		megamans(log, e, e->object);
	}
	return e;
}

static HookAct on_sound(HookRegs *r, void *user) {
	XdLog *log = user;
	XdEvent *e = call(log, r, XD_SOUND, 0, false);
	/* (a sound of BN5's BN6 plays from a copy, xsong.h: its own number; in a Navi's spec, BN6's own song of a number BN5
	 * plays as another, its number with bit 16, never taken for BN5's: a Navi's code plays each of its sounds from its copy.
	 * In a family's, BN6's own song of the number is the engine's, a twin of BN5's routine playing each game's own by its
	 * number: Bugtank's bomb's and MetFire's fire column's 0x70, legit.json's sound) */
	int copy = log->game == 6 ? xd_song_copy((int)r->r[0]) : -1;
	bool other = log->game == 6 && log->navis && copy < 0 && xsong_sound_slot(XROM_BN5_COLONEL_US, (int)r->r[0]) >= 0;
	if (e) e->a = copy >= 0 ? (uint32_t)copy : r->r[0] | (other ? 0x10000u : 0u);
	return HOOK_CONTINUE;
}

static HookAct on_effect(HookRegs *r, void *user) {
	XdEvent *e = call(user, r, XD_EFFECT, 0, false);
	if (e) e->a = r->r[4] & 0xFF;
	return HOOK_CONTINUE;
}

static HookAct on_attack(HookRegs *r, void *user) {
	XdEvent *e = call(user, r, XD_ATTACK, (int)r->r[2], false);
	if (e) {
		e->a = r->r[0];
		e->b = e->object;
		e->object = r->r[5];
	}
	return HOOK_CONTINUE;
}

static HookAct on_reserved(HookRegs *r, void *user) {
	XdEvent *e = call(user, r, XD_RESERVE, 0, true);
	if (e) e->object = r->r[5];
	return HOOK_CONTINUE;
}

/* (its sprite by its bytes, a: the two games number theirs otherwise, xdiff_sprite.h; b its list's offset and number) */
static HookAct on_sprite(HookRegs *r, void *user) {
	const XdLog *log = user;
	XdEvent *e = call(user, r, XD_SPRITE, 0, false);
	if (e) {
		e->object = r->r[5];
		e->a = xd_sprite_crc(log->game, r->r[1], r->r[2]);
		e->b = (r->r[1] & 0xFFu) << 8 | (r->r[2] & 0xFFu);
	}
	return HOOK_CONTINUE;
}

static HookAct on_blink(HookRegs *r, void *user) {
	XdLog *log = user;
	XdEvent *e = call(log, r, XD_BLINK, 0, true);
	if (e) {
		e->object = r->r[5];
		e->a = hook_read16(hook_read32(r->r[5] + BN6_T1_COLLISION) + log->g->flash);
		e->b = (e->a & 2) == log->g->hides;
	}
	return HOOK_CONTINUE;
}

/* (r0 the side, r1 the gain: what the mood rises by, as the routine will make it) */
static HookAct on_mood(HookRegs *r, void *user) {
	XdLog *log = user;
	XdEvent *e = call(log, r, XD_MOOD, 0, false);
	if (!e) return HOOK_CONTINUE;
	uint32_t mood = hook_read8(log->g->navi + (r->r[0] & 1) * log->g->navi_size + BN6_BATTLE_MOOD - BN6_BATTLE_NAVI), now = mood + r->r[1];
	e->a = r->r[0] & 1;
	e->b = mood == 0 || mood == 0xFF ? 0 : (now > 0xFE ? 0xFE : now) - mood;
	return HOOK_CONTINUE;
}

/* (BN5_MOOD_SET: r0 the side; BN5_MOOD_CALM: r5 him) the mood written outright, as BN6's twins write its own: what the
 * gains had raised it by is gone (tools/xdiff_events.py's reading of BN5's mood in BN6's terms) */
static void mood_set(XdLog *log, const HookRegs *r, uint32_t side) {
	XdEvent *e = call(log, r, XD_MOOD, 0, false);
	if (!e) return;
	e->a = (side & 1) | XD_MOOD_SET;
	e->b = 0;
}

static HookAct on_mood_set(HookRegs *r, void *user) {
	mood_set(user, r, r->r[0]);
	return HOOK_CONTINUE;
}

static HookAct on_mood_calm(HookRegs *r, void *user) {
	mood_set(user, r, hook_read8(r->r[5] + BN6_T1_ALLIANCE));
	return HOOK_CONTINUE;
}

/* ---- the logs ---- */

/* The chips' units a spec's folder plays (src/core/xchips_moved.c: its chips' families, by number): their code alone the
 * family's. Another unit's copy of BN5's code, which BN5's own code calls too (BoyBomb's explosion spawner, which a
 * Handi's bomb calls in BN5), is BN5's own there and BN6's twin's in BN6, so it is no family's in either. */
static bool unit_used[XC_UNITS];

/* the units chip `id`'s moved code runs */
static void units_of(int id) {
	const XChipMoved *m = xchips_moved_of(id);
	for (int u = 0; m && u < xchips_bn5_count && u < XC_UNITS; ++u) {
		const XChipUnit *x = &xchips_bn5[u];
		if ((x->kind == XCHIP_ATTACK && x->number == m->family) ||
		    (x->kind == XCHIP_SUPPORT && m->family == BN6_CHIP_SUPPORT_FAMILY && x->number == m->sub) ||
		    (x->kind == XCHIP_NAVI && m->family == BN6_CHIP_NAVI_FAMILY && x->number == m->sub))
			unit_used[u] = true;
	}
}

static bool in_folder(const uint16_t *folder, int n, int id) {
	for (int i = 0; i < n; ++i)
		if ((int)chip_entry_id(folder[i]) == id) return true;   /* (a spec's folder, BN6's: this build's form) */
	return false;
}

void xd_log_units(const uint16_t *folder, int n) {
	memset(unit_used, 0, sizeof unit_used);
	for (int i = 0; i < n; ++i) units_of((int)chip_entry_id(folder[i]));
	/* (and a Program Advance the folder's chips form, BN6's list grown with BN5's recipes: src/core/xchips_pas.c) */
	for (int k = 0; k < xchips_pas_count; ++k) {
		bool all = true;
		for (int j = 0; j < (xchips_pas[k].kind == 4 ? xchips_pas[k].n : 1) && all; ++j) all = in_folder(folder, n, xchips_pas[k].parts[j]);
		if (all) units_of(xchips_pas[k].pa);
	}
}

static bool moved_for(const XVirusFamily *f, const XVirusFamily *of);

void xd_log_shared(const XVirusFamily *const *fams, int n) {
	for (int u = 0; u < xchips_bn5_count && u < XC_UNITS; ++u)
		for (int i = 0; i < n && !unit_used[u]; ++i) unit_used[u] = moved_for(&xchips_bn5[u].code, fams[i]);
}

void xd_log_routes(void) {
	for (int i = 0; i < soulcode_bn5.nroutes; ++i)
		for (int u = 0; u < xchips_bn5_count && u < XC_UNITS; ++u)
			for (int k = 0; !unit_used[u] && k < xchips_bn5[u].code.nblocks; ++k) {
				const XvBlock *b = &xchips_bn5[u].code.blocks[k];
				unit_used[u] = soulcode_bn5.routes[i].bn6 >= b->at && soulcode_bn5.routes[i].bn6 < b->at + (b->end - b->start);
			}
}

/* whether BN6's copy at `to` is a used unit's */
static bool chip_used(uint32_t to);

bool xd_moved_kept(uint32_t to) {
	return !(to >= FREE_CHIP_MOVED && to < XC_END) || (to >= XC_HELPERS && to < XC_HOMES) || chip_used(to);
}

static bool chip_used(uint32_t to) {
	for (int u = 0; u < xchips_bn5_count && u < XC_UNITS; ++u)
		for (int k = 0; unit_used[u] && k < xchips_bn5[u].code.nblocks; ++k) {
			const XvBlock *b = &xchips_bn5[u].code.blocks[k];
			if (to >= b->at && to < b->at + (b->end - b->start)) return true;
		}
	return false;
}

/* a family that moved a type `of` runs as shared (XV_TYPE_SHARED): its copy is the one `of` runs */
static bool moved_for(const XVirusFamily *f, const XVirusFamily *of) {
	for (int i = 0; i < of->ntypes; ++i)
		for (int j = 0; of->types[i].to == XV_TYPE_SHARED && j < f->ntypes; ++j)
			if (f->types[j].to == XV_TYPE_MOVED && f->types[j].kind == of->types[i].kind && f->types[j].from == of->types[i].from) return true;
	return false;
}

/* whether a moved range is a block or clone of the spec's families (or of one that moved a type they share); all where none
 * and no foe is a Navi (a spec of MegaMan's, of the fields'), none in a Navi's (the families' copies of BN5's engine code are
 * its engine's there) */
static bool spec_code(const XvMoved *m, const XVirusFamily *const *fams, int nfams, bool navis) {
	if (!nfams) return !navis;
	/* (the spec's own families first, BN4's among them, xvirus_bn4.h; then BN5's that moved a type one of them shares) */
	for (int k = -nfams; k < xvirus_bn5_count; ++k) {
		const XVirusFamily *f = k < 0 ? fams[k + nfams] : xvirus_bn5[k];
		bool mine = k < 0;
		for (int i = 0; i < nfams && !mine; ++i) mine = fams[i] == f || moved_for(f, fams[i]);
		for (int i = 0; mine && i < f->nblocks; ++i)
			if (f->blocks[i].start == m->from && f->blocks[i].at == m->to) return true;
		for (int i = 0; mine && i < f->nclones; ++i)
			if (f->clones[i].start == m->from && f->clones[i].at == m->to) return true;
	}
	return false;
}

/* the spec's foes' BN5 ids (xd_log_navis): a moved Navi's code is a foe's where its Navi is one of them */
static int spec_ids[XD_NAVI_IDS_MAX], nspec_ids;

void xd_log_navis(const int *ids5, int n) {
	nspec_ids = n < XD_NAVI_IDS_MAX ? n : XD_NAVI_IDS_MAX;
	memcpy(spec_ids, ids5, (size_t)nspec_ids * sizeof *spec_ids);
}

/* whether a moved range is a block or clone of a Navi among the spec's foes (another Navi's copy of BN5's engine code, the
 * Dark Soul's of its chip AI's region, is its engine's there, as the families' are in spec_code) */
static bool navi_code(const XvMoved *m) {
	for (int k = 0; k < xnavi_bn5_count; ++k) {
		const XNavi *n = xnavi_bn5[k];
		bool mine = false;
		for (int v = 0; v < n->versions && !mine; ++v)
			for (int i = 0; i < nspec_ids && !mine; ++i) mine = n->code.names[v] == spec_ids[i];
		for (int i = 0; mine && i < n->code.nblocks; ++i)
			if (n->code.blocks[i].start == m->from && n->code.blocks[i].at == m->to) return true;
		for (int i = 0; mine && i < n->code.nclones; ++i)
			if (n->code.clones[i].start == m->from && n->code.clones[i].at == m->to) return true;
	}
	return false;
}

static XdLog *log_make(int game, bool alone, const XVirusFamily *const *fams, int nfams, bool navis) {
	int n;
	const XvMoved *m = xvirus_moved(&n);   /* (every range moved, or no log: xvirus_moved.h) */
	XdLog *log = m && game_of(game) ? log_of(game) : NULL;
	if (!log) return NULL;
	log->game = game;
	log->alone = alone;
	log->navis = navis;
	log->g = game_of(game);
	/* (the spec's families' code, BN4's late families' in their own room (XV_LATE_CODE) and BN4's Navis' among them
	 * (xd4_families: their code in their own room, XN_BN4_CODE), the Navis' where a foe is one, xnavi_recipe.h, and the moved
	 * code of the chips the spec plays,
	 * xd_log_units: its draws keyed by BN5's own places as a family's are; not the chips' helpers', BN5's code every chip and
	 * MegaMan's own arm share, which BN6's own chips do not run: the muzzle flash) */
	for (int i = 0; i < n; ++i)
		if ((((m[i].to >= XV_CODE && m[i].to < XV_ART) || (m[i].to >= XV_LATE_CODE && m[i].to < XV_LATE_CODE_END) ||
		      (m[i].to >= XN_BN4_CODE && m[i].to < XN_BN4_ART)) && spec_code(&m[i], fams, nfams, navis)) ||
		    (navis && xn_code_at(m[i].to) && navi_code(&m[i])) ||
		    (m[i].to >= FREE_CHIP_MOVED && m[i].to < FREE_TAIL && !(m[i].to >= XC_HELPERS && m[i].to < XC_HOMES) && chip_used(m[i].to)))
			if (!xd_log_key(log, m[i].from, m[i].to, m[i].len)) return NULL;
	/* (the control's code where BN6 has it: its own Mettaur's, and the control Navi's HeatMan's, which the moved copy's
	 * places are keyed as) */
	for (int i = 0; alone && i < xvirus_control.nblocks + xnavi_control.code.nblocks; ++i) {
		const XvBlock *b = i < xvirus_control.nblocks ? &xvirus_control.blocks[i] : &xnavi_control.code.blocks[i - xvirus_control.nblocks];
		if (!xd_log_key(log, b->start, b->start, b->end - b->start)) return NULL;
	}
	return log;
}

bool xd_log_key(XdLog *log, uint32_t from, uint32_t to, uint32_t len) {
	if (!log) return true;
	if (log->nfam == FAMILY_MAX) {
		fprintf(stderr, "xdiff: the code a log watches past %d blocks (%08x): the log is refused\n", FAMILY_MAX, (unsigned)from);
		return false;
	}
	log->fam[log->nfam++] = (XvMoved){ from, to, len };
	return true;
}


XdLog *xd_log_start(int game, const XVirusFamily *const *fams, int n, bool navis) {
	XdLog *log = log_make(game, false, fams, n, navis);
	if (!log) return NULL;
	bool (*put)(uint32_t, EmuHook, void *) = game == 6 ? emu_hook : guest_hook;
	const Game *g = log->g;
	bool ok = true;
	for (int i = 0; i < 4; ++i) ok = put(g->draw[i], on_draw, log) && ok;
	ok = put(g->spawn, on_spawn, log) && put(g->spawn_end, on_spawned, log) && ok;
	ok = put(g->sound, on_sound, log) && put(g->effect, on_effect, log) && (!g->attack || put(g->attack, on_attack, log)) && ok;
	ok = put(g->reserved, on_reserved, log) && put(g->blink, on_blink, log) && put(g->sprite, on_sprite, log) && ok;
	if (g->mood_gain) ok = put(g->mood_gain, on_mood, log) && ok;
	if (g->mood_set) ok = put(g->mood_set, on_mood_set, log) && put(g->mood_calm, on_mood_calm, log) && ok;
	if (!ok) fprintf(stderr, "xdiff: a hook could not be placed on BN%d's core\n", game);
	return ok ? log : NULL;
}

void xd_replay(XdLog *subject, const XdLog *reference, const XdTwin *twins, int ntwins) {
	subject->ref = reference;
	subject->twins = twins;
	subject->ntwins = ntwins;
	subject->keyed_t = 0xFFFFFFFFu;
}

void xd_replay_lag(XdLog *subject, int lag) { subject->lag = lag; }

void xd_replay_setup(XdLog *subject, XdLog *reference) {
	subject->setup_draws = reference->setup_draws = true;
	subject->nsetup_keys = subject->nsetup_skips = 0;
}

bool xd_replay_setup_skip(XdLog *subject, uint32_t key, int n) {
	if (!subject || subject->nsetup_skips >= XD_SETUP_SKIPS) return false;
	subject->setup_skips[subject->nsetup_skips++] = (Keyed){ key & ~1u, 0, n };
	return true;
}

void xd_setup(XdLog *log, bool setup) { log->setup = setup; }

void xd_frame(XdLog *log, uint32_t frame, uint32_t t, bool on, bool seeded) {
	log->frame = frame;
	log->t = t;
	log->on = on;
	log->seeded = seeded;
}

int xd_take(XdLog *log, XdEvent *out, int max) {
	int n = log->nev < max ? log->nev : max;
	memcpy(out, log->ev, (size_t)n * sizeof *out);
	log->dropped += log->nev - n;
	log->nev = 0;
	return n;
}

void xd_counts(const XdLog *log, int out[XD_COUNTS]) {
	out[0] = log->replayed;
	out[1] = log->unmatched;
	out[2] = log->unmapped;
	out[3] = log->dropped;
}

/* ---- the control's runs: BN6's draws kept, or replayed from a run that kept them ---- */

static bool control_start(const char *how, XdLog **log, FILE **out) {
	*log = log_make(6, true, NULL, 0, true);
	if (!*log) return false;
	const Game *g = (*log)->g;
	bool ok = true;
	for (int i = 0; i < 4; ++i) ok = emu_hook(g->draw[i], on_draw, *log) && ok;
	if (!ok) return false;
	if (!strcmp(how, "keep")) return (*out = emu_debug_file("xdraws.bin")) != NULL;
	/* (the reference: the draws another run kept, four words each) */
	XdLog *ref = log_of(5);
	FILE *in = ref ? fopen(how, "rb") : NULL;
	uint8_t w[16];
	while (in && ref->ndraws < DRAWS_MAX && fread(w, 1, sizeof w, in) == sizeof w)
		ref->draws[ref->ndraws++] = (Draw){ get32(w), get32(w + 4), get32(w + 8), get32(w + 12) };
	if (in) fclose(in);
	xd_replay(*log, ref, NULL, 0);
	return in != NULL;
}

void xd_control_frame(const char *how) {
	static XdLog *log;
	static FILE *out;
	static int written;
	static uint32_t frame;
	static bool done;
	uint8_t sub = emu_read8(BN6_GAMESTATE);
	bool battle = (sub == BN6_SUB_BATTLE_INIT || sub == BN6_SUB_BATTLE) && emu_read8(BN6_BATTLE_PHASE) != BN6_PHASE_RESULTS;
	if (!log && battle && !done && !control_start(how, &log, &out)) {
		fprintf(stderr, "xdraws: %s could not be begun\n", how);
		done = true;
	}
	if (!log || done) return;
	for (; out && written < log->ndraws; ++written) {
		const Draw *d = &log->draws[written];
		uint8_t w[16];
		put32(w, d->t);
		put32(w + 4, d->key);
		put32(w + 8, d->word);
		put32(w + 12, d->which);
		fwrite(w, 1, sizeof w, out);
	}
	if (out) fflush(out);
	log->nev = 0;   /* (the events themselves are the differential's) */
	if (battle) {
		xd_frame(log, frame++, emu_read32(BN6_BATTLE_TIMER), true, true);
		return;
	}
	xd_frame(log, frame, 0, false, false);
	if (out) printf("xdraws: %d kept\n", log->ndraws);
	else printf("xdraws: %d replayed, %d unmatched, %d unmapped\n", log->replayed, log->unmatched, log->unmapped);
	if (out) fclose(out);
	out = NULL;
	done = true;
}

int xd_song_copy(int slot) {
	int xrom = -1, song = xsong_copy_of(slot, &xrom);
	return xrom == XROM_BN5_COLONEL_US ? song : -1;
}
