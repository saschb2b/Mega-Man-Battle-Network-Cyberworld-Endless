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
#include "soulproxy.h"
#include "soulunite.h"
#include "thumb_reloc.h"
#include "xdiff_events.h"
#include "xfields.h"
#include "xnavi_install.h"
#include "xsong.h"
#include "xvirus_dispatch.h"
#include "xvirus_families.h"

#define MAX_KEYS  128
#define MAX_POKES 8
#define PANELS    40      /* the field's panels, 8 a row, rows 0-4 */
#define SONGS     0x400   /* the song table's entries looked through for a header */
#define OPENING   3000    /* the frames a battle may take to its fight */
#define MAX_TWINS 16
#define TRACE_VERSION 5   /* (the player block, the Souls' version 4, and the dispatch's types, the dispatch's 3, in one) */
#define USED_SIZE 4   /* the word of what was used this battle (BN5_SOULS_USED, BN6_CROSSES_USED) */
#define SOULS_SIZE BN6_SOULS_SIZE   /* the Souls' state in BN6's engine (BN6_SOULS: both sides', the rest of its range) */
#define PLAYER_SIZE (USED_SIZE + BN6_BATTLE_NAVI_SIZE + BN6_CROSS_PENDING_SIZE + SOULS_SIZE + BN6_ELEMENT_SIZE)   /* the player block */
#define MAX_FLAGS 16
#define MAX_SCRIPTS 8
#define MAX_SCRIPT_STEPS 32

typedef struct { uint32_t t, n, keys; } Key;
typedef struct { uint32_t at, v; int size; bool both; } Poke;   /* size 0: a call's veneer, from BN6's routine `at` to `v`; both: BN5's copy too */

/* A Custom screen's keys (cust5=, cust6=): game 5 or 6's screen n (1 the
 * first), each step its keys for its frames from the frame its window first
 * reads open */
typedef struct { uint32_t keys; int n; } ScriptStep;
typedef struct { int game, screen, nsteps; ScriptStep steps[MAX_SCRIPT_STEPS]; } CustomScript;

typedef struct {
	int nfoes;
	struct { int id, panel; } foes[MAX_FOES];
	int mm, hp, frames;
	int field, field6;   /* the battlefield, BN5's number, and BN6's (-1: as xfields.h installs it), 0 plain */
	int mm_element;      /* MegaMan's element as the fight begins, in both (-1 his own) */
	int flags[2][MAX_FLAGS], nflags[2];        /* event flags set before the battle: [0] BN5's, [1] BN6's */
	uint16_t folder[2][BN6_FOLDER_ENTRIES];    /* its first folder, each game's (0 none given) */
	int nfolder[2];
	uint16_t deck[2][BN6_FOLDER_ENTRIES];      /* each game's battle deck in this order, written as its battle runs (0 none given) */
	int ndeck[2];
	unsigned souls6;                           /* the Souls BN6's MegaMan holds, the Souls installed in its engine (soulunite.h); 0 none */
	int unitebreak;                            /* (the comparator's own test) UNITE's rule broken (SOULUNITE_BREAK_*) */
	int buster[3];                             /* Attack, Speed and Charge in both, levels less one; -1 each game's own */
	bool safe;                                 /* MegaMan kept from harm: his collision data's BN6_OBJ_INVULNERABLE every frame */
	CustomScript scripts[MAX_SCRIPTS];
	int nscripts;
	Key keys[MAX_KEYS];
	int nkeys;
	uint32_t seed[2];
	Poke pokes[MAX_POKES];
	int npokes;
	XdTwin twins[MAX_TWINS];
	int ntwins;
	char out[256];
	bool bn5, bn6, follow, replay;
} Spec;

/* ---- the spec ---- */

static uint32_t buttons(const char *s) {
	static const struct { const char *name; uint32_t key; } names[] = {
		{ "A", KEY_A }, { "B", KEY_B }, { "SELECT", KEY_SELECT }, { "START", KEY_START }, { "RIGHT", KEY_RIGHT },
		{ "LEFT", KEY_LEFT }, { "UP", KEY_UP }, { "DOWN", KEY_DOWN }, { "R", KEY_R }, { "L", KEY_L },
	};
	char buf[64];
	snprintf(buf, sizeof buf, "%s", s);
	uint32_t k = 0;
	char *save = NULL;
	for (char *t = strtok_r(buf, "+", &save); t; t = strtok_r(NULL, "+", &save))
		for (size_t i = 0; i < sizeof names / sizeof *names; ++i)
			if (!strcmp(t, names[i].name)) k |= names[i].key;
	return k;
}

