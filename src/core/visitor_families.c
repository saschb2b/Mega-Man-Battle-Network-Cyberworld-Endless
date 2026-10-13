/* visitor_families.h: each net's virus families as their own game sets
 * them. */
#include "visitor_families.h"

#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "data.h"
#include "formations.h"
#include "loot.h"
#include "rom.h"
#include "run.h"
#include "visitors.h"
#include "xbattles.h"
#include "xvirus.h"

#define BUS 0x08000000u
#define XROM XROM_BN5_COLONEL_US

/* What the records set, by family (BN6's AI index): its placements in the
 * enemy's columns (first, middle, last), those on a plain or cracked
 * panel, and its stage (3 none yet) */
typedef struct {
	int16_t cols[VISITOR_KIN_MAX][3];
	int16_t plain[VISITOR_KIN_MAX];
	int8_t stage[VISITOR_KIN_MAX];
} Tally;

static struct {
	const uint8_t *rom6, *rom5;
	bool made;
	int n[2];
	VisitorKin kin[2][VISITOR_KIN_MAX];
	int16_t first_id[VISITOR_KIN_MAX];   /* (BN5's moved families') their V1's id, by family */
	int8_t role[2][VISITOR_KIN_MAX];     /* every family's role in each net's records, by family */
} K;

/* A field's panel type (its game's numbering) at column `col`, row `row`
 * (from 0): `table` its battlefields (12 bytes each, three u32 rows, a
 * nibble a column, the low first), in ROM file `d` */
static int field_panel(const uint8_t *d, uint32_t table, int field, int col, int row) {
	if (!d || field < 0 || field > 0xFF || col < 0 || col > 5 || row < 0 || row > 2) return -1;
	uint32_t at = table - BUS + 12u * (uint32_t)field + 4u * (uint32_t)row;
	if (at + 4 > ROM_SIZE) return -1;
	uint32_t w = (uint32_t)d[at] | (uint32_t)d[at + 1] << 8 | (uint32_t)d[at + 2] << 16 | (uint32_t)d[at + 3] << 24;
	return (int)(w >> (4 * col) & 15);
}

int visitor_panel(const Encounter *e, int col, int row) {
	const uint8_t *d5 = XR[XROM].data;
	if (e->xrecord) return d5 && e->xrecord >= BUS && e->xrecord - BUS < ROM_SIZE ? field_panel(d5, BN5_FIELDS, d5[e->xrecord - BUS], col, row) : -1;
	return e->field >= 0 && e->field < BN6_FIELD_COUNT ? field_panel(R.data, BN6_FIELDS, e->field, col, row) : -1;
}

/* (a placement of `family` at column `col` from 0, on panel type `panel`) */
static void tally_one(Tally *t, int family, int col, int panel, int stage) {
	if (family <= 0 || family >= VISITOR_KIN_MAX || col < 3 || col > 5) return;
	++t->cols[family][col - 3];
	t->plain[family] += panel == 2 || panel == 3;
	if (stage < t->stage[family]) t->stage[family] = (int8_t)stage;
}

/* ---- BN6's: its areas' random battles, each formation once ---- */

static uint32_t formation_hash(const Formation *f) {
	uint32_t h = 2166136261u ^ f->battlefield;
	for (int k = 0; k < f->n; ++k) h = (h ^ (uint32_t)(f->ent[k].panel << 16 | f->ent[k].id)) * 16777619u;
	return h;
}

static void tally_bn6(Tally *t) {
	static uint32_t seen[2048];
	int nseen = 0;
	for (int b = 0; b < BIOME_COUNT; ++b) {
		const Formation *list;
		int n = formations_of(b, &list);
		for (int i = 0; i < n; ++i) {
			uint32_t h = formation_hash(&list[i]);
			bool again = false;
			for (int s = 0; s < nseen && !again; ++s) again = seen[s] == h;
			if (again || nseen >= 2048) continue;
			seen[nseen++] = h;
			for (int k = 0; k < list[i].n; ++k) {
				const uint8_t *row = enemy_triple(list[i].ent[k].id);
				/* (a virus: the first entry of its family and version; the rest of
				 * type 0 are rocks, cubes and other objects, loot.c) */
				if (!row || row[1] != 0 || row[2] < 1 || row[2] > 29 || enemy_id(0, row[2], row[0]) != list[i].ent[k].id) continue;
				int col = (list[i].ent[k].panel & 15) - 1, r = (list[i].ent[k].panel >> 4) - 1;
				tally_one(t, row[2], col, field_panel(R.data, BN6_FIELDS, list[i].battlefield, col, r), loot_family_stage(row[2]));
			}
		}
	}
}

