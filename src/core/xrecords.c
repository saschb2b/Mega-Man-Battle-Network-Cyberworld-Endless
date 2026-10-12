/* Another game's battle records and Navis, read from its ROM file with no
 * core needed (xrecords.h; docs/MULTIROM.md): a net map's records, the
 * pool a battle is picked from to the act's band (the director's and the
 * pacing report's), a record's viruses taken up their versions, and its
 * Navis' records and stats as guardians (docs/BOSSES.md, BN5's Navis). */
#include "xrecords.h"

#include <stdbool.h>
#include <stdint.h>

#include "bn5.h"
#include "rom.h"

/* ---- its battle records, from the ROM file ---- */

static uint32_t rom32(const uint8_t *d, uint32_t a) {
	if (a < 0x08000000u || a - 0x08000000u + 4 > ROM_SIZE) return 0;
	const uint8_t *p = d + (a - 0x08000000u);
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

/* the first record of net map (group, number), 0 for none */
static uint32_t records_at(int xrom, int group, int number) {
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data || group < 0x80 || number < 0 || number > 0xFF) return 0;
	const uint8_t *d = XR[xrom].data;
	uint32_t groups = rom32(d, BN5_BATTLE_TABLES + 4), maps = groups ? rom32(d, groups + 4u * (uint32_t)(group - 0x80)) : 0;
	return maps ? rom32(d, maps + 4u * (uint32_t)number) : 0;
}

/* enemy `id`'s HP and damage, from its game's tables; false for a Navi or
 * none */
static bool xenemy(const uint8_t *d, int id, int *hp, int *damage) {
	if (id <= 0 || id >= 0x200) return false;
	const uint8_t *e = d + (BN5_ENEMY_IDS - 0x08000000u) + 3u * (uint32_t)id;
	if (e[1] != 0) return false;   /* (type 1 a Navi, 2 an object) */
	uint32_t types = rom32(d, BN5_ENEMY_STATS + 4u * e[1]), ais = types ? rom32(d, types + 4u * e[2]) : 0;
	if (!ais || ais - 0x08000000u + 6u * e[0] + 6 > ROM_SIZE) return false;
	const uint8_t *r = d + (ais - 0x08000000u) + 6u * e[0];
	*hp = (r[0] | r[1] << 8) & 0xFFF;
	*damage = (r[4] | r[5] << 8) & 0xFFF;
	return true;
}

/* BN5's viruses by family: the id of family `ai` (its ids table's AI
 * index, type 0) at version `v`, 0 for none */
static int family_id(const uint8_t *d, int ai, int v) {
	for (uint32_t i = 1; i < 0x200; ++i) {
		const uint8_t *e = d + (BN5_ENEMY_IDS - 0x08000000u) + 3u * i;
		if (e[1] == 0 && e[2] == ai && e[0] == v) return (int)i;
	}
	return 0;
}

const uint8_t *xrecords_triple(int xrom, int id) {
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data || id <= 0 || id >= 0x200) return NULL;
	return XR[xrom].data + (BN5_ENEMY_IDS - 0x08000000u) + 3u * (uint32_t)id;
}

int xrecords_family_id(int xrom, int ai, int v) {
	return xrom == XROM_BN5_COLONEL_US && XR[xrom].data ? family_id(XR[xrom].data, ai, v) : 0;
}

int xrecords_version_up(const uint8_t *d, int id, XScale sc) {
	if (sc.up <= 0 || id <= 0 || id >= 0x200) return id;
	const uint8_t *e = d + (BN5_ENEMY_IDS - 0x08000000u) + 3u * (uint32_t)id;
	if (e[1] != 0) return id;
	int top = sc.vcap > e[0] ? sc.vcap : e[0], v = e[0] + sc.up > top ? top : e[0] + sc.up;
	int r = v != e[0] ? family_id(d, e[2], v) : id;
	return r ? r : id;
}

int xrecords_foes_scaled(int xrom, uint32_t record, XScale sc, int *ids, int max) {
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data || record < 0x08000000u || record - 0x08000000u + 16 > ROM_SIZE) return 0;
	const uint8_t *d = XR[xrom].data;
	int n = 0;
	for (uint32_t a = rom32(d, record + 12), k = 0; a && k < 16 && a - 0x08000000u + 4 <= ROM_SIZE && d[a - 0x08000000u] != 0xF0; a += 4, ++k) {
		const uint8_t *e = d + (a - 0x08000000u);
		if (e[0] == 0x11 && n < max) ids[n++] = xrecords_version_up(d, e[2] | e[3] << 8, sc);
	}
	return n;
}

