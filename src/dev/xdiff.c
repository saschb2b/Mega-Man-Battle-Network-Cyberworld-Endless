/* xdiff.h: the differential's two battles, each traced. A trace is
 * "XDF1", its version (5), the game (5 or 6), its collision data's and
 * panel's sizes, the ranges the install moved (xvirus.h's XvMoved: from,
 * to, len; BN6's alone), the player block's size, and then a record per
 * fight frame (each new value of the battle's timer while it fights):
 * "FRAM", the core's frame, the timer, the sub-mode, the phase, how many
 * battle objects of the first, third and fourth kinds are in play, sounds
 * begun and objects in the run order, both RNG words, how many events its
 * core's hooks logged since the record before (xdiff_events.h; the first
 * record's from the battle's start); MegaMan's side's player block (his
 * battle NaviStats, BN6_BATTLE_NAVI_SIZE bytes, BN5's padded; the word of
 * what was used this battle, BN5_SOULS_USED or BN6_CROSSES_USED; his side's
 * pending form, BN5_SOUL_PENDING's or BN6_CROSS_PENDING's 16 bytes; the
 * Souls' state in BN6's engine, BN6_SOULS' SOULS_SIZE bytes, BN5's zero;
 * the Custom screen's element 0xB, UNITE, BN6_CUSTOM_UNITE's or
 * BN5_CUSTOM_UNITE's 12 bytes, as its last screen left it); then each
 * object of the first kind (its slot, whether its AI's data and collision
 * data are read, its type as the second-level dispatch runs it where its
 * type is its kind's dispatch type (its game's number in the side table by
 * its slot, xvirus_dispatch.h; BN6's alone, else 0), its 0xD8 bytes from
 * its list node on (BN6_OBJECT_NODE before it), its AI's 0x100 and its
 * collision data's BN6_COLL_SIZE, BN5's padded), each of the third (its
 * slot, its type as the dispatch runs it, its bytes, its collision data),
 * each of the fourth (its slot, its type so, its 0xC8 bytes from its node),
 * the 40 panels (BN5_PANEL_SIZE each,
 * BN6's padded), the run order (each object's address), the sounds
 * begun (a music player and the song's number), and the events
 * (XD_EVENT_WORDS words each: kind, sub and flags in the first). All u32
 * little-endian but the counts and slots. Local only: both ROMs, and what
 * it writes is their RAM (kept in .build, never in git). */
#include "xdiff.h"


#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "boot.h"
#include "bytes.h"
#include "emu.h"
#include "encounter.h"
#include "flags.h"
#include "foes.h"
#include "guest.h"
#include "rom.h"
#include "soulunite.h"
#include "thumb_reloc.h"
#include "xchips_install.h"
#include "xdiff_bn4.h"
#include "xdiff_spec.h"   /* (and xdiff_events.h, its twins'; xdiff_ncp.h, its ncp items') */
#include "xfields.h"
#include "xinstall.h"
#include "xvirus_bn5.h"
#include "xvirus_dispatch.h"

#define PANELS    40      /* the field's panels, 8 a row, rows 0-4 */
#define SONGS     0x400   /* the song table's entries looked through for a header */
#define OPENING   3000    /* the frames a battle may take to its fight */
#define TRACE_VERSION 8   /* (the moved state, BN5's green flash, in the state kept apart; the chip AI's lists and the Navis' own state, version 7; the
                             state BN6's engine keeps apart, BN5's sea's submerge, version 6; the player block, the Souls' version 4, and the
                             dispatch's types, the dispatch's 3, in one) */
#define APART_SEA (RAM_DARK_LISTS - RAM_SEA)   /* what BN6's engine keeps apart of the objects': the sea's room (xsea.h: the submerged, their markers), */
#define APART_LISTS (RAM_NAVIS - RAM_DARK_LISTS)   /* ... then the chip AI's lists, both sides (BN6_DARK_LISTS; BN5's BN5_DARK_LISTS), */
#define APART_NAVIS (RAM_DARK_AI - RAM_NAVIS)   /* ... then the Navis' own state (Colonel's sides' bits: BN5's of BattleState +0x5C, the rest 0), */
#define APART_DARK (BN5_AI_DATA_SIZE - BN6_AI_DATA_SIZE)   /* ... then the Dark Soul's AI data past BN6's 0x100 (its chip AI's state; 0 none out) */
#define APART_MOVED (RAM_DISPATCH_SIDE - RAM_BATTLE)   /* ... then the moved state (BN5's green flash by side, XC_FLASH: BN5's AIData +0x3C of
                                                        each side's player) */
#define APART_SIZE (APART_SEA + APART_LISTS + APART_NAVIS + APART_DARK + APART_MOVED)
#define COLONEL_SIDES 0x30u   /* (BN5_BATTLE_BITS' bits a Colonel's: 0x10 side 0, 0x20 side 1) */
_Static_assert(APART_SEA <= 0x100u && APART_LISTS == 2u * BN5_DARK_LIST_SIZE && APART_NAVIS <= 0x100u && APART_DARK <= 0x100u,
	"the apart block's parts as bytes_out writes them");
#define METER_DARK 400   /* (dark=1) BN5's dark meter under its 470 (0x0800DDE4, 0x08012832: the battle opens dark) */
#define USED_SIZE 4   /* the word of what was used this battle (BN5_SOULS_USED, BN6_CROSSES_USED) */
#define SOULS_SIZE BN6_SOULS_SIZE   /* the Souls' state in BN6's engine (BN6_SOULS: both sides', the rest of its range) */
#define PLAYER_SIZE (USED_SIZE + BN6_BATTLE_NAVI_SIZE + BN6_CROSS_PENDING_SIZE + SOULS_SIZE + BN6_ELEMENT_SIZE)   /* the player block */

/* ---- a game's battle as the trace reads it ---- */

typedef struct {
	int game;   /* 5, 6; 4 BN4's, its parts written in BN6's units (xdiff_bn4_trace.h) */
	uint32_t gamestate, state, window, rng[2];
	uint32_t t1, t3, t4;
	int nt1;
	uint32_t panels, panel_size, list, list_end, coll_size, mplays;   /* (BN4's: BN6's sizes, its parts' in the trace) */
	uint32_t navi, navi_size, used, pending, side;   /* MegaMan's player block's places (BN6_BATTLE_NAVI's or BN5's; 0 none) */
	uint32_t souls, unite, deck;   /* ... BN6_SOULS (0: none, BN5's), the Custom screen's element 0xB, the battle's deck (0 none) */
	uint32_t paralysis;            /* a collision data's paralysis timer (BN6_COLL_PARALYSIS, BN5_COLL_PARALYSIS) */
	uint32_t coll_flags, coll_struck;   /* ... its flags and what a frame's hits left it (BN6_COLL_FLAGS, BN6_COLL_STRUCK: BN5's too) */
	const uint8_t *rom;
	uint32_t songs;   /* (its song table, a ROM offset) */
	uint8_t (*sub)(void);   /* its sub-mode as BN6's (BN4's, xd4_sub); NULL: rd8(gamestate) */
	uint8_t (*rd8)(uint32_t);
	uint32_t (*rd32)(uint32_t);
	void (*wr32)(uint32_t, uint32_t);
	void (*wr16)(uint32_t, uint16_t);
	void (*frame)(uint32_t);
} View;

static uint32_t rom32(const uint8_t *rom, uint32_t a) { return get32(rom + (a - 0x08000000u)); }
static uint8_t sub_of(const View *v) { return v->sub ? v->sub() : v->rd8(v->gamestate); }
static bool in_ewram(uint32_t a) { return a >= BN6_EWRAM && a < BN6_EWRAM_END; }

/* ---- a game's battle as it runs: its view, its trace, the sounds its music players began ---- */

typedef struct { uint32_t info, header, clock; } Player;
#define BEGUN_MAX 128   /* (a frame's songs begun kept: a support chip's time freeze begins many between two fight frames, ShotStar's 63; the record's count is a byte) */

