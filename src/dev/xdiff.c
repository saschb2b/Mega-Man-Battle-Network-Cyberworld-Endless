/* xdiff.h: the differential's two battles, each traced. A trace is
 * "XDF1", its version (1), the game (5 or 6), its collision data's and
 * panel's sizes, the ranges the install moved (xvirus.h's XvMoved: from,
 * to, len; BN6's alone) and then a record per fight frame (each new value
 * of the battle's timer while it fights): "FRAM", the core's frame, the
 * timer, the sub-mode, the phase, how many battle objects of the first,
 * third and fourth kinds are in play, sounds begun and objects in the run
 * order, both RNG words; then each object of the first kind (its slot,
 * whether its AI's data and collision data are read, its 0xD8 bytes from
 * its list node on (BN6_OBJECT_NODE before it), its AI's 0x100 and its
 * collision data's BN6_COLL_SIZE, BN5's padded), each of the third (its
 * slot, its bytes, its collision data), each of the fourth (its slot, its
 * 0xC8 bytes from its node), the 40 panels (BN5_PANEL_SIZE each,
 * BN6's padded), the run order (each object's address), and the sounds
 * begun (a music player and the song's number). All u32 little-endian but
 * the counts and slots. Local only: both ROMs, and what it writes is
 * their RAM (kept in .build, never in git). */
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
#include "foes.h"
#include "guest.h"
#include "rom.h"
#include "thumb_reloc.h"
#include "xfields.h"
#include "xvirus.h"
#include "xvirus_families.h"

#define MAX_KEYS  128
#define MAX_POKES 8
#define PANELS    40      /* the field's panels, 8 a row, rows 0-4 */
#define SONGS     0x400   /* the song table's entries looked through for a header */
#define OPENING   3000    /* the frames a battle may take to its fight */
#define MOVED_MAX 32

typedef struct { uint32_t t, n, keys; } Key;
typedef struct { uint32_t at, v; int size; } Poke;   /* size 0: a call's veneer, from BN6's routine `at` to `v` */

