/* xnavi_install.h: a Navi's recipe carried out on the core's ROM copy, its
 * writes staged (xvirus_tx.h) and made once all of its checks passed. */
#include "xnavi_install.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "data.h"
#include "emu.h"
#include "rom.h"
#include "xchips.h"
#include "xnavi.h"
#include "xnavi_bn5.h"
#include "xnavi_objects.h"
#include "xnavi_room.h"
#include "xnavi_sprites.h"
#include "xsong.h"
#include "xvirus.h"
#include "xvirus_grow.h"
#include "xvirus_move.h"
#include "xvirus_tx.h"

#define BUS        0x08000000u
#define HELD_LISTS 0xC0     /* six held-chip lists, 0x20 bytes each */
#define TEXT_END   0xE6     /* a text archive's entry ends with it */
#define NAME_MAX   20       /* a name's bytes at most, its end among them */

/* ---- its game's Navi tables ---- */

typedef struct {
	const uint8_t *rom;
	uint32_t tables, step, count;   /* its Navi tables: the first, bytes apart, entries */
	int of[BN6_NAVI_TABLE_COUNT];   /* BN6's table k as its game's (-1: BN6's alone) */
	uint32_t names, rows;           /* its Navis' names archive, its reward rows */
	bool bn5;                       /* its text and chips BN5's */
	uint32_t lists;                 /* its sprite lists (a ROM offset, RomLayout.sprite_lists) */
} Game;

static bool game_of(int game, Game *g) {
	memset(g, 0, sizeof *g);
	if (game == XV_GAME_BN6 && R.data) {
		*g = (Game){ R.data, BN6_NAVI_TABLES, BN6_NAVI_TABLE_STEP, BN6_NAVIS, { 0 }, get32(R.data + (BN6_NAVI_NAMES_REF - BUS)), BN6_DROP_ROWS, false,
			R.layout ? R.layout->sprite_lists : 0 };
		for (int k = 0; k < BN6_NAVI_TABLE_COUNT; ++k) g->of[k] = k;
		return true;
	}
	if (game != XROM_BN5_COLONEL_US || !XR[game].data) return false;
	static const int of5[BN6_NAVI_TABLE_COUNT] = { 0, 1, -1, 2, 3, 4, 5, 6, 7, -1, -1, 8, 9 };
	*g = (Game){ XR[game].data, BN5_NAVI_TABLES, BN5_NAVI_TABLE_STEP, BN5_NAVIS, { 0 }, BN5_NAVI_NAMES, BN5_REWARD_ROWS, true,
		XR[game].layout ? XR[game].layout->sprite_lists : 0 };
	memcpy(g->of, of5, sizeof of5);
	return true;
}

static bool in_rom(uint32_t a, uint32_t n) { return a >= BUS && a - BUS <= ROM_SIZE && n <= ROM_SIZE - (a - BUS); }
static uint32_t rom32(const uint8_t *rom, uint32_t a) { return get32(rom + (a - BUS)); }
/* its game's table k's entry for its AI index (k BN6's), 0 where its game has none */
static uint32_t entry_of(const Game *g, int k, int ai) {
	return g->of[k] < 0 ? 0 : rom32(g->rom, g->tables + g->step * (uint32_t)g->of[k] + 4u * (uint32_t)ai);
}

/* its records where its game's tables have them, its state table one of its zeroed ranges */
static bool sane(const XNavi *n, const Game *g) {
	const XVirusFamily *f = &n->code;
	char why[200];
	if (!xnavi_recipe_ok(n, why, sizeof why)) return xv_fail("%s", why);
	if (f->src_ai < 0 || (uint32_t)f->src_ai >= g->count) return xv_fail("its AI index in its game, %d", f->src_ai);
	if (entry_of(g, XN_STRUCT1, f->src_ai) != f->struct1 || entry_of(g, XN_STRUCT2, f->src_ai) != f->struct2)
		return xv_fail("its records are not where its game's tables have them");
	uint32_t st = entry_of(g, XN_STATES, f->src_ai);
	const XvData *d = xvirus_data_at(f, st);
	if (!d || d->kind != XV_ZERO || d->start != st) return xv_fail("its state table is at %08x in its game", (unsigned)st);
	return in_rom(f->struct1, BN6_TRAITS_SIZE) && in_rom(f->struct2, XV_VERSIONS * BN6_STATS_SIZE) ? true : xv_fail("its records");
}