/* ---- BN5's: the records of the areas it lends a run, as BN6's engine
 * fights them (a family both games have by name BN6's) ---- */

static void tally_bn5(Tally *t) {
	const uint8_t *d = XR[XROM].data;
	for (int k = 0; d && k < XAREAS_MAX; ++k) {
		const NetAreaDef *a = net_area_def(NET_AREAS + k);
		if (!a || a->xrom - 1 != XROM) continue;
		int stage = a->like == BIOME_UNDERNET ? 1 : a->like == BIOME_GRAVEYARD ? 2 : 0;
		for (int m = 0; m < 3; ++m)
			for (int i = 0, n = xrecords_count(XROM, a->xbattles[m][0], a->xbattles[m][1]); i < n; ++i) {
				uint32_t r = xrecords_at(XROM, a->xbattles[m][0], a->xbattles[m][1], i);
				Encounter e;
				int what;
				if (d[r - BUS + 7] || xbattle_translate(XROM, r, (XScale){ 0, 0 }, &e, &what) != XB_FIGHTABLE) continue;
				for (int f = 0; f < e.nfoes; ++f)
					tally_one(t, e.foes[f].family, e.foes[f].col, field_panel(d, BN5_FIELDS, d[r - BUS], e.foes[f].col, e.foes[f].row), stage);
			}
	}
}

/* (net `net`'s kin from its tally: every family it sets, the role of each;
 * those the other net has none of by name may visit) */
static void kin_make(int net, const Tally *t) {
	K.n[net] = 0;
	for (int f = 1; f < VISITOR_KIN_MAX; ++f) {
		const int16_t *c = t->cols[f];
		int placed = c[0] + c[1] + c[2];
		K.role[net][f] = placed ? (int8_t)visitor_role(c[0], c[1], c[2]) : VISITOR_FREE;
		if (!placed || K.n[net] >= VISITOR_KIN_MAX) continue;
		bool own = net == VISIT_BN5 ? f >= 32 && K.first_id[f] > 0 : f <= 29 && !xbattle_shared(XROM, f);
		VisitorKin *v = &K.kin[net][K.n[net]++];
		*v = (VisitorKin){ f, K.role[net][f], t->stage[f], t->plain[f] == 0, false, (int16_t)placed };
		v->visits = own && !v->bound;
	}
}

/* (BN5's families in BN6's engine: their V1's ids, by BN6's AI index) */
static int installed(void) {
	int n = 0;
	memset(K.first_id, 0, sizeof K.first_id);
	for (int ai = 1; XR[XROM].data && ai < 64; ++ai) {
		const XVirusFamily *f = xvirus_installed(XROM, ai);
		if (f && f->ai < VISITOR_KIN_MAX) { K.first_id[f->ai] = (int16_t)f->first_id; ++n; }
	}
	return n;
}

static void make(void) {
	K.rom6 = R.data;
	K.rom5 = XR[XROM].data;
	K.made = true;
	K.n[0] = K.n[1] = 0;
	if (!R.data || !XR[XROM].data || !installed()) return;
	static Tally t;
	for (int net = 0; net < 2; ++net) {
		memset(&t, 0, sizeof t);
		memset(t.stage, 3, sizeof t.stage);
		if (net == VISIT_BN6) tally_bn6(&t);
		else tally_bn5(&t);
		kin_make(net, &t);
	}
}

/* (made again where the ROMs changed, or the families came in since) */
static void ready(void) {
	if (!K.made || K.rom6 != R.data || K.rom5 != XR[XROM].data || (!K.n[VISIT_BN5] && K.rom5 && installed())) make();
}

int visitor_kin(int net, const VisitorKin **out) {
	ready();
	*out = net == VISIT_BN5 ? K.kin[VISIT_BN5] : K.kin[VISIT_BN6];
	return net == VISIT_BN5 || net == VISIT_BN6 ? K.n[net] : 0;
}

int visitor_role_in(int net, int family) {
	ready();
	return (net == VISIT_BN5 || net == VISIT_BN6) && family > 0 && family < VISITOR_KIN_MAX ? K.role[net][family] : VISITOR_FREE;
}

int visitor_id(int net, int family, int version) {
	if (net == VISIT_BN6) return family >= 1 && family <= 29 && version >= 0 && version <= 3 ? enemy_id(0, family, version) : -1;
	ready();
	return family >= 32 && family < VISITOR_KIN_MAX && K.first_id[family] > 0 && version >= 0 && version < 6 ? K.first_id[family] + version : -1;
}
