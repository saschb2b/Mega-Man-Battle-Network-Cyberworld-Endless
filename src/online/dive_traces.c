/* dive_traces.h */
#include "dive_traces.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bn6_expression.h"
#include "bn6_fields.h"
#include "claims.h"
#include "data.h"
#include "director.h"
#include "dive_words.h"
#include "emu.h"
#include "npc.h"
#include "online_text.h"
#include "online_transport.h"
#include "platform.h"
#include "puppet.h"
#include "run.h"
#include "talk.h"
#include "text.h"

#define WRITER "dive traces"
#define FX_LIST 5            /* BN6's effects' sprite list */
#define SKULL_ANIM 3         /* BN6_FX_BURST's skull */
#define SPARK_ANIMS 2        /* BN6_FX_SPARKLE's small star bursts, 0 and 8, in turn */
#define SPARK_EVERY 90       /* frames between a sparkle's bursts */
#define SEEN_FAR 220         /* world units: a trace farther from MegaMan stays hidden */
#define REACH 22             /* ... and A reaches one this near the point before him */
#define SHIMMER_RISE 14      /* a sparkle over a Mystery Data, this far up */

static struct {
	DiveWay way;
	DiveMark marks[ONLINE_TRACES_MOST];
	DivePhrase phrases[ONLINE_TRACES_MOST];
	DiveFind finds[ONLINE_TRACES_MOST];
	DiveSplit splits[ONLINE_TRACES_MOST];
	int nmarks, nphrases, nfinds, nsplits;
	uint8_t opened[32];      /* a bit per Mystery Data index others opened */
	uint32_t waved[ONLINE_TRACES_MOST];
	int nwaved;
} T;

static struct {
	uint32_t skull, spark;   /* the puppets, 0 none */
	uint32_t seed;
	int depth;
	int frame;
} O;

/* ---- the objects, made as the layer is built ---- */

int dive_traces_more(NpcList *npcs, int room) {
	memset(&O, 0, sizeof O);
	int taken = 0;
	if (room >= 3 && npc_need_sprite(npcs, FX_LIST, BN6_FX_BURST) && (O.skull = puppet_make(FX_LIST, BN6_FX_BURST)) != 0) {
		npcs->script[npcs->n++] = O.skull;
		++taken;
	}
	if (room >= 5 && npc_need_sprite(npcs, FX_LIST, BN6_FX_SPARKLE) && (O.spark = puppet_make(FX_LIST, BN6_FX_SPARKLE)) != 0) {
		npcs->script[npcs->n++] = O.spark;
		++taken;
	}
	O.seed = run.layer_seed;
	O.depth = run.depth;
	return taken;
}

/* ---- what the server keeps ---- */

void dive_traces_room(DiveWay way) {
	memset(&T, 0, sizeof T);
	T.way = way;
}

/* (a list's next entry, the oldest out where it is full) */
static int slot(void *list, int *n, size_t size) {
	if (*n < ONLINE_TRACES_MOST) return (*n)++;
	memmove(list, (uint8_t *)list + size, size * (ONLINE_TRACES_MOST - 1));
	return ONLINE_TRACES_MOST - 1;
}

static void phrase(const DivePhrase *p) {
	for (int k = 0; k < T.nphrases; ++k)
		if (T.phrases[k].id == p->id) { T.phrases[k] = *p; return; }
	T.phrases[slot(T.phrases, &T.nphrases, sizeof T.phrases[0])] = *p;
}

static void gone(uint32_t id) {
	for (int k = 0; k < T.nphrases; ++k)
		if (T.phrases[k].id == id) {
			memmove(T.phrases + k, T.phrases + k + 1, sizeof T.phrases[0] * (size_t)(T.nphrases - k - 1));
			--T.nphrases;
			return;
		}
}

static void find(const DiveFind *f) {
	for (int k = 0; k < T.nfinds; ++k)
		if (T.finds[k].index == f->index) return;
	T.finds[slot(T.finds, &T.nfinds, sizeof T.finds[0])] = *f;
}

static void split(const DiveSplit *s) {
	for (int k = 0; k < T.nsplits; ++k)
		if (T.splits[k].index == s->index && T.splits[k].party == s->party) { T.splits[k] = *s; return; }
	T.splits[slot(T.splits, &T.nsplits, sizeof T.splits[0])] = *s;
}

void dive_traces_mail(const DiveTraces *t) {
	for (int k = 0; k < t->count; ++k) {
		switch (t->kind) {
		case ONLINE_TRACE_MARKS: T.marks[slot(T.marks, &T.nmarks, sizeof T.marks[0])] = t->u.marks[k]; break;
		case ONLINE_TRACE_PHRASES: phrase(&t->u.phrases[k]); break;
		case ONLINE_TRACE_FINDS: find(&t->u.finds[k]); break;
		case ONLINE_TRACE_OPENED: T.opened[t->u.opened[k] >> 3] |= (uint8_t)(1u << (t->u.opened[k] & 7)); break;
		case ONLINE_TRACE_SPLITS: split(&t->u.splits[k]); break;
		case ONLINE_TRACE_GONE: gone(t->u.gone[k]); break;
		default: break;
		}
	}
	fprintf(stderr, "dive: traces of kind %u: %u (marks %d, phrases %d, finds %d, splits %d)\n", t->kind, t->count, T.nmarks, T.nphrases, T.nfinds, T.nsplits);
}