/* ---- its records ---- */

static struct {
	uint8_t traits[BN6_TRAITS_SIZE], stats[XV_VERSIONS * BN6_STATS_SIZE];
	uint8_t name[XV_VERSIONS][NAME_MAX];
	uint32_t name_len[XV_VERSIONS];
	uint16_t drops[XV_VERSIONS][ENEMY_DROPS];
} rec;

/* its struct1 BN6's way: its game's as it is, or BN5's laid out as BN6's
 * (its battle sprite list 2's number, its shadow at +7, no element of the
 * second wheel), checked against what the recipe says it is first */
static void traits_set(const XNavi *n, const uint8_t *src) {
	const XvTraits *t = &n->code.traits;
	memcpy(rec.traits, src, BN6_TRAITS_SIZE);
	if (!t->laid) return;
	int number = xnavi_sprite_number(n->sprite_of);
	if (number < 0 || src[BN6_TRAITS_MODE] != t->mode || !src[BN5_TRAITS_SHADOW] != !t->shadow) {
		xv_fail("its struct1 is not the one its recipe names");
		return;
	}
	rec.traits[BN6_TRAITS_LIST] = 4u * BN6_SPRITES_NAVIS;
	rec.traits[BN6_TRAITS_SPRITE] = (uint8_t)number;
	rec.traits[BN6_TRAITS_ELEMENT] = t->element;
	rec.traits[BN6_TRAITS_WEAKNESS] = t->weakness;
	rec.traits[BN6_TRAITS_SHADOW] = t->shadow;
}

/* its state table, struct1, struct2 and held chips in its home; its entries of BN6's thirteen tables and the triples' Navi
 * words */
static void records_set(const XNavi *n, const Game *g) {
	const XVirusFamily *f = &n->code;
	uint8_t *st = xv_put(f->home + XV_HOME_STATES, 4u * (uint32_t)f->nstates, true);
	for (int i = 0; st && i < f->nstates; ++i) put32(st + 4 * i, xv_move_ref(f->states[i]));
	traits_set(n, g->rom + (f->struct1 - BUS));
	memcpy(rec.stats, g->rom + (f->struct2 - BUS), sizeof rec.stats);
	uint8_t *s1 = xv_put(f->home + XV_HOME_STRUCT1, BN6_TRAITS_SIZE, true), *s2 = xv_put(f->home + XV_HOME_STRUCT2, sizeof rec.stats, true);
	if (s1) memcpy(s1, rec.traits, BN6_TRAITS_SIZE);
	if (s2) memcpy(s2, rec.stats, sizeof rec.stats);
	uint32_t held = f->held;
	if (!held) {
		uint8_t *h = xv_put(f->home + XV_HOME_HELD, HELD_LISTS, true);
		if (h) memset(h, 0xFF, HELD_LISTS);
		for (uint32_t k = 0; h && f->held_chips && k < HELD_LISTS / 2; ++k) put16(h + 2 * k, f->held_chips[k]);
		held = f->home + XV_HOME_HELD;
	}
	const uint32_t entry[BN6_NAVI_TABLE_COUNT] = {
		f->home + XV_HOME_STATES, xv_move_ref(n->prestep), xv_move_ref(n->hook_a), f->home + XV_HOME_STRUCT1, f->home + XV_HOME_STRUCT2,
		xv_move_ref(f->hooks[0]), xv_move_ref(f->hooks[1]), xv_move_ref(f->hooks[2]), xv_move_ref(f->hooks[3]), xv_move_ref(f->hooks[4]),
		xv_move_ref(f->hooks[5]), xv_move_ref(f->hooks[6]), xv_move_ref(f->hooks[7]),
	};
	for (int k = 0; k < BN6_NAVI_TABLE_COUNT && !xv_failed(); ++k) xn_set_word(XN_TABLE + k, (uint32_t)f->ai, entry[k]);
	const uint8_t ice = (uint8_t)f->ice;
	if (!xv_failed()) xn_set(XN_T_ICE, (uint32_t)f->ai, &ice);
	if (!xv_failed()) xn_set_word(XN_T_HELD, (uint32_t)f->ai, held);
	if (!xv_failed()) xn_set_word(XN_T_COMPANION, (uint32_t)f->ai, xv_move_ref(n->companion));
	if (!xv_failed()) xn_set_word(XN_T_COMPANION_END, (uint32_t)f->ai, xv_move_ref(n->companion_end));
}