/* "4F@62,01@51": enemy ids at column, row */
static bool foes_parse(Spec *s, char *v) {
	char *save = NULL;
	for (char *t = strtok_r(v, ",", &save); t; t = strtok_r(NULL, ",", &save)) {
		unsigned id;
		int c, r;
		if (sscanf(t, "%x@%1d%1d", &id, &c, &r) != 3 || c < 1 || c > 6 || r < 1 || r > 3 || s->nfoes >= MAX_FOES) return false;
		s->foes[s->nfoes].id = (int)id;
		s->foes[s->nfoes++].panel = r << 4 | c;
	}
	return s->nfoes > 0;
}

/* "150:UP:5,200:B+UP:2" */
static bool keys_parse(Spec *s, char *v) {
	char *save = NULL;
	for (char *t = strtok_r(v, ",", &save); t; t = strtok_r(NULL, ",", &save)) {
		unsigned at, n;
		char k[48];
		if (sscanf(t, "%u:%47[^:]:%u", &at, k, &n) != 3 || s->nkeys >= MAX_KEYS) return false;
		s->keys[s->nkeys++] = (Key){ at, n, buttons(k) };
	}
	return true;
}

/* "0000,000C,0236": hex numbers into out (at most max); how many, -1 where one is not */
static int hex_list(char *v, int *out, int max) {
	int n = 0;
	char *save = NULL;
	for (char *t = strtok_r(v, ",", &save); t; t = strtok_r(NULL, ",", &save)) {
		char *end;
		long x = strtol(t, &end, 16);
		if (*end || x < 0 || x > 0xFFFF || n >= max) return -1;
		out[n++] = (int)x;
	}
	return n;
}

/* "1:A.4/-.8/RIGHT.4": a Custom screen's keys, its screen and its steps */
static bool script_parse(Spec *s, int game, char *v) {
	if (s->nscripts >= MAX_SCRIPTS) return false;
	CustomScript *c = &s->scripts[s->nscripts];
	char *colon = strchr(v, ':');
	if (!colon || (c->screen = atoi(v)) < 1) return false;
	c->game = game;
	c->nsteps = 0;
	char *save = NULL;
	for (char *t = strtok_r(colon + 1, "/", &save); t; t = strtok_r(NULL, "/", &save)) {
		char *dot = strrchr(t, '.');
		if (!dot || c->nsteps >= MAX_SCRIPT_STEPS || atoi(dot + 1) < 1) return false;
		*dot = 0;
		c->steps[c->nsteps++] = (ScriptStep){ strcmp(t, "-") ? buttons(t) : 0, atoi(dot + 1) };
	}
	++s->nscripts;
	return c->nsteps > 0;
}

/* MegaMan's setup in each game: event flags, a folder, his buster, kept from harm, the Custom screens' keys; -1 where
 * `name` is none of them */
static int megaman_parse(Spec *s, const char *name, char *v) {
	int k = name[strlen(name) - 1] == '5' ? 0 : 1, got[BN6_FOLDER_ENTRIES];
	if (!strcmp(name, "flags5") || !strcmp(name, "flags6")) return (s->nflags[k] = hex_list(v, s->flags[k], MAX_FLAGS)) > 0;
	if (!strcmp(name, "folder5") || !strcmp(name, "folder6")) {
		int n = hex_list(v, got, BN6_FOLDER_ENTRIES);
		for (int i = 0; n > 0 && i < BN6_FOLDER_ENTRIES; ++i) s->folder[k][i] = (uint16_t)got[i % n];
		return (s->nfolder[k] = n) > 0;
	}
	if (!strcmp(name, "deck5") || !strcmp(name, "deck6")) {
		int n = hex_list(v, got, BN6_FOLDER_ENTRIES);
		for (int i = 0; n > 0 && i < BN6_FOLDER_ENTRIES; ++i) s->deck[k][i] = (uint16_t)got[i % n];
		return (s->ndeck[k] = n > 0 ? BN6_FOLDER_ENTRIES : 0) > 0;
	}
	if (!strcmp(name, "souls6")) return (s->souls6 = (unsigned)strtoul(v, NULL, 16)) != 0;
	if (!strcmp(name, "unitebreak")) return (s->unitebreak = atoi(v)) > 0;
	if (!strcmp(name, "cust5") || !strcmp(name, "cust6")) return script_parse(s, 5 + k, v);
	if (!strcmp(name, "buster"))
		return sscanf(v, "%d:%d:%d", &s->buster[0], &s->buster[1], &s->buster[2]) == 3 && s->buster[0] >= 0 && s->buster[0] <= BN5_BUSTER_MAX &&
			s->buster[1] >= 0 && s->buster[1] <= BN5_BUSTER_MAX && s->buster[2] >= 0 && s->buster[2] <= BN5_BUSTER_MAX;
	if (!strcmp(name, "safe")) return (s->safe = atoi(v) != 0), 1;
	return -1;
}