const char *dive_traces_finder(unsigned index) {
	for (int k = 0; k < T.nfinds; ++k)
		if (T.finds[k].index == index) return T.finds[k].name;
	return "";
}

bool dive_traces_opened(unsigned index) { return index < 256 && (T.opened[index >> 3] >> (index & 7) & 1); }

unsigned dive_traces_best(unsigned index) {
	for (int k = 0; k < T.nsplits; ++k)
		if (T.splits[k].index == index && T.splits[k].party == 1) return T.splits[k].tenths;
	return 0;
}

/* ---- each frame: the nearest of each kind shown ---- */

static int far_from(int px, int py, int x, int y) { return abs(x - px) + abs(y - py); }

/* the nearest mark within sight, -1 none */
static int nearest_mark(int px, int py) {
	int best = -1, d = SEEN_FAR;
	for (int k = 0; k < T.nmarks; ++k) {
		int e = far_from(px, py, T.marks[k].x, T.marks[k].y);
		if (e < d) { d = e; best = k; }
	}
	return best;
}

/* the nearest sparkle within sight: a phrase (0..nphrases-1), else a
 * Mystery Data others opened that MegaMan has not (64 + its index); -1 none */
static int nearest_spark(int px, int py, int *x, int *y, int *z) {
	int best = -1, d = SEEN_FAR;
	for (int k = 0; k < T.nphrases; ++k) {
		int e = far_from(px, py, T.phrases[k].x, T.phrases[k].y);
		if (e < d) { d = e; best = k; *x = T.phrases[k].x; *y = T.phrases[k].y; *z = T.phrases[k].z; }
	}
	for (int k = 0; k < director_md_count(); ++k) {
		if (!dive_traces_opened((unsigned)k) || director_md_opened(k)) continue;
		int mx, my, mz;
		director_md_place(k, &mx, &my, &mz);
		int e = far_from(px, py, mx, my);
		if (e < d) { d = e; best = 64 + k; *x = mx; *y = my; *z = mz + SHIMMER_RISE; }
	}
	return best;
}

static void show(int px, int py) {
	int m = O.skull ? nearest_mark(px, py) : -1;
	if (O.skull) {
		puppet_show(O.skull, m >= 0);
		if (m >= 0) { puppet_anim(O.skull, SKULL_ANIM); puppet_place(O.skull, T.marks[m].x, T.marks[m].y, T.marks[m].z); }
	}
	if (!O.spark) return;
	int x = 0, y = 0, z = 0, s = nearest_spark(px, py, &x, &y, &z);
	puppet_show(O.spark, s >= 0);
	if (s < 0) return;
	puppet_anim(O.spark, O.frame / SPARK_EVERY % SPARK_ANIMS ? 8 : 0);
	puppet_place(O.spark, x, y, z);
}

/* ---- A on a trace ---- */

static bool waved(uint32_t id) {
	for (int k = 0; k < T.nwaved; ++k)
		if (T.waved[k] == id) return true;
	if (T.nwaved < ONLINE_TRACES_MOST) T.waved[T.nwaved++] = id;
	return false;
}

static void mark_said(const DiveMark *m) {
	char cause[24] = "";
	if (m->cause) enemy_name(m->cause, cause, sizeof cause);
	talk_start(dive_mark_words(m->name, cause, m->how == ONLINE_DIVE_WON), FACE_MEGAMAN);
	fprintf(stderr, "dive: A on %s's mark (cause %u)\n", m->name, m->cause);
}

static void phrase_said(const DivePhrase *p) {
	const char *words = p->phrase < ONLINE_PHRASE_COUNT ? online_phrases[p->phrase] : "...";
	talk_start(dive_phrase_words(p->name, words, p->waves), FACE_MEGAMAN);
	if (waved(p->id)) return;
	uint8_t m[ONLINE_DIVE_WAVE_BYTES];
	size_t n = dive_wire_wave(m, p->id);
	if (n && online_transport_send(m, n)) fprintf(stderr, "dive: waved at %s's phrase %u\n", p->name, p->id);
}

/* the trace before MegaMan, 0..nmarks-1 a mark, 64 + k a phrase; -1 none */
static int ahead(void) {
	int dx, dy;
	npc_probe(emu_read8(BN6_PLAYER_FACING) & 7, &dx, &dy);
	int px = bn6_player_x() + dx * 2, py = bn6_player_y() + dy * 2;
	for (int k = 0; k < T.nmarks; ++k)
		if (far_from(px, py, T.marks[k].x, T.marks[k].y) < REACH) return k;
	for (int k = 0; k < T.nphrases; ++k)
		if (far_from(px, py, T.phrases[k].x, T.phrases[k].y) < REACH) return 64 + k;
	return -1;
}

static void press(void) {
	if (!btn_pressed(BTN_A) || talk_busy() || emu_read8(BN6_CHATBOX) || !director_on_map()) return;
	int k = ahead();
	if (k >= 64) phrase_said(&T.phrases[k - 64]);
	else if (k >= 0) mark_said(&T.marks[k]);
}

void dive_traces_frame(void) {
	if (!director_on_layer()) return;
	press();
	/* (the objects only on the layer they were made on: another map took their memory since) */
	if ((!O.skull && !O.spark) || O.seed != run.layer_seed || O.depth != run.depth) return;
	++O.frame;
	claims_begin(WRITER);
	show(bn6_player_x(), bn6_player_y());
	claims_end();
}