int xrecords_scaled(int xrom, uint32_t record, XScale sc, int *hp, int *damage) {
	*hp = *damage = 0;
	int ids[16] = { 0 }, n = xrecords_foes_scaled(xrom, record, sc, ids, 16);
	const uint8_t *d = n ? XR[xrom].data : NULL;
	if (!d || d[record - 0x08000000u + 7]) return 0;
	for (int i = 0; i < n; ++i) {
		int h = 0, dm = 0;
		if (!xenemy(d, ids[i], &h, &dm)) return 0;
		*hp += h;
		if (dm > *damage) *damage = dm;
	}
	return n;
}

int xrecords_count(int xrom, int group, int number) {
	uint32_t r = records_at(xrom, group, number);
	int n = 0;
	while (r && n < 64 && r - 0x08000000u + 16u * (uint32_t)(n + 1) <= ROM_SIZE && XR[xrom].data[r - 0x08000000u + 16u * (uint32_t)n] != 0xFF) ++n;
	return n;
}

uint32_t xrecords_at(int xrom, int group, int number, int i) {
	return i >= 0 && i < xrecords_count(xrom, group, number) ? records_at(xrom, group, number) + 16u * (uint32_t)i : 0;
}

/* record r's strength so scaled, as the pool measures it */
static int measured(const XPool *p, uint32_t r, XScale sc, int *hp, int *dmg) {
	return (p->measure ? p->measure : xrecords_scaled)(p->xrom, r, sc, hp, dmg);
}

/* The most versions up (0-3) at which record r stays under the band's cap
 * (its HP to `hi`, its strongest hit to `cap`), the fewest that make it so
 * strong (past the band's vcap a version more changes nothing), -1 none;
 * *hp its HP so */
static int scale_fit(const XPool *p, uint32_t r, int *hp) {
	const XBand *band = p->band;
	int best = -1, last = -1;
	for (int up = 0; up <= 3; ++up) {
		int h, dmg;
		if (!measured(p, r, (XScale){ up, band->vcap }, &h, &dmg) || h > band->hi || dmg > band->cap) break;
		if (h != last) { best = up; *hp = h; }
		last = h;
	}
	return best;
}

/* the records of `maps` the pool keeps under the band's cap, each at its
 * most versions up, and from its floor where `floor` (inside the band), in
 * their order, after the n in the pool's out */
static int records_fit(XPool *p, const uint8_t (*maps)[2], int nmaps, bool floor, int n) {
	for (int m = 0; m < nmaps && n < p->max; ++m)
		for (int i = 0, k = xrecords_count(p->xrom, maps[m][0], maps[m][1]); i < k && n < p->max; ++i) {
			uint32_t r = xrecords_at(p->xrom, maps[m][0], maps[m][1], i);
			bool had = p->keep && !p->keep(p->xrom, r);
			for (int j = 0; j < n && !had; ++j) had = p->out[j] == r;
			int hp, up = had ? -1 : scale_fit(p, r, &hp);
			if (up < 0 || (floor && hp < p->band->lo)) continue;
			p->out[n] = r;
			p->ups[n++] = (uint8_t)up;
		}
	return n;
}

#define POOL_LEAST 4   /* records a battle is picked from before other maps' join */

/* the maps of `area` (pass 0) or of every area game `xrom` lends (pass 1) */
static int area_maps(int xrom, const NetAreaDef *area, int pass, uint8_t (*maps)[2]) {
	int nm = 0;
	for (int k = 0; k < XAREAS_MAX; ++k) {
		const NetAreaDef *x = net_area_def(NET_AREAS + k);
		if (!x || x->xrom - 1 != xrom || (pass == 0 && x != area)) continue;
		for (int j = 0; j < 3; ++j)
			if (x->xbattles[j][0]) { maps[nm][0] = x->xbattles[j][0]; maps[nm][1] = x->xbattles[j][1]; ++nm; }
	}
	return nm;
}

/* (nothing under the cap: the map's own weakest the pool keeps, as it is) */
static int weakest(XPool *p) {
	uint32_t best = 0;
	int least = 1 << 30;
	for (int i = 0, k = xrecords_count(p->xrom, p->group, p->number); i < k; ++i) {
		uint32_t r = xrecords_at(p->xrom, p->group, p->number, i);
		int hp, dmg;
		if ((!p->keep || p->keep(p->xrom, r)) && measured(p, r, (XScale){ 0, 0 }, &hp, &dmg) && hp < least) { least = hp; best = r; }
	}
	if (!best) return 0;
	p->out[0] = best;
	p->ups[0] = 0;
	return 1;
}

