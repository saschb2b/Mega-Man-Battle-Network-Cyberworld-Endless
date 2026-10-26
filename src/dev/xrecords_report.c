/* xrecords_report.h: BN5's territories' records in BN6's engine, each as
 * xbattles.h translates it. */
#include "xrecords_report.h"

#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "data.h"
#include "emu.h"
#include "game.h"
#include "loot.h"
#include "pacing.h"
#include "rom.h"
#include "xbattles.h"
#include "xfields.h"
#include "xinstall.h"
#include "xvirus.h"

#define BUS 0x08000000u
#define XROM XROM_BN5_COLONEL_US

static const char *why_text(int why) {
	static const char *const text[] = { "fightable", "no record", "a story's battle (byte 7)", "a Navi of BN5's", "a virus family BN6 lacks",
		"a field with panels BN6 lacks", "an object BN6's records never set", "more than a battle of BN6's holds" };
	return why >= 0 && why < (int)(sizeof text / sizeof *text) ? text[why] : "?";
}

/* its game's name for enemy `id` ("" none) */
static const char *bn5_name(int id) {
	static char name[20];
	xrom_text(XROM, BN5_ENEMY_NAMES - BUS, id, name, sizeof name);
	return name;
}

/* The record's entities as BN5 has them: "Mettaur(01)@15 gem(05)@25" */
static void bn5_entities(FILE *out, uint32_t record) {
	const uint8_t *d = XR[XROM].data, *r = d + (record - BUS);
	uint32_t a = (uint32_t)r[12] | (uint32_t)r[13] << 8 | (uint32_t)r[14] << 16 | (uint32_t)r[15] << 24;
	for (uint32_t k = 0; k < 16 && a >= BUS && a - BUS + 4u * k + 4 <= ROM_SIZE && d[a - BUS + 4u * k] != 0xF0; ++k) {
		const uint8_t *e = d + (a - BUS) + 4u * k;
		int arg = e[2] | e[3] << 8;
		const uint8_t *t = e[0] == 0x11 ? xrecords_triple(XROM, arg) : NULL;
		if (t && t[1] == BN5_NAVI_TYPE) fprintf(out, " Navi(%02x)@%02x", arg, e[1]);
		else if (e[0] == 0x11) fprintf(out, " %s(%02x)@%02x", bn5_name(arg), arg, e[1]);
		else if (e[0] >> 4 == 2) fprintf(out, " gem(%02x)@%02x", arg, e[1]);
		else if (e[0] >> 4 == 3) fprintf(out, " rock@%02x", e[1]);
		else if (e[0]) fprintf(out, " kind%02x(%x)@%02x", e[0], arg, e[1]);
	}
}

/* ... and as BN6's battle has it */
static void bn6_entities(FILE *out, const Encounter *e) {
	for (int i = 0; i < e->nfoes; ++i) {
		char name[20];
		enemy_name(e->foes[i].id, name, sizeof name);
		fprintf(out, " %s(%02x)@%d%d", name, e->foes[i].id, e->foes[i].row + 1, e->foes[i].col + 1);
	}
	for (int i = 0; i < e->nobj; ++i) {
		int chance = e->obj[i].arg & 0xFF;
		if (e->obj[i].kind >> 4 != FIELD_GEM) fprintf(out, " rock@%02x", e->obj[i].panel);
		else if (chance >= 15) fprintf(out, " gem@%02x", e->obj[i].panel);
		else fprintf(out, " gem@%02x (there %d in 15)", e->obj[i].panel, chance);
	}
}

/* Every family a record names that BN6 cannot field (a bit by its AI
 * index in BN5), and whether it names a Navi */
static uint64_t lacking(uint32_t record, bool *navi) {
	const uint8_t *d = XR[XROM].data, *r = d + (record - BUS);
	uint32_t a = (uint32_t)r[12] | (uint32_t)r[13] << 8 | (uint32_t)r[14] << 16 | (uint32_t)r[15] << 24;
	uint64_t out = 0;
	for (uint32_t k = 0; k < 16 && a >= BUS && a - BUS + 4u * k + 4 <= ROM_SIZE && d[a - BUS + 4u * k] != 0xF0; ++k) {
		const uint8_t *e = d + (a - BUS) + 4u * k;
		int why, what;
		if (e[0] != 0x11 || xbattle_enemy_bn6(XROM, e[2] | e[3] << 8, &why, &what) >= 0) continue;
		if (why == XB_NAVI) *navi = true;
		else if (what < 64) out |= (uint64_t)1 << what;
	}
	return out;
}