/* ---- its companion's overlay of a kind BN6 lacks ---- */

/* its masks copied from its game's ROM (their CRC32 checked) and its kind's entries in the grown overlay tables: the masks'
 * copy, its sprite's list offset and number */
static void overlay_set(const XNavi *n, const Game *g) {
	const XnOverlay *o = n->overlay;
	if (!o) return;
	if (!in_rom(o->masks, o->masks_len) || xvirus_crc32(g->rom + (o->masks - BUS), o->masks_len) != o->crc) {
		xv_fail("its overlay's masks at %08x are not the ones expected", (unsigned)o->masks);
		return;
	}
	uint8_t *m = xv_put(o->masks_at, o->masks_len, true);
	if (m) memcpy(m, g->rom + (o->masks - BUS), o->masks_len);
	const uint8_t sprite[2] = { (uint8_t)(4u * o->list), o->number };
	if (!xv_failed()) xn_set_word(XN_T_OVL_MASKS, o->kind, o->masks_at);
	if (!xv_failed()) xn_set(XN_T_OVL_SPRITES, o->kind, sprite);
}

/* ---- its ids: their triples, names and reward rows ---- */

/* A Navi's name of BN5's in BN6's text codes: its version marks as BN6's
 * glyphs where BN6 has one (SP, BN5's 0xE4 0x23, BN6's 0x43), spelt in
 * letters where it has none (V2 and V3, BN5's 0xAE and 0xAF; the DS) */
static uint32_t name_bn6(const Game *g, const uint8_t *s, uint32_t n, uint8_t *out, uint32_t max) {
	if (!g->bn5) {
		uint32_t k = n < max ? n : max;
		memcpy(out, s, k);
		return k;
	}
	uint32_t k = 0;
	for (uint32_t i = 0; i < n && k + 2 <= max;) {
		const uint8_t v2[2] = { 0x0B + 'V' - 'A', 0x01 + 2 }, v3[2] = { 0x0B + 'V' - 'A', 0x01 + 3 }, sp = 0x43;
		if (s[i] == 0xAE || s[i] == 0xAF) {
			memcpy(out + k, s[i] == 0xAE ? v2 : v3, 2);
			k += 2;
			++i;
		} else if (s[i] == 0xE4 && i + 1 < n && s[i + 1] == 0x23) {
			out[k++] = sp;
			i += 2;
		} else {
			uint32_t run = i;
			while (run < n && s[run] != 0xAE && s[run] != 0xAF && s[run] != 0xE4) ++run;
			k += (uint32_t)xrom_text_bn6(XROM_BN5_COLONEL_US, s + i, run - i, out + k, max - k);
			i = run;
		}
	}
	return k;
}

/* its versions' names from its game's Navi archive (each version's as its
 * game shows it), BN6's codes, each ended */
static void names_of(const XNavi *n, const Game *g) {
	for (int v = 0; v < n->versions && !xv_failed(); ++v) {
		uint32_t i = (uint32_t)(n->shown[v] ? n->shown[v] : n->code.names[v]) - 0x100u, at = g->names - BUS + 2 * i;
		if (i >= 0x100 || at + 2 > ROM_SIZE) { xv_fail("its name %d", v); return; }
		uint32_t p = g->names - BUS + get16(g->rom + at), len = 0;
		while (p + len < ROM_SIZE && len < NAME_MAX && g->rom[p + len] != TEXT_END) ++len;
		if (p + len >= ROM_SIZE || len >= NAME_MAX) { xv_fail("its name %d has no end", v); return; }
		uint32_t k = name_bn6(g, g->rom + p, len, rec.name[v], NAME_MAX - 1);
		rec.name[v][k] = TEXT_END;
		rec.name_len[v] = k + 1;
	}
}