int xrecords_pool(XPool *p) {
	const uint8_t own[1][2] = { { (uint8_t)p->group, (uint8_t)p->number } };
	uint8_t maps[3 * XAREAS_MAX][2];
	int n = 0;
	/* (inside the band, its floor too, first: the map's own, then, while
	 * fewer than four, its area's and its game's too; none inside at all,
	 * the same under the floor. ACDC Area's own at act 2 held two records
	 * inside, which would come every battle) */
	for (int floor = 1; floor >= 0 && !n; --floor) {
		n = records_fit(p, own, 1, floor, 0);
		for (int pass = 0; pass < 2 && n < POOL_LEAST; ++pass)
			n = records_fit(p, (const uint8_t (*)[2])maps, area_maps(p->xrom, p->area, pass, maps), floor, n);
	}
	p->fits = n > 0;
	return n || p->max < 1 ? n : weakest(p);
}

/* ---- its Navis, from the ROM file (docs/BOSSES.md, BN5's Navis) ---- */

int xrecords_navi_id(const uint8_t *d, int ai, int v) {
	for (uint32_t i = 1; i < 0x200; ++i) {
		const uint8_t *e = d + (BN5_ENEMY_IDS - 0x08000000u) + 3u * i;
		if (e[1] == BN5_NAVI_TYPE && e[2] == ai && e[0] == v) return (int)i;
	}
	return 0;
}

uint32_t xrecords_navi_stats(const uint8_t *d, int ai, int v) {
	uint32_t types = rom32(d, BN5_ENEMY_STATS + 4u * BN5_NAVI_TYPE), ais = types && ai > 0 && ai < 0x40 ? rom32(d, types + 4u * (uint32_t)ai) : 0;
	if (!ais || v < 0 || v > 5 || ais - 0x08000000u + 6u * (uint32_t)v + 6 > ROM_SIZE) return 0;
	return ais - 0x08000000u + 6u * (uint32_t)v;
}

int xrecords_navi_hp(int xrom, int ai, int version, int *element) {
	if (element) *element = 0;
	const uint8_t *d = xrom == XROM_BN5_COLONEL_US ? XR[xrom].data : NULL;
	uint32_t at = d ? xrecords_navi_stats(d, ai, version) : 0;
	if (!at) return -1;
	int w = d[at] | d[at + 1] << 8;
	if (element) *element = w >> 12 <= 4 ? w >> 12 : 0;
	return w & 0xFFF;
}

int xrecords_soul_kind(int xrom, int ai) {
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data || ai < 1 || ai > 12) return -1;
	return XR[xrom].data[BN5_SOUL_KINDS - 0x08000000u + (uint32_t)(ai - 1)];
}

/* Whether record `r` sets enemy `id` and no other */
static bool record_alone(const uint8_t *d, uint32_t r, int id) {
	if (r < 0x08000000u || r - 0x08000000u + 16 > ROM_SIZE) return false;
	int n = 0, found = 0;
	for (uint32_t a = rom32(d, r + 12), k = 0; a && k < 16 && a - 0x08000000u + 4 <= ROM_SIZE && d[a - 0x08000000u] != 0xF0; a += 4, ++k) {
		const uint8_t *e = d + (a - 0x08000000u);
		if (e[0] != 0x11) continue;
		++n;
		found += (e[2] | e[3] << 8) == id;
	}
	return n == 1 && found == 1;
}

uint32_t xrecords_navi_record(int xrom, int ai, int version) {
	const uint8_t *d = xrom == XROM_BN5_COLONEL_US ? XR[xrom].data : NULL;
	int id = d ? xrecords_navi_id(d, ai, version) : 0;
	if (!id) return 0;
	for (uint32_t i = 0; i < BN5_STORY_BATTLE_COUNT; ++i)
		if (record_alone(d, BN5_STORY_BATTLES + 16u * i, id)) return BN5_STORY_BATTLES + 16u * i;
	/* (his SP: the net maps' records, where he roams once its story is over) */
	for (int g = 0x80; g < 0x80 + XR[xrom].layout->net_groups; ++g)
		for (int m = 0; m < 16; ++m)
			for (int i = 0, k = xrecords_count(xrom, g, m); i < k; ++i)
				if (record_alone(d, xrecords_at(xrom, g, m, i), id)) return xrecords_at(xrom, g, m, i);
	return 0;
}
