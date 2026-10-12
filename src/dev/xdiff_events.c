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
#include "hook.h"
#include "rom.h"
#include "xvirus.h"
#include "xvirus_dispatch.h"
#include "xdiff_sprite.h"
#include "xnavi_bn5.h"
#include "xsong.h"
#include "xvirus_families.h"

#define EVENTS_MAX  8192    /* a log's events between two takes (a battle's opening, a frame of the fight) */
#define DRAWS_MAX   32768   /* the reference's draws kept for the replay: a battle's */
#define KEYS_MAX    64      /* the keys a frame's draws take */
#define CHAIN_WORDS 96      /* the stack words looked through for callers further out */
#define FAMILY_MAX  128     /* the families' and the Navis' blocks of code */
#define SPAWNER_LR  0x18    /* a spawner's frame (push {r7,lr}; sub sp,#0x14) from its five words: its lr, its caller */
#define DISPATCH_FRAME 0x1C /* ... and above it, where the second-level dispatch's spawner called it, that one's (push {r0,lr}): its
                               game's type, its own caller (xvirus_dispatch.c) */
#define ROM_END     0x0A000000u
#define IWRAM_END   0x03008000u   /* (the stack's top lies below it: a scan stops there, short of IWRAM's mirrors) */

/* Each game's routines (bn5.h, bn6.h) */
typedef struct {
	uint32_t draw[4], spawn, spawn_end, sound, effect, attack, reserved, blink, sprite;
	uint32_t runner_ret, running, rng[2];
	uint32_t flash, hides;   /* the flash timer's place in a collision data; its bit 1 as the blink hides an object */
	uint32_t mood_gain, navi, navi_size;   /* the mood raised as a hit lands (0 none: BN6's) and the battle NaviStats it raises */
} Game;

static const Game games[2] = {
	{ { BN5_RNG_DRAW, BN5_RNG_DRAW_POSITIVE, BN5_RNG_DRAW_SECOND, BN5_RNG_DRAW_SECOND_POSITIVE }, BN5_SPAWN_COMMON, BN5_SPAWN_COMMON_END,
		BN5_PLAY_SOUND, BN5_SPAWN_EFFECT, BN5_SET_ATTACK, BN5_RESERVE_PANEL_DONE, BN5_BLINK_FLASHING, BN5_SPRITE_LOAD, BN5_OBJECT_RUNNER_RET,
		BN5_OBJECT_RUNNING,
		{ BN5_RNG_PRIMARY, BN5_RNG_SECONDARY }, BN5_COLL_FLASH, 0, BN5_MOOD_GAIN, BN5_BATTLE_NAVI, BN5_BATTLE_NAVI_SIZE },
	{ { BN6_RNG_DRAW, BN6_RNG_DRAW_POSITIVE, BN6_RNG_DRAW_SECOND, BN6_RNG_DRAW_SECOND_POSITIVE }, BN6_SPAWN_COMMON, BN6_SPAWN_COMMON_END,
		BN6_PLAY_SOUND, BN6_SPAWN_EFFECT, BN6_SET_ATTACK, BN6_RESERVE_PANEL_DONE, BN6_BLINK_FLASHING, BN6_SPRITE_LOAD, BN6_OBJECT_RUNNER_RET,
		BN6_OBJECT_RUNNING,
		{ BN6_RNG_PRIMARY, BN6_RNG_SECONDARY }, BN6_COLL_FLASH, 2, 0, BN6_BATTLE_NAVI, BN6_BATTLE_NAVI_SIZE },
};

/* A draw kept: the battle's timer, its key (a BN5 address), the word it was drawn from, which word */
typedef struct { uint32_t t, key, word, which; } Draw;
/* a key's draws in the frame replayed so far */
typedef struct { uint32_t key, which; int n; } Keyed;

struct XdLog {
	const Game *g;
	int game;
	bool alone;   /* BN6's against BN6's (xd_control_frame): every place keyed as it is, the control's own code as its copy */
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
	uint32_t keyed_t;
	Keyed keys[KEYS_MAX];
	int nkeys, replayed, unmatched, unmapped;
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
	if (running_now == (log->game == 6 ? BN6_T1_OBJECTS : BN5_T1_OBJECTS)) e->family = 0;
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
	if (log->game == 5 || log->alone) return e->caller & ~1u;
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
	log->draws[log->ndraws++] = (Draw){ log->t, key_of(log, e, &which), e->a, which };
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
	}
	for (int i = 0; i < log->nkeys; ++i)
		if (log->keys[i].key == key && log->keys[i].which == which) return log->keys[i].n++;
	if (log->nkeys < KEYS_MAX) log->keys[log->nkeys++] = (Keyed){ key, which, 1 };
	return 0;
}