typedef struct Side_ {
	View v;
	XdLog *log;   /* its core's hooks' events (xdiff_events.h) */
	FILE *out;
	Player players[BN6_MPLAY_COUNT];
	uint16_t begun[BEGUN_MAX][2];   /* (player, song) begun since the last frame traced */
	int nbegun, custom, screen, traced;   /* (custom: the frames since this Custom screen's window first read open, -1 none; screen: its number) */
	int behind;                     /* (BN6's, lag=N) the fight frames its battle runs behind the reference's: its keys by the timer
	                                 * taken that much later, that many more frames traced (docs/devtools/differential.md, The BN4 trace) */
	uint32_t frame, last;           /* the core's frames run; the timer after the last */
	uint32_t still;                 /* the core's frames run since its timer last read otherwise (a hold's: a Navi chip's, a turn's change) */
	uint32_t filmed;                /* (film=) the timer of its last picture, + 1; 0 none */
	bool seeded, fought, done, decked, held, paneled;
	const struct Side_ *ref;        /* (BN6's) the reference's side, BN5's, its fight begun first */
} Side;

static void w32(Side *g, uint32_t v) {
	uint8_t b[4];
	put32(b, v);
	fwrite(b, 1, 4, g->out);
}
static void w8s(Side *g, uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
	const uint8_t q[4] = { a, b, c, d };
	fwrite(q, 1, 4, g->out);
}

/* `len` bytes from `at` (0: none, zeros), padded to `room` */
static void bytes_out(Side *g, uint32_t at, uint32_t len, uint32_t room) {
	uint8_t buf[0x100] = { 0 };
	for (uint32_t i = 0; at && i < len; i += 4) put32(buf + i, g->v.rd32(at + i));
	fwrite(buf, 1, room, g->out);
}

/* (BN4's) a part's bytes from `at` (0: none, zeros) in BN6's layout (xd4_translate), padded to `room` */
static void part_out(Side *g, int part, uint32_t at, uint32_t room) {
	uint8_t raw[0x100] = { 0 }, buf[0x100] = { 0 };
	for (uint32_t i = 0, n = xd4_size(part); at && i < n; i += 4) put32(raw + i, g->v.rd32(at + i));
	if (at && part == XD4_PANEL) xd4_panel_blinks(at, raw);
	if (at) xd4_translate(part, raw, buf);
	fwrite(buf, 1, room, g->out);
}

/* a part of an object's: BN4's translated, BN5's and BN6's as they are */
static void obj_part(Side *g, int part, uint32_t at, uint32_t len, uint32_t room) {
	if (g->v.game == 4) part_out(g, part, at, room);
	else bytes_out(g, at, len, room);
}

/* the objects of a kind in play: their slots */
static int in_play(const View *v, uint32_t base, uint32_t size, int count, uint8_t *slots) {
	int n = 0;
	for (int k = 0; k < count; ++k)
		if (v->rd8(base + (uint32_t)k * size + BN6_T1_IN_PLAY) & 1) slots[n++] = (uint8_t)k;
	return n;
}

/* ---- sounds: a music player's song begun (its header new, or its clock back) ---- */

static void players_init(Side *g) {
	for (uint32_t i = 0; i < BN6_MPLAY_COUNT; ++i) {
		uint32_t info = rom32(g->v.rom, g->v.mplays + 12 * i);
		g->players[i] = (Player){ in_ewram(info) ? info : 0, 0, 0 };
	}
	g->nbegun = 0;
}

/* (a song by the player's ROM's table, else by the core's: another game's sound copied into one of BN6's empty slots, a moved
 * family's or Navi's, is the core's, not the ROM file's) */
static int song_of(const View *v, uint32_t header) {
	for (uint32_t n = 0; n < SONGS; ++n)
		if (get32(v->rom + v->songs + 8 * n) == header) return (int)n;
	/* (a copy of another game's song in one of BN6's slots, xsong.h: by the slot's own number there) */
	/* (... in BN6's live table, grown or not, its new slots among them: song_table.h) */
	int n = v->game == 6 ? xd_song_slot(header) : -1;
	if (n >= 0) {
		int copy = xd_song_copy(n);
		return copy >= 0 ? copy : n;
	}
	return -1;
}

/* after each frame: the songs it began, kept for the next frame traced */
static void sounds(Side *g) {
	for (int i = 0; i < BN6_MPLAY_COUNT; ++i) {
		Player *p = &g->players[i];
		if (!p->info) continue;
		uint32_t h = g->v.rd32(p->info), c = g->v.rd32(p->info + 0xC);
		if (h && (h != p->header || c < p->clock) && g->nbegun < BEGUN_MAX) {
			g->begun[g->nbegun][0] = (uint16_t)i;
			g->begun[g->nbegun++][1] = (uint16_t)song_of(&g->v, h);
		}
		p->header = h;
		p->clock = c;
	}
}

/* ---- the trace ---- */

static void header_out(Side *g) {
	fwrite("XDF1", 1, 4, g->out);
	w32(g, TRACE_VERSION);
	w32(g, (uint32_t)g->v.game);
	w32(g, g->v.coll_size);
	w32(g, g->v.panel_size);
	int n = 0, kept = 0;
	const XvMoved *m = g->v.game == 6 ? xvirus_moved(&n) : NULL;   /* (whole: subject() begins no battle without them) */
	for (int i = 0; i < n; ++i) kept += xd_moved_kept(m[i].to);
	w32(g, (uint32_t)kept);
	for (int i = 0; i < n; ++i)
		if (xd_moved_kept(m[i].to)) { w32(g, m[i].from); w32(g, m[i].to); w32(g, m[i].len); }
	w32(g, PLAYER_SIZE);
	w32(g, APART_SIZE);
}

/* MegaMan's side's player block: his battle NaviStats, what was used this battle, his pending form */
static void player_out(Side *g) {
	const View *v = &g->v;
	uint32_t side = v->rd8(v->side) & 1;
	bytes_out(g, v->navi + side * v->navi_size, v->navi_size, BN6_BATTLE_NAVI_SIZE);
	w32(g, v->used ? v->rd32(v->used) : 0);
	if (v->game == 4)   /* (BN4's pending form in its AI data: XD4_PENDING) */
		part_out(g, XD4_PENDING, xd4_places.ai + side * xd4_places.ai_size, BN6_CROSS_PENDING_SIZE);
	else bytes_out(g, v->pending ? v->pending + side * BN6_CROSS_PENDING_SIZE : 0, BN6_CROSS_PENDING_SIZE, BN6_CROSS_PENDING_SIZE);
	bytes_out(g, v->souls, SOULS_SIZE, SOULS_SIZE);
	bytes_out(g, v->unite, BN6_ELEMENT_SIZE, BN6_ELEMENT_SIZE);
}

/* the objects in the order they run, their addresses into `order`: how many */
static int run_order(const View *v, uint32_t *order, int max) {
	int n = 0;
	for (uint32_t node = v->rd32(v->list + 4); n < max && in_ewram(node) && node != v->list_end; node = v->rd32(node + 4))
		order[n++] = node + BN6_OBJECT_NODE;
	return n;
}

/* the type an object of BN6's runs as through the second-level dispatch:
 * its game's number in its kind's side table, where its type is the
 * kind's dispatch type; 0 else (and in BN5's battle, which has none) */
static uint8_t dispatched(const View *v, int kind, uint32_t o) {
	if (v->game != 6 || !xvirus_dispatch_runs(kind, v->rd8(o + BN6_T1_TYPE))) return 0;
	return v->rd8(xvirus_dispatch_side(kind) + v->rd8(o + BN6_T1_SLOT));
}

static void objects_out(Side *g, const uint8_t *s1, int n1, const uint8_t *s3, int n3, const uint8_t *s4, int n4) {
	const View *v = &g->v;
	for (int i = 0; i < n1; ++i) {
		uint32_t o = v->t1 + s1[i] * BN6_T1_SIZE, ai = v->rd32(o + BN6_T1_AI_DATA), coll = v->rd32(o + BN6_T1_COLLISION);
		w8s(g, s1[i], in_ewram(ai), in_ewram(coll), dispatched(v, 1, o));
		obj_part(g, XD4_OBJECT, o - BN6_OBJECT_NODE, BN6_T1_SIZE, BN6_T1_SIZE);
		obj_part(g, XD4_AI, in_ewram(ai) ? ai : 0, BN6_AI_DATA_SIZE, BN6_AI_DATA_SIZE);
		obj_part(g, XD4_COLL, in_ewram(coll) ? coll : 0, v->coll_size, BN6_COLL_SIZE);
	}
	for (int i = 0; i < n3; ++i) {
		uint32_t o = v->t3 + s3[i] * BN6_T1_SIZE, coll = v->rd32(o + BN6_T1_COLLISION);
		w8s(g, s3[i], 0, in_ewram(coll), dispatched(v, 3, o));
		obj_part(g, XD4_OBJECT, o - BN6_OBJECT_NODE, BN6_T1_SIZE, BN6_T1_SIZE);
		obj_part(g, XD4_COLL, in_ewram(coll) ? coll : 0, v->coll_size, BN6_COLL_SIZE);
	}
	for (int i = 0; i < n4; ++i) {
		uint32_t o = v->t4 + s4[i] * BN6_T4_SIZE;
		w8s(g, s4[i], 0, 0, dispatched(v, 4, o));
		bytes_out(g, o - BN6_OBJECT_NODE, BN6_T4_SIZE, BN6_T4_SIZE);
	}
}