/* the panel types BN5's field `f` holds that BN6 has none of (xfields.h) */
static void lacking_panels(FILE *out, int f) {
	const uint8_t *in = XR[XROM].data + (BN5_FIELDS - BUS) + (uint32_t)f * BN6_FIELD_SIZE;
	unsigned seen = 0;
	for (int i = 0; i < BN6_FIELD_SIZE; ++i) seen |= 1u << (in[i] & 15) | 1u << (in[i] >> 4);
	for (int t = 0; t < 16; ++t)
		if (seen >> t & 1 && xfield_type(t) < 0) fprintf(out, " (its panels of type %X%s)", t, t == BN5_PANEL_LAVA ? ", lava" : t == BN5_PANEL_SEA ? ", sea" : "");
}

typedef struct { int records, fightable, story; uint64_t families; int need[64], fields[256]; } Tally;

/* one record's line, tallied */
static void record_line(FILE *out, uint32_t record, Tally *t) {
	const uint8_t *r = XR[XROM].data + (record - BUS);
	int what = 0, why = xbattle_why(XROM, record, &what);
	fprintf(out, "    %08X field %02x", record, r[0]);
	bn5_entities(out, record);
	if (why == XB_STORY) { fprintf(out, "  | %s %02x\n", why_text(why), what); ++t->story; return; }
	++t->records;
	if (why == XB_FIGHTABLE) {
		Encounter e = { 0 };
		xbattle_translate(XROM, record, (XScale){ 0, 0 }, &e, &what);
		fprintf(out, "  | BN6 field %02x:", e.field);
		bn6_entities(out, &e);
		fprintf(out, "%s\n", e.gem_own ? " (its finds BN5's row)" : "");
		++t->fightable;
		return;
	}
	bool navi = false;
	uint64_t lack = lacking(record, &navi);
	fprintf(out, "  | not fightable: %s", why_text(why));
	if (why == XB_FIELD) {
		fprintf(out, " %02x", what);
		lacking_panels(out, what & 0xFF);
		++t->fields[what & 0xFF];
	}
	for (int ai = 1; ai < 64; ++ai)
		if (lack >> ai & 1) { fprintf(out, " %s", bn5_name(xrecords_family_id(XROM, ai, 0))); ++t->need[ai]; }
	t->families |= lack;
	fprintf(out, "%s\n", navi ? " (a Navi)" : "");
}

static void area_report(FILE *out, int area, Tally *all) {
	const NetAreaDef *a = net_area_def(area);
	Tally t;
	memset(&t, 0, sizeof t);
	fprintf(out, "\n== %s (area %d)%s\n", a->name, area, a->held ? ", held out of runs" : "");
	for (int m = 0; m < 3; ++m) {
		int g = a->xbattles[m][0], n = xrecords_count(XROM, g, a->xbattles[m][1]);
		fprintf(out, "  map %02x:%d, its act's %s layer: %d records\n", g, a->xbattles[m][1], m == 0 ? "first" : m == 1 ? "middle" : "guardian's", n);
		for (int i = 0; i < n; ++i) record_line(out, xrecords_at(XROM, g, a->xbattles[m][1], i), &t);
	}
	fprintf(out, "  %s: %d of %d random records fightable in BN6's engine%s", a->name, t.fightable, t.records,
		xterritory_native(area) ? ", native" : "");
	if (t.families) {
		fprintf(out, "; families it lacks (records each holds back):");
		for (int ai = 1; ai < 64; ++ai) if (t.need[ai]) fprintf(out, " %s %d", bn5_name(xrecords_family_id(XROM, ai, 0)), t.need[ai]);
	}
	for (int f = 0; f < 256; ++f) if (t.fields[f]) fprintf(out, "; field %02x (panels BN6 lacks) %d", f, t.fields[f]);
	fprintf(out, "\n");
	all->records += t.records;
	all->fightable += t.fightable;
	all->story += t.story;
}