/* the spec's numbers: MegaMan's panel and HP, the frames, the seed; -1 where `name` is none of them */
static int numbers_parse(Spec *s, const char *name, const char *v) {
	unsigned a, b;
	if (!strcmp(name, "hp")) return (s->hp = atoi(v)) > 0 && s->hp <= 9999;
	if (!strcmp(name, "frames")) return (s->frames = atoi(v)) > 0;
	bool mm = !strcmp(name, "mm"), seed = !strcmp(name, "seed");
	if (!mm && !seed) return -1;
	if ((mm ? sscanf(v, "%1u%1u", &a, &b) : sscanf(v, "%x:%x", &a, &b)) != 2) return 0;
	if (seed) {
		s->seed[0] = a;
		s->seed[1] = b;
		return 1;
	}
	if (a < 1 || a > 3 || b < 1 || b > 3) return 0;
	s->mm = (int)(b << 4 | a);
	return 1;
}

/* "080050EC:08004C98:1A[:0]": a BN6 routine whose body is BN5's, a draw within it keyed by its BN5 twin (rng=replay), and
 * the word BN5's draws there take where not BN6's */
static bool twin_parse(Spec *s, const char *v) {
	unsigned a, b, n;
	int word = -1;
	int got = sscanf(v, "%x:%x:%x:%d", &a, &b, &n, &word);
	if (s->ntwins >= MAX_TWINS || got < 3 || !n || word < -1 || word > 1) return false;
	s->twins[s->ntwins++] = (XdTwin){ a, b, n, word };
	return true;
}

/* the comparator's own test's breaks: a poke, a call aimed elsewhere; and
 * a spec's change made in both games' copies alike (both=, an address of
 * BN5's: an element given, to put the engine's handling of it under
 * test); -1 where `name` is none of them */
static int breaks_parse(Spec *s, const char *name, const char *v) {
	unsigned a, b;
	int n = 2;
	bool call = !strcmp(name, "call"), both = !strcmp(name, "both");
	if (!call && !both && strcmp(name, "poke")) return -1;
	if (s->npokes >= MAX_POKES) return 0;
	if (call ? sscanf(v, "%x:%x", &a, &b) != 2 : sscanf(v, "%x:%x:%d", &a, &b, &n) < 2 || (n != 1 && n != 2 && n != 4) || (both && n == 1)) return 0;
	s->pokes[s->npokes++] = (Poke){ a, b, call ? 0 : n, both };
	return 1;
}

static bool item_parse(Spec *s, const char *name, char *v) {
	int r = numbers_parse(s, name, v);
	if (r < 0) r = breaks_parse(s, name, v);
	if (r < 0) r = megaman_parse(s, name, v);
	if (r >= 0) return r > 0;
	if (!strcmp(name, "foes")) return foes_parse(s, v);
	if (!strcmp(name, "keys")) return keys_parse(s, v);
	if (!strcmp(name, "out")) return snprintf(s->out, sizeof s->out, "%s", v) > 0;
	if (!strcmp(name, "rng")) return (s->follow = !strcmp(v, "follow")) || (s->replay = !strcmp(v, "replay"));
	if (!strcmp(name, "twin")) return twin_parse(s, v);
	if (!strcmp(name, "mm_element")) return (s->mm_element = atoi(v)) >= 0 && s->mm_element <= 4;
	if (!strcmp(name, "field")) {
		unsigned f, f6 = 0x100;
		int got = sscanf(v, "%x:%x", &f, &f6);
		s->field = got >= 1 && f < 0x100 ? (int)f : -1;
		s->field6 = got == 2 && f6 < 0x100 ? (int)f6 : -1;
		return s->field >= 0 && (got == 1 || s->field6 >= 0);
	}
	if (!strcmp(name, "game")) {
		s->bn5 = !strcmp(v, "bn5");
		s->bn6 = !strcmp(v, "bn6");
		return s->bn5 || s->bn6;
	}
	return false;
}

static bool spec_parse(const char *text, Spec *s) {
	*s = (Spec){ .mm = 0x22, .hp = 100, .frames = 1200, .field6 = -1, .mm_element = -1, .seed = { 0x12345678u, 0x9ABCDEF0u }, .bn5 = true, .bn6 = true,
		.buster = { -1, -1, -1 } };
	snprintf(s->out, sizeof s->out, "xdiff");
	char buf[4096];
	snprintf(buf, sizeof buf, "%s", text);
	char *save = NULL;
	for (char *t = strtok_r(buf, ";", &save); t; t = strtok_r(NULL, ";", &save)) {
		char *eq = strchr(t, '=');
		if (!eq) { fprintf(stderr, "xdiff: %s is no item (NAME=VALUE)\n", t); return false; }
		*eq = 0;
		if (!item_parse(s, t, eq + 1)) { fprintf(stderr, "xdiff: %s=%s is not understood\n", t, eq + 1); return false; }
	}
	return s->nfoes > 0;
}