/* the events its hooks logged since the last record (made with the logs) */
#define EVENTS_OUT 8192
static XdEvent *events;

/* BN5's Dark Soul's AI data (BN5_DARK_SOUL_ID's object in play among the first kind's), 0 none */
static uint32_t dark_ai5(const View *v, const uint8_t *s1, int n1) {
	for (int i = 0; i < n1; ++i) {
		uint32_t obj = v->t1 + (uint32_t)s1[i] * BN6_T1_SIZE;
		if ((v->rd32(obj + BN6_T1_NAME_ID) & 0xFFFFu) == BN5_DARK_SOUL_ID) return v->rd32(obj + BN6_T1_AI_DATA);
	}
	return 0;
}

static void apart_out(Side *g, const uint8_t *s1, int n1);
static void apart4_out(Side *g);

static void frame_out(Side *g) {
	const View *v = &g->v;
	int nev = g->log && events ? xd_take(g->log, events, EVENTS_OUT) : 0;
	uint8_t s1[BN6_T1_COUNT], s3[BN6_T3_COUNT], s4[BN6_T4_COUNT];
	int n1 = in_play(v, v->t1, BN6_T1_SIZE, v->nt1, s1), n3 = in_play(v, v->t3, BN6_T1_SIZE, BN6_T3_COUNT, s3);
	int n4 = in_play(v, v->t4, BN6_T4_SIZE, BN6_T4_COUNT, s4);
	uint32_t order[96];
	int no = run_order(v, order, 96), ns = g->nbegun;
	fwrite("FRAM", 1, 4, g->out);
	w32(g, g->frame);
	w32(g, v->rd32(v->state + 0x40));
	w8s(g, sub_of(v), v->rd8(v->state + 1), (uint8_t)n1, (uint8_t)n3);
	w8s(g, (uint8_t)n4, (uint8_t)ns, (uint8_t)no, 0);
	w32(g, v->rd32(v->rng[0]));
	w32(g, v->rd32(v->rng[1]));
	w32(g, (uint32_t)nev);
	player_out(g);
	if (v->game == 4) apart4_out(g);
	else apart_out(g, s1, n1);
	objects_out(g, s1, n1, s3, n3, s4, n4);
	for (uint32_t p = 0; p < PANELS; ++p) obj_part(g, XD4_PANEL, v->panels + p * v->panel_size, v->panel_size, BN5_PANEL_SIZE);
	for (int i = 0; i < no; ++i) w32(g, order[i]);
	for (int i = 0; i < ns; ++i) { w32(g, g->begun[i][0]); w32(g, g->begun[i][1]); }
	g->nbegun = 0;
	for (int i = 0; i < nev; ++i) {
		const XdEvent *e = &events[i];
		w8s(g, e->kind, e->sub, (uint8_t)e->flags, (uint8_t)(e->flags >> 8));
		const uint32_t words[XD_EVENT_WORDS - 1] = { e->frame, e->t, e->caller, e->family, e->outer, e->object, e->a, e->b, e->ref };
		for (int k = 0; k < XD_EVENT_WORDS - 1; ++k) w32(g, words[k]);
	}
}

/* What BN6's engine keeps apart of the objects, BN5's read as BN6 keeps it (APART_*) */
static void apart_out(Side *g, const uint8_t *s1, int n1) {
	const View *v = &g->v;
	/* (BN6's: RAM_SEA on; BN5 keeps the sea's in the objects' collision data, read by the differential as BN6 keeps it, and its
	 * lists at its own address) */
	bytes_out(g, v->game == 6 ? RAM_SEA : 0, APART_SEA, APART_SEA);
	for (uint32_t k = 0; k < APART_LISTS; k += BN5_DARK_LIST_SIZE)
		bytes_out(g, (v->game == 6 ? RAM_DARK_LISTS : BN5_DARK_LISTS) + k, BN5_DARK_LIST_SIZE, BN5_DARK_LIST_SIZE);
	if (v->game == 6) bytes_out(g, RAM_NAVIS, APART_NAVIS, APART_NAVIS);
	else {
		w32(g, v->rd32(v->state + BN5_BATTLE_BITS) & COLONEL_SIDES);
		bytes_out(g, 0, APART_NAVIS - 4, APART_NAVIS - 4);
	}
	uint32_t dark = v->game == 6 ? (v->rd32(BN6_DARK_AI_OWNER) ? BN6_DARK_AI : 0) : dark_ai5(v, s1, n1);
	bytes_out(g, dark ? dark + BN6_AI_DATA_SIZE : 0, APART_DARK, APART_DARK);
	if (v->game == 6) bytes_out(g, RAM_BATTLE, APART_MOVED, APART_MOVED);
	else {
		uint32_t flash = 0;
		for (uint32_t side = 0; side < 2; ++side) {
			uint32_t obj = v->rd32(v->state + BN6_BATTLE_PLAYERS - BN6_BATTLE_STATE + side * BN6_BATTLE_PLAYER_STEP);
			uint32_t ai = in_ewram(obj) ? v->rd32(obj + BN6_T1_AI_DATA) : 0;
			flash |= (in_ewram(ai) ? v->rd32(ai + BN5_AI_FLASH) & 0xFFFFu : 0) << (16 * side);
		}
		w32(g, flash);
		bytes_out(g, 0, APART_MOVED - 4, APART_MOVED - 4);
	}
}

/* (BN4's) the same block: BN4 has none of what BN6's engine keeps apart (no sea's submerge, no Dark Soul's lists, no moved
 * Navis' state, no green flash by side), zeros, but for its battle's sprite cache in the sea's room (xd4_places.sprites:
 * where its preload put each foe's compressed sprite, by its list and number, which tools/xdiff_bn4.py maps a pointer into
 * by; docs/devtools/differential.md, The BN4 trace) */
static void apart4_out(Side *g) {
	static const uint32_t parts[] = { APART_SEA, BN5_DARK_LIST_SIZE, BN5_DARK_LIST_SIZE, APART_NAVIS, APART_DARK, APART_MOVED };
	_Static_assert(APART_SEA >= 0x38u, "BN4's sprite cache in the sea's room");
	bytes_out(g, xd4_places.sprites, xd4_places.sprites_size, parts[0]);
	for (uint32_t k = 1; k < sizeof parts / sizeof *parts; ++k) bytes_out(g, 0, parts[k], parts[k]);
}

/* ---- a battle fought ---- */

static uint32_t keys_at(const Spec *s, uint32_t t) {
	uint32_t k = 0;
	for (int i = 0; i < s->nkeys; ++i)
		if (t >= s->keys[i].t && t < s->keys[i].t + s->keys[i].n) k |= s->keys[i].keys;
	return k;
}

/* a byte of the game's memory, through the view's words */
static void wr8(const View *v, uint32_t a, uint8_t b) {
	uint32_t at = a & ~3u, shift = (a & 3u) * 8, w = v->rd32(at);
	v->wr32(at, (w & ~(0xFFu << shift)) | (uint32_t)b << shift);
}

/* MegaMan's element (the spec's mm_element), his object's and his collision
 * data's, as the fight begins: the first battle object of the first kind in
 * both games, the record's first entity */
static void element_set(const View *v, int element) {
	uint32_t coll = v->rd32(v->t1 + BN6_T1_COLLISION);
	wr8(v, v->t1 + BN6_T1_ELEMENT, (uint8_t)element);
	if (in_ewram(coll)) wr8(v, coll + BN6_COLL_ELEMENT, (uint8_t)element);
}

/* A Custom screen's keys in its frame f (from its window first reading
 * open): the spec's script for this game's screen, else START, then A (OK,
 * no chip) */