typedef struct {
	int nfoes;
	struct { int id, panel; } foes[MAX_FOES];
	int mm, hp, frames;
	int field, field6;   /* the battlefield, BN5's number, and BN6's (-1: as xfields.h installs it), 0 plain */
	Key keys[MAX_KEYS];
	int nkeys;
	uint32_t seed[2];
	Poke pokes[MAX_POKES];
	int npokes;
	char out[256];
	bool bn5, bn6, follow;
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

/* the comparator's own test's breaks: a poke, a call aimed elsewhere; -1 where `name` is neither */
static int breaks_parse(Spec *s, const char *name, const char *v) {
	unsigned a, b;
	int n = 2;
	bool call = !strcmp(name, "call");
	if (!call && strcmp(name, "poke")) return -1;
	if (s->npokes >= MAX_POKES) return 0;
	if (call ? sscanf(v, "%x:%x", &a, &b) != 2 : sscanf(v, "%x:%x:%d", &a, &b, &n) < 2 || (n != 2 && n != 4)) return 0;
	s->pokes[s->npokes++] = (Poke){ a, b, call ? 0 : n };
	return 1;
}

static bool item_parse(Spec *s, const char *name, char *v) {
	int r = numbers_parse(s, name, v);
	if (r < 0) r = breaks_parse(s, name, v);
	if (r >= 0) return r > 0;
	if (!strcmp(name, "foes")) return foes_parse(s, v);
	if (!strcmp(name, "keys")) return keys_parse(s, v);
	if (!strcmp(name, "out")) return snprintf(s->out, sizeof s->out, "%s", v) > 0;
	if (!strcmp(name, "rng")) return (s->follow = !strcmp(v, "follow"));
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
	*s = (Spec){ .mm = 0x22, .hp = 100, .frames = 1200, .field6 = -1, .seed = { 0x12345678u, 0x9ABCDEF0u }, .bn5 = true, .bn6 = true };
	snprintf(s->out, sizeof s->out, "xdiff");
	char buf[2048];
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
	const uint8_t *rom;
	uint32_t songs;   /* (its song table, a ROM offset) */
	uint8_t (*rd8)(uint32_t);
	uint32_t (*rd32)(uint32_t);
	void (*wr32)(uint32_t, uint32_t);
	void (*frame)(uint32_t);
} View;

static uint32_t rom32(const uint8_t *rom, uint32_t a) { return get32(rom + (a - 0x08000000u)); }
static bool in_ewram(uint32_t a) { return a >= BN6_EWRAM && a < BN6_EWRAM_END; }

/* ---- a game's battle as it runs: its view, its trace, the sounds its music players began ---- */

typedef struct { uint32_t info, header, clock; } Player;
#define BEGUN_MAX 32

typedef struct {
	View v;
	FILE *out;
	Player players[BN6_MPLAY_COUNT];
	uint16_t begun[BEGUN_MAX][2];   /* (player, song) begun since the last frame traced */
	int nbegun, custom, traced;
	uint32_t frame, last;           /* the core's frames run; the timer after the last */
	bool seeded, fought, done;
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
	w32(g, 1);
	w32(g, (uint32_t)g->v.game);
	w32(g, g->v.coll_size);
	w32(g, g->v.panel_size);
	XvMoved m[MOVED_MAX];
	int n = g->v.game == 6 ? xvirus_moved(m, MOVED_MAX) : 0;
	w32(g, (uint32_t)n);
	for (int i = 0; i < n; ++i) { w32(g, m[i].from); w32(g, m[i].to); w32(g, m[i].len); }
}

/* the objects in the order they run, their addresses into `order`: how many */
static int run_order(const View *v, uint32_t *order, int max) {
	int n = 0;
	for (uint32_t node = v->rd32(v->list + 4); n < max && in_ewram(node) && node != v->list_end; node = v->rd32(node + 4))
		order[n++] = node + BN6_OBJECT_NODE;
	return n;
}

static void objects_out(Side *g, const uint8_t *s1, int n1, const uint8_t *s3, int n3, const uint8_t *s4, int n4) {
	const View *v = &g->v;
	for (int i = 0; i < n1; ++i) {
		uint32_t o = v->t1 + s1[i] * BN6_T1_SIZE, ai = v->rd32(o + BN6_T1_AI_DATA), coll = v->rd32(o + BN6_T1_COLLISION);
		w8s(g, s1[i], in_ewram(ai), in_ewram(coll), 0);
		bytes_out(g, o - BN6_OBJECT_NODE, BN6_T1_SIZE, BN6_T1_SIZE);
		bytes_out(g, in_ewram(ai) ? ai : 0, BN6_AI_DATA_SIZE, BN6_AI_DATA_SIZE);
		bytes_out(g, in_ewram(coll) ? coll : 0, v->coll_size, BN6_COLL_SIZE);
	}
	for (int i = 0; i < n3; ++i) {
		uint32_t o = v->t3 + s3[i] * BN6_T1_SIZE, coll = v->rd32(o + BN6_T1_COLLISION);
		w8s(g, s3[i], 0, in_ewram(coll), 0);
		bytes_out(g, o - BN6_OBJECT_NODE, BN6_T1_SIZE, BN6_T1_SIZE);
		bytes_out(g, in_ewram(coll) ? coll : 0, v->coll_size, BN6_COLL_SIZE);
	}
	for (int i = 0; i < n4; ++i) {
		w8s(g, s4[i], 0, 0, 0);
		bytes_out(g, v->t4 + s4[i] * BN6_T4_SIZE - BN6_OBJECT_NODE, BN6_T4_SIZE, BN6_T4_SIZE);
	}
}

static void frame_out(Side *g) {
	const View *v = &g->v;
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
	objects_out(g, s1, n1, s3, n3, s4, n4);
	for (uint32_t p = 0; p < PANELS; ++p) bytes_out(g, v->panels + p * v->panel_size, v->panel_size, BN5_PANEL_SIZE);
	for (int i = 0; i < no; ++i) w32(g, order[i]);
	for (int i = 0; i < ns; ++i) { w32(g, g->begun[i][0]); w32(g, g->begun[i][1]); }
	g->nbegun = 0;
}

/* ---- a battle fought ---- */

static uint32_t keys_at(const Spec *s, uint32_t t) {
	uint32_t k = 0;
	for (int i = 0; i < s->nkeys; ++i)
		if (t >= s->keys[i].t && t < s->keys[i].t + s->keys[i].n) k |= s->keys[i].keys;
	return k;
}

/* One frame of a battle: the Custom screen passed with START, then A (OK,
 * no chip), its window open; the fight's keys by its timer; both RNG words
 * written after its first frame; a new value of its timer while it fights
 * traced. Done after `frames` of them, its end, or its frames' budget.
 * True where this frame was traced. */
static bool step(Side *g, const Spec *s) {
	const View *v = &g->v;
	if (g->done) return false;
	uint8_t sub = v->rd8(v->gamestate), phase = v->rd8(v->state + 1);
	uint32_t t = v->rd32(v->state + 0x40), keys = 0;
	bool battle = sub == BN6_SUB_BATTLE;
	if (battle && phase == BN6_PHASE_CUSTOM && v->rd8(v->window) >= BN6_WINDOW_OPEN) {
		++g->custom;
		keys = g->custom < 4 ? KEY_START : g->custom >= 12 && g->custom < 16 ? KEY_A : 0;
	} else g->custom = -1;
	if (battle && phase == BN6_PHASE_FIGHT) keys |= keys_at(s, t);
	v->frame(keys);
	++g->frame;
	sub = v->rd8(v->gamestate);
	t = v->rd32(v->state + 0x40);
	bool fighting = sub == BN6_SUB_BATTLE && v->rd8(v->state + 1) == BN6_PHASE_FIGHT, traced = false;
	if (fighting && !g->seeded) {
		v->wr32(v->rng[0], s->seed[0]);
		v->wr32(v->rng[1], s->seed[1]);
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

/* Both battles, each to its fight's first frame, then a frame of each in
 * turn; with `follow`, BN6's RNG words written from BN5's after each frame
 * both traced at the same time (rng=follow) */
static void fight(Side *a, Side *b, const Spec *s) {
	while (a && !a->seeded && !a->done) step(a, s);
	while (b && !b->seeded && !b->done) step(b, s);
	while ((a && !a->done) || (b && !b->done)) {
		bool ta = a && step(a, s), tb = b && step(b, s);
		if (s->follow && ta && tb && a->last == b->last) {
			b->v.wr32(b->v.rng[0], a->v.rd32(a->v.rng[0]));
			b->v.wr32(b->v.rng[1], a->v.rd32(a->v.rng[1]));
		}
	}
}

/* ---- the two games ---- */

static void bn6_frame(uint32_t keys) { emu_frame(keys); }
static void bn5_frame(uint32_t keys) { guest_frame_quiet(keys); }

/* BN5's id as BN6 has it: the moved family's, else the same */
static int bn6_id(int id) {
	const XVirusFamily *f = &xvirus_cangard;
	for (int v = 0; v < XV_VERSIONS; ++v)
		if (f->names[v] == id) return f->first_id + v;
	return id;
}

/* the veneer of the moved family's calls on to BN6's routine `to`, its bus address; 0 none */
static uint32_t veneer_of(uint32_t to) {
	const XVirusFamily *f = &xvirus_cangard;
	for (uint32_t at = f->home + XV_HOME_VENEERS; at + THUMB_VENEER_SIZE <= f->home + XV_HOME_SIZE; at += THUMB_VENEER_SIZE) {
		uint8_t v[THUMB_VENEER_SIZE];
		uint32_t t;
		for (uint32_t i = 0; i < THUMB_VENEER_SIZE; ++i) v[i] = emu_read8(at + i);
		if (!thumb_veneer_target(v, &t)) return 0;
		if (t == (to & ~1u)) return at;
	}
	return 0;
}

/* the comparator's own test: bytes written into BN6's copy after the
 * install (an address of BN5's moved, to its copy), or a call's veneer
 * aimed at another routine */
static void pokes_put(const Spec *s) {
	XvMoved m[MOVED_MAX];
	int n = xvirus_moved(m, MOVED_MAX);
	for (int i = 0; i < s->npokes; ++i) {
		const Poke *p = &s->pokes[i];
		uint32_t at = p->at;
		uint8_t b[4];
		if (!p->size) {
			at = veneer_of(p->at);
			put32(b, p->v | 1);
			if (at) emu_write(at + 8, b, 4);
			printf("xdiff: the call to %08x (veneer %08x) aimed at %08x\n", (unsigned)p->at, (unsigned)at, (unsigned)p->v);
			continue;
		}
		for (int k = 0; k < n; ++k)
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
	if (g->out) {
		header_out(g);
		players_init(g);
	}
	return g->out != NULL;
}

/* BN6's battle with the families installed, forced as the foes step forces one, the ids BN6's */
static bool subject(Side *g, const Spec *s) {
	if (!emu_init(R.data, ROM_SIZE)) return false;
	xvirus_install(XVIRUS_FAMILIES);
	pokes_put(s);
	emu_encounters_install();
	emu_boot();
	const uint8_t hp[4] = { (uint8_t)s->hp, (uint8_t)(s->hp >> 8), (uint8_t)s->hp, (uint8_t)(s->hp >> 8) };
	emu_write(BN6_NAVI_BASE_MAX_HP, hp, 2);
	emu_write(BN6_NAVI_HP, hp, 4);
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
		.coll_size = BN6_COLL_SIZE, .mplays = BN6_MPLAYS, .rom = R.data, .songs = R.layout->song_table,
		.rd8 = emu_read8, .rd32 = emu_read32, .wr32 = emu_write32, .frame = bn6_frame };
	return open_out(g, s, "bn6");
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
	g->v = (View){ .game = 5, .gamestate = BN5_GAMESTATE, .state = BN5_BATTLE_STATE, .window = BN5_CUSTOM_WINDOW,
		.rng = { BN5_RNG_PRIMARY, BN5_RNG_SECONDARY }, .t1 = BN5_T1_OBJECTS, .t3 = BN5_T3_OBJECTS, .t4 = BN5_T4_OBJECTS, .nt1 = BN5_T1_COUNT,
		.panels = BN5_FIELD_PANELS, .panel_size = BN5_PANEL_SIZE, .list = BN5_OBJECT_LIST, .list_end = BN5_OBJECT_LIST_END,
		.coll_size = BN5_COLL_SIZE, .mplays = BN5_MPLAYS, .rom = x->data, .songs = x->layout->song_table,
		.rd8 = guest_read8, .rd32 = guest_read32, .wr32 = guest_write32, .frame = bn5_frame };
	return open_out(g, s, "bn5");
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
	fight(ra ? &a : NULL, rb ? &b : NULL, &s);
	if (a.out) fclose(a.out);
	if (b.out) fclose(b.out);
	printf("xdiff: BN5's battle %d fight frames traced, BN6's %d%s\n", ra && a.fought ? a.traced : -1, rb && b.fought ? b.traced : -1,
		s.follow ? " (BN6's RNG words following BN5's)" : "");
	return (ra && !a.fought) || (rb && !b.fought) ? 1 : 0;
}