static void ids_set(const XNavi *n, const Game *g) {
	const XVirusFamily *f = &n->code;
	names_of(n, g);
	uint32_t off = xn_names_end();
	for (int v = 0; v < n->versions && !xv_failed(); ++v) {
		uint32_t id = (uint32_t)(f->first_id + v);
		const uint8_t t[3] = { (uint8_t)v, 1, (uint8_t)f->ai };
		xg_set(XG_IDS, id, t);
		if (!xv_failed() && xn_name(id & 0xFF, off, rec.name[v], rec.name_len[v])) off += rec.name_len[v];
		uint32_t from = g->rows - BUS + 2u * ENEMY_DROPS * f->names[v];
		if (from + 2u * ENEMY_DROPS > ROM_SIZE) { xv_fail("its reward row %d", v); return; }
		uint8_t row[2 * ENEMY_DROPS];
		for (uint32_t k = 0; k < ENEMY_DROPS; ++k) {
			uint16_t e = get16(g->rom + from + 2 * k);
			rec.drops[v][k] = g->bn5 ? xchips_reward_bn6(e) : e;
			put16(row + 2 * k, rec.drops[v][k]);
		}
		if (!xv_failed()) xg_set(XG_ROWS, id, row);
	}
}

/* data.c's readers told of its ids, as the engine reads them now */
static void enemies_tell(const XNavi *n) {
	for (int v = 0; v < n->versions; ++v) {
		EnemyMoved e = { { (uint8_t)v, 1, (uint8_t)n->code.ai }, { 0 }, { 0 }, "", true, { 0 } };
		memcpy(e.drops, rec.drops[v], sizeof e.drops);
		memcpy(e.stats, rec.stats + BN6_STATS_SIZE * v, BN6_STATS_SIZE);
		memcpy(e.traits, rec.traits, BN6_TRAITS_SIZE);
		rom_text_bytes(rec.name[v], rec.name_len[v], e.name, sizeof e.name);
		if (!enemy_moved_set(n->code.first_id + v, &e)) fprintf(stderr, "xnavi: enemy id %#x read as BN6's (no room)\n", n->code.first_id + v);
	}
}

/* ---- its sounds BN6 numbers otherwise ---- */

#define SOUNDS_MAX 16
static int slots[SOUNDS_MAX];

/* each sound's slot in BN6, its game's song copied in (before the step: the songs' copies are xsong.c's own, made at once) */
static void sounds_in(const XNavi *n) {
	for (int i = 0; i < n->nsounds && i < SOUNDS_MAX; ++i) slots[i] = xsong_install_sound(n->code.game, n->sounds[i].song, -1);
}

/* movs rd,#imm / adds rd,#imm (imm8 forms) */
#define MOVS_IMM(rd, imm) (0x2000u | (uint32_t)(rd) << 8 | (uint32_t)(imm))
#define ADDS_IMM(rd, imm) (0x3000u | (uint32_t)(rd) << 8 | (uint32_t)(imm))

/* its sounds' immediates staged as the copies' slots, each checked against its game's instruction first (where a song could
 * not be copied, BN6's song of its number plays, said so) */
static void sounds_set(const XNavi *n) {
	const XVirusFamily *f = &n->code;
	const uint8_t *rom = xv_move_rom();
	for (int i = 0; i < n->nsounds && i < SOUNDS_MAX && !xv_failed(); ++i) {
		const XnSound *x = &n->sounds[i];
		const XvBlock *b = xvirus_block_of(f, x->at), *b2 = x->at2 ? xvirus_block_of(f, x->at2) : NULL;
		uint32_t lo = x->at2 ? x->song - 0xFFu : x->song;
		if (!b || (x->at2 && !b2) || get16(rom + (x->at - BUS)) != MOVS_IMM(0, lo) || (x->at2 && get16(rom + (x->at2 - BUS)) != ADDS_IMM(0, 0xFF))) {
			xv_fail("its sound %#x at %08x is not the one expected", x->song, (unsigned)x->at);
			return;
		}
		if (slots[i] < 0 || slots[i] > 0xFF) {
			fprintf(stderr, "xnavi: %s's sound %#x not copied in: BN6's of its number plays\n", f->name, x->song);
			continue;
		}
		uint8_t *p = xv_put(b->at + (x->at - b->start), 2, false), *q = x->at2 ? xv_put(b2->at + (x->at2 - b2->start), 2, false) : NULL;
		if (p) put16(p, (uint16_t)MOVS_IMM(0, slots[i]));
		if (q) put16(q, (uint16_t)ADDS_IMM(0, 0));
	}
}

/* ---- a Navi ---- */