static uint32_t custom_keys(const Spec *s, int game, int screen, int f) {
	for (int i = 0; i < s->nscripts; ++i) {
		const CustomScript *c = &s->scripts[i];
		if (c->game != game || c->screen != screen) continue;
		for (int k = 0; k < c->nsteps; f -= c->steps[k++].n)
			if (f < c->steps[k].n) return c->steps[k].keys;
		return 0;
	}
	/* (BN4's takes A only after its cursor's slide to OK: START 2 frames, A 2 frames 18 later, again each 40 frames, xdiff_bn4.c's) */
	if (game == 4) return f % 40 < 2 ? KEY_START : f % 40 >= 20 && f % 40 < 22 ? KEY_A : 0;
	return f < 4 ? KEY_START : f >= 12 && f < 16 ? KEY_A : 0;
}

/* (hold=N) each foe paralysed as a hit leaves it (its struck word's BN6_STRUCK_PARALYSIS, the same bit in both games, and its
 * timer), its timer N again before each fight frame: the chip's target holds still in both, each game's own status step
 * running it (BN5 0x0800CB48 / BN6 0x0800E730, the same steps) */
static void hold_foes(const View *v, int frames, bool first) {
	for (int k = 1; k < v->nt1; ++k) {
		uint32_t o = v->t1 + (uint32_t)k * BN6_T1_SIZE, coll = v->rd32(o + BN6_T1_COLLISION);
		if (!(v->rd8(o + BN6_T1_IN_PLAY) & 1) || !v->rd8(o + BN6_T1_ALLIANCE) || !in_ewram(coll)) continue;
		v->wr16(coll + v->paralysis, (uint16_t)frames);
		if (first) v->wr32(coll + v->coll_struck, v->rd32(coll + v->coll_struck) | BN6_STRUCK_PARALYSIS);
	}
}

/* (hold=N) the foes held from the fight's first seeded frame on, before each fight frame */
static void hold_step(Side *g, const Spec *s) {
	if (!s->hold || !g->seeded) return;
	hold_foes(&g->v, s->hold, !g->held);
	g->held = true;
}

/* (panels=) a BN4 spec's panels' types written in each game before its first fight frame (xdiff_bn4.c's xd4_panels_put) */
static void panels_step(Side *g, const Spec *s) {
	if (!g->paneled) { xd4_panels_put(s, g->v.game); g->paneled = true; }
}

/* (safe=1) MegaMan's collision data invulnerable, before each frame of the battle */
static void keep_safe(const View *v) {
	uint32_t coll = v->rd32(v->t1 + BN6_T1_COLLISION);
	if (in_ewram(coll)) v->wr32(coll + v->coll_flags, v->rd32(coll + v->coll_flags) | BN6_OBJ_INVULNERABLE);
}

/* (deck5=, deck6=) the battle's deck in the spec's order, as the battle's
 * phase first reads the Custom screen's (each game makes and shuffles its
 * deck as the first turn begins, both by then) and before its window opens
 * and takes a pick: each game's hands its own chips in the same places,
 * whatever each game's shuffle drew (the hand's chips are its elements'
 * pointers into the deck, BN6_ELEMENT_DECK) */
static void deck_put(Side *g, const Spec *s) {
	int k = g->v.game == 6 ? 1 : 0;   /* (the reference's, BN5's or BN4's, the spec's first) */
	g->decked = true;
	for (int i = 0; g->v.deck && i < s->ndeck[k]; ++i) g->v.wr16(g->v.deck + 2 * (uint32_t)i, s->deck[k][i]);
}

/* BN6's chip AI's lists written from BN5's (dslist=1), its chip ids BN6's (xnavi_dark.h), after BN6's fight's first frame
 * (BN5's had its first first): the Dark Soul plays the chips BN5's battle holds for it */
static void lists_copy(const Side *g) {
	uint8_t b5[2 * BN5_DARK_LIST_SIZE], b6[sizeof b5];
	for (uint32_t i = 0; i < sizeof b5; i += 4) put32(b5 + i, g->ref->v.rd32(BN5_DARK_LISTS + i));
	int lost = xnavi_dark_lists_from(b5, b6, sizeof b6);
	for (uint32_t i = 0; i < sizeof b6; i += 4) g->v.wr32(RAM_DARK_LISTS + i, get32(b6 + i));
	printf("xdiff: BN6's chip AI's lists written from BN5's: %u and %u chips, %d none in BN6\n", (unsigned)get32(b6 + BN5_DARK_LIST_COUNT),
		(unsigned)get32(b6 + BN5_DARK_LIST_SIZE + BN5_DARK_LIST_COUNT), lost);
}

/* MegaMan's side's chip AI list in BN5 (dschips=), after its fight's first frame: its chips in order, none (0xFFFF) after
 * them, their count, no sequences, as BN5's battle makes one (0x0802C7A4: the list filled with 0xFF first) */
static void dschips_put(const Side *g, const Spec *s) {
	uint32_t at = BN5_DARK_LISTS + BN5_DARK_LIST_SIZE * (g->v.rd8(BN5_BATTLE_SIDE) & 1u);
	for (uint32_t i = 0; i < BN5_DARK_LIST_SIZE; i += 4) g->v.wr32(at + i, 0xFFFFFFFFu);
	for (int i = 0; i + 1 < s->ndschips; i += 2) g->v.wr32(at + 2u * (uint32_t)i, s->dschips[i] | (uint32_t)s->dschips[i + 1] << 16);
	if (s->ndschips & 1) g->v.wr32(at + 2u * (uint32_t)(s->ndschips - 1), s->dschips[s->ndschips - 1] | 0xFFFF0000u);
	g->v.wr32(at + BN5_DARK_LIST_COUNT, (uint32_t)s->ndschips);
}

/* After a fight's first frame: both RNG words, MegaMan's element, the chip AI's lists (BN5's given, BN6's from BN5's) */
static void seed(Side *g, const Spec *s) {
	const View *v = &g->v;
	v->wr32(v->rng[0], s->seed[0]);
	v->wr32(v->rng[1], s->seed[1]);
	if (s->mm_element >= 0 && v->game != 4) element_set(v, s->mm_element);   /* (BN4's collision data's element unmapped) */
	if (s->ndschips && v->game == 5) dschips_put(g, s);
	if (s->dslist && v->game == 6 && g->ref->v.game == 5 && g->ref->seeded) lists_copy(g);
	g->seeded = true;
	g->nbegun = 0;   /* (the fight's sounds from here) */
}

/* (shot=) the pictures of this frame of a Custom screen: the core's picture, as the canvas shows it */
static void shots_take(const Side *g, const Spec *s) {
	for (int i = 0; i < s->nshots; ++i) {
		const Shot *h = &s->shots[i];
		if (h->game != g->v.game || h->screen != g->screen || h->frame != g->custom) continue;
		static uint32_t px[EMU_W * EMU_H];
		const uint32_t *v = g->v.game == 6 ? emu_video() : guest_video();   /* (the reference, BN5 or BN4, is the guest) */
		for (int k = 0; k < EMU_W * EMU_H; ++k) px[k] = v[k] | 0xFF000000u;
		SDL_Surface *f = SDL_CreateRGBSurfaceWithFormatFrom(px, EMU_W, EMU_H, 32, EMU_W * 4, SDL_PIXELFORMAT_ABGR8888);   /* (the core's pixels' order: emu_picture.c) */
		if (!f || SDL_SaveBMP(f, h->path) != 0) fprintf(stderr, "xdiff: %s not saved\n", h->path);
		SDL_FreeSurface(f);
	}
}

/* (film=) its picture of the fight at timer `t`, once a timer value, where the spec films it */
static void film_take(Side *g, const Spec *s, uint32_t t) {
	const Film *m = &s->film;
	if (!m->step || t < (uint32_t)m->from || t > (uint32_t)m->to || (t - (uint32_t)m->from) % (uint32_t)m->step || g->filmed == t + 1) return;
	g->filmed = t + 1;
	static uint32_t px[EMU_W * EMU_H];
	const uint32_t *v = g->v.game == 6 ? emu_video() : guest_video();   /* (the reference, BN5 or BN4, is the guest) */
	for (int k = 0; k < EMU_W * EMU_H; ++k) px[k] = v[k] | 0xFF000000u;
	char path[200];
	snprintf(path, sizeof path, "%s_bn%d_t%04u.bmp", m->prefix, g->v.game, (unsigned)t);
	SDL_Surface *f = SDL_CreateRGBSurfaceWithFormatFrom(px, EMU_W, EMU_H, 32, EMU_W * 4, SDL_PIXELFORMAT_ABGR8888);   /* (the core's pixels' order: emu_picture.c) */
	if (!f || SDL_SaveBMP(f, path) != 0) fprintf(stderr, "xdiff: %s not saved\n", path);
	SDL_FreeSurface(f);
}

