/* Another game's battles on a second core (guest.h). The guest is BN5: its
 * ROM, read beside BN6's (rom.c, XR), padded to EMU_ROM_SIZE as BN6's copy
 * is, in an mGBA core of its own. It boots once to a playable state (its
 * title, NEW GAME, and the intro pressed through to Lan's room) kept in
 * the data directory, and runs only while BN6's core waits, on the main
 * thread and without hooks: its encounter roll, patched in its ROM copy,
 * returns the record a battle is given, and the battle's end is read from
 * its game state (docs/ROM_DATA.md, BN5 guest battles). In the browser,
 * whose page runs one frame at a time, its boot runs a slice a frame
 * (guest_boot_slice). */
#include "guest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "data.h"
#include "debug.h"
#include "emu.h"
#include "rom.h"

/* (every build but the 3DS's, whose heap holds one ROM) */
#if !defined(__3DS__)
#define CW_GUEST 1
#endif

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

/* Enemy `id` scaled: `up` versions up, to the scale's vcap at most and
 * never below its own; itself where its family has no such version */
static int version_up(const uint8_t *d, int id, GuestScale sc) {
	if (sc.up <= 0 || id <= 0 || id >= 0x200) return id;
	const uint8_t *e = d + (BN5_ENEMY_IDS - 0x08000000u) + 3u * (uint32_t)id;
	if (e[1] != 0) return id;
	int top = sc.vcap > e[0] ? sc.vcap : e[0], v = e[0] + sc.up > top ? top : e[0] + sc.up;
	int r = v != e[0] ? family_id(d, e[2], v) : id;
	return r ? r : id;
}

int guest_record_foes_scaled(int xrom, uint32_t record, GuestScale sc, int *ids, int max) {
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data || record < 0x08000000u || record - 0x08000000u + 16 > ROM_SIZE) return 0;
	const uint8_t *d = XR[xrom].data;
	int n = 0;
	for (uint32_t a = rom32(d, record + 12), k = 0; a && k < 16 && a - 0x08000000u + 4 <= ROM_SIZE && d[a - 0x08000000u] != 0xF0; a += 4, ++k) {
		const uint8_t *e = d + (a - 0x08000000u);
		if (e[0] == 0x11 && n < max) ids[n++] = version_up(d, e[2] | e[3] << 8, sc);
	}
	return n;
}