/* ---- a game's battle as the trace reads it ---- */

typedef struct {
	int game;   /* 5, 6 */
	uint32_t gamestate, state, window, rng[2];
	uint32_t t1, t3, t4;
	int nt1;
	uint32_t panels, panel_size, list, list_end, coll_size, mplays;
	uint32_t navi, navi_size, used, pending, side;   /* MegaMan's player block's places (BN6_BATTLE_NAVI's or BN5's) */
	uint32_t souls, unite, deck;   /* ... BN6_SOULS (0: none, BN5's), the Custom screen's element 0xB, the battle's deck */
	const uint8_t *rom;
	uint32_t songs;   /* (its song table, a ROM offset) */
	uint8_t (*rd8)(uint32_t);
	uint32_t (*rd32)(uint32_t);
	void (*wr32)(uint32_t, uint32_t);
	void (*wr16)(uint32_t, uint16_t);
	void (*frame)(uint32_t);
} View;

static uint32_t rom32(const uint8_t *rom, uint32_t a) { return get32(rom + (a - 0x08000000u)); }
static bool in_ewram(uint32_t a) { return a >= BN6_EWRAM && a < BN6_EWRAM_END; }

/* ---- a game's battle as it runs: its view, its trace, the sounds its music players began ---- */

typedef struct { uint32_t info, header, clock; } Player;
#define BEGUN_MAX 32