/* One frame of a battle: each Custom screen passed by its keys (custom_keys)
 * from its window's opening; the fight's keys by its timer; both RNG words
 * written after its first frame; a new value of its timer while it fights
 * traced. Done after `frames` of them, its end, or its frames' budget.
 * True where this frame was traced. */
/* (its frames at its timer's value, from the seed: each game's opening its own length) */
static void held_count(Side *g, uint32_t t, bool seeding) { g->still = t == g->last && !seeding ? g->still + 1 : 0; }

static bool step(Side *g, const Spec *s) {
	const View *v = &g->v;
	if (g->done) return false;
	uint8_t sub = sub_of(v), phase = v->rd8(v->state + 1);
	uint32_t t = v->rd32(v->state + 0x40), keys = 0;
	bool battle = sub == BN6_SUB_BATTLE, custom = battle && phase == BN6_PHASE_CUSTOM;
	/* (a screen counted as its window first reads open: the phase passes the Custom screen's for a frame as a battle begins) */
	if (!custom) g->custom = -1;
	else if (g->custom >= 0 || v->rd8(v->window) >= BN6_WINDOW_OPEN) {
		if (g->custom < 0) ++g->screen;
		keys = custom_keys(s, v->game, g->screen, ++g->custom);
	}
	if (battle && phase == BN6_PHASE_FIGHT) {
		keys |= t >= (uint32_t)g->behind ? keys_at(s, t - (uint32_t)g->behind) : 0;
		hold_step(g, s);
		panels_step(g, s);
	}
	if (battle && s->safe) keep_safe(v);
	xd4_dark_rules(s, v->game, battle);   /* (calm4, confuse6, mood4: xdiff_bn4.h) */
	if (custom && !g->decked) deck_put(g, s);
	if (g->log) {
		xd_frame(g->log, g->frame, t, battle, g->seeded);
		xd_setup(g->log, sub == BN6_SUB_BATTLE_INIT);
	}
	v->frame(keys);
	++g->frame;
	if (custom && g->custom >= 0) shots_take(g, s);
	sub = sub_of(v);
	t = v->rd32(v->state + 0x40);
	bool fighting = sub == BN6_SUB_BATTLE && v->rd8(v->state + 1) == BN6_PHASE_FIGHT, traced = false;
	if (fighting) film_take(g, s, t);
	bool seeding = fighting && !g->seeded;
	if (seeding) seed(g, s);
	sounds(g);
	held_count(g, t, seeding);
	if (fighting && t != g->last) {
		frame_out(g);
		g->fought = traced = true;
		g->done = ++g->traced >= s->frames + g->behind;
	}
	g->last = t;
	g->done |= (g->fought && sub != BN6_SUB_BATTLE) || g->frame >= (uint32_t)(OPENING + 4 * s->frames);
	return traced;
}

/* Both battles, each to its fight's first frame, then by the battle's
 * clock: a frame of each in turn while their timers read the same, the one
 * whose timer has gone on waiting while the other's still holds (a turn's
 * change, a unison's crossing: each game takes frames of its own there), so
 * each game's frames of a timer value run with the other's at it too, the
 * reference's first (the replay finds its draws made), and through a hold
 * up to LEAD frames ahead: a frame the reference's game spends without its
 * loop (BN5 loading a Darkloid's LZ77 sprite as his Navi chip comes,
 * three frames, where BN6's copy is whole) leaves its draws still made
 * before the subject's ask for them; with `follow`, BN6's RNG words written
 * from BN5's once both have traced a timer value (rng=follow) */
#define LEAD 8
/* (whether the reference `a`, or the subject, steps next while both run) */
static bool goes(const Side *a, const Side *b, bool reference) {
	if (a->last != b->last) return reference ? a->last < b->last : b->last < a->last;
	return reference ? a->still < b->still + LEAD : b->still + LEAD <= a->still;
}

/* (rng=follow: BN6's RNG words written from BN5's once both have traced a timer value) */
static void follow(const Side *a, Side *b, const Spec *s, uint32_t *followed) {
	if (!s->follow || a->last != b->last || a->last == *followed) return;
	*followed = a->last;
	b->v.wr32(b->v.rng[0], a->v.rd32(a->v.rng[0]));
	b->v.wr32(b->v.rng[1], a->v.rd32(a->v.rng[1]));
}

static void fight(Side *a, Side *b, const Spec *s) {
	while (a && !a->seeded && !a->done) step(a, s);
	while (b && !b->seeded && !b->done) step(b, s);
	uint32_t followed = 0;
	while ((a && !a->done) || (b && !b->done)) {
		bool both = a && b && !a->done && !b->done;
		bool go_a = a && (!both || goes(a, b, true)), go_b = b && (!both || goes(a, b, false));
		bool ta = go_a && step(a, s), tb = go_b && step(b, s);
		if (both && (ta || tb)) follow(a, b, s, &followed);
	}
}

/* ---- the two games ---- */

static void bn6_frame(uint32_t keys) { emu_frame(keys); }
static void bn5_frame(uint32_t keys) { guest_frame_quiet(keys); }

/* BN5's id as BN6 has it: a moved family's (xvirus_bn5) or Navi's (xnavi_install.h), else the same */
static int bn6_id(int id) {
	for (int k = 0; k < xvirus_bn5_count; ++k)
		for (int v = 0; v < XV_VERSIONS; ++v)
			if (xvirus_bn5[k]->names[v] == id) return xvirus_bn5[k]->first_id + v;
	int navi = xnavi_bn6_id(id);
	return navi >= 0 ? navi : id;
}

/* the veneer of family f's calls on to BN6's routine `to`, its bus address; 0 none */
static uint32_t veneer_of(const XVirusFamily *f, uint32_t to) {
	for (uint32_t at = f->home + XV_HOME_VENEERS; at + THUMB_VENEER_SIZE <= f->home + XV_HOME_SIZE; at += THUMB_VENEER_SIZE) {
		uint8_t v[THUMB_VENEER_SIZE];
		uint32_t t;
		for (uint32_t i = 0; i < THUMB_VENEER_SIZE; ++i) v[i] = emu_read8(at + i);
		if (!thumb_veneer_target(v, &t)) return 0;
		if (t == (to & ~1u)) return at;
	}
	return 0;
}

/* each family's veneer on to BN6's routine `at` aimed at `v` instead */
static void call_aim(uint32_t at, uint32_t v) {
	uint8_t b[4];
	put32(b, v | 1);
	/* (BN5's families, then BN4's where they are installed: xdiff_bn4.h's xd4_family) */
	for (int k = 0; k < xvirus_bn5_count + xd4_family_count(); ++k) {
		const XVirusFamily *f = k < xvirus_bn5_count ? xvirus_bn5[k] : xd4_family(k - xvirus_bn5_count);
		uint32_t veneer = veneer_of(f, at);
		if (!veneer) continue;
		emu_write(veneer + 8, b, 4);
		printf("xdiff: %s's call to %08x (veneer %08x) aimed at %08x\n", f->name, (unsigned)at, (unsigned)veneer, (unsigned)v);
	}
}

/* whether `to`, a moved range's copy, is in the code of a BN5 Navi none of the spec's foes is: each Navi's copy of BN5's
 * code is his own, no relocation of a routine, and its BN5 addresses lie among BN6's own code and BN4's, so a poke there is
 * not his unless the spec fights him (toadman_sea_elec's BN6 0x080C64EB lies in Meddy's copy of BN5's 0x080C6000-0x080C665C,
 * which took the poke from ToadMan's battle once Meddy was moved; BN4's WoodMan's 0x080F50E4 in SearchMan's) */
static bool another_navis(const Spec *s, uint32_t to) {
	int ids[MAX_FOES], n = 0;
	for (int k = 0; !s->bn4 && k < s->nfoes; ++k) ids[n++] = s->foes[k].id;
	return xd_navi_copy_apart(ids, n, to);
}

