/* encounter_tags.h: the families, the acts' bands and the areas' own
 * battles as tab-separated lines, a kind first:
 *   band  act depth lo hi cap easy_hi vlo vhi stage
 *   fam   net ai src_ai summoned       (net 0 BN6, 1 BN5, 2 BN4; ai BN6's AI index)
 *   ver   net ai version id hp hit elem elem2 weak2 flags traits(16 hex) drops(20 x 4 hex)
 *   kin   net ai role front middle back stage bound visits   (a net's records' placements)
 *   pool  act biome...
 *   form  biome field weight navi id:panel:type:ai:version...
 *   xarea area xrom like held native
 *   xform area layer field ai:version:id:col:row...
 * Numbers alone: no name, text or art of the games'. */
#include "encounter_tags.h"

#include <stdio.h>

#include "data.h"
#include "emu.h"
#include "formations.h"
#include "loot.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "visitor_families.h"
#include "xbattles.h"
#include "xinstall.h"
#include "xvirus.h"
#include "xvirus_bn4.h"

#define BUS 0x08000000u

static void bands(FILE *out) {
	for (int act = 0; act < PACING_ACTS; ++act) {
		int depth = act < 6 ? act * 3 + 2 : 19, vlo, vhi;
		PacingBand b = pacing_band(depth, false, false), easy = pacing_band(depth, false, true);
		pacing_virus_versions(depth, false, &vlo, &vhi);
		fprintf(out, "band\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", act, depth, b.lo, b.hi, b.cap, easy.hi, vlo, vhi, loot_stage_allowed(depth));
	}
	for (int act = 0; act < 4; ++act) {
		uint8_t pool[PACING_AREA_POOL];
		int n = pacing_area_pool(act, pool);
		fprintf(out, "pool\t%d", act);
		for (int i = 0; i < n; ++i) fprintf(out, "\t%d", pool[i]);
		fprintf(out, "\n");
	}
}

/* (a family's versions: BN6's own by its ids table, V1 to SP and its two
 * rare ones; a moved one's six from its first id) */
static void family(FILE *out, int net, int ai, int src_ai, int first, bool summoned) {
	fprintf(out, "fam\t%d\t%d\t%d\t%d\n", net, ai, src_ai, summoned);
	for (int v = 0; v < 6; ++v) {
		int id = net == VISIT_BN6 ? enemy_id(0, ai, v) : first + v, hp, hit;
		uint8_t s[6], t[8];
		if (id < 0 || !enemy_stats(id, &hp, &hit) || !enemy_raw(id, s, t)) continue;
		fprintf(out, "ver\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t", net, ai, v, id, hp, hit, s[1] >> 4, t[5], t[6], s[3]);
		for (int k = 0; k < 8; ++k) fprintf(out, "%02x", t[k]);
		fprintf(out, "\t");
		for (int k = 0; k < ENEMY_DROPS; ++k) fprintf(out, "%04x", enemy_drop(id, k));
		fprintf(out, "\n");
	}
}

static void families(FILE *out) {
	for (int ai = 1; ai <= 29; ++ai)
		if (enemy_id(0, ai, 0) >= 0) family(out, VISIT_BN6, ai, ai, 0, false);
	for (int ai = 1; XR[XROM_BN5_COLONEL_US].data && ai < 64; ++ai) {
		const XVirusFamily *f = xvirus_installed(XROM_BN5_COLONEL_US, ai);
		if (f) family(out, VISIT_BN5, f->ai, f->src_ai, f->first_id, f->summoned);
	}
	for (int i = 0; XR[XROM_BN4_RED_SUN_US].data && i < xvirus_bn4_count; ++i) {
		const XVirusFamily *f = xvirus_installed(XROM_BN4_RED_SUN_US, xvirus_bn4[i]->src_ai);
		if (f) family(out, VISIT_BN4, f->ai, f->src_ai, f->first_id, f->summoned);
	}
	for (int net = 0; net < VISIT_NETS; ++net) {
		const VisitorKin *k;
		int n = visitor_kin(net, &k);
		for (int i = 0; i < n; ++i)
			fprintf(out, "kin\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", net, k[i].family, k[i].role, k[i].cols[0], k[i].cols[1], k[i].cols[2],
				k[i].stage, k[i].bound, k[i].visits);
	}
}

/* (BN6's areas' own random battles, each formation as its tables list it) */
static void forms(FILE *out) {
	for (int b = 0; b < BIOME_COUNT; ++b) {
		const Formation *list;
		int n = formations_of(b, &list);
		for (int i = 0; i < n; ++i) {
			fprintf(out, "form\t%d\t%d\t%d\t%d", b, list[i].battlefield, list[i].weight, list[i].navi);
			for (int k = 0; k < list[i].n; ++k) {
				const uint8_t *row = enemy_triple(list[i].ent[k].id);
				fprintf(out, "\t%d:%d:%d:%d:%d", list[i].ent[k].id, list[i].ent[k].panel, row ? row[1] : -1, row ? row[2] : -1, row ? row[0] : -1);
			}
			fprintf(out, "\n");
		}
	}
}

/* (one of another game's records as BN6's engine fights it, its viruses) */
static void xform(FILE *out, int k, int m, int xrom, uint32_t r) {
	Encounter e;
	int what;
	if (XR[xrom].data[r - BUS + 7] || xbattle_translate(xrom, r, (XScale){ 0, 0 }, &e, &what) != XB_FIGHTABLE) return;
	fprintf(out, "xform\t%d\t%d\t%d", k, m, e.field);
	for (int f = 0; f < e.nfoes; ++f)
		if (e.foes[f].kind == FOE_VIRUS) fprintf(out, "\t%d:%d:%d:%d:%d", e.foes[f].family, e.foes[f].version, e.foes[f].id, e.foes[f].col, e.foes[f].row);
	fprintf(out, "\n");
}

/* (the other games' areas a run takes, and their records) */
static void xforms(FILE *out) {
	for (int k = 0; k < XAREAS_MAX; ++k) {
		const NetAreaDef *a = net_area_def(NET_AREAS + k);
		if (!a || a->xrom <= 0) continue;
		fprintf(out, "xarea\t%d\t%d\t%d\t%d\t%d\n", k, a->xrom - 1, a->like, a->held, xterritory_native(NET_AREAS + k));
		for (int m = 0; XR[a->xrom - 1].data && m < 3; ++m)
			for (int i = 0, n = xrecords_count(a->xrom - 1, a->xbattles[m][0], a->xbattles[m][1]); i < n; ++i)
				xform(out, k, m, a->xrom - 1, xrecords_at(a->xrom - 1, a->xbattles[m][0], a->xbattles[m][1], i));
	}
}

int encounter_tags_run(const char *path) {
	if (XR[XROM_BN5_COLONEL_US].data || XR[XROM_BN4_RED_SUN_US].data) {
		if (!emu_init(R.data, ROM_SIZE)) return 1;
		xinstall_core(XVIRUS_FAMILIES, XNAVI_BN5);
	}
	FILE *out = fopen(path, "w");
	if (!out) return 1;
	bands(out);
	families(out);
	forms(out);
	xforms(out);
	bool ok = !ferror(out);
	return fclose(out) == 0 && ok ? 0 : 1;
}