#define INSTALLED_MAX XN_HOMES_MAX
static const XNavi *installed[INSTALLED_MAX];
static int ninstalled;

bool xnavi_install(const XNavi *n) {
	const XVirusFamily *f = &n->code;
	for (int i = 0; i < ninstalled; ++i)
		if (installed[i] == n) return true;
	if (!xn_grow() || (n->sprite_of >= 0 && !xnavi_sprite_install(n->sprite_of))) {
		fprintf(stderr, "xnavi: %s not installed: BN6's tables not grown, or its battle sprite not placed\n", f->name);
		return false;
	}
	for (int i = 0; i < n->nneeds; ++i)
		if (!xnavi_objects_have(n->needs[i].kind, n->needs[i].from)) {
			fprintf(stderr, "xnavi: %s not installed: the moved objects' type %d:%#x is not there\n", f->name, n->needs[i].kind, n->needs[i].from);
			return false;
		}
	Game g;
	sounds_in(n);
	xv_tx_begin();
	if (ninstalled >= INSTALLED_MAX) xv_fail("too many Navis");
	else if (!emu_ready() || !game_of(f->game, &g)) xv_fail("its game's ROM is not read");
	else if (sane(n, &g) && xv_move_begin(f)) {
		xv_move_code();
		if (!xv_failed()) records_set(n, &g);
		if (!xv_failed()) overlay_set(n, &g);
		if (!xv_failed()) ids_set(n, &g);
		xv_move_art();
		sounds_set(n);
	}
	if (xv_failed()) {
		fprintf(stderr, "xnavi: %s not installed: %s\n", f->name, xv_why());
		xv_tx_end();
		return false;
	}
	uint32_t used = xv_used();
	int veneers = xv_move_veneers();
	xv_commit_by(xn_writer);
	xv_tx_end();
	for (int i = 0; i < f->nblocks; ++i) xv_moved_add(f->blocks[i].start, f->blocks[i].at, f->blocks[i].end - f->blocks[i].start);
	for (int i = 0; i < f->nclones; ++i) xv_moved_add(f->clones[i].start, f->clones[i].at, f->clones[i].end - f->clones[i].start);
	xv_moved_add(f->struct1, f->home + XV_HOME_STRUCT1, BN6_TRAITS_SIZE);
	xv_moved_add(f->struct2, f->home + XV_HOME_STRUCT2, XV_VERSIONS * BN6_STATS_SIZE);
	xv_moved_add(entry_of(&g, XN_STATES, f->src_ai), f->home + XV_HOME_STATES, 4u * (uint32_t)f->nstates);
	if (n->overlay) xv_moved_add(n->overlay->masks, n->overlay->masks_at, n->overlay->masks_len);
	/* (each routine of its game's a word of its code names, and BN6's twin in its place: a callback it keeps reads alike in
	 * the differential's two traces) */
	for (int i = 0; i < f->nwords; ++i)
		if (f->words[i].from >= BUS && f->words[i].to >= BUS && f->words[i].to < EMU_FREE) xv_moved_add(f->words[i].from & ~1u, f->words[i].to & ~1u, 2);
	installed[ninstalled++] = n;
	enemies_tell(n);
	printf("xnavi: %s as Navi AI %d, enemy ids %#x-%#x: %u bytes written, %d veneers\n", f->name, f->ai, f->first_id, f->first_id + n->versions - 1,
		(unsigned)used, veneers);
	return true;
}

int xnavi_bn6_id(int id) {
	for (int i = 0; i < ninstalled; ++i)
		for (int v = 0; installed[i]->code.game == XROM_BN5_COLONEL_US && v < installed[i]->versions; ++v)
			if (installed[i]->code.names[v] == id) return installed[i]->code.first_id + v;
	return -1;
}

void xnavi_install_all(int which) {
	static bool done;
	if (done || !emu_ready() || which == XNAVI_NONE) return;
	if (which == XNAVI_BN5 && !XR[XROM_BN5_COLONEL_US].data) return;
	done = true;
	if (which == XNAVI_CONTROL) xnavi_install(&xnavi_control);
	else {
		xnavi_objects_install();
		for (int i = 0; i < xnavi_bn5_count; ++i) xnavi_install(xnavi_bn5[i]);
	}
}

void xnavi_install_core(int families, int navis) {
	xvirus_install(families);
	xnavi_install_all(navis);
}