/* the moved range a poke's address is in, -1 none: the family's own ranges, moved last, first (BN6's tables grown are
 * among the ranges too); the chips' moved code for a poke5 alone, its BN5 addresses lying among BN6's own attack families'
 * code; never BN4's Navis' for a poke5, BN4's addresses of their blocks lying among BN5's (ThunderMan's, WindMan's and
 * FireMan's, moved after it), nor BN4's chips' units' (PanlSht's shot's block, 0x080DCAA4-0x080DCC5C, holds BN5's
 * BlizzardMan's and CloudMan's chips' spawns' addresses); and never a BN5 Navi's copy but the spec's own foes'
 * (another_navis), nor a BN5 virus family's in a spec of BN4's (xd_bn5_family_copy), nor a BN4 chip unit's but xd_spec_bn4_code's */
static int poke_range(const Spec *s, const XvMoved *m, int n, const Poke *p) {
	int other = -1;   /* (a poke5's) another Navi's copy holding it */
	for (int k = n - 1; k >= 0; --k)
		if (p->at >= m[k].from && p->at < m[k].from + m[k].len && (p->bn5 || !(m[k].to >= FREE_CHIP_MOVED && m[k].to < FREE_TAIL)) &&
		    !(p->bn5 && m[k].to >= FREE_BN4_NAVIS && m[k].to < FREE_BN4_NAVI_SPRITE) &&
		    !((p->bn5 || !xd_spec_bn4_code(s)) && m[k].to >= FREE_BN4_CHIPS && m[k].to < FREE_BN4_CHIPS_END) && !(s->bn4 && xd_bn5_family_copy(m[k].to))) {
			if (!another_navis(s, m[k].to)) return k;
			if (p->bn5 && other < 0) other = k;
		}
	return other;
}

/* whether `to`, a moved range's copy, is in BN4's chips' code (FREE_BN4_CHIPS) */
static bool bn4_chips_code(uint32_t to) {
	return to >= FREE_BN4_CHIPS && to < FREE_BN4_CHIPS_END;
}

/* the comparator's own test: bytes written into BN6's copy after the
 * install (an address of BN5's moved, to its copy; each copy of a block more than one family copies), or a call's veneer
 * aimed at another routine; those in RAM (`ram`) once the game has booted,
 * over what it copied there (IWRAM's routines, from their image) */
static void pokes_put(const Spec *s, bool ram) {
	int n;
	const XvMoved *m = xvirus_moved(&n);
	for (int i = 0; i < s->npokes; ++i) {
		const Poke *p = &s->pokes[i];
		uint32_t at = p->at;
		uint8_t b[4];
		if ((p->size && (at >> 24 == 2 || at >> 24 == 3)) != ram) continue;   /* (EWRAM, IWRAM) */
		if (!p->size) {
			call_aim(p->at, p->v);
			continue;
		}
		int k = poke_range(s, m, n, p);
		if (k >= 0) at = at - m[k].from + m[k].to;
		if (p->bn5 && at == p->at) {
			fprintf(stderr, "xdiff: poke5 %08x is in no moved range\n", (unsigned)p->at);
			continue;
		}
		put32(b, p->v);
		emu_write(at, b, (size_t)p->size);
		printf("xdiff: poke %08x (%08x) = %0*x\n", (unsigned)p->at, (unsigned)at, 2 * p->size, (unsigned)p->v);
		/* (a block more than one family copies, each copy: BN4's summoned appear, Mettaur's and Melody's; and each BN4 chip
		 * unit's copy holding the address, a block of one unit inside another's: ShotStar's star's spawner, which
		 * Meteors' unit copies alone, in ShotStar's own) */
		for (int j = 0; k >= 0 && j < k; ++j)
			if (((m[j].from == m[k].from && m[j].len == m[k].len) || (bn4_chips_code(m[j].to) && bn4_chips_code(m[k].to) &&
			     p->at >= m[j].from && p->at < m[j].from + m[j].len)) && m[j].to != m[k].to) {
				emu_write(p->at - m[j].from + m[j].to, b, (size_t)p->size);
				printf("xdiff: poke %08x (%08x, a copy) = %0*x\n", (unsigned)p->at, (unsigned)(p->at - m[j].from + m[j].to), 2 * p->size, (unsigned)p->v);
			}
	}
}

static bool open_out(Side *g, const Spec *s, const char *game) {
	char path[300];
	snprintf(path, sizeof path, "%s.%s", s->out, game);
	g->out = fopen(path, "wb");
	if (!g->out) fprintf(stderr, "xdiff: cannot write %s\n", path);
	g->custom = -1;
	g->screen = 0;
	if (g->out) {
		header_out(g);
		players_init(g);
	}
	return g->out != NULL;
}

static void bn6_wr16(uint32_t a, uint16_t v) { const uint8_t b[2] = { (uint8_t)v, (uint8_t)(v >> 8) }; emu_write(a, b, 2); }

/* MegaMan in BN6 as the spec has him: its event flags, its folder, his buster */
static void subject_setup(const Spec *s) {
	for (int i = 0; i < s->nflags[1]; ++i) flag_set(s->flags[1][i]);
	static const FolderPlaces places = { BN6_TOOLKIT, BN6_TOOLKIT_CHIPS, BN6_TOOLKIT_CHIP_MARKS, BN6_CHIP_KEYS, BN6_CHIP_KEY_XOR, BN6_NAVI_FOLDER1_REG,
		BN6_NAVI_FOLDER1_TAG, CHIP_ID_MASK, emu_read8, emu_read32, emu_write8, bn6_wr16 };
	if (s->nfolder[1]) {
		xd_folder_put(&places, s->folder[1]);
		xd_pack_own(s->folder[1]);
	}
	static const uint32_t buster[3] = { BN6_NAVI_ATTACK, BN6_NAVI_SPEED, BN6_NAVI_CHARGE };
	for (int i = 0; i < 3; ++i)
		if (s->buster[i] >= 0) emu_write8(buster[i], (uint8_t)s->buster[i]);
}

/* BN6's battle with the families installed, forced as the foes step forces one, the ids BN6's */
static bool subject(Side *g, const Spec *s) {
	if (!emu_init(R.data, ROM_SIZE)) return false;
	/* (as the game's own start makes them: the engine's objects, the Navis, BN5's chips' moved code, the families and the sea,
	 * xinstall_core; then BN6's chip tables with the registry's chips in them, game_hooks.c) */
	xinstall_core(XVIRUS_FAMILIES, XNAVI_BN5);
	xchips_install();
	/* (souls6=: the Souls in BN6's engine over the proxy form, MegaMan holding them) */
	if (!soulunite_dev(s->souls6, s->unitebreak, s->soulbreak, s->dark)) {
		fprintf(stderr, "xdiff: the Souls could not be installed in BN6's engine\n");
		return false;
	}
	/* (every range the installs moved, or no battle: with a part of them some moved code goes uncompared, xvirus_moved.h) */
	int nmoved;
	if (!xvirus_moved(&nmoved)) return false;
	printf("xdiff: %d ranges moved\n", nmoved);
	if (s->bn4) xd4_pool_point(s);
	pokes_put(s, false);
	emu_encounters_install();
	emu_boot();
	pokes_put(s, true);
	if (!xd_ncp_subject(s->ncp[1].variant, s->ncp[1].col, s->ncp[1].row)) return false;
	const uint8_t hp[4] = { (uint8_t)s->hp, (uint8_t)(s->hp >> 8), (uint8_t)s->hp, (uint8_t)(s->hp >> 8) };
	emu_write(BN6_NAVI_BASE_MAX_HP, hp, 2);
	emu_write(BN6_NAVI_HP, hp, 4);
	subject_setup(s);
	Encounter e;
	memset(&e, 0, sizeof e);
	e.player = s->mm;
	e.sides = s->sides;
	/* (BN5's field as BN6's: xfields.h, or the spec's own number for it) */
	e.field = s->field6 >= 0 ? s->field6 : s->field ? xfield_bn6(XROM_BN5_COLONEL_US, s->field) : 0;
	if (e.field < 0) {
		fprintf(stderr, "xdiff: BN5's field %#x is none of BN6's\n", s->field);
		return false;
	}
	for (int i = 0; i < s->nfoes; ++i) {
		int id = s->bn4 ? xd4_bn6_id(s->foes[i].id) : bn6_id(s->foes[i].id), p = s->foes[i].panel;
		e.foes[e.nfoes++] = (Foe){ FOE_VIRUS, 0, 0, (p & 0xF) - 1, (p >> 4) - 1, id };
		printf("xdiff: BN%d's %#x at column %d, row %d: BN6's %#x\n", s->bn4 ? 4 : 5, s->foes[i].id, p & 0xF, p >> 4, id);
	}
	emu_battle_force(&e);
	for (int f = 0; f < 600 && emu_read8(BN6_GAMESTATE) != BN6_SUB_BATTLE_INIT && emu_read8(BN6_GAMESTATE) != BN6_SUB_BATTLE; ++f) emu_frame(0);
	emu_battle_release();
	HookEvent ev[64];
	while (emu_hook_events(ev, 64) == 64) {}
	g->v = (View){ .game = 6, .gamestate = BN6_GAMESTATE, .state = BN6_BATTLE_STATE, .window = BN6_CUSTOM_WINDOW,
		.rng = { BN6_RNG_PRIMARY, BN6_RNG_SECONDARY }, .t1 = BN6_T1_OBJECTS, .t3 = BN6_T3_OBJECTS, .t4 = BN6_T4_OBJECTS, .nt1 = BN6_T1_COUNT,
		.panels = BN6_FIELD_PANELS, .panel_size = BN6_PANEL_SIZE, .list = BN6_OBJECT_LIST, .list_end = BN6_OBJECT_LIST_END,
		.coll_size = BN6_COLL_SIZE, .mplays = BN6_MPLAYS, .navi = BN6_BATTLE_NAVI, .navi_size = BN6_BATTLE_NAVI_SIZE, .used = BN6_CROSSES_USED,
		.pending = BN6_CROSS_PENDING, .side = BN6_BATTLE_SIDE, .souls = BN6_SOULS, .unite = BN6_CUSTOM_UNITE, .deck = BN6_BATTLE_DECK,
		.paralysis = BN6_COLL_PARALYSIS, .coll_flags = BN6_COLL_FLAGS, .coll_struck = BN6_COLL_STRUCK,
		.rom = R.data, .songs = R.layout->song_table, .rd8 = emu_read8, .rd32 = emu_read32, .wr32 = emu_write32, .wr16 = bn6_wr16,
		.frame = bn6_frame };
	return open_out(g, s, "bn6");
}