/* The reference's draw of this key in this frame, the k-th: its word
 * written into the RNG word the draw reads, before it reads it; or none,
 * drawn as it is and said so */
static void replay(XdLog *log, XdEvent *e, uint32_t own) {
	uint32_t which = own, key = key_of(log, e, &which);
	if (!key) {
		e->flags |= XD_UNMAPPED;
		++log->unmapped;
		return;
	}
	const XdLog *ref = log->ref;
	int k = nth(log, key, which);
	for (int i = log->cursor; i < ref->ndraws && ref->draws[i].t == log->t; ++i) {
		const Draw *d = &ref->draws[i];
		if (d->key != key || d->which != which || k-- > 0) continue;
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
	XdEvent *e = add(log, XD_DRAW, sub);
	if (!e) return HOOK_CONTINUE;
	uint32_t which = (uint32_t)sub >> 1;
	callers(log, e, r->lr, r->sp);
	e->object = running(log);
	e->a = e->b = hook_read32(log->g->rng[which]);
	if (log->seeded && log->ref) replay(log, e, which);
	else if (log->seeded) keep(log, e, which);
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
	/* (a sound of BN5's BN6 plays from a copy, xsong.h: its own number; BN6's own song of a number BN5 plays as another,
	 * its number with bit 16, never taken for BN5's) */
	int copy = log->game == 6 ? xsong_copy_of((int)r->r[0], NULL) : -1;
	bool other = log->game == 6 && copy < 0 && xsong_sound_slot(XROM_BN5_COLONEL_US, (int)r->r[0]) >= 0;
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

/* ---- the logs ---- */

static XdLog *log_make(int game, bool alone, bool navis) {
	XdLog *log = log_of(game);
	if (!log) return NULL;
	log->game = game;
	log->alone = alone;
	log->g = &games[game == 6];
	static XvMoved m[XV_MOVED_MAX];
	int n = xvirus_moved(m, XV_MOVED_MAX);
	/* (the families' code, and the Navis' where a foe is one: xnavi_recipe.h) */
	for (int i = 0; i < n; ++i)
		if ((m[i].to >= XV_CODE && m[i].to < XV_ART) || (navis && m[i].to >= XN_CODE && m[i].to < XN_ART)) {
			if (log->nfam < FAMILY_MAX) log->fam[log->nfam++] = m[i];
			else fprintf(stderr, "xdiff: the families' and Navis' code past %d blocks: %08x not watched\n", FAMILY_MAX, (unsigned)m[i].from);
		}
	/* (the control's code where BN6 has it: its own Mettaur's, and the control Navi's HeatMan's, which the moved copy's
	 * places are keyed as) */
	for (int i = 0; alone && i < xvirus_control.nblocks + xnavi_control.code.nblocks && log->nfam < FAMILY_MAX; ++i) {
		const XvBlock *b = i < xvirus_control.nblocks ? &xvirus_control.blocks[i] : &xnavi_control.code.blocks[i - xvirus_control.nblocks];
		log->fam[log->nfam++] = (XvMoved){ b->start, b->start, b->end - b->start };
	}
	return log;
}

XdLog *xd_log_start(int game, bool navis) {
	XdLog *log = log_make(game, false, navis);
	if (!log) return NULL;
	bool (*put)(uint32_t, EmuHook, void *) = game == 6 ? emu_hook : guest_hook;
	const Game *g = log->g;
	bool ok = true;
	for (int i = 0; i < 4; ++i) ok = put(g->draw[i], on_draw, log) && ok;
	ok = put(g->spawn, on_spawn, log) && put(g->spawn_end, on_spawned, log) && ok;
	ok = put(g->sound, on_sound, log) && put(g->effect, on_effect, log) && put(g->attack, on_attack, log) && ok;
	ok = put(g->reserved, on_reserved, log) && put(g->blink, on_blink, log) && put(g->sprite, on_sprite, log) && ok;
	if (g->mood_gain) ok = put(g->mood_gain, on_mood, log) && ok;
	if (!ok) fprintf(stderr, "xdiff: a hook could not be placed on BN%d's core\n", game);
	return ok ? log : NULL;
}

void xd_replay(XdLog *subject, const XdLog *reference, const XdTwin *twins, int ntwins) {
	subject->ref = reference;
	subject->twins = twins;
	subject->ntwins = ntwins;
	subject->keyed_t = 0xFFFFFFFFu;
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
	*log = log_make(6, true, true);
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