int guest_record_scaled(int xrom, uint32_t record, GuestScale sc, int *hp, int *damage) {
	*hp = *damage = 0;
	int ids[16] = { 0 }, n = guest_record_foes_scaled(xrom, record, sc, ids, 16);
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

int guest_records(int xrom, int group, int number) {
	uint32_t r = records_at(xrom, group, number);
	int n = 0;
	while (r && n < 64 && r - 0x08000000u + 16u * (uint32_t)(n + 1) <= ROM_SIZE && XR[xrom].data[r - 0x08000000u + 16u * (uint32_t)n] != 0xFF) ++n;
	return n;
}

uint32_t guest_record(int xrom, int group, int number, int i) {
	return i >= 0 && i < guest_records(xrom, group, number) ? records_at(xrom, group, number) + 16u * (uint32_t)i : 0;
}

/* The most versions up (0-3) at which record r stays under the band's cap
 * (its HP to `hi`, its strongest hit to `cap`), the fewest that make it so
 * strong (past the band's vcap a version more changes nothing), -1 none;
 * *hp its HP so */
static int scale_fit(int xrom, uint32_t r, const GuestBand *band, int *hp) {
	int best = -1, last = -1;
	for (int up = 0; up <= 3; ++up) {
		int h, dmg;
		if (!guest_record_scaled(xrom, r, (GuestScale){ up, band->vcap }, &h, &dmg) || h > band->hi || dmg > band->cap) break;
		if (h != last) { best = up; *hp = h; }
		last = h;
	}
	return best;
}

/* the records of `maps` under the band's cap, each at its most versions
 * up, and from its floor where `floor` (inside the band), in their order,
 * after the n in out */
static int records_fit(int xrom, const uint8_t (*maps)[2], int nmaps, const GuestBand *band, bool floor, uint32_t *out, uint8_t *ups, int n,
                       int max) {
	for (int m = 0; m < nmaps && n < max; ++m)
		for (int i = 0, k = guest_records(xrom, maps[m][0], maps[m][1]); i < k && n < max; ++i) {
			uint32_t r = guest_record(xrom, maps[m][0], maps[m][1], i);
			bool had = false;
			for (int j = 0; j < n && !had; ++j) had = out[j] == r;
			int hp, up = had ? -1 : scale_fit(xrom, r, band, &hp);
			if (up < 0 || (floor && hp < band->lo)) continue;
			out[n] = r;
			ups[n++] = (uint8_t)up;
		}
	return n;
}

#define GUEST_POOL_LEAST 4   /* records a battle is picked from before other maps' join */

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

int guest_pool(int xrom, const NetAreaDef *area, int group, int number, const GuestBand *band, uint32_t *out, uint8_t *ups, int max, bool *fits) {
	const uint8_t own[1][2] = { { (uint8_t)group, (uint8_t)number } };
	uint8_t maps[3 * XAREAS_MAX][2];
	int n = 0;
	/* (inside the band, its floor too, first: the map's own, then, while
	 * fewer than four, its area's and its game's too; none inside at all,
	 * the same under the floor. ACDC Area's own at act 2 held two records
	 * inside, which would come every battle) */
	for (int floor = 1; floor >= 0 && !n; --floor) {
		n = records_fit(xrom, own, 1, band, floor, out, ups, 0, max);
		for (int pass = 0; pass < 2 && n < GUEST_POOL_LEAST; ++pass)
			n = records_fit(xrom, (const uint8_t (*)[2])maps, area_maps(xrom, area, pass, maps), band, floor, out, ups, n, max);
	}
	if (fits) *fits = n > 0;
	if (n || max < 1) return n;
	/* (nothing under the cap: the map's own weakest, as it is) */
	uint32_t best = 0;
	int least = 1 << 30;
	for (int i = 0, k = guest_records(xrom, group, number); i < k; ++i) {
		uint32_t r = guest_record(xrom, group, number, i);
		int hp, dmg;
		if (guest_record_scaled(xrom, r, (GuestScale){ 0, 0 }, &hp, &dmg) && hp < least) { least = hp; best = r; }
	}
	if (!best) return 0;
	out[0] = best;
	ups[0] = 0;
	return 1;
}

/* ---- its Navis, from the ROM file (docs/BOSSES.md, BN5's Navis) ---- */

/* the ids table's id of Navi `ai` at version `v`, 0 none */
static int navi_id(const uint8_t *d, int ai, int v) {
	for (uint32_t i = 1; i < 0x200; ++i) {
		const uint8_t *e = d + (BN5_ENEMY_IDS - 0x08000000u) + 3u * i;
		if (e[1] == BN5_NAVI_TYPE && e[2] == ai && e[0] == v) return (int)i;
	}
	return 0;
}

/* Navi `ai`'s stats row at version `v` (BN5_ENEMY_STATS: a u16 element <<
 * 12 | HP first), its ROM offset, 0 none */
static uint32_t navi_stats(const uint8_t *d, int ai, int v) {
	uint32_t types = rom32(d, BN5_ENEMY_STATS + 4u * BN5_NAVI_TYPE), ais = types && ai > 0 && ai < 0x40 ? rom32(d, types + 4u * (uint32_t)ai) : 0;
	if (!ais || v < 0 || v > 5 || ais - 0x08000000u + 6u * (uint32_t)v + 6 > ROM_SIZE) return 0;
	return ais - 0x08000000u + 6u * (uint32_t)v;
}

int guest_navi_hp(int xrom, int ai, int version, int *element) {
	if (element) *element = 0;
	const uint8_t *d = xrom == XROM_BN5_COLONEL_US ? XR[xrom].data : NULL;
	uint32_t at = d ? navi_stats(d, ai, version) : 0;
	if (!at) return -1;
	int w = d[at] | d[at + 1] << 8;
	if (element) *element = w >> 12 <= 4 ? w >> 12 : 0;
	return w & 0xFFF;
}

int guest_soul_kind(int xrom, int ai) {
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

uint32_t guest_navi_record(int xrom, int ai, int version) {
	const uint8_t *d = xrom == XROM_BN5_COLONEL_US ? XR[xrom].data : NULL;
	int id = d ? navi_id(d, ai, version) : 0;
	if (!id) return 0;
	for (uint32_t i = 0; i < BN5_STORY_BATTLE_COUNT; ++i)
		if (record_alone(d, BN5_STORY_BATTLES + 16u * i, id)) return BN5_STORY_BATTLES + 16u * i;
	/* (his SP: the net maps' records, where he roams once its story is over) */
	for (int g = 0x80; g < 0x80 + XR[xrom].layout->net_groups; ++g)
		for (int m = 0; m < 16; ++m)
			for (int i = 0, k = guest_records(xrom, g, m); i < k; ++i)
				if (record_alone(d, guest_record(xrom, g, m, i), id)) return guest_record(xrom, g, m, i);
	return 0;
}

bool guest_dev_worried;
int guest_backdrop = -1;

const char *guest_dark_name(int k) {
	static char name[16];
	if (k < 0 || k >= GUEST_DARK_KINDS) return "";
	xrom_text(XROM_BN5_COLONEL_US, BN5_CHIP_NAMES_LOW, BN5_DARK_FIRST + k, name, sizeof name);
	return name;
}

#ifdef CW_GUEST

#include <SDL.h>
#include <mgba/core/blip_buf.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba-util/vfs.h>

#include "compat.h"
#include "game.h"
#include "platform.h"
#include "xchips.h"

/* ---- the run's chips as its game's: by name, from both ROMs ---- */

#define BN6_CHIPS 314   /* BN6's standard, Mega and Giga chips, those a folder holds */
#define BN6_STANDARD_CHIPS 203   /* ... its standard ones, 1-202 */
static uint16_t to_bn5[BN6_CHIPS], from_bn5[BN5_CHIPS];

/* Pairs BN6's chips with BN5's of the same name (xchips.c); how many
 * paired. */
static int chips_pair(int xrom) {
	static char names5[BN5_CHIPS][XCHIP_NAME], names6[BN6_CHIPS][XCHIP_NAME];
	for (int i = 1; i < BN5_CHIPS; ++i)
		xrom_text(xrom, i < 256 ? BN5_CHIP_NAMES_LOW : BN5_CHIP_NAMES_HIGH, i < 256 ? i : i - 256, names5[i], sizeof names5[i]);
	for (int id = 1; id < BN6_CHIPS; ++id) {
		ChipInfo ci;
		chip_info(id, &ci);
		snprintf(names6[id], sizeof names6[id], "%s", ci.name);
	}
	return xchips_pair((const char (*)[XCHIP_NAME])names6, BN6_CHIPS, (const char (*)[XCHIP_NAME])names5, BN5_CHIPS, to_bn5, from_bn5);
}

static struct mCore *core;
static bool ready, failed, active, result_due;
static int paired;   /* BN6's chips paired by name with its game's (chips_pair), as the core is made */
static uint32_t video[EMU_W * EMU_H];
static GuestResult result;
static int frames;
static int recoded, recode_ex[3];   /* chips that went in with another code, and the first: BN6 id, code, its code there */
static uint8_t dark_in[GUEST_DARK_KINDS];   /* the run's DarkChips as the battle began */
static bool dark_used;                       /* ... and one was used in it (latched from BN5_DARK_USED) */
static bool buster_told;                     /* (debug) its battle's buster was printed */

/* a battle's course: waiting for the guest's boot (the browser's), asked
 * for, begun */
enum { PH_IDLE, PH_BOOT, PH_ASKED, PH_BATTLE };
static int phase;
/* (the battle that waits for the boot: its record, and MegaMan with his
 * folder as he went in) */
static uint32_t pend_record;
static GuestScale pend_sc;   /* ... its scaling to the act */
static GuestMegaMan pend_mm;
static GuestBoss pend_boss;   /* ... a guardian's, his (ai 0: none) */
static uint16_t pend_folder[30];
/* MegaMan's mood as a battle opens, from his HP: a battle begun calm, 0x80,
 * needed seven hits, each 10 off, to worry him (1-0x40), so a playtester at
 * 10 of 120 HP never saw his DarkChip (session 67). Worried from the start
 * at a quarter of his HP or less, else as much calmer as he is whole. BN5
 * sets 0x80 itself as the opening runs, so it is written over then. */
static int start_mood = 0x80;
static int mood_of(int hp, int max_hp) { return max_hp <= 0 ? 0x80 : hp * 4 <= max_hp ? 0x30 : 0x40 + 0x40 * hp / max_hp; }

static uint8_t rd8(uint32_t a) { return (uint8_t)core->rawRead8(core, a, -1); }
static uint16_t rd16(uint32_t a) { return (uint16_t)core->rawRead16(core, a, -1); }
static uint32_t rd32(uint32_t a) { return core->rawRead32(core, a, -1); }
static void wr16(uint32_t a, uint16_t v) { core->rawWrite16(core, a, -1, v); }

bool guest_possible(int xrom) { return xrom == XROM_BN5_COLONEL_US && XR[xrom].data && (ready || !failed); }

int guest_kind_chips(int kind, uint16_t *out, int max) {
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	if (!d || !R.data || kind < 0) return 0;
	if (!paired) paired = chips_pair(XROM_BN5_COLONEL_US);
	int n = 0;
	for (int id = 1; id < BN6_STANDARD_CHIPS && n < max; ++id)
		if (to_bn5[id] && d[BN5_CHIP_RECORDS + 0x2Cu * to_bn5[id] + BN5_CHIP_KIND] == kind) out[n++] = (uint16_t)id;
	return n;
}

uint8_t guest_read8(uint32_t a) { return core ? rd8(a) : 0; }
uint16_t guest_read16(uint32_t a) { return core ? rd16(a) : 0; }
uint32_t guest_read32(uint32_t a) { return core ? rd32(a) : 0; }
void guest_write16(uint32_t a, uint16_t v) { if (core) wr16(a, v); }
int guest_chip_bn6(int id) { return id > 0 && id < BN5_CHIPS ? from_bn5[id] : 0; }

/* The deck's compaction, replaced (BN5_COMPACT): as BN5's, the live chips
 * to the front, 0xFFFF after; but a DarkChip stays live only where BN5's
 * worried rule could have put it (at the hand's end while MegaMan is
 * worried or dark, or in the hand after the battle's first Custom screen),
 * the others shelved behind the padding, where no draw reaches and the
 * rule, which reads all 30, still finds them: DarkChips come only by its
 * rule (docs/META.md). Thumb, its literals 0x08012705 (emotion(side)),
 * the deck 0x0203C830 and 0xFFFF; called with r5 the Custom screen's state
 * and sl the toolkit, as BN5's (written and tested in romlab: the worried
 * MegaMan offered the shelved DarkCirc in his hand's last slot, the calm
 * one none). */
static const uint8_t shelf[188] = {
	0xf0, 0xb5, 0x84, 0xb0, 0x50, 0x46, 0x80, 0x69, 0xc1, 0x79, 0x00, 0x22, 0x01, 0x29, 0x00, 0xd9, 0x01, 0x22, 0x03, 0x92, 0x40, 0x7b,
	0x26, 0x4b, 0x00, 0xf0, 0x49, 0xf8, 0x01, 0x28, 0x01, 0xd0, 0x05, 0x28, 0x03, 0xd1, 0x03, 0x9a, 0x02, 0x21, 0x0a, 0x43, 0x03, 0x92,
	0xab, 0x79, 0x5b, 0x00, 0x20, 0x4c, 0x00, 0x26, 0x00, 0x27, 0x00, 0x20, 0x02, 0x90, 0xa0, 0x5b, 0x1e, 0x49, 0x88, 0x42, 0x1b, 0xd0,
	0xc1, 0x05, 0xc9, 0x0d, 0xbb, 0x29, 0x15, 0xd3, 0xc6, 0x29, 0x13, 0xd8, 0x03, 0x9a, 0x9e, 0x42, 0x04, 0xd3, 0x06, 0xd1, 0x02, 0x21,
	0x0a, 0x42, 0x0c, 0xd1, 0x02, 0xe0, 0x01, 0x21, 0x0a, 0x42, 0x08, 0xd1, 0x02, 0x99, 0x04, 0x29, 0x05, 0xd2, 0x4a, 0x00, 0x01, 0x31,
	0x02, 0x91, 0x69, 0x46, 0x88, 0x52, 0x01, 0xe0, 0xe0, 0x53, 0x02, 0x37, 0x02, 0x36, 0x3c, 0x2e, 0xdc, 0xd3, 0x02, 0x99, 0x1e, 0x22,
	0x52, 0x1a, 0x52, 0x00, 0x0b, 0x48, 0x97, 0x42, 0x02, 0xd2, 0xe0, 0x53, 0x02, 0x37, 0xfa, 0xe7, 0x00, 0x26, 0x02, 0x98, 0x40, 0x00,
	0x86, 0x42, 0x05, 0xd2, 0x69, 0x46, 0x88, 0x5b, 0xe0, 0x53, 0x02, 0x37, 0x02, 0x36, 0xf5, 0xe7, 0x04, 0xb0, 0xf0, 0xbd, 0x18, 0x47,
	0x05, 0x27, 0x01, 0x08, 0x30, 0xc8, 0x03, 0x02, 0xff, 0xff, 0x00, 0x00,
};
#define SHELF_AT (BN5_FREE + 0x100)

/* BN5's compaction pointed at the shelf (ldr r3, =shelf + 1; bx r3; nop) */
static void shelf_install(void) {
	for (uint32_t i = 0; i < sizeof shelf; i += 2) core->rawWrite16(core, SHELF_AT + i, -1, (uint16_t)(shelf[i] | shelf[i + 1] << 8));
	static const uint16_t jump[3] = { 0x4B01, 0x4718, 0x46C0 };
	for (uint32_t i = 0; i < 3; ++i) core->rawWrite16(core, BN5_COMPACT + 2 * i, -1, jump[i]);
	core->rawWrite32(core, BN5_COMPACT + 6, -1, SHELF_AT + 1);
}

/* The All * helper in the guest (docs/META.md): its chips' records hold *
 * alone in its ROM copy, as BN6's then do, so the run's chips come into its
 * Custom screen in * (a code its record lacks it draws as nothing), and its
 * check of a Program Advance of codes in a row takes three * (as BN6's,
 * BN5_PA_STAR_LIMIT); else its ROM's own again, where a run before had it. */
static bool starred;
static void star_records(bool on) {
	if (on == starred) return;
	starred = on;
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	static const uint8_t star[4] = { CHIP_CODE_STAR, 0xFF, 0xFF, 0xFF };
	for (uint32_t x = 1; x < BN5_CHIPS; ++x)
		for (uint32_t i = 0, rec = BN5_CHIP_RECORDS + 0x2Cu * x; i < 4; ++i)
			core->rawWrite8(core, 0x08000000u + rec + i, -1, on ? star[i] : d[rec + i]);
	uint32_t at = BN5_PA_STAR_LIMIT - 0x08000000u;
	uint16_t limit = (uint16_t)(d[at] | d[at + 1] << 8);
	wr16(BN5_PA_STAR_LIMIT, on && limit == 0x2A01 ? 0x2A03 : limit);
}

/* the run's DarkChips into the guest's folder (three at most, one of each,
 * as BN5's folder takes them), each in its own code (in * with All *),
 * where chips sat out: one empty slot stays, the shelf's edge */
static void dark_in_folder(uint32_t folders, uint32_t marks) {
	static const uint16_t entry[GUEST_DARK_KINDS] = { 0x22BB, 0x32BC, 0x10BD, 0x34BE, 0x2CBF, 0x26C0, 0x18C1, 0x0EC2, 0x0AC3, 0x06C4, 0x26C5, 0x08C6 };
	int empty[30], ne = 0, put = 0;
	for (int i = 29; i >= 0; --i) if (rd16(folders + 2u * (uint32_t)i) == 0xFFFF) empty[ne++] = i;
	for (int k = 0; k < GUEST_DARK_KINDS && put < 3 && put + 1 < ne; ++k) {
		if (!dark_in[k]) continue;
		wr16(folders + 2u * (uint32_t)empty[put++], starred ? chip_entry_star(entry[k]) : entry[k]);
		uint32_t id = entry[k] & 0x1FF;
		core->rawWrite8(core, marks + id, -1, (uint8_t)(rd8(BN5_CHIP_KEYS + id) ^ BN5_CHIP_KEY_XOR));
	}
}
static int main_mode(void) { return rd8(rd32(BN5_TOOLKIT)); }
static int sub_mode(void) { return rd8(BN5_GAMESTATE); }
static bool on_map(void) { return main_mode() == BN5_MODE_GAME && sub_mode() == BN5_SUB_MAP; }

/* The roll answers `record` (0: no battle): ldr r0,=record; tst r0,r0;
 * bx lr, the record in the literal after it. */
static void patch_roll(uint32_t record) {
	static const uint16_t code[4] = { 0x4801, 0x4200, 0x4770, 0x46C0 };
	for (uint32_t i = 0; i < 4; ++i) core->rawWrite16(core, BN5_ROLL + 2 * i, -1, code[i]);
	core->rawWrite32(core, BN5_ROLL + 8, -1, record);
}

/* bump the number when the boot changes */
static void state_path(char *out, size_t n) { snprintf(out, n, "%s/guest-bn5-1.state", g_data_dir); }

static bool load_boot(void) {
	char path[600];
	state_path(path, sizeof path);
	struct VFile *vf = VFileOpen(path, O_RDONLY);
	if (!vf) return false;
	bool ok = mCoreLoadStateNamed(core, vf, SAVESTATE_SAVEDATA);
	vf->close(vf);
	return ok && on_map();
}

/* ---- its boot: its title, NEW GAME, then A pressed through the intro
 * until Lan stands in his room, the same frames every time (emulation is
 * deterministic), once; then a state ---- */

/* Its frames: the title's 600, START tapped six times 70 frames apart, A
 * once and 130 frames, A every 50 frames through the intro 120 times, and
 * 60 more (a tap is held two frames) */
#define BOOT_FRAMES (600 + 6 * 70 + 130 + 120 * 50 + 60)

/* ... the keys held in its frame f */
static uint32_t boot_keys(int f) {
	if ((f -= 600) < 0) return 0;
	if (f < 6 * 70) return f % 70 < 2 ? KEY_START : 0;
	if ((f -= 6 * 70) < 130) return f < 2 ? KEY_A : 0;
	if ((f -= 130) < 120 * 50) return f % 50 < 2 ? KEY_A : 0;
	return 0;
}

static int boot_at = -1;      /* its next frame while it runs, -1 none */
static uint64_t boot_ticks;   /* (the time its frames took, for its line) */

/* Its frames from boot_at on, until the performance counter's `until` (0:
 * all); true once all have run */
static bool boot_frames(uint64_t until) {
	uint64_t t0 = SDL_GetPerformanceCounter();
	while (boot_at < BOOT_FRAMES && (!until || SDL_GetPerformanceCounter() < until)) {
		core->setKeys(core, boot_keys(boot_at));
		core->runFrame(core);
		++boot_at;
	}
	boot_ticks += SDL_GetPerformanceCounter() - t0;
	return boot_at >= BOOT_FRAMES;
}

/* Its end: Lan stands in his room, kept as the state (in the browser's
 * storage too); false where he does not */
static bool boot_kept(void) {
	if (!on_map()) return false;
	char path[600], tmp[640];
	state_path(path, sizeof path);
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	struct VFile *vf = VFileOpen(tmp, O_CREAT | O_TRUNC | O_RDWR);
	if (vf) {
		bool ok = mCoreSaveStateNamed(core, vf, SAVESTATE_SAVEDATA);
		vf->close(vf);
		if (ok) cw_rename(tmp, path);
		else remove(tmp);
	}
	platform_persist();
	printf("guest: booted in %d frames, %.2f s of its frames' time\n", BOOT_FRAMES, (double)boot_ticks / (double)SDL_GetPerformanceFrequency());
	return true;
}

/* The core for extra ROM `xrom`: its copy padded to EMU_ROM_SIZE as BN6's
 * is, its picture and its sound at the rate BN6's plays; and the chips of
 * both games paired by name */
static bool core_make(int xrom) {
	emu_log_quiet();   /* (made before BN6's core in the browser, guest_warm) */
	uint8_t *copy = malloc(EMU_ROM_SIZE);
	if (!copy) return false;
	memcpy(copy, XR[xrom].data, ROM_SIZE);
	memset(copy + ROM_SIZE, 0xFF, EMU_ROM_SIZE - ROM_SIZE);
	struct VFile *vf = VFileMemChunk(copy, EMU_ROM_SIZE);
	free(copy);
	if (!vf) return false;
	core = mCoreFindVF(vf);
	if (!core || !core->init(core)) { vf->close(vf); core = NULL; return false; }
	mCoreInitConfig(core, NULL);
	core->setVideoBuffer(core, (color_t *)video, EMU_W);
	if (!core->loadROM(core, vf)) { core->deinit(core); core = NULL; return false; }
	core->loadSave(core, VFileMemChunk(NULL, 0));
	core->setAudioBufferSize(core, 1024);
	blip_set_rates(core->getAudioChannel(core, 0), core->frequency(core), emu_audio_out_rate());
	blip_set_rates(core->getAudioChannel(core, 1), core->frequency(core), emu_audio_out_rate());
	core->reset(core);
	paired = chips_pair(xrom);
	return true;
}

/* Booted: its roll answers no battle until one is asked, and the deck's
 * shelf is in (after its boot, which runs its ROM as it is) */
static void boot_done(void) {
	boot_at = -1;
	patch_roll(0);
	shelf_install();
	ready = true;
	printf("guest: %s ready for battles, %d of BN6's chips by name\n", XR[XROM_BN5_COLONEL_US].layout->tag, paired);
}

bool guest_start(int xrom) {
	if (ready || failed) return ready;
	if (boot_at >= 0) return true;   /* (its boot under way, the browser's) */
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data) return false;
	failed = true;
	if (!core_make(xrom)) return false;
	failed = false;
	if (load_boot()) {
		boot_done();
		return true;
	}
	core->reset(core);
	boot_at = 0;
	boot_ticks = 0;
#ifdef __EMSCRIPTEN__
	/* (the page runs one frame at a time: its boot a slice a frame,
	 * guest_boot_slice, which a battle waits for) */
	return true;
#else
	if (boot_frames(0) && boot_kept()) {
		boot_done();
		return true;
	}
	boot_at = -1;
	failed = true;
	fprintf(stderr, "guest: %s did not reach its map\n", XR[xrom].layout->tag);
	return false;
#endif
}

/* `record` copied past BN5's ROM without its GAME OVER: a loss ends the
 * battle on the map with the result 2, and the run's own GAME OVER follows
 * (BN6's); its copy's address. Its entities stay the original's. */
/* The zenny a reward row's chip pays where BN6 has none of its name: the
 * results screen shows what the run gets (docs/MULTIROM.md, Guest
 * battles) */
#define REWARD_ZENNY 200

/* The reward rows of the enemies in `record` rewritten where a chip has no
 * BN6 chip of its name: zenny instead (its game's own encoding, as BN6's
 * rewards are); with All * the others in * (its results screen shows the
 * chip as the run gets it), else as its ROM has them. Its entities follow
 * the record's pointer at +0xC, four bytes each (0x11 an enemy, its id in
 * the last two), 0xF0 ending them. */
static uint8_t fit_codes[3];   /* the folder's codes for this battle's rewards (GuestMegaMan.codes) */

/* Chip entry v of a reward row in one of the folder's codes where both
 * BN5's chip and BN6's of its name have it, on every other row (xchips_fit,
 * issue #63), so BN5's own results screen shows it as the run gets it. */
static uint16_t row_coded(uint16_t v, uint32_t row) {
	int x = v & 0x1FF;
	if (starred || !from_bn5[x]) return v;
	uint8_t other[4], bn6[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	for (uint32_t i = 0; i < 4; ++i) other[i] = rd8(0x08000000u + BN5_CHIP_RECORDS + 0x2Cu * (uint32_t)x + i);
	ChipInfo ci;
	chip_info(from_bn5[x], &ci);
	for (int i = 0; i < ci.ncodes && i < 4; ++i) bn6[i] = ci.codes[i] == '*' ? 26 : (uint8_t)(ci.codes[i] - 'A');
	return (uint16_t)(x | xchips_fit(other, bn6, fit_codes, v >> 9 & 0x1F, (int)row) << 9);
}

/* A guardian's battle under way (guest_boss_battle): his id, and the zenny
 * his reward rows' chips pay (BN6's guardians' battles pay so where their
 * row holds their chip, which their Guardian Data gives) */
static int boss_id, boss_zenny;

static void rows_fit(const int *ids, int n) {
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	for (int k = 0; k < n; ++k) {
		/* (its enemies read a byte at a time, guest_record_foes: a list can
		 * start at an odd address, and a halfword read there takes the even
		 * one below; CanGard's MrkCan1 S showed on a playtester's results
		 * screen, came back as zenny, session 66; scaled as the battle is) */
		int id = ids[k];
		if (emu_debug_on()) fprintf(stderr, "guest: the reward rows of enemy %#x fitted\n", id);
		if (id <= 0 || id >= 0x200) continue;
		for (uint32_t i = 0; i < 20; ++i) {
			uint32_t a = BN5_REWARD_ROWS + (uint32_t)id * 0x28 + 2 * i, o = a - 0x08000000u;
			uint16_t v = (uint16_t)(d[o] | d[o + 1] << 8);
			if (v == 0xFFFF || v >> 14 || (int)(v & 0x1FF) >= BN5_CHIPS) continue;
			if (id == boss_id && boss_zenny > 0) wr16(a, (uint16_t)(1u << 14 | (boss_zenny > 0x3FFF ? 0x3FFF : boss_zenny)));
			else wr16(a, !from_bn5[v & 0x1FF] ? (uint16_t)(1u << 14 | REWARD_ZENNY) : starred ? chip_entry_star(v) : row_coded(v, i));
		}
	}
}

/* A guardian's HP held to his act's band: his stats row's HP written in the
 * ROM copy before his battle, as he spawns from it (seen in romlab:
 * KnightMan's V1 row written 450 spawned him at 450 of 450), and its own
 * written back as the battle ends */
static uint32_t capped_at;
static uint16_t capped_was;

static void boss_uncap(void) {
	if (capped_at) wr16(capped_at, capped_was);
	capped_at = 0;
}

static void boss_cap(const GuestBoss *b) {
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	uint32_t at = navi_stats(d, b->ai, b->version);
	uint16_t w = at ? (uint16_t)(d[at] | d[at + 1] << 8) : 0;
	if (!at || b->hp_cap <= 0 || (w & 0xFFF) <= b->hp_cap) return;
	capped_at = 0x08000000u + at;
	capped_was = w;
	wr16(capped_at, (uint16_t)((w & 0xF000) | (b->hp_cap & 0xFFF)));
}

/* (and its background the dressed area's where it leaves it to the map:
 * every guest battle stood in front of the room its boot state stands in,
 * yellow rings for each area) */
#define ENTITIES_AT (BN5_FREE + 0x20)   /* a scaled record's entity list, 16 entries and the 0xF0 at most */

static uint32_t record_copy(uint32_t record, GuestScale sc) {
	for (uint32_t i = 0; i < 16; i += 2) {
		uint16_t v = rd16(record + i);
		if (i == 8) v = (uint16_t)(v & ~BN5_OPT_GAME_OVER);
		/* (a guardian's: no running from him, as from BN6's, and his results
		 * screen on, which his V1 story records leave off) */
		if (i == 8 && boss_id) v = (uint16_t)((v & ~BN5_OPT_RUN) | BN5_OPT_RESULTS);
		if (i == BN5_RECORD_BACKDROP && (v & 0xFF) == 0xFF && guest_backdrop >= 0) v = (uint16_t)((v & 0xFF00) | (guest_backdrop & 0xFF));
		wr16(BN5_FREE + i, v);
	}
	/* (and its entities, a byte at a time, each virus at its scaled version:
	 * the copy pointed at them) */
	if (sc.up <= 0) return BN5_FREE;
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	uint32_t from = rd32(record + 0xC), k = 0;
	for (; k < 16 && from - 0x08000000u + 4u * k + 4 <= ROM_SIZE && d[from - 0x08000000u + 4u * k] != 0xF0; ++k) {
		const uint8_t *e = d + (from - 0x08000000u) + 4u * k;
		int id = e[0] == 0x11 ? version_up(d, e[2] | e[3] << 8, sc) : e[2] | e[3] << 8;
		const uint8_t to[4] = { e[0], e[1], (uint8_t)id, (uint8_t)(id >> 8) };
		for (uint32_t j = 0; j < 4; ++j) core->rawWrite8(core, ENTITIES_AT + 4u * k + j, -1, to[j]);
	}
	core->rawWrite8(core, ENTITIES_AT + 4u * k, -1, 0xF0);
	core->rawWrite32(core, BN5_FREE + 0xC, -1, ENTITIES_AT);
	return BN5_FREE;
}

/* The run's folder as the guest's first, each chip as its game's of the
 * same name (its own code), owned; a chip it has none of sits out, its
 * slot empty. How many went in. */
/* A chip's code as BN5's chip of its name has it: the same letter where it
 * has that one, else its '*', else its first. BN5's Custom screen draws a
 * chip with a code it lacks as nothing, which can be chosen and does
 * nothing (a playtester's Storm folder had nine of them, its Thunder S
 * among them: BN5's Thunder, its chip 19, has B, L, P and *; session 65;
 * the Storm folder's ElcPuls1 and DolThdr1 BN5 has none of). */
static int bn5_code(int x, int code) {
	/* (its record as its ROM copy has it: in * alone with All *) */
	uint8_t rec[4];
	for (uint32_t i = 0; i < 4; ++i) rec[i] = rd8(0x08000000u + BN5_CHIP_RECORDS + 0x2Cu * (uint32_t)x + i);
	return xchips_code(rec, code);
}

/* ... and a chip won there as BN6's chip of its name has it, the same way */
static int bn6_code(int id, int code) {
	ChipInfo ci;
	chip_info(id, &ci);
	uint8_t rec[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	for (int i = 0; i < ci.ncodes && i < 4; ++i) rec[i] = ci.codes[i] == '*' ? 26 : (uint8_t)(ci.codes[i] - 'A');
	return xchips_code(rec, code);
}

static int folder_in(const uint16_t *folder) {
	uint32_t folders = rd32(BN5_TOOLKIT + BN5_TOOLKIT_CHIPS), marks = rd32(BN5_TOOLKIT + BN5_TOOLKIT_CHIP_MARKS);
	if (folders < 0x02000000u || folders >= 0x02040000u || marks < 0x02000000u || marks >= 0x02040000u) return 0;
	int in = 0;
	recoded = 0;
	for (uint32_t i = 0; i < 30; ++i) {
		int id = folder[i] & 0x1FF, x = id > 0 && id < BN6_CHIPS ? to_bn5[id] : 0, code = folder[i] >> 9, there = x ? bn5_code(x, code) : code;
		core->rawWrite16(core, folders + 2 * i, -1, x ? (uint16_t)(x | there << 9) : 0xFFFF);
		if (!x) continue;
		if (there != code && !recoded++) { recode_ex[0] = id; recode_ex[1] = code; recode_ex[2] = there; }
		core->rawWrite8(core, marks + (uint32_t)x, -1, (uint8_t)(rd8(BN5_CHIP_KEYS + (uint32_t)x) ^ BN5_CHIP_KEY_XOR));
		++in;
	}
	core->rawWrite8(core, BN5_NAVI_FOLDER, -1, 0);
	dark_in_folder(folders, marks);
	return in;
}

int guest_sitting_out(const uint16_t *folder, uint16_t *out, int max) {
	return paired ? xchips_out(folder, 30, to_bn5, BN6_CHIPS, out, max) : 0;
}

/* (debug: the folder's chips that sat out, slot by slot) */
static void out_tell(const uint16_t *folder) {
	char line[600];
	int k = 0, n = 0;
	for (int i = 0; folder && i < 30 && k < (int)sizeof line - 24; ++i) {
		int id = folder[i] & 0x1FF, code = folder[i] >> 9;
		if (!id || (id < BN6_CHIPS && to_bn5[id])) continue;
		ChipInfo ci;
		chip_info(id, &ci);
		k += snprintf(line + k, sizeof line - (size_t)k, "%s%s %c", n++ ? ", " : "", ci.name, code == 26 ? '*' : code < 26 ? 'A' + code : '?');
	}
	if (n) fprintf(stderr, "guest: %d sat out: %s\n", n, line);
}

/* One of its event flags set or cleared (BN5_TOOLKIT_FLAGS) */
static void flag_put(uint32_t flag, bool on) {
	uint32_t flags = rd32(BN5_TOOLKIT + BN5_TOOLKIT_FLAGS);
	if (flags < 0x02000000u || flags >= 0x02040000u) return;
	uint32_t a = flags + (flag >> 3);
	uint8_t bit = (uint8_t)(0x80u >> (flag & 7)), v = rd8(a);
	core->rawWrite8(core, a, -1, (uint8_t)(on ? v | bit : v & ~bit));
}

/* The run's Souls as its game's own (docs/META.md, Souls in BN5 territory):
 * each one's flag, and Double Soul and Chaos Unison learned while it holds
 * one, so its Custom screen offers UNITE by its own rule; its boot state,
 * early in its story, holds none of them */
static int souls_seen;   /* (debug) frames its Custom screen has been up, -1 told */
static void souls_in(uint8_t souls) {
	flag_put(BN5_FLAG_DOUBLE_SOUL, souls != 0);
	flag_put(BN5_FLAG_CHAOS, souls != 0);
	for (uint32_t k = 0; k < 6; ++k) flag_put(BN5_FLAG_SOUL + k, souls >> k & 1);
	souls_seen = souls && emu_debug_on() ? 0 : -1;
}

/* (debug: once a battle with Souls held, whether UNITE stood on its first
 * Custom screen, half a second up, as its opening sets it up; and each
 * unison as it begins) */
static void souls_tell(void) {
	static uint8_t united;
	static uint32_t used;   /* (the unisons told: a Chaos Unison is the one whose bit is new) */
	if (phase != PH_BATTLE || sub_mode() != BN5_SUB_BATTLE) { united = 0; used = 0; return; }
	if (souls_seen >= 0 && rd8(BN5_BATTLE_PHASE) == BN5_PHASE_CUSTOM && ++souls_seen == 30) {
		fprintf(stderr, "guest: UNITE %s on its first Custom screen\n", rd8(BN5_CUSTOM_UNITE) == 2 ? "stands" : "does not stand");
		souls_seen = -1;
	}
	uint8_t soul = rd8(BN5_BATTLE_SOUL + BN5_BATTLE_NAVI_SIZE * rd8(BN5_BATTLE_SIDE));
	if (soul != united && soul && emu_debug_on()) {
		uint32_t now = rd32(BN5_SOULS_USED);
		fprintf(stderr, "guest: MegaMan united with Soul %d%s\n", soul, (now & ~used) >> (16 + soul) & 1 ? ", Chaos Unison" : "");
		used = now;
	}
	united = soul;
}

/* His buster as the run's NaviCust makes it: BN5's boot state's fired 1 a
 * shot and 10 a charge, whatever programs the run had run */
static void buster_in(const uint8_t buster[3]) {
	static const uint32_t at[3] = { BN5_NAVI_ATTACK, BN5_NAVI_SPEED, BN5_NAVI_CHARGE };
	for (int i = 0; i < 3; ++i) core->rawWrite8(core, at[i], -1, buster[i] > BN5_BUSTER_MAX ? BN5_BUSTER_MAX : buster[i]);
	buster_told = !emu_debug_on();
}

/* (debug: the buster the battle took, from its own copy, once its Custom
 * screen is up) */
static void buster_tell(void) {
	if (buster_told || phase != PH_BATTLE || sub_mode() != BN5_SUB_BATTLE || rd8(BN5_BATTLE_STATE + 1) != BN5_PHASE_CUSTOM) return;
	buster_told = true;
	uint32_t s = BN5_BATTLE_NAVI + BN5_BATTLE_NAVI_SIZE * rd8(BN5_BATTLE_SIDE);
	fprintf(stderr, "guest: its battle's buster Attack %d, Speed %d, Charge %d\n", rd8(s + (BN5_NAVI_ATTACK - BN5_NAVI_STATS)) + 1,
		rd8(s + (BN5_NAVI_SPEED - BN5_NAVI_STATS)) + 1, rd8(s + (BN5_NAVI_CHARGE - BN5_NAVI_STATS)) + 1);
}

static bool battle_begin(uint32_t record, GuestScale sc, const GuestMegaMan *mm, const GuestBoss *boss) {
	memcpy(dark_in, mm->dark, sizeof dark_in);
	emu_sync();   /* (BN6's frame done first, where its core has a thread) */
	if (!on_map() && !load_boot()) return false;
	/* (a guardian's: his band, his rows' zenny, his options; none else) */
	boss_uncap();
	boss_id = boss ? navi_id(XR[XROM_BN5_COLONEL_US].data, boss->ai, boss->version) : 0;
	boss_zenny = boss ? boss->zenny : 0;
	if (boss) boss_cap(boss);
	souls_in(mm->souls);
	star_records(mm->star);
	memcpy(fit_codes, mm->codes, sizeof fit_codes);
	/* MegaMan as the run has him: his HP (BN5 copies it back after a battle
	 * whose options carry 0x40, as its random battles' do) */
	int hp = mm->hp, max_hp = mm->max_hp;
	wr16(BN5_NAVI_BASE_MAX_HP, (uint16_t)max_hp);
	wr16(BN5_NAVI_MAX_HP, (uint16_t)max_hp);
	wr16(BN5_NAVI_HP, (uint16_t)(hp < 1 ? 1 : hp > max_hp ? max_hp : hp));
	buster_in(mm->buster);
	/* (calm at its start, as each of the run's battles begins: BN5's dark
	 * meter and mood would carry its last DarkChip's darkness on; then, as
	 * its opening runs, as worried as his HP says, start_mood) */
	core->rawWrite8(core, BN5_NAVI_MOOD, -1, 0x80);
	start_mood = mood_of(hp, max_hp);
	wr16(BN5_NAVI_METER, 500);
	uint32_t check = rd32(BN5_TOOLKIT + BN5_TOOLKIT_METER_CHECK);
	if (check >= 0x02000000u && check < 0x02040000u) core->rawWrite32(core, check, -1, 500u ^ rd32(BN5_METER_KEY));
	dark_used = false;
	const uint16_t *folder = mm->folder;
	int in = folder ? folder_in(folder) : 0;
	int ids[16], n = guest_record_foes_scaled(XROM_BN5_COLONEL_US, record, sc, ids, 16);
	rows_fit(ids, n);
	if (emu_debug_on()) {
		fprintf(stderr, "guest: battle %08X, HP %d/%d, mood %#x, %d of the folder's 30 in, buster Attack %d, Speed %d, Charge %d\n", record, hp, max_hp,
			start_mood, in,
			rd8(BN5_NAVI_ATTACK) + 1, rd8(BN5_NAVI_SPEED) + 1, rd8(BN5_NAVI_CHARGE) + 1);
		out_tell(folder);
	}
	patch_roll(record_copy(record, sc));
	active = true;
	phase = PH_ASKED;
	frames = 0;
	return true;
}

/* (begun at once, or, its boot still under way, the browser's, waiting
 * for it, BN6's core with it: guest_boot_slice) */
static bool battle_ask(uint32_t record, GuestScale sc, const GuestMegaMan *mm, const GuestBoss *boss) {
	if (active || !record) return false;
	if (boot_at < 0) return ready && battle_begin(record, sc, mm, boss);
	pend_record = record;
	pend_sc = sc;
	pend_mm = *mm;
	pend_boss = boss ? *boss : (GuestBoss){ 0 };
	if (mm->folder) {
		memcpy(pend_folder, mm->folder, sizeof pend_folder);
		pend_mm.folder = pend_folder;
	}
	active = true;
	phase = PH_BOOT;
	return true;
}

bool guest_battle(uint32_t record, GuestScale sc, const GuestMegaMan *mm) { return battle_ask(record, sc, mm, NULL); }

bool guest_boss_battle(const GuestBoss *boss, const GuestMegaMan *mm) {
	uint32_t record = guest_navi_record(XROM_BN5_COLONEL_US, boss->ai, boss->version);
	if (emu_debug_on())
		fprintf(stderr, "guest: guardian %d at version %d, record %08X, his HP %d, at most %d, his rows %d zenny\n", boss->ai, boss->version, record,
			guest_navi_hp(XROM_BN5_COLONEL_US, boss->ai, boss->version, NULL), boss->hp_cap, boss->zenny);
	return battle_ask(record, (GuestScale){ 0, 0 }, mm, boss);
}

int guest_boot_progress(void) { return boot_at < 0 ? -1 : boot_at * 100 / BOOT_FRAMES; }

void guest_boot_slice(int ms) {
	if (boot_at < 0) return;
	if (!boot_frames(SDL_GetPerformanceCounter() + SDL_GetPerformanceFrequency() * (uint64_t)ms / 1000)) return;
	if (boot_kept()) boot_done();
	else {
		boot_at = -1;
		failed = true;
		fprintf(stderr, "guest: %s did not reach its map\n", XR[XROM_BN5_COLONEL_US].layout->tag);
	}
	/* (the battle that waited begins; where it cannot, it ends unfought,
	 * MegaMan as he went in) */
	if (phase != PH_BOOT) return;
	active = false;
	phase = PH_IDLE;
	if (ready && battle_begin(pend_record, pend_sc, &pend_mm, pend_boss.ai ? &pend_boss : NULL)) return;
	result = (GuestResult){ .outcome = GUEST_ESCAPED, .hp = pend_mm.hp, .reward_from = -1 };
	memcpy(result.dark, pend_mm.dark, sizeof result.dark);
	result_due = true;
}

#ifdef __EMSCRIPTEN__
void guest_warm(int ms) {
	static bool looked;
	if (!looked) {
		looked = true;
		char path[600];
		state_path(path, sizeof path);
		FILE *f = XR[XROM_BN5_COLONEL_US].data ? fopen(path, "rb") : NULL;
		if (f) fclose(f);
		/* (kept already: a layer loads it, as quick as a state loads) */
		if (f || !XR[XROM_BN5_COLONEL_US].data || !guest_start(XROM_BN5_COLONEL_US)) return;
		printf("guest: its first boot begun, a slice a frame\n");
	}
	if (!active) guest_boot_slice(ms);
}
#endif

bool guest_active(void) { return active; }

bool guest_on_screen(void) { return active && phase == PH_BATTLE && sub_mode() == BN5_SUB_BATTLE; }

int guest_custom_gauge(void) {
	return guest_on_screen() && rd8(BN5_BATTLE_PHASE) == BN5_PHASE_FIGHT ? rd16(BN5_CUSTOM_GAUGE) : -1;
}

bool guest_fight_hp(int *hp, int *max) {
	if (!guest_on_screen()) return false;
	*hp = rd16(BN5_FIGHT_HP);
	*max = rd16(BN5_FIGHT_HP + 2);
	return *max > 0;   /* (0/0 in the battle's first frames, before it sets MegaMan) */
}

static void finish(int outcome) {
	result = (GuestResult){ .outcome = outcome, .frames = frames, .hp = phase == PH_BATTLE ? rd16(BN5_BATTLE_HP) : rd16(BN5_NAVI_HP), .recoded = recoded,
		.recode_chip = recode_ex[0], .recode_from = recode_ex[1], .recode_to = recode_ex[2], .reward_from = -1 };
	memcpy(result.dark, dark_in, sizeof result.dark);   /* (BN5 keeps a DarkChip once used: the run's stay) */
	result.dark_used = dark_used;
	/* (what its results screen gave, as the run's: a chip by its name, or
	 * zenny; HP+N it gave there, in the HP above) */
	uint16_t v = outcome == GUEST_WON ? rd16(BN5_REWARD) : 0;
	if (v && v >> 14 == 1) result.zenny = v & 0x3FFF;
	else if (v && v >> 14 == BN5_REWARD_HP) result.heal = v & 0x3FFF;
	else if (v && v >> 14 == 0 && (v & 0x1FF) < BN5_CHIPS) {
		result.chip = from_bn5[v & 0x1FF];
		result.code = result.chip ? bn6_code(result.chip, v >> 9 & 0x1F) : 0;
		if (result.chip && result.code != (v >> 9 & 0x1F)) result.reward_from = v >> 9 & 0x1F;
		if (!result.chip) result.zenny = REWARD_ZENNY;
	}
	result_due = true;
	patch_roll(0);
	boss_uncap();
	boss_id = boss_zenny = 0;
	active = false;
	phase = PH_IDLE;
}

/* CYBERWORLD_EMU_DEBUG's line every 30 of its frames, as BN6's core's
 * (debug.c): its modes, its battle's phase, MegaMan's HP, the gauge and
 * the battle's clock */
static void debug_line(int mode, int sub) {
	if (frames % 30 || !emu_debug_on()) return;
	fprintf(stderr, "guest t%d mode %02x sub %02x phase %02x hp %d/%d gauge %d%% timer %u mood %02x\n", frames, mode, sub, rd8(BN5_BATTLE_PHASE),
		rd16(BN5_T1_OBJECTS + BN6_T1_HP), rd16(BN5_T1_OBJECTS + BN6_T1_MAX_HP), rd16(BN5_CUSTOM_GAUGE) * 100 / 0x4000,
		(unsigned)rd32(BN5_BATTLE_TIMER), rd8(BN5_BATTLE_MOOD));
}

static void step(uint32_t keys, bool quiet);

/* (the battle's opening: its mood as worried as his HP says, start_mood,
 * over the calm BN5 sets as it runs) */
static void mood_open(int sub) {
	if (phase == PH_BATTLE && sub == BN5_SUB_BATTLE && rd8(BN5_BATTLE_STATE + 1) == BN5_PHASE_INTRO && rd8(BN5_BATTLE_MOOD) > start_mood)
		core->rawWrite8(core, BN5_BATTLE_MOOD, -1, (uint8_t)start_mood);
}

void guest_frame(uint32_t keys) { step(keys, false); }
void guest_frame_quiet(uint32_t keys) { step(keys, true); }

static void step(uint32_t keys, bool quiet) {
	if (!active || phase == PH_BOOT) return;
	core->setKeys(core, keys);
	core->runFrame(core);
	if (quiet) emu_audio_drop_from(core);
	else emu_audio_from(core);
	++frames;
	buster_tell();
	souls_tell();
	int mode = main_mode(), sub = sub_mode();
	debug_line(mode, sub);
	if (phase == PH_ASKED && (sub == BN5_SUB_BATTLE_INIT || sub == BN5_SUB_BATTLE)) {
		phase = PH_BATTLE;
		core->rawWrite32(core, BN5_ROLL + 8, -1, 0);   /* (one battle: the roll answers none again) */
		core->rawWrite8(core, BN5_BATTLE_RESULT + 1, -1, 0);
		wr16(BN5_REWARD, 0);
	} else if (phase == PH_ASKED && frames > 600) finish(GUEST_ESCAPED);   /* (never began: nothing happened) */
	/* (a DarkChip used: latched while its battle is fought, as leaving it
	 * wipes the flag; not in the battle's first frames, which hold the last
	 * battle's bytes until BN5 clears them: every battle had cost 20 max HP) */
	else if (phase == PH_BATTLE && sub == BN5_SUB_BATTLE && rd8(BN5_BATTLE_PHASE) == BN5_PHASE_FIGHT &&
		rd8(BN5_DARK_USED + 8u * rd8(BN5_BATTLE_SIDE)))
		dark_used = true;
	mood_open(sub);
	if (guest_dev_worried && phase == PH_BATTLE && sub == BN5_SUB_BATTLE && rd8(BN5_BATTLE_MOOD) > 0x40) core->rawWrite8(core, BN5_BATTLE_MOOD, -1, 0x20);
	/* (back on the map: how it ended, from BN5's own result, and MegaMan's
	 * HP as the battle left it: BN5 copies it back to his NaviStats only on
	 * its own maps' terms, which a forced battle does not meet) */
	if (phase == PH_BATTLE && mode == BN5_MODE_GAME && sub == BN5_SUB_MAP) {
		int r = rd8(BN5_BATTLE_RESULT + 1), hp = rd16(BN5_BATTLE_HP);
		finish(r == BN5_RESULT_LOST || hp == 0 ? GUEST_LOST : r == BN5_RESULT_ESCAPED ? GUEST_ESCAPED : GUEST_WON);
	}
	/* (a GAME OVER all the same: its records' copies leave it out) */
	else if (phase == PH_BATTLE && mode == BN5_MODE_GAME_OVER) finish(GUEST_LOST);
}

const uint32_t *guest_video(void) { return video; }

bool guest_take_result(GuestResult *out) {
	if (!result_due) return false;
	*out = result;
	result_due = false;
	return true;
}

#else   /* (one ROM: the 3DS) */

bool guest_start(int xrom) { (void)xrom; return false; }
bool guest_battle(uint32_t record, GuestScale sc, const GuestMegaMan *mm) {
	(void)record; (void)sc; (void)mm;
	return false;
}
int guest_boot_progress(void) { return -1; }
void guest_boot_slice(int ms) { (void)ms; }
bool guest_boss_battle(const GuestBoss *boss, const GuestMegaMan *mm) { (void)boss; (void)mm; return false; }
bool guest_possible(int xrom) { (void)xrom; return false; }
int guest_kind_chips(int kind, uint16_t *out, int max) { (void)kind; (void)out; (void)max; return 0; }
int guest_sitting_out(const uint16_t *folder, uint16_t *out, int max) { (void)folder; (void)out; (void)max; return 0; }
bool guest_active(void) { return false; }
bool guest_on_screen(void) { return false; }
bool guest_fight_hp(int *hp, int *max) { (void)hp; (void)max; return false; }
int guest_custom_gauge(void) { return -1; }
void guest_frame(uint32_t keys) { (void)keys; }
void guest_frame_quiet(uint32_t keys) { (void)keys; }
const uint32_t *guest_video(void) { return NULL; }
bool guest_take_result(GuestResult *out) { (void)out; return false; }
uint8_t guest_read8(uint32_t a) { (void)a; return 0; }
uint16_t guest_read16(uint32_t a) { (void)a; return 0; }
uint32_t guest_read32(uint32_t a) { (void)a; return 0; }
void guest_write16(uint32_t a, uint16_t v) { (void)a; (void)v; }
int guest_chip_bn6(int id) { (void)id; return 0; }

#endif