/* (the guest's bytes: a halfword's half written) */
static void guest_wr8(uint32_t a, uint8_t v) {
	uint16_t h = guest_read16(a & ~1u);
	guest_write16(a & ~1u, (uint16_t)(a & 1 ? (h & 0xFF) | v << 8 : (h & 0xFF00) | v));
}

/* MegaMan in BN5 as the spec has him, once its battle is asked and before it begins: its event flags (eToolkit +0x44:
 * Double Soul's, a Soul's, Chaos Unison's), its folder, his buster */
static void reference_setup(const Spec *s) {
	uint32_t flags = guest_read32(BN5_TOOLKIT + BN5_TOOLKIT_FLAGS);
	for (int i = 0; i < s->nflags[0] && in_ewram(flags); ++i) {
		uint32_t f = (uint32_t)s->flags[0][i], a = flags + (f >> 3);
		guest_wr8(a, (uint8_t)(guest_read8(a) | 0x80u >> (f & 7)));
	}
	static const FolderPlaces places = { BN5_TOOLKIT, BN5_TOOLKIT_CHIPS, BN5_TOOLKIT_CHIP_MARKS, BN5_CHIP_KEYS, BN5_CHIP_KEY_XOR, BN5_NAVI_FOLDER1_REG,
		0, CHIP9_MASK, guest_read8, guest_read32, guest_wr8, guest_write16 };
	if (s->nfolder[0]) xd_folder_put(&places, s->folder[0]);
	static const uint32_t buster[3] = { BN5_NAVI_ATTACK, BN5_NAVI_SPEED, BN5_NAVI_CHARGE };
	for (int i = 0; i < 3; ++i)
		if (s->buster[i] >= 0) guest_wr8(buster[i], (uint8_t)s->buster[i]);
	/* (dark=1: his dark meter under 470, which the battle's copy takes as it begins and opens at mood 0, 0x08010EFE-0x08010F1C) */
	if (s->dark) {
		guest_write16(BN5_NAVI_METER, METER_DARK);
		uint32_t check = guest_read32(BN5_TOOLKIT + BN5_TOOLKIT_METER_CHECK);
		if (in_ewram(check)) guest_write32(check, METER_DARK ^ guest_read32(BN5_METER_KEY));
	}
}

/* BN5's own battle on the guest core, as BN5 has it (guest_dev_battle) */
static bool reference(Side *g, const Spec *s) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	if (!x->data || !guest_start(XROM_BN5_COLONEL_US)) return false;
	/* (BN5's random battles' record: its field, music, background the map's, side, options; MegaMan, then the foes) */
	uint8_t rec[0x80] = { (uint8_t)s->field, 0x14, 0x15, 0x00, 0xFF, 0x00, (uint8_t)(s->sides ? s->sides : 0x38), 0x00, 0xE2, 0x49, 0x00, 0x00 };
	uint32_t len = 0x10;
	const uint8_t mm[4] = { 0x00, (uint8_t)s->mm, 0, 0 };
	memcpy(rec + len, mm, 4);
	len += 4;
	for (int i = 0; i < s->nfoes; ++i, len += 4) {
		const uint8_t f[4] = { 0x11, (uint8_t)s->foes[i].panel, (uint8_t)s->foes[i].id, (uint8_t)(s->foes[i].id >> 8) };
		memcpy(rec + len, f, 4);
	}
	rec[len++] = 0xF0;
	if (!xd_ncp_reference(s->ncp[0].variant, s->ncp[0].col, s->ncp[0].row)) return false;
	if (!guest_dev_battle(rec, len, s->hp, s->seed[0])) return false;
	/* (a spec's change in both games: BN5's copy at its own address, or at BN5's own place) */
	for (int i = 0; i < s->npokes; ++i) {
		const Poke *p = &s->pokes[i];
		uint32_t at = p->at5 ? p->at5 : p->at;
		if (!p->both && !p->at5) continue;
		if (p->size == 4) guest_write32(at, p->v);
		else if (p->size == 2) guest_write16(at, (uint16_t)p->v);
		else guest_wr8(at, (uint8_t)p->v);
		printf("xdiff: both games' %08x = %0*x\n", (unsigned)at, 2 * p->size, (unsigned)p->v);
	}
	reference_setup(s);
	g->v = (View){ .game = 5, .gamestate = BN5_GAMESTATE, .state = BN5_BATTLE_STATE, .window = BN5_CUSTOM_WINDOW,
		.rng = { BN5_RNG_PRIMARY, BN5_RNG_SECONDARY }, .t1 = BN5_T1_OBJECTS, .t3 = BN5_T3_OBJECTS, .t4 = BN5_T4_OBJECTS, .nt1 = BN5_T1_COUNT,
		.panels = BN5_FIELD_PANELS, .panel_size = BN5_PANEL_SIZE, .list = BN5_OBJECT_LIST, .list_end = BN5_OBJECT_LIST_END,
		.coll_size = BN5_COLL_SIZE, .mplays = BN5_MPLAYS, .navi = BN5_BATTLE_NAVI, .navi_size = BN5_BATTLE_NAVI_SIZE, .used = BN5_SOULS_USED,
		.pending = BN5_SOUL_PENDING, .side = BN5_BATTLE_SIDE, .souls = 0, .unite = BN5_CUSTOM_UNITE, .deck = BN5_BATTLE_DECK,
		.paralysis = BN5_COLL_PARALYSIS, .coll_flags = BN6_COLL_FLAGS, .coll_struck = BN6_COLL_STRUCK,
		.rom = x->data, .songs = x->layout->song_table, .rd8 = guest_read8, .rd32 = guest_read32, .wr32 = guest_write32,
		.wr16 = guest_write16, .frame = bn5_frame };
	return open_out(g, s, "bn5");
}

/* (and BN4_SUBSYSTEM_BYTE watched after it, xd4_watch) */
static void bn4_frame(uint32_t keys) {
	guest_frame_quiet(keys);
	xd4_watch();
	xd4_form_watch();
}

/* BN4 Red Sun's own battle on the guest core (xdiff_bn4.h's record), traced in BN6's units (xdiff_bn4_trace.h): its
 * Custom screens passed and its fight keyed by its own window and timer, as BN5's */