/* ---- a territory's random battles as BN6's roll would pick them
 * (xterritory_encounter, issue #82's M3): each act's layers, a few rolls
 * each as battles fought one after another, and again from the same seed,
 * which must pick the same ---- */

#define ROLLS 6

/* the rolls of one layer: each pick's record and HP into `out`; how many over the band */
static int layer_rolls(int depth, int area, uint32_t *picks, int *hps) {
	PacingBand b = pacing_band(depth, false, false);
	int over = 0;
	LootMemory none = { -1, -1, 0, 0, 0 };
	loot_memory_set(&none);
	rng_seed(0xC0FFEEu ^ (uint32_t)(depth * 2654435761u) ^ (uint32_t)area);
	for (int r = 0; r < ROLLS; ++r) {
		Encounter e = { 0 };
		picks[r] = xterritory_encounter(depth, area, ENC_NORMAL, &e) ? e.xrecord : 0;
		hps[r] = 0;
		for (int i = 0; picks[r] && i < e.nfoes; ++i) {
			int h, d;
			if (enemy_stats(e.foes[i].id, &h, &d)) hps[r] += h;
		}
		over += picks[r] && hps[r] > b.hi;
		if (picks[r]) loot_battle_fought(&e);
	}
	return over;
}

static int rolls_report(FILE *out, int area) {
	int bad = 0;
	fprintf(out, "  its random battles as BN6's roll picks them (%d a layer, from one seed; the same seed again must pick the same):\n", ROLLS);
	for (int depth = 1; depth <= 18; ++depth) {
		uint32_t picks[ROLLS], again[ROLLS];
		int hps[ROLLS], hps2[ROLLS], over = layer_rolls(depth, area, picks, hps);
		layer_rolls(depth, area, again, hps2);
		bool same = !memcmp(picks, again, sizeof picks);
		PacingBand b = pacing_band(depth, false, false);
		fprintf(out, "    depth %2d band %3d-%3d:", depth, b.lo, b.hi);
		for (int r = 0; r < ROLLS; ++r) fprintf(out, picks[r] ? " %07X %3d" : " none", (unsigned)(picks[r] & 0xFFFFFFF), hps[r]);
		fprintf(out, "%s%s\n", over ? "  OVER" : "", same ? "" : "  NOT THE SAME AGAIN");
		bad += !same;
	}
	return bad;
}

int xrecords_report_run(const char *path) {
	if (!XR[XROM].data) {
		fprintf(stderr, "xrecords: BN5 Team Colonel's ROM is not beside BN6's\n");
		return 2;
	}
	/* (the families, fields and Navis a run's core has, the engine's objects they run before them: installed as it starts) */
	if (!emu_init(R.data, ROM_SIZE)) return 1;
	xinstall_core(XVIRUS_FAMILIES, XNAVI_BN5);
	FILE *out = fopen(path, "w");
	if (!out) return 1;
	fprintf(out, "BN5's battle records in BN6's own engine (issue #82, docs/multirom/one-net.md): each random record of\n"
		"its areas' maps as BN5 has it, then as BN6's engine fights it (xbattles.h) or why it cannot yet.\n"
		"A family both games have by name is BN6's (its version by tier), a family moved into the engine its own ids.\n"
		"Panels are row then column (BN5's records' own numbers: 0x15 row 1, column 5).\n");
	Tally all;
	memset(&all, 0, sizeof all);
	int unsteady = 0;
	for (int k = 0; k < XAREAS_MAX; ++k) {
		const NetAreaDef *a = net_area_def(NET_AREAS + k);
		if (!a || a->xrom - 1 != XROM) continue;
		area_report(out, NET_AREAS + k, &all);
		int fightable;
		xterritory_records(NET_AREAS + k, &fightable);
		if (fightable) unsteady += rolls_report(out, NET_AREAS + k);
	}
	fprintf(out, "\nAll: %d of %d random records fightable in BN6's engine (%d story records apart)\n", all.fightable, all.records, all.story);
	fclose(out);
	printf("xrecords: %d of %d random records fightable, written to %s%s\n", all.fightable, all.records, path,
		unsteady ? "; a roll picked otherwise from the same seed" : "");
	return unsteady ? 1 : 0;
}
