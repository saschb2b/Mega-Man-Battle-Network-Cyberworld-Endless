/* visitor_families.h: each net's virus families as their own game sets
 * them. */
#include "visitor_families.h"

#include <string.h>

#include "bn4.h"
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
#include "xvirus_bn4.h"

#define BUS 0x08000000u
#define XROM XROM_BN5_COLONEL_US
#define XROM4 XROM_BN4_RED_SUN_US

/* What the records set, by family (BN6's AI index): its placements in the
 * enemy's columns (first, middle, last), those on a plain or cracked
 * panel, and its stage (3 none yet) */
typedef struct {
	int16_t cols[VISITOR_KIN_MAX][3];
	int16_t plain[VISITOR_KIN_MAX];
	int8_t stage[VISITOR_KIN_MAX];
} Tally;

static struct {
	const uint8_t *rom6, *rom5, *rom4;
	bool made;
	int n[VISIT_NETS];
	VisitorKin kin[VISIT_NETS][VISITOR_KIN_MAX];
	int16_t first_id[VISITOR_KIN_MAX];   /* (BN5's and BN4's moved families') their V1's id, by family */
	int8_t net[VISITOR_KIN_MAX];         /* ... their net (VISIT_BN6 every other's), by family */
	uint8_t type_ai[256];                /* (BN4's moved families) the family of each of BN4's object types, 0 none */
	int8_t role[VISIT_NETS][VISITOR_KIN_MAX];   /* every family's role in each net's records, by family */
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

/* ---- BN4's: its net's random records, each list of viruses once (its
 * story stages repeat them), where its ROM is read: a
 * family moved into the engine by its object type. Its stage by the
 * group: the Undernet's 1, Black Earth's 2, the rest of the net 0. Its
 * fields are not read (a record's +1 is not mapped), so every placement
 * counts as plain ---- */

static uint32_t word_at(const uint8_t *d, uint32_t a) {
	if (a < BUS || a - BUS + 4 > ROM_SIZE) return 0;
	const uint8_t *p = d + (a - BUS);
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

/* (a record's entities at `at`: each virus of a family moved in, once a
 * list) */
static void tally_entities(Tally *t, const uint8_t *d, uint32_t at, int stage, uint32_t *seen, int *nseen) {
	if (at < BUS || at - BUS + 8 * BN4_ENTITY_SIZE > ROM_SIZE) return;
	const uint8_t *e = d + (at - BUS);
	int n = 0;
	uint32_t h = 2166136261u;
	while (n < 8 && e[n * BN4_ENTITY_SIZE] != 0xFF) {
		for (int b = 0; b < BN4_ENTITY_SIZE; ++b) h = (h ^ e[n * BN4_ENTITY_SIZE + b]) * 16777619u;
		++n;
	}
	for (int s = 0; s < *nseen; ++s)
		if (seen[s] == h) return;
	if (*nseen >= 2048) return;
	seen[(*nseen)++] = h;
	for (int i = 0; i < n; ++i) {
		const uint8_t *x = e + i * BN4_ENTITY_SIZE;
		if (x[BN4_ENTITY_KIND] != 1 || x[0] >= BN4_ENEMY_COUNT) continue;
		int ai = K.type_ai[d[BN4_ENEMY_STATS - BUS + (uint32_t)x[0] * BN4_ENEMY_STAT_SIZE + BN4_ENEMY_TYPE]];
		if (ai) tally_one(t, ai, x[BN4_ENTITY_COL] - 1, 2, stage);
	}
}

static void tally_bn4(Tally *t) {
	const uint8_t *d = XR[XROM4].data;
	const XRomLayout *l = XR[XROM4].layout;
	static uint32_t seen[2048];
	int nseen = 0;
	uint32_t groups = d ? word_at(d, BN4_BATTLE_TABLES + 4) : 0;
	for (int g = 0; groups && l && g < l->net_groups; ++g) {
		/* (a group with no maps fights none of its records) */
		if (!word_at(d, BUS + l->map_table + 4u * (uint32_t)g)) continue;
		uint32_t list = word_at(d, groups + 4u * (uint32_t)g);
		for (int k = 0; k < 64; ++k) {
			uint32_t r = word_at(d, list + 4u * (uint32_t)k);
			if (r < BUS || r - BUS >= ROM_SIZE) break;
			int map = k % BN4_STORY_MAPS, stage = BN4_NET_GROUP + g != BN4_GROUP_DEEP ? 0 : map >= BN4_MAP_BLACK_EARTH && map < BN4_MAP_BLACK_EARTH + 2 ? 2 : 1;
			for (int i = 0; i < 64 && r - BUS + BN4_RECORD_SIZE <= ROM_SIZE && d[r - BUS] != 0xFF; ++i, r += BN4_RECORD_SIZE)
				tally_entities(t, d, word_at(d, r + BN4_RECORD_ENTITIES), stage, seen, &nseen);
		}
	}
}

/* (net `net`'s kin from its tally: every family it sets, the role of each;
 * those the other nets have none of by name may visit: BN4's moved
 * families are the ones neither has, docs/multirom/bn4.md) */
static void kin_make(int net, const Tally *t) {
	K.n[net] = 0;
	for (int f = 1; f < VISITOR_KIN_MAX; ++f) {
		const int16_t *c = t->cols[f];
		int placed = c[0] + c[1] + c[2];
		K.role[net][f] = placed ? (int8_t)visitor_role(c[0], c[1], c[2]) : VISITOR_FREE;
		if (!placed || K.n[net] >= VISITOR_KIN_MAX) continue;
		bool own = net == VISIT_BN6 ? f <= 29 && !xbattle_shared(XROM, f) : K.net[f] == net && K.first_id[f] > 0;
		VisitorKin *v = &K.kin[net][K.n[net]++];
		*v = (VisitorKin){ f, K.role[net][f], t->stage[f], t->plain[f] == 0, false, (int16_t)placed, { c[0], c[1], c[2] } };
		v->visits = own && !v->bound;
	}
}

/* (a family in BN6's engine: its V1's id and its net, by BN6's AI index) */
static void moved(const XVirusFamily *f, int net) {
	K.first_id[f->ai] = (int16_t)f->first_id;
	K.net[f->ai] = (int8_t)net;
	if (net == VISIT_BN4 && f->src_ai > 0 && f->src_ai < 256) K.type_ai[f->src_ai] = (uint8_t)f->ai;
}

/* (net `net`'s families in BN6's engine, BN5's or BN4's, each where its
 * ROM is read, how many) */
static int installed(int net) {
	int n = 0;
	if (net == VISIT_BN5)
		for (int ai = 1; XR[XROM].data && ai < 64; ++ai) {
			const XVirusFamily *f = xvirus_installed(XROM, ai);
			if (f && f->ai < VISITOR_KIN_MAX) { moved(f, net); ++n; }
		}
	else
		for (int i = 0; XR[XROM4].data && i < xvirus_bn4_count; ++i) {
			const XVirusFamily *f = xvirus_installed(XROM4, xvirus_bn4[i]->src_ai);
			/* (a family moved for a summon alone, BN6's by name: no kin of the net's, docs/multirom/bn4.md) */
			if (f && !f->summoned && f->ai < VISITOR_KIN_MAX) { moved(f, net); ++n; }
		}
	return n;
}

static void make(void) {
	K.rom6 = R.data;
	K.rom5 = XR[XROM].data;
	K.rom4 = XR[XROM4].data;
	K.made = true;
	memset(K.n, 0, sizeof K.n);
	memset(K.first_id, 0, sizeof K.first_id);
	memset(K.net, 0, sizeof K.net);
	memset(K.type_ai, 0, sizeof K.type_ai);
	memset(K.role, 0, sizeof K.role);
	if (!R.data) return;
	int bn5 = installed(VISIT_BN5), bn4 = installed(VISIT_BN4);
	static Tally t;
	/* (BN6's with BN6's ROM alone too: the encounter groups' pool takes its families' places from it, group_pool.h) */
	for (int net = 0; net < VISIT_NETS; ++net) {
		if ((net == VISIT_BN5 && !bn5) || (net == VISIT_BN4 && !bn4)) continue;
		memset(&t, 0, sizeof t);
		memset(t.stage, 3, sizeof t.stage);
		if (net == VISIT_BN6) tally_bn6(&t);
		else if (net == VISIT_BN5) tally_bn5(&t);
		else tally_bn4(&t);
		kin_make(net, &t);
	}
}

/* (made again where the ROMs changed, or the families came in since) */
static void ready(void) {
	if (!K.made || K.rom6 != R.data || K.rom5 != XR[XROM].data || K.rom4 != XR[XROM4].data ||
		(!K.n[VISIT_BN5] && K.rom5 && installed(VISIT_BN5)) || (!K.n[VISIT_BN4] && K.rom4 && installed(VISIT_BN4)))
		make();
}

int visitor_kin(int net, const VisitorKin **out) {
	ready();
	bool ok = net >= 0 && net < VISIT_NETS;
	*out = K.kin[ok ? net : VISIT_BN6];
	return ok ? K.n[net] : 0;
}

int visitor_net(int family) {
	ready();
	return family > 0 && family < VISITOR_KIN_MAX ? K.net[family] : VISIT_BN6;
}

int visitor_id(int net, int family, int version) {
	if (net == VISIT_BN6) return family >= 1 && family <= 29 && version >= 0 && version <= 3 ? enemy_id(0, family, version) : -1;
	ready();
	return net < VISIT_NETS && family >= 32 && family < VISITOR_KIN_MAX && K.net[family] == net && K.first_id[family] > 0 && version >= 0 && version < 6
		? K.first_id[family] + version : -1;
}