static bool reference4(Side *g, const Spec *s) {
	const XRom *x = &XR[XROM_BN4_RED_SUN_US];
	if (!guest_start(XROM_BN4_RED_SUN_US) || !x->data) return false;
	uint8_t rec[0x80];
	uint32_t len = xd4_record(s, rec);
	if (!guest_dev_battle(rec, len, s->hp, s->seed[0])) return false;
	xd4_folder_put(s);   /* (a chip spec's folder4, and its chips through BN4's gate: xdiff_bn4.h; after the battle is asked, which sets its own) */
	xd4_souls_put(s);    /* (a Soul spec's souls4, its flags and pseudo-chips past the gate) */
	const Xd4Places *p = &xd4_places;
	g->v = (View){ .game = 4, .state = p->state, .window = p->window, .rng = { p->rng[0], p->rng[1] }, .t1 = p->t1, .t3 = p->t3,
		.t4 = p->t4, .nt1 = (int)p->nt1, .panels = p->panels, .panel_size = BN6_PANEL_SIZE, .list = p->list, .list_end = p->list_end,
		.coll_size = BN6_COLL_SIZE, .mplays = p->mplays, .navi = p->navi, .navi_size = p->navi_size, .side = p->side,
		.paralysis = p->paralysis, .coll_flags = p->coll_flags, .coll_struck = p->coll_struck, .rom = x->data,
		.songs = x->layout->song_table, .sub = xd4_sub, .rd8 = guest_read8, .rd32 = guest_read32, .wr32 = guest_write32,
		.wr16 = guest_write16, .frame = bn4_frame, .deck = p->deck };
	return open_out(g, s, "bn4");
}

/* Each core's hooks, once both battles are made (BN5's code is the families' as xvirus_moved has it), and the
 * subject's draws replayed from the reference's where the spec says so; false where they cannot be */
/* the spec's families: those its foes are versions of; their code alone is the families' in its traces */
/* (a BN5 Colonel among the foes: his side arms the obstacle strike, whose soldier runs other tracks' copies, xstrike.h) */
static bool colonel_foe(const Spec *s) {
	for (int i = 0; i < s->nfoes; ++i)
		if (s->foes[i].id >= BN5_COLONEL_ID && s->foes[i].id <= BN5_COLONEL_ID + 3) return true;
	return false;
}

static int spec_families(const Spec *s, const XVirusFamily **fams) {
	if (s->bn4) return xd4_families(s, fams);   /* (BN4's foes: its families moved, xdiff_bn4.h) */
	int n = 0;
	for (int i = 0; i < s->nfoes; ++i)
		for (int k = 0; k < xvirus_bn5_count; ++k) {
			const XVirusFamily *f = xvirus_bn5[k];
			bool mine = false, had = false;
			for (int v = 0; v < XV_VERSIONS; ++v) mine = mine || f->names[v] == s->foes[i].id;
			for (int j = 0; j < n; ++j) had = had || fams[j] == f;
			if (mine && !had) fams[n++] = f;
		}
	return n;
}

/* (a BN4 Navi's summon's draw keyed as BN4_SPAWN_ENEMY's in both logs, a BN4 spec's: xdiff_bn4.h xd4_summon_draws; false
 * where a log had no room for one) */
static bool summon_keys(const Spec *s, XdLog *a, XdLog *b, const XVirusFamily *const *fams, int n) {
	XvMoved keys[4];
	bool ok = true;
	for (int k = 0, nk = s->bn4 ? xd4_summon_draws(fams, n, keys, 4) : 0; k < nk; ++k) {
		ok = xd_log_key(a, keys[k].from, keys[k].to, keys[k].len) && ok;
		ok = xd_log_key(b, keys[k].from, keys[k].to, keys[k].len) && ok;
	}
	return ok;
}

static bool logs_start(Side *a, Side *b, const Spec *s) {
	const XVirusFamily *fams[MAX_FOES];
	int n = spec_families(s, fams);
	/* (and whether a foe is a moved Navi: the Navis' code is then a foe's too) */
	bool navis = false;
	int ids[XD_NAVI_IDS_MAX], nids = 0;
	for (int i = 0; i < s->nfoes; ++i) {
		navis = navis || xnavi_bn6_id(s->foes[i].id) >= 0;
		if (nids < XD_NAVI_IDS_MAX) ids[nids++] = s->foes[i].id;
	}
	xd_log_navis(ids, nids);
	/* (BN4's battle on its own routines, xdiff_bn4_trace.h's: bn4.h, each BN6's twin by its body) */
	bool hooked = a->v.game == 5 || a->v.game == 4;
	if (a->v.game == 4) xd_log_bn4(&xd4_routines);
	a->log = a->out && hooked ? xd_log_start(a->v.game, fams, n, navis) : NULL;
	b->log = b->out ? xd_log_start(6, fams, n, navis) : NULL;
	if (!summon_keys(s, a->log, b->log, fams, n)) return false;
	events = malloc(EVENTS_OUT * sizeof *events);
	if ((a->out && hooked && !a->log) || (b->out && !b->log) || !events) return false;
	if (s->replay && a->log && b->log) xd_replay(b->log, a->log, s->twins, s->ntwins);
	if (s->replay && b->log) xd_replay_lag(b->log, s->lag);
	if (s->replay && s->bn4 && a->log && b->log) xd_replay_setup(b->log, a->log);   /* (the family's init draws: BN4's alone) */
	return xd_setup_skips(s, a->log, b->log);
}

/* what was traced, and what became of each core's draws */
static void summary(const Side *a, const Side *b, const Spec *s) {
	int ref = s->bn4 ? 4 : 5;
	printf("xdiff: BN%d's battle %d fight frames traced, BN6's %d%s\n", ref, a->fought ? a->traced : -1, b->fought ? b->traced : -1,
		s->follow ? " (BN6's RNG words following the reference's)" : s->replay && s->bn4 ? " (BN6's draws replayed from BN4's)"
		: s->replay ? " (BN6's draws replayed from BN5's)" : "");
	if (s->bn4 && a->fought) xd4_watched();
	for (int k = 0; k < 2; ++k) {
		const Side *g = k ? b : a;
		int n[XD_COUNTS];
		if (g->out && !g->fought)
			printf("xdiff: BN%d's battle never reached its fight: its game state %02x, its battle's phase %02x, Custom screen %d, after %u frames\n",
				k ? 6 : ref, sub_of(&g->v), g->v.rd8(g->v.state + 1), g->screen, (unsigned)g->frame);
		if (!g->log) continue;
		xd_counts(g->log, n);
		printf("xdiff: BN%d's hooks: %d draws replayed, %d unmatched, %d unmapped; %d events lost\n", k ? 6 : ref, n[0], n[1], n[2], n[3]);
	}
}

int xdiff_run(const char *text) {
	static Spec s;
	static Side a, b;
	xvirus_moved_keep();   /* (before any install: the traces' moved ranges, xvirus_moved.h) */
	if (!xd_spec_parse(text, &s)) {
		fprintf(stderr, "xdiff: a spec names foes=ID@CR,... (docs/devtools/differential.md)\n");
		return 2;
	}
	if (s.bn4 == XD_BN4_ALONE) return xdiff_bn4_run(&s);
	if (!s.bn4 && !XR[XROM_BN5_COLONEL_US].data) {
		fprintf(stderr, "xdiff: BN5 Team Colonel's ROM is not beside BN6's\n");
		return 2;
	}
	xd_log_units(s.folder[1], s.nfolder[1]);
	const XVirusFamily *fams[MAX_FOES];
	xd_log_shared(fams, spec_families(&s, fams));
	if (s.souls6 || colonel_foe(&s)) xd_log_routes();
	/* (the reference BN4's where the spec says game=bn4:trace or bn4:bn6: xdiff_bn4_trace.h) */
	bool want = s.bn5 || s.bn4, ra = s.bn4 ? reference4(&a, &s) : s.bn5 && reference(&a, &s), rb = s.bn6 && subject(&b, &s);
	if ((want && !ra) || (s.bn6 && !rb)) {
		fprintf(stderr, "xdiff: a battle could not be begun\n");
		return 1;
	}
	if (!logs_start(&a, &b, &s)) return 1;
	b.ref = &a;   /* (read where BN5's battle is fought too: dslist) */
	b.behind = s.bn4 ? s.lag : 0;   /* (BN6's whole battle a fight frame behind BN4's: the spec's lag, tools/xdiff_bn4.py) */
	fight(ra ? &a : NULL, rb ? &b : NULL, &s);
	if (a.out) fclose(a.out);
	if (b.out) fclose(b.out);
	summary(&a, &b, &s);
	return (ra && !a.fought) || (rb && !b.fought) ? 1 : 0;
}