typedef struct {
	View v;
	XdLog *log;   /* its core's hooks' events (xdiff_events.h) */
	FILE *out;
	Player players[BN6_MPLAY_COUNT];
	uint16_t begun[BEGUN_MAX][2];   /* (player, song) begun since the last frame traced */
	int nbegun, custom, screen, traced;   /* (custom: the frames since this Custom screen's window first read open, -1 none; screen: its number) */
	uint32_t frame, last;           /* the core's frames run; the timer after the last */
	bool seeded, fought, done, decked;
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

static int song_of(const View *v, uint32_t header) {
	for (uint32_t n = 0; n < SONGS; ++n)
		if (get32(v->rom + v->songs + 8 * n) == header) return (int)n;
	/* (a copy of another game's song in one of BN6's slots, xsong.h: by the slot's own number there) */
	for (uint32_t n = 0; v->game == 6 && n < SONGS; ++n)
		if (v->rd32(0x08000000u + v->songs + 8 * n) == header) {
			int copy = xsong_copy_of((int)n, NULL);
			return copy >= 0 ? copy : (int)n;
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
	static XvMoved m[XV_MOVED_MAX];
	int n = g->v.game == 6 ? xvirus_moved(m, XV_MOVED_MAX) : 0;
	w32(g, (uint32_t)n);
	for (int i = 0; i < n; ++i) { w32(g, m[i].from); w32(g, m[i].to); w32(g, m[i].len); }
	w32(g, PLAYER_SIZE);
}

/* MegaMan's side's player block: his battle NaviStats, what was used this battle, his pending form */
static void player_out(Side *g) {
	const View *v = &g->v;
	uint32_t side = v->rd8(v->side) & 1;
	bytes_out(g, v->navi + side * v->navi_size, v->navi_size, BN6_BATTLE_NAVI_SIZE);
	w32(g, v->rd32(v->used));
	bytes_out(g, v->pending + side * BN6_CROSS_PENDING_SIZE, BN6_CROSS_PENDING_SIZE, BN6_CROSS_PENDING_SIZE);
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
	if (v->game != 6 || v->rd8(o + BN6_T1_TYPE) != xvirus_dispatch_type(kind)) return 0;
	return v->rd8(xvirus_dispatch_side(kind) + v->rd8(o + BN6_T1_SLOT));
}

static void objects_out(Side *g, const uint8_t *s1, int n1, const uint8_t *s3, int n3, const uint8_t *s4, int n4) {
	const View *v = &g->v;
	for (int i = 0; i < n1; ++i) {
		uint32_t o = v->t1 + s1[i] * BN6_T1_SIZE, ai = v->rd32(o + BN6_T1_AI_DATA), coll = v->rd32(o + BN6_T1_COLLISION);
		w8s(g, s1[i], in_ewram(ai), in_ewram(coll), dispatched(v, 1, o));
		bytes_out(g, o - BN6_OBJECT_NODE, BN6_T1_SIZE, BN6_T1_SIZE);
		bytes_out(g, in_ewram(ai) ? ai : 0, BN6_AI_DATA_SIZE, BN6_AI_DATA_SIZE);
		bytes_out(g, in_ewram(coll) ? coll : 0, v->coll_size, BN6_COLL_SIZE);
	}
	for (int i = 0; i < n3; ++i) {
		uint32_t o = v->t3 + s3[i] * BN6_T1_SIZE, coll = v->rd32(o + BN6_T1_COLLISION);
		w8s(g, s3[i], 0, in_ewram(coll), dispatched(v, 3, o));
		bytes_out(g, o - BN6_OBJECT_NODE, BN6_T1_SIZE, BN6_T1_SIZE);
		bytes_out(g, in_ewram(coll) ? coll : 0, v->coll_size, BN6_COLL_SIZE);
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
	w8s(g, v->rd8(v->gamestate), v->rd8(v->state + 1), (uint8_t)n1, (uint8_t)n3);
	w8s(g, (uint8_t)n4, (uint8_t)ns, (uint8_t)no, 0);
	w32(g, v->rd32(v->rng[0]));
	w32(g, v->rd32(v->rng[1]));
	w32(g, (uint32_t)nev);
	player_out(g);
	objects_out(g, s1, n1, s3, n3, s4, n4);
	for (uint32_t p = 0; p < PANELS; ++p) bytes_out(g, v->panels + p * v->panel_size, v->panel_size, BN5_PANEL_SIZE);
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
	return f < 4 ? KEY_START : f >= 12 && f < 16 ? KEY_A : 0;
}

/* (safe=1) MegaMan's collision data invulnerable, before each frame of the battle */
static void keep_safe(const View *v) {
	uint32_t coll = v->rd32(v->t1 + BN6_T1_COLLISION);
	if (in_ewram(coll)) v->wr32(coll + BN6_COLL_FLAGS, v->rd32(coll + BN6_COLL_FLAGS) | BN6_OBJ_INVULNERABLE);
}

/* (deck5=, deck6=) the battle's deck in the spec's order, as the battle's
 * phase first reads the Custom screen's (each game makes and shuffles its
 * deck as the first turn begins, both by then) and before its window opens
 * and takes a pick: each game's hands its own chips in the same places,
 * whatever each game's shuffle drew (the hand's chips are its elements'
 * pointers into the deck, BN6_ELEMENT_DECK) */
static void deck_put(Side *g, const Spec *s) {
	int k = g->v.game == 5 ? 0 : 1;
	g->decked = true;
	for (int i = 0; i < s->ndeck[k]; ++i) g->v.wr16(g->v.deck + 2 * (uint32_t)i, s->deck[k][i]);
}

/* One frame of a battle: each Custom screen passed by its keys (custom_keys)
 * from its window's opening; the fight's keys by its timer; both RNG words
 * written after its first frame; a new value of its timer while it fights
 * traced. Done after `frames` of them, its end, or its frames' budget.
 * True where this frame was traced. */
static bool step(Side *g, const Spec *s) {
	const View *v = &g->v;
	if (g->done) return false;
	uint8_t sub = v->rd8(v->gamestate), phase = v->rd8(v->state + 1);
	uint32_t t = v->rd32(v->state + 0x40), keys = 0;
	bool battle = sub == BN6_SUB_BATTLE, custom = battle && phase == BN6_PHASE_CUSTOM;
	/* (a screen counted as its window first reads open: the phase passes the Custom screen's for a frame as a battle begins) */
	if (!custom) g->custom = -1;
	else if (g->custom >= 0 || v->rd8(v->window) >= BN6_WINDOW_OPEN) {
		if (g->custom < 0) ++g->screen;
		keys = custom_keys(s, v->game, g->screen, ++g->custom);
	}
	if (battle && phase == BN6_PHASE_FIGHT) keys |= keys_at(s, t);
	if (battle && s->safe) keep_safe(v);
	if (custom && !g->decked) deck_put(g, s);
	if (g->log) {
		xd_frame(g->log, g->frame, t, battle, g->seeded);
		xd_setup(g->log, sub == BN6_SUB_BATTLE_INIT);
	}
	v->frame(keys);
	++g->frame;
	sub = v->rd8(v->gamestate);
	t = v->rd32(v->state + 0x40);
	bool fighting = sub == BN6_SUB_BATTLE && v->rd8(v->state + 1) == BN6_PHASE_FIGHT, traced = false;
	if (fighting && !g->seeded) {
		v->wr32(v->rng[0], s->seed[0]);
		v->wr32(v->rng[1], s->seed[1]);
		if (s->mm_element >= 0) element_set(v, s->mm_element);
		g->seeded = true;
		g->nbegun = 0;   /* (the fight's sounds from here) */
	}
	sounds(g);
	if (fighting && t != g->last) {
		frame_out(g);
		g->fought = traced = true;
		g->done = ++g->traced >= s->frames;
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
 * reference's first (the replay finds its draws made); with `follow`, BN6's
 * RNG words written from BN5's once both have traced a timer value
 * (rng=follow) */
static void fight(Side *a, Side *b, const Spec *s) {
	while (a && !a->seeded && !a->done) step(a, s);
	while (b && !b->seeded && !b->done) step(b, s);
	uint32_t followed = 0;
	while ((a && !a->done) || (b && !b->done)) {
		bool both = a && b && !a->done && !b->done, go_a = a && (!both || a->last <= b->last), go_b = b && (!both || b->last <= a->last);
		bool ta = go_a && step(a, s), tb = go_b && step(b, s);
		if (s->follow && both && (ta || tb) && a->last == b->last && a->last != followed) {
			followed = a->last;
			b->v.wr32(b->v.rng[0], a->v.rd32(a->v.rng[0]));
			b->v.wr32(b->v.rng[1], a->v.rd32(a->v.rng[1]));
		}
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
	for (int k = 0; k < xvirus_bn5_count; ++k) {
		uint32_t veneer = veneer_of(xvirus_bn5[k], at);
		if (!veneer) continue;
		emu_write(veneer + 8, b, 4);
		printf("xdiff: %s's call to %08x (veneer %08x) aimed at %08x\n", xvirus_bn5[k]->name, (unsigned)at, (unsigned)veneer, (unsigned)v);
	}
}

/* the comparator's own test: bytes written into BN6's copy after the
 * install (an address of BN5's moved, to its copy), or a call's veneer
 * aimed at another routine; those in RAM (`ram`) once the game has booted,
 * over what it copied there (IWRAM's routines, from their image) */
static void pokes_put(const Spec *s, bool ram) {
	static XvMoved m[XV_MOVED_MAX];
	int n = xvirus_moved(m, XV_MOVED_MAX);
	for (int i = 0; i < s->npokes; ++i) {
		const Poke *p = &s->pokes[i];
		uint32_t at = p->at;
		uint8_t b[4];
		if ((p->size && (at >> 24 == 2 || at >> 24 == 3)) != ram) continue;   /* (EWRAM, IWRAM) */
		if (!p->size) {
			call_aim(p->at, p->v);
			continue;
		}
		/* (the family's own ranges, moved last, first: BN6's tables grown are among the ranges too) */
		for (int k = n - 1; k >= 0; --k)
			if (at >= m[k].from && at < m[k].from + m[k].len) { at = at - m[k].from + m[k].to; break; }
		put32(b, p->v);
		emu_write(at, b, (size_t)p->size);
		printf("xdiff: poke %08x (%08x) = %0*x\n", (unsigned)p->at, (unsigned)at, 2 * p->size, (unsigned)p->v);
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

/* A folder written as its game's first, each chip owned (its mark its key XOR the game's), with no Regular chip or
 * TagChips: the folders' pointer and marks' at their eToolkit places (game's own read and write) */
typedef struct {
	uint32_t toolkit, folders, marks, keys, key_xor, regular, tags;   /* (the NaviStats' Folder1Reg, its two TagChips; 0 none) */
	uint8_t (*rd8)(uint32_t);
	uint32_t (*rd32)(uint32_t);
	void (*wr8)(uint32_t, uint8_t);
	void (*wr16)(uint32_t, uint16_t);
} FolderPlaces;

static void folder_put(const FolderPlaces *p, const uint16_t *folder) {
	uint32_t folders = p->rd32(p->toolkit + p->folders), marks = p->rd32(p->toolkit + p->marks);
	if (!in_ewram(folders) || !in_ewram(marks)) { fprintf(stderr, "xdiff: no folder in this game's state\n"); return; }
	for (uint32_t i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		uint32_t id = folder[i] & 0x1FF;
		p->wr16(folders + 2 * i, folder[i]);
		p->wr8(marks + id, (uint8_t)(p->rd8(p->keys + id) ^ p->key_xor));
	}
	p->wr8(p->regular, 0xFF);
	for (uint32_t i = 0; p->tags && i < 2; ++i) p->wr8(p->tags + i, 0xFF);
}

static void bn6_wr16(uint32_t a, uint16_t v) { const uint8_t b[2] = { (uint8_t)v, (uint8_t)(v >> 8) }; emu_write(a, b, 2); }

/* MegaMan in BN6 as the spec has him: its event flags, its folder, his buster */
static void subject_setup(const Spec *s) {
	for (int i = 0; i < s->nflags[1]; ++i) flag_set(s->flags[1][i]);
	static const FolderPlaces places = { BN6_TOOLKIT, BN6_TOOLKIT_CHIPS, BN6_TOOLKIT_CHIP_MARKS, BN6_CHIP_KEYS, BN6_CHIP_KEY_XOR, BN6_NAVI_FOLDER1_REG,
		BN6_NAVI_FOLDER1_TAG, emu_read8, emu_read32, emu_write8, bn6_wr16 };
	if (s->nfolder[1]) folder_put(&places, s->folder[1]);
	static const uint32_t buster[3] = { BN6_NAVI_ATTACK, BN6_NAVI_SPEED, BN6_NAVI_CHARGE };
	for (int i = 0; i < 3; ++i)
		if (s->buster[i] >= 0) emu_write8(buster[i], (uint8_t)s->buster[i]);
}

/* BN6's battle with the families installed, forced as the foes step forces one, the ids BN6's */
static bool subject(Side *g, const Spec *s) {
	if (!emu_init(R.data, ROM_SIZE)) return false;
	xnavi_install_core(XVIRUS_FAMILIES, XNAVI_BN5);
	/* (souls6=: the Souls in BN6's engine over the proxy form, MegaMan holding them) */
	if (s->souls6 && !(soulproxy_install(0) && soulunite_install(s->unitebreak))) {
		fprintf(stderr, "xdiff: the Souls could not be installed in BN6's engine\n");
		return false;
	}
	soulunite_held(s->souls6);
	pokes_put(s, false);
	emu_encounters_install();
	emu_boot();
	pokes_put(s, true);
	const uint8_t hp[4] = { (uint8_t)s->hp, (uint8_t)(s->hp >> 8), (uint8_t)s->hp, (uint8_t)(s->hp >> 8) };
	emu_write(BN6_NAVI_BASE_MAX_HP, hp, 2);
	emu_write(BN6_NAVI_HP, hp, 4);
	subject_setup(s);
	Encounter e;
	memset(&e, 0, sizeof e);
	e.player = s->mm;
	/* (BN5's field as BN6's: xfields.h, or the spec's own number for it) */
	e.field = s->field6 >= 0 ? s->field6 : s->field ? xfield_bn6(XROM_BN5_COLONEL_US, s->field) : 0;
	if (e.field < 0) {
		fprintf(stderr, "xdiff: BN5's field %#x is none of BN6's\n", s->field);
		return false;
	}
	for (int i = 0; i < s->nfoes; ++i) {
		int id = bn6_id(s->foes[i].id), p = s->foes[i].panel;
		e.foes[e.nfoes++] = (Foe){ FOE_VIRUS, 0, 0, (p & 0xF) - 1, (p >> 4) - 1, id };
		printf("xdiff: BN5's %#x at column %d, row %d: BN6's %#x\n", s->foes[i].id, p & 0xF, p >> 4, id);
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
		0, guest_read8, guest_read32, guest_wr8, guest_write16 };
	if (s->nfolder[0]) folder_put(&places, s->folder[0]);
	static const uint32_t buster[3] = { BN5_NAVI_ATTACK, BN5_NAVI_SPEED, BN5_NAVI_CHARGE };
	for (int i = 0; i < 3; ++i)
		if (s->buster[i] >= 0) guest_wr8(buster[i], (uint8_t)s->buster[i]);
}

/* BN5's own battle on the guest core, as BN5 has it (guest_dev_battle) */
static bool reference(Side *g, const Spec *s) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	if (!x->data || !guest_start(XROM_BN5_COLONEL_US)) return false;
	/* (BN5's random battles' record: its field, music, background the map's, side, options; MegaMan, then the foes) */
	uint8_t rec[0x80] = { (uint8_t)s->field, 0x14, 0x15, 0x00, 0xFF, 0x00, 0x38, 0x00, 0xE2, 0x49, 0x00, 0x00 };
	uint32_t len = 0x10;
	const uint8_t mm[4] = { 0x00, (uint8_t)s->mm, 0, 0 };
	memcpy(rec + len, mm, 4);
	len += 4;
	for (int i = 0; i < s->nfoes; ++i, len += 4) {
		const uint8_t f[4] = { 0x11, (uint8_t)s->foes[i].panel, (uint8_t)s->foes[i].id, (uint8_t)(s->foes[i].id >> 8) };
		memcpy(rec + len, f, 4);
	}
	rec[len++] = 0xF0;
	if (!guest_dev_battle(rec, len, s->hp, s->seed[0])) return false;
	/* (a spec's change in both games: BN5's copy at its own address) */
	for (int i = 0; i < s->npokes; ++i) {
		const Poke *p = &s->pokes[i];
		if (!p->both) continue;
		if (p->size == 4) guest_write32(p->at, p->v);
		else guest_write16(p->at, (uint16_t)p->v);
		printf("xdiff: both games' %08x = %0*x\n", (unsigned)p->at, 2 * p->size, (unsigned)p->v);
	}
	reference_setup(s);
	g->v = (View){ .game = 5, .gamestate = BN5_GAMESTATE, .state = BN5_BATTLE_STATE, .window = BN5_CUSTOM_WINDOW,
		.rng = { BN5_RNG_PRIMARY, BN5_RNG_SECONDARY }, .t1 = BN5_T1_OBJECTS, .t3 = BN5_T3_OBJECTS, .t4 = BN5_T4_OBJECTS, .nt1 = BN5_T1_COUNT,
		.panels = BN5_FIELD_PANELS, .panel_size = BN5_PANEL_SIZE, .list = BN5_OBJECT_LIST, .list_end = BN5_OBJECT_LIST_END,
		.coll_size = BN5_COLL_SIZE, .mplays = BN5_MPLAYS, .navi = BN5_BATTLE_NAVI, .navi_size = BN5_BATTLE_NAVI_SIZE, .used = BN5_SOULS_USED,
		.pending = BN5_SOUL_PENDING, .side = BN5_BATTLE_SIDE, .souls = 0, .unite = BN5_CUSTOM_UNITE, .deck = BN5_BATTLE_DECK,
		.rom = x->data, .songs = x->layout->song_table, .rd8 = guest_read8, .rd32 = guest_read32, .wr32 = guest_write32,
		.wr16 = guest_write16, .frame = bn5_frame };
	return open_out(g, s, "bn5");
}

/* Each core's hooks, once both battles are made (BN5's code is the families' as xvirus_moved has it), and the
 * subject's draws replayed from the reference's where the spec says so; false where they cannot be */
static bool logs_start(Side *a, Side *b, const Spec *s) {
	bool navis = false;
	for (int i = 0; i < s->nfoes; ++i) navis = navis || xnavi_bn6_id(s->foes[i].id) >= 0;
	a->log = a->out ? xd_log_start(5, navis) : NULL;
	b->log = b->out ? xd_log_start(6, navis) : NULL;
	events = malloc(EVENTS_OUT * sizeof *events);
	if ((a->out && !a->log) || (b->out && !b->log) || !events) return false;
	if (s->replay && a->log && b->log) xd_replay(b->log, a->log, s->twins, s->ntwins);
	return true;
}

/* what was traced, and what became of each core's draws */
static void summary(const Side *a, const Side *b, const Spec *s) {
	printf("xdiff: BN5's battle %d fight frames traced, BN6's %d%s\n", a->fought ? a->traced : -1, b->fought ? b->traced : -1,
		s->follow ? " (BN6's RNG words following BN5's)" : s->replay ? " (BN6's draws replayed from BN5's)" : "");
	for (int k = 0; k < 2; ++k) {
		const Side *g = k ? b : a;
		int n[XD_COUNTS];
		if (!g->log) continue;
		xd_counts(g->log, n);
		printf("xdiff: BN%d's hooks: %d draws replayed, %d unmatched, %d unmapped; %d events lost\n", 5 + k, n[0], n[1], n[2], n[3]);
	}
}

int xdiff_run(const char *text) {
	static Spec s;
	static Side a, b;
	if (!spec_parse(text, &s)) {
		fprintf(stderr, "xdiff: a spec names foes=ID@CR,... (docs/DEVTOOLS.md, The differential)\n");
		return 2;
	}
	if (!XR[XROM_BN5_COLONEL_US].data) {
		fprintf(stderr, "xdiff: BN5 Team Colonel's ROM is not beside BN6's\n");
		return 2;
	}
	bool ra = s.bn5 && reference(&a, &s), rb = s.bn6 && subject(&b, &s);
	if ((s.bn5 && !ra) || (s.bn6 && !rb)) {
		fprintf(stderr, "xdiff: a battle could not be begun\n");
		return 1;
	}
	if (!logs_start(&a, &b, &s)) return 1;
	fight(ra ? &a : NULL, rb ? &b : NULL, &s);
	if (a.out) fclose(a.out);
	if (b.out) fclose(b.out);
	summary(&a, &b, &s);
	return (ra && !a.fought) || (rb && !b.fought) ? 1 : 0;
}
