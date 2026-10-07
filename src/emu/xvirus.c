/* xvirus.h: a family's recipe (xvirus_families.c) carried out on the
 * core's ROM copy. Every write is staged first and made only once all of
 * the family's checks have passed; then BN6's tables, ids and names point at
 * what was written. */
#include "xvirus.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "data.h"
#include "debug.h"
#include "rom.h"
#include "xnavi.h"
#include "xvirus_families.h"

#define BUS        0x08000000u
#define HELD_LISTS 0xC0     /* a family's six held-chip lists, 0x20 bytes each */
#define NAMES_MAX  0x1000   /* the rebuilt names archive's room (XV_NAMES) */
#define TEXT_END   0xE6     /* a text archive's entry ends with it */

/* ---- the family's game: its ROM, its per-family tables and its viruses' names ---- */

typedef struct {
	const uint8_t *rom;
	uint32_t tables, count, step;   /* its per-family tables: the first, entries each, bytes apart */
	uint32_t names;                 /* its viruses' names, a text archive (a ROM offset) */
	uint32_t ice, ice_count;        /* the sizes of the ice blocks its viruses are frozen in, a byte per AI index (0: none) */
	uint32_t sprite_lists;          /* its sprite lists (a ROM offset, as RomLayout.sprite_lists) */
	uint32_t effects, neffects;     /* its effects, 4 bytes each (as BN6_EFFECTS) */
	uint32_t types;                 /* its attacks' routines by type (as BN6_T3_TYPES) */
} Game;

static bool game_of(int game, Game *g) {
	memset(g, 0, sizeof *g);
	if (!R.data || !R.layout) return false;
	if (game == XV_GAME_BN6) {
		/* (the sizes as BN6 reads them for its 32 AI indexes: AI 30's and 31's past its viruses' 30, in the Navis') */
		*g = (Game){ R.data, BN6_FAMILY_TABLES, BN6_FAMILIES, BN6_FAMILY_STEP, R.layout->enemy_names[0], BN6_ICE_SIZES, BN6_FAMILIES,
			R.layout->sprite_lists, BN6_EFFECTS, BN6_EFFECT_COUNT, BN6_T3_TYPES };
		return true;
	}
	if (game == XROM_BN5_COLONEL_US && XR[game].data && XR[game].layout) {
		/* (BN5 freezes no virus: no ice blocks' sizes) */
		*g = (Game){ XR[game].data, BN5_FAMILY_TABLES, BN5_FAMILIES, BN5_FAMILY_STEP, BN5_ENEMY_NAMES - BUS, 0, 0,
			XR[game].layout->sprite_lists, BN5_EFFECTS, BN5_EFFECT_COUNT, BN5_T3_TYPES };
		return true;
	}
	return false;
}

static bool in_rom(uint32_t a, uint32_t n) { return a >= BUS && a - BUS <= ROM_SIZE && n <= ROM_SIZE - (a - BUS); }
static uint32_t rom32(const uint8_t *rom, uint32_t a) { return get32(rom + (a - BUS)); }

/* ---- staged writes ---- */

#define TX_BYTES  0x10000
#define TX_WRITES 128
static struct {
	uint8_t bytes[TX_BYTES];
	uint32_t used;
	int n;
	struct { uint32_t at, off, len; } w[TX_WRITES];
	bool failed;
	char why[240];
} tx;

static void tx_begin(void) {
	tx.used = 0;
	tx.n = 0;
	tx.failed = false;
	tx.why[0] = 0;
}

static bool fail(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static bool fail(const char *fmt, ...) {
	if (!tx.failed) {
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(tx.why, sizeof tx.why, fmt, ap);
		va_end(ap);
	}
	tx.failed = true;
	return false;
}

/* room for len bytes to be written at `at`, zeroed; `fresh`: where the
 * core's copy must still be free (0xFF, as emu.c pads it). NULL once the
 * family has failed. */
static uint8_t *tx_put(uint32_t at, uint32_t len, bool fresh) {
	if (tx.failed) return NULL;
	if (tx.n >= TX_WRITES || len > TX_BYTES - tx.used) { fail("more to write than %u bytes", (unsigned)TX_BYTES); return NULL; }
	if (fresh)
		for (uint32_t i = 0; i < len; ++i)
			if (emu_read8(at + i) != 0xFF) { fail("%08x is not free", (unsigned)(at + i)); return NULL; }
	tx.w[tx.n].at = at;
	tx.w[tx.n].off = tx.used;
	tx.w[tx.n].len = len;
	++tx.n;
	uint8_t *p = tx.bytes + tx.used;
	memset(p, 0, len);
	tx.used += (len + 3) & ~3u;
	return p;
}

/* a word of BN6's own (or of what is already installed) changed: it must
 * read `old` in the core's copy now */
static void tx_word(uint32_t at, uint32_t old, uint32_t v) {
	uint32_t now = emu_read32(at);
	if (now != old) { fail("%08x reads %08x, not %08x", (unsigned)at, (unsigned)now, (unsigned)old); return; }
	uint8_t *p = tx_put(at, 4, false);
	if (p) put32(p, v);
}

static void tx_commit(void) {
	for (int i = 0; i < tx.n; ++i) emu_write(tx.w[i].at, tx.bytes + tx.w[i].off, tx.w[i].len);
}

/* ---- what was moved where ---- */

#define MOVED_MAX 32
static XvMoved moved[MOVED_MAX];
static int nmoved;

int xvirus_moved(XvMoved *out, int max) {
	int n = nmoved < max ? nmoved : max;
	memcpy(out, moved, (size_t)n * sizeof *out);
	return n;
}

static void moved_add(uint32_t from, uint32_t to, uint32_t len) {
	for (int i = 0; i < nmoved; ++i)
		if (moved[i].from == from && moved[i].to == to) return;
	if (nmoved < MOVED_MAX) moved[nmoved++] = (XvMoved){ from, to, len };
}

/* ---- a family's code ---- */

typedef struct {
	const XVirusFamily *f;
	const uint8_t *rom;
	ThumbVeneers pool;
	uint8_t veneers[XV_HOME_SIZE - XV_HOME_VENEERS];
	uint32_t missing;   /* a call the recipe does not name */
} Ctx;

/* where an address of a copied block is in BN6 (its Thumb bit kept); 0 for none */
static uint32_t moved_to(const XVirusFamily *f, uint32_t a) {
	const XvBlock *b = xvirus_block_of(f, a & ~1u);
	return b ? a - b->start + b->at : 0;
}

/* a pointer into the blocks, moved: a Thumb one must name code, any
 * other the start of a range of data */
static bool pointer_ok(const XVirusFamily *f, uint32_t v) {
	if (v & 1) return xvirus_is_code(f, v & ~1u);
	const XvData *d = xvirus_data_at(f, v);
	return d && d->start == v;
}

static uint32_t resolve(uint32_t target, void *user) {
	Ctx *c = user;
	if (xvirus_block_of(c->f, target)) return xvirus_is_code(c->f, target) ? moved_to(c->f, target) : 0;
	for (int i = 0; i < c->f->ncalls; ++i)
		if (c->f->calls[i].from == target) {
			uint32_t to = c->f->calls[i].to & ~1u;
			if (!in_rom(to, 2)) break;
			return thumb_veneer_get(&c->pool, to);
		}
	c->missing = target;
	return 0;
}

/* One instruction of a block's code checked: a load from its literal pool
 * must find a word of the recipe's pools, and a branch stay in the block. */
static bool insn_ok(const XVirusFamily *f, const XvBlock *b, uint32_t a, uint16_t op) {
	uint32_t pc4 = (a + 4) & ~3u;
	if ((op & 0xF800) == 0x4800) {   /* ldr rd,[pc,#imm] */
		uint32_t t = pc4 + (op & 0xFFu) * 4;
		const XvData *d = xvirus_data_at(f, t);
		return (d && d->kind == XV_POOL && t + 4 <= d->end) || fail("the load at %08x reads %08x, in no pool", (unsigned)a, (unsigned)t);
	}
	if ((op & 0xF800) == 0xA000) {   /* add rd,pc,#imm */
		uint32_t t = pc4 + (op & 0xFFu) * 4;
		return (t >= b->start && t < b->end) || fail("the address at %08x is out of its block", (unsigned)a);
	}
	int32_t off = 0;
	if ((op & 0xF800) == 0xE000) off = (int32_t)((uint32_t)(op & 0x7FF) << 21) >> 20;           /* b */
	else if ((op & 0xF000) == 0xD000 && (op & 0x0F00) < 0x0E00) off = (int8_t)(op & 0xFF) * 2;  /* b<cond> */
	else if (op == 0x4778) return fail("ARM code at %08x", (unsigned)a);                          /* bx pc */
	else return true;
	uint32_t t = a + 4 + (uint32_t)off;
	return (t >= b->start && t < b->end && xvirus_is_code(f, t)) || fail("the branch at %08x leaves its code", (unsigned)a);
}

/* The block's code checked, run by run, and its BLs found (offsets in the block) */
static int code_walk(const XVirusFamily *f, const XvBlock *b, const uint8_t *code, uint32_t *offs, int max) {
	int n = 0;
	for (uint32_t a = b->start; a < b->end && !tx.failed;) {
		if (!xvirus_is_code(f, a)) { a += 2; continue; }
		uint32_t run = a;
		while (a < b->end && xvirus_is_code(f, a)) a += 2;
		int got = thumb_bl_scan(code + (run - b->start), a - run, offs + n, max - n);
		if (got < 0 || n + got > max) {
			fail("half a BL, or too many, in %08x-%08x", (unsigned)run, (unsigned)a);
			return -1;
		}
		for (int i = n; i < n + got; ++i) offs[i] += run - b->start;
		for (uint32_t p = run; p < a; p += 2) {
			uint16_t op = get16(code + (p - b->start));
			if (thumb_is_bl(op, p + 4 <= a ? get16(code + (p + 2 - b->start)) : 0)) { p += 2; continue; }
			if (!insn_ok(f, b, p, op)) return -1;
		}
		n += got;
	}
	return tx.failed ? -1 : n;
}

/* One word of a block's data: moved where it points into the blocks,
 * named anew where it is one of the recipe's words, kept where it is a
 * number; false (the family failed) where none of these may be */
static bool word_fix(const XVirusFamily *f, int kind, uint32_t a, uint8_t *w) {
	uint32_t v = get32(w);
	if (xvirus_block_of(f, v & ~1u)) {
		if (kind == XV_RAW) return fail("raw data at %08x points into the code (%08x)", (unsigned)a, (unsigned)v);
		if (!pointer_ok(f, v) || (kind == XV_TABLE && !(v & 1))) return fail("the word at %08x points at %08x, neither code nor data's start", (unsigned)a, (unsigned)v);
		put32(w, moved_to(f, v));
		return true;
	}
	if (kind == XV_TABLE) return fail("the jump table's word at %08x points out (%08x)", (unsigned)a, (unsigned)v);
	/* (RAM, I/O and ROM: what the code names there is its game's) */
	if (kind == XV_RAW || v < 0x02000000u || v >= 0x0E000000u) return true;
	for (int k = 0; k < f->nwords; ++k)
		if (f->words[k].from == v) {
			put32(w, f->words[k].to);
			return true;
		}
	return fail("the literal at %08x names %08x, which the recipe does not", (unsigned)a, (unsigned)v);
}

/* A block's data: pools' words and tables moved, raw bytes checked, dead ones zeroed */
static void data_fix(const XVirusFamily *f, const XvBlock *b, uint8_t *code) {
	for (int i = 0; i < f->ndata && !tx.failed; ++i) {
		const XvData *d = &f->data[i];
		if (d->start < b->start || d->start >= b->end) continue;
		if (d->kind == XV_ZERO) {
			memset(code + (d->start - b->start), 0, d->end - d->start);
			continue;
		}
		for (uint32_t a = (d->start + 3) & ~3u; a + 4 <= d->end && word_fix(f, d->kind, a, code + (a - b->start)); a += 4) {}
	}
}

static void imms_patch(const XVirusFamily *f, const XvBlock *b, uint8_t *code) {
	for (int i = 0; i < f->nimms && !tx.failed; ++i) {
		const XvImm *m = &f->imms[i];
		if (m->at < b->start || m->at >= b->end) continue;
		uint8_t *p = code + (m->at - b->start);
		uint16_t insn = get16(p);
		ThumbImm was = { m->form, m->rd, m->rn, m->was };
		if (!xvirus_is_code(f, m->at) || (insn & 0xF000) == 0xF000 || !thumb_imm_patch(&insn, &was, m->now)) {
			fail("the immediate at %08x is not the one expected", (unsigned)m->at);
			return;
		}
		put16(p, insn);
	}
}

#define BLS_MAX 256

static void block_move(Ctx *c, const XvBlock *b) {
	const XVirusFamily *f = c->f;
	uint32_t len = b->end - b->start;
	if (!len || !in_rom(b->start, len)) { fail("block %08x-%08x outside its ROM", (unsigned)b->start, (unsigned)b->end); return; }
	if (xvirus_crc32(c->rom + (b->start - BUS), len) != b->crc) { fail("block %08x-%08x is not the one expected (its CRC32)", (unsigned)b->start, (unsigned)b->end); return; }
	uint8_t *code = tx_put(b->at, len, true);
	if (!code) return;
	memcpy(code, c->rom + (b->start - BUS), len);
	static uint32_t offs[BLS_MAX];
	int n = code_walk(f, b, code, offs, BLS_MAX);
	if (n < 0) return;
	data_fix(f, b, code);
	imms_patch(f, b, code);
	if (tx.failed) return;
	ThumbBlock tb = { code, len, b->start, b->at };
	c->missing = 0;
	if (!thumb_block_rebase(&tb, offs, n, resolve, c)) {
		if (c->missing) fail("a call to %08x the recipe does not name", (unsigned)c->missing);
		else fail("a call out of %08x-%08x cannot be made (veneers full, or out of reach)", (unsigned)b->start, (unsigned)b->end);
	}
}

/* ---- its records and BN6's tables ---- */

static uint32_t ref_to(const XVirusFamily *f, XvRef r) {
	if (r.own) {
		uint32_t to = (r.addr & 1) && xvirus_is_code(f, r.addr & ~1u) ? moved_to(f, r.addr) : 0;
		if (!to) fail("its own %08x is no code of its blocks", (unsigned)r.addr);
		return to;
	}
	if (!(r.addr & 1) || !in_rom(r.addr & ~1u, 2)) fail("%08x is no routine of BN6's", (unsigned)r.addr);
	return r.addr;
}

/* the literal that names table k (1 and 2: the triples' virus words RomLayout has) */
static uint32_t ref_of(int k) {
	static const uint32_t refs[XV_TABLES] = {
		BN6_FAMILY_REF_STATES, 0, 0, BN6_FAMILY_REF_FRAME, BN6_FAMILY_REF_SPAWN, BN6_FAMILY_REF_SPAWNED, BN6_FAMILY_REF_PARALYSIS,
		BN6_FAMILY_REF_FREEZE, BN6_FAMILY_REF_BUBBLE, BN6_FAMILY_REF_FLINCH, BN6_FAMILY_REF_DRAG,
	};
	return k == 1 ? BUS + R.layout->enemy_traits : k == 2 ? BUS + R.layout->enemy_stats : refs[k];
}

static uint32_t table_at(int k) { return BN6_FAMILY_TABLES + (uint32_t)k * BN6_FAMILY_STEP; }
static uint32_t grown_at(int k) { return XV_GROWN + (uint32_t)k * 0x100u; }

/* Its entry in each of the eleven tables: grown first where they are not
 * yet, each table's 32 entries as BN6 has them and AI 0's (TestVirs') in
 * the rest, and their literals pointed at them */
static void tables_set(const XVirusFamily *f, const uint32_t entry[XV_TABLES]) {
	bool grown = emu_read32(ref_of(0)) == grown_at(0);
	for (int k = 0; k < XV_TABLES && !tx.failed; ++k) {
		if (grown) {
			tx_word(grown_at(k) + 4u * (uint32_t)f->ai, emu_read32(grown_at(k)), entry[k]);
			continue;
		}
		uint8_t *t = tx_put(grown_at(k), 4u * XV_FAMILIES, true);
		if (!t) return;
		for (uint32_t i = 0; i < XV_FAMILIES; ++i) {
			uint32_t from = table_at(k) + 4u * (i < BN6_FAMILIES ? i : 0);
			uint32_t v = emu_read32(from);
			if (v != rom32(R.data, from)) { fail("BN6's table %d was changed (%08x)", k, (unsigned)from); return; }
			put32(t + 4 * i, i == (uint32_t)f->ai ? entry[k] : v);
		}
		tx_word(ref_of(k), table_at(k), grown_at(k));
	}
}

/* its versions' enemy ids: (version, a virus, its AI index), where BN6
 * gives them Mettaur's (BN6_FILLER_AI) */
static void ids_set(const XVirusFamily *f) {
	for (int v = 0; v < XV_VERSIONS && !tx.failed; ++v) {
		uint32_t at = BUS + R.layout->enemy_ids + 3u * (uint32_t)(f->first_id + v);
		if (emu_read8(at) != v || emu_read8(at + 1) || emu_read8(at + 2) != BN6_FILLER_AI) {
			fail("enemy id %#x is not a spare one", f->first_id + v);
			return;
		}
		uint8_t *p = tx_put(at, 3, false);
		if (p) { p[0] = (uint8_t)v; p[1] = 0; p[2] = (uint8_t)f->ai; }
	}
}

/* ---- the ice block a frozen virus is held in: its size by its AI index (BN6_ICE_SIZES) ---- */

/* the family's size: the recipe's, or its game's for its AI index there; -1 none */
static int ice_of(const XVirusFamily *f, const Game *g) {
	if (f->ice != XV_ICE_OWN) return f->ice;
	if (!g->ice || f->src_ai < 0 || (uint32_t)f->src_ai >= g->ice_count) {
		fail("no ice block size in its game for AI %d", f->src_ai);
		return -1;
	}
	return g->rom[g->ice - BUS + (uint32_t)f->src_ai];
}

/* BN6's sizes grown first where they are not yet (each of BN6's AI indexes
 * as BN6 reads it now, AI 0's in the rest) and the triple's virus word
 * pointed at them; the family's in its slot, which must still hold AI 0's */
static void ice_set(const XVirusFamily *f, const Game *g) {
	int size = ice_of(f, g);
	if (size < 0) return;
	uint32_t at = XV_ICE + (uint32_t)f->ai;
	if (emu_read32(BN6_ICE_SIZES_REF) == XV_ICE) {
		if (emu_read8(at) != emu_read8(XV_ICE)) { fail("the ice block size of AI %d is taken", f->ai); return; }
		uint8_t *p = tx_put(at, 1, false);
		if (p) *p = (uint8_t)size;
		return;
	}
	uint8_t *t = tx_put(XV_ICE, XV_FAMILIES, true);
	if (!t) return;
	for (uint32_t i = 0; i < XV_FAMILIES; ++i) {
		uint32_t from = BN6_ICE_SIZES + (i < BN6_FAMILIES ? i : 0);
		t[i] = emu_read8(from);
		if (t[i] != R.data[from - BUS]) { fail("BN6's ice block sizes were changed (%08x)", (unsigned)from); return; }
	}
	t[f->ai] = (uint8_t)size;
	tx_word(BN6_ICE_SIZES_REF, BN6_ICE_SIZES, XV_ICE);
}

/* ---- the viruses' names: BN6's archive rebuilt, the families' after its own ---- */

static struct {
	uint8_t bytes[NAMES_MAX];
	uint32_t len, entries, own;   /* (own: BN6's archive's length) */
	XvMoved moved[XV_VERSIONS + 1];   /* the last family's names, from its game to the archive, and BN6's own the first time */
	int nmoved;
} names;

/* an archive's entry `i` in `rom` (ending with TEXT_END): its bytes and length, 0 for none */
static uint32_t entry_of(const uint8_t *rom, uint32_t archive, uint32_t i, const uint8_t **out) {
	if (archive + 2 * i + 2 > ROM_SIZE) return 0;
	uint32_t p = archive + get16(rom + archive + 2 * i), n = 0;
	while (p + n < ROM_SIZE && n < 32 && rom[p + n] != TEXT_END) ++n;
	*out = rom + p;
	return p + n < ROM_SIZE && n < 32 ? n + 1 : 0;
}

static bool names_start(void) {
	if (names.len) return true;
	uint32_t a = R.layout->enemy_names[0];
	names.entries = get16(R.data + a) / 2u;
	uint32_t end = 0;
	for (uint32_t i = 0; i < names.entries; ++i) {
		const uint8_t *s;
		uint32_t n = entry_of(R.data, a, i, &s);
		if (!n) return fail("BN6's names archive unread");
		uint32_t to = (uint32_t)(s - (R.data + a)) + n;
		if (to > end) end = to;
	}
	if (end > NAMES_MAX) return fail("BN6's names archive is %u bytes", (unsigned)end);
	memcpy(names.bytes, R.data + a, end);
	names.len = names.own = end;
	return true;
}

/* its versions' names after the archive's, their entries pointed at them;
 * the archive kept as it was where the family fails */
static void names_set(const XVirusFamily *f, const Game *g) {
	bool first = emu_read32(BN6_ENEMY_NAMES_REF) != XV_NAMES;
	if (!names_start()) return;
	static uint8_t work[NAMES_MAX];
	uint32_t len = names.len;
	memcpy(work, names.bytes, len);
	names.nmoved = 0;
	if (first) names.moved[names.nmoved++] = (XvMoved){ BUS + R.layout->enemy_names[0], XV_NAMES, names.own };
	for (int v = 0; v < XV_VERSIONS; ++v) {
		const uint8_t *s;
		uint32_t n = entry_of(g->rom, g->names, f->names[v], &s), id = (uint32_t)(f->first_id + v);
		if (!n || id >= names.entries || len + n > NAMES_MAX) { fail("no room for its name %d", v); return; }
		put16(work + 2 * id, len);
		memcpy(work + len, s, n);
		names.moved[names.nmoved++] = (XvMoved){ BUS + (uint32_t)(s - g->rom), XV_NAMES + len, n };
		len += n;
	}
	uint8_t *p = tx_put(XV_NAMES, len, first);
	if (!p) return;
	memcpy(p, work, len);
	if (first) {
		tx_word(BN6_ENEMY_NAMES_REF, BUS + R.layout->enemy_names[0], XV_NAMES);
		tx_word(BN6_ENEMY_NAMES_REF2, BUS + R.layout->enemy_names[0], XV_NAMES);
	}
	if (tx.failed) return;
	memcpy(names.bytes, work, len);   /* (kept for the next family: written whole with it) */
	names.len = len;
}

/* ---- what it draws: its sprites and effects, and its attacks' types ---- */

/* One of BN6's tables of words grown past its end, named by the one
 * literal at `ref`: BN6's `count` words from `from`, then `fill` to `cap`
 * words at `to`; staged once a family, as it stands where an earlier one
 * grew it, or made and its literal pointed at it */
typedef struct { uint32_t ref, from, count, to, cap, fill; uint8_t *t; } Grown;

static uint8_t *grown(Grown *gr) {
	if (gr->t || tx.failed) return gr->t;
	bool done = emu_read32(gr->ref) == gr->to;
	uint8_t *t = tx_put(gr->to, 4 * gr->cap, !done);
	for (uint32_t i = 0; t && i < gr->cap; ++i) {
		uint32_t at = done ? gr->to + 4 * i : gr->from + 4 * i, v = done || i < gr->count ? emu_read32(at) : gr->fill;
		if (!done && i < gr->count && v != rom32(R.data, at)) { fail("BN6's table at %08x was changed", (unsigned)gr->from); return NULL; }
		put32(t + 4 * i, v);
	}
	if (t && !done) tx_word(gr->ref, gr->from, gr->to);
	return gr->t = t;
}

/* a grown table's word i, which must still read its fill (free), set to v */
static void grown_set(Grown *gr, uint32_t i, uint32_t v) {
	uint8_t *t = grown(gr);
	if (!t) return;
	if (i >= gr->cap || get32(t + 4 * i) != gr->fill) { fail("%08x's entry %#x is taken", (unsigned)gr->to, (unsigned)i); return; }
	put32(t + 4 * i, v);
}

/* Its sprites copied (a compressed one decompressed: BN6 draws a battle
 * sprite whose pointer has bit 31 only once its battle's start put it in
 * a cache) and listed: list 4's past its end in the grown list, another
 * list's at a number of BN6's that points at the placeholder */
static void sprites_set(const XVirusFamily *f, const Game *g, Grown *list4) {
	for (int i = 0; i < f->nsprites && !tx.failed; ++i) {
		const XvSprite *s = &f->sprites[i];
		uint32_t len;
		uint8_t *d = xnavi_sprite_bytes(g->rom, g->sprite_lists, s->list, s->index, &len);
		if (!d || len > s->room) {
			fail("its sprite %d:%#x (%u bytes, %u of room)", s->list, s->index, (unsigned)len, (unsigned)s->room);
			free(d);
			return;
		}
		uint8_t *p = tx_put(s->at, len, true);
		if (p) memcpy(p, d, len);
		free(d);
		if (s->to_list == BN6_SPRITES_ATTACKS) {
			grown_set(list4, s->to_index, s->at);
			continue;
		}
		uint32_t entry = rom32(R.data, BUS + R.layout->sprite_lists + 4u * s->to_list) + 4u * s->to_index;
		if (!in_rom(entry, 4) || rom32(R.data, entry) != BN6_SPRITE_PLACEHOLDER) { fail("BN6's sprite %d:%#x is no placeholder", s->to_list, s->to_index); return; }
		tx_word(entry, BN6_SPRITE_PLACEHOLDER, s->at);
	}
}

/* Its effects in the grown effects' table: its game's entry with its
 * sprite as BN6 lists it (one of the recipe's) */
static void effects_set(const XVirusFamily *f, const Game *g, Grown *fx) {
	for (int i = 0; i < f->neffects && !tx.failed; ++i) {
		const XvId *e = &f->effects[i];
		const uint8_t *src = e->from < g->neffects ? g->rom + (g->effects - BUS) + 4u * e->from : NULL;
		const XvSprite *s = NULL;
		for (int k = 0; src && k < f->nsprites; ++k)
			if (4u * f->sprites[k].list == src[0] && f->sprites[k].index == src[1]) s = &f->sprites[k];
		if (!s) { fail("its effect %#x draws none of its sprites", e->from); return; }
		grown_set(fx, e->to, (uint32_t)(4u * s->to_list) | (uint32_t)s->to_index << 8 | (uint32_t)src[2] << 16 | (uint32_t)src[3] << 24);
	}
}

/* Its attacks: each type's routine its own code, given an empty type of BN6's */
static void types_set(const XVirusFamily *f, const Game *g) {
	if (f->ntypes && (emu_read32(BN6_T3_TYPES_REF) != BN6_T3_TYPES || rom32(R.data, BN6_T3_TYPES_REF) != BN6_T3_TYPES)) {
		fail("BN6's attacks' table moved");
		return;
	}
	for (int i = 0; i < f->ntypes && !tx.failed; ++i) {
		const XvId *t = &f->types[i];
		uint32_t own = rom32(g->rom, g->types + 4u * t->from), slot = BN6_T3_TYPES + 4u * t->to;
		uint32_t to = (own & 1) && xvirus_is_code(f, own & ~1u) ? moved_to(f, own) : 0;
		if (!to) fail("its attack %#x (%08x) is none of its code", t->from, (unsigned)own);
		else if (rom32(R.data, slot)) fail("BN6's attack %#x is not empty", t->to);
		else tx_word(slot, 0, to);
	}
}

/* the grown tables, where they were, for a trace's reader */
static void art_moved(void) {
	uint32_t ref = BUS + R.layout->sprite_lists + 4u * BN6_SPRITES_ATTACKS;
	if (emu_read32(ref) == XV_LIST) moved_add(rom32(R.data, ref), XV_LIST, 4u * BN6_SPRITES_ATTACKS_COUNT);
	if (emu_read32(BN6_EFFECTS_REF) == XV_EFFECTS) moved_add(BN6_EFFECTS, XV_EFFECTS, 4u * BN6_EFFECT_COUNT);
}

static void art_set(const XVirusFamily *f, const Game *g) {
	uint32_t ref = BUS + R.layout->sprite_lists + 4u * BN6_SPRITES_ATTACKS;
	Grown list4 = { ref, rom32(R.data, ref), BN6_SPRITES_ATTACKS_COUNT, XV_LIST, XV_LIST_MAX, BN6_SPRITE_PLACEHOLDER, NULL };
	Grown fx = { BN6_EFFECTS_REF, BN6_EFFECTS, BN6_EFFECT_COUNT, XV_EFFECTS, XV_EFFECTS_MAX, rom32(R.data, BN6_EFFECTS), NULL };
	sprites_set(f, g, &list4);
	effects_set(f, g, &fx);
	types_set(f, g);
}

/* ---- deleted: BN6's count is 32 bytes, one per AI index, and a rare version's
 * AI index names a chat after the results (BN6_DELETE_RARE): none for 32 and up ---- */

static HookAct counted(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] < BN6_FAMILIES) return HOOK_CONTINUE;
	r->r[0] = 0;   /* (the read's answer: none) */
	return HOOK_RETURN;
}

static void counts_hook(void) {
	static bool done;
	if (done) return;
	done = true;
	static const uint32_t at[3] = { BN6_DELETE_COUNT, BN6_DELETE_COUNT_READ, BN6_DELETE_RARE };
	for (int i = 0; i < 3; ++i)
		if (!emu_hook(at[i], counted, NULL)) fprintf(stderr, "xvirus: no hook at %08x: a moved family's deletion counts past BN6's\n", (unsigned)at[i]);
}

/* ---- a family ---- */

/* the recipe whole, and its records and its state table where its game's
 * tables have them (the state table the range of its block it zeroes) */
static bool sane(const XVirusFamily *f, const Game *g) {
	if (!xvirus_recipe_ok(f, tx.why, sizeof tx.why)) {
		tx.failed = true;
		return false;
	}
	if (f->src_ai < 0 || (uint32_t)f->src_ai >= g->count) return fail("its AI index in its game, %d", f->src_ai);
	uint32_t at = g->tables + 4u * (uint32_t)f->src_ai, st = rom32(g->rom, at);
	uint32_t s1 = rom32(g->rom, at + g->step), s2 = rom32(g->rom, at + 2 * g->step);
	if (s1 != f->struct1 || s2 != f->struct2) return fail("its records are at %08x and %08x in its game", (unsigned)s1, (unsigned)s2);
	const XvData *d = xvirus_data_at(f, st);
	if (!d || d->kind != XV_ZERO || d->start != st) return fail("its state table is at %08x in its game", (unsigned)st);
	return true;
}

/* Its struct1 as BN6 reads it (BN6_TRAITS_*): its game's as it is, or
 * laid out BN6's way where the recipe's traits are laid, its game's
 * record (BN5's layout) naming the recipe's battle sprite, mode and
 * shadow first */
static void traits_of(const XVirusFamily *f, const uint8_t *src, uint8_t out[BN6_TRAITS_SIZE]) {
	memcpy(out, src, BN6_TRAITS_SIZE);
	if (!f->traits.laid) return;
	const XvSprite *s = &f->sprites[0];
	if (src[BN6_TRAITS_LIST] != 4u * s->list || src[BN6_TRAITS_SPRITE] != s->index || src[BN6_TRAITS_MODE] != f->traits.mode ||
		!src[BN5_TRAITS_SHADOW] != !f->traits.shadow) {
		fail("its struct1 is not the one its recipe names");
		return;
	}
	out[BN6_TRAITS_LIST] = (uint8_t)(4u * s->to_list);
	out[BN6_TRAITS_SPRITE] = s->to_index;
	out[BN6_TRAITS_ELEMENT] = f->traits.element;
	out[BN6_TRAITS_WEAKNESS] = f->traits.weakness;
	out[BN6_TRAITS_SHADOW] = f->traits.shadow;
}

/* its records as the engine reads them, kept for data.c's readers
 * (enemies_tell) once it is installed */
static struct { uint8_t traits[BN6_TRAITS_SIZE], stats[XV_VERSIONS * BN6_STATS_SIZE]; } rec;

static void records_set(const XVirusFamily *f, const Game *g, uint32_t entry[XV_TABLES]) {
	uint8_t *st = tx_put(f->home + XV_HOME_STATES, 4u * (uint32_t)f->nstates, true);
	for (int i = 0; st && i < f->nstates; ++i) put32(st + 4 * i, ref_to(f, f->states[i]));
	uint8_t *s1 = tx_put(f->home + XV_HOME_STRUCT1, BN6_TRAITS_SIZE, true), *s2 = tx_put(f->home + XV_HOME_STRUCT2, sizeof rec.stats, true);
	if (!s1 || !s2 || !in_rom(f->struct1, BN6_TRAITS_SIZE) || !in_rom(f->struct2, sizeof rec.stats)) { fail("its records"); return; }
	traits_of(f, g->rom + (f->struct1 - BUS), rec.traits);
	memcpy(rec.stats, g->rom + (f->struct2 - BUS), sizeof rec.stats);
	memcpy(s1, rec.traits, BN6_TRAITS_SIZE);
	memcpy(s2, rec.stats, sizeof rec.stats);
	entry[0] = f->home + XV_HOME_STATES;
	entry[1] = f->home + XV_HOME_STRUCT1;
	entry[2] = f->home + XV_HOME_STRUCT2;
	for (int k = 0; k < XV_HOOKS; ++k) entry[3 + k] = ref_to(f, f->hooks[k]);
	uint32_t held = f->held;
	if (!held) {
		uint8_t *h = tx_put(f->home + XV_HOME_HELD, HELD_LISTS, true);
		if (h) memset(h, 0xFF, HELD_LISTS);
		held = f->home + XV_HOME_HELD;
	} else if (!in_rom(held, HELD_LISTS)) fail("its held chips at %08x", (unsigned)held);
	uint32_t at = BN6_HELD_CHIPS + 4u * (uint32_t)f->ai;
	tx_word(at, rom32(R.data, at), held);
}

/* data.c's readers told of its ids (EnemyMoved), as the engine reads them now */
static void enemies_tell(const XVirusFamily *f) {
	for (int v = 0; v < XV_VERSIONS; ++v) {
		EnemyMoved e = { { (uint8_t)v, 0, (uint8_t)f->ai }, { 0 }, { 0 }, "" };
		memcpy(e.stats, rec.stats + BN6_STATS_SIZE * v, BN6_STATS_SIZE);
		memcpy(e.traits, rec.traits, BN6_TRAITS_SIZE);
		uint32_t at = get16(names.bytes + 2u * (uint32_t)(f->first_id + v));
		if (at < names.len) rom_text_bytes(names.bytes + at, names.len - at, e.name, sizeof e.name);
		if (!enemy_moved_set(f->first_id + v, &e)) fprintf(stderr, "xvirus: enemy id %#x read as BN6's (no room)\n", f->first_id + v);
	}
}

#define INSTALLED_MAX 8
static const XVirusFamily *installed[INSTALLED_MAX];
static int ninstalled;

bool xvirus_install_family(const XVirusFamily *f) {
	for (int i = 0; i < ninstalled; ++i)
		if (installed[i] == f) return true;
	tx_begin();
	Game g = { 0 };
	static Ctx c;
	memset(&c, 0, sizeof c);
	c.f = f;
	c.pool = (ThumbVeneers){ c.veneers, f->home + XV_HOME_VENEERS, (int)(sizeof c.veneers / THUMB_VENEER_SIZE), 0 };
	if (ninstalled >= INSTALLED_MAX) fail("too many families");
	else if (!emu_ready() || !game_of(f->game, &g)) fail("its game's ROM is not read");
	else if (sane(f, &g)) {
		c.rom = g.rom;
		for (int i = 0; i < f->nblocks && !tx.failed; ++i) block_move(&c, &f->blocks[i]);
		uint32_t entry[XV_TABLES] = { 0 };
		if (!tx.failed) records_set(f, &g, entry);
		uint8_t *v = c.pool.n ? tx_put(c.pool.base, (uint32_t)c.pool.n * THUMB_VENEER_SIZE, true) : NULL;
		if (v) memcpy(v, c.veneers, (size_t)c.pool.n * THUMB_VENEER_SIZE);
		if (!tx.failed) tables_set(f, entry);
		if (!tx.failed) ice_set(f, &g);
		if (!tx.failed) ids_set(f);
		if (!tx.failed) names_set(f, &g);
		if (!tx.failed) art_set(f, &g);
	}
	if (tx.failed) {
		fprintf(stderr, "xvirus: %s not installed: %s\n", f->name, tx.why);
		return false;
	}
	tx_commit();
	counts_hook();
	for (int i = 0; i < f->nblocks; ++i) moved_add(f->blocks[i].start, f->blocks[i].at, f->blocks[i].end - f->blocks[i].start);
	moved_add(f->struct1, f->home + XV_HOME_STRUCT1, 8);
	moved_add(f->struct2, f->home + XV_HOME_STRUCT2, 36);
	moved_add(rom32(g.rom, g.tables + 4u * (uint32_t)f->src_ai), f->home + XV_HOME_STATES, 4u * (uint32_t)f->nstates);
	for (int i = 0; i < names.nmoved; ++i) moved_add(names.moved[i].from, names.moved[i].to, names.moved[i].len);
	art_moved();
	installed[ninstalled++] = f;
	enemies_tell(f);
	printf("xvirus: %s as AI %d, enemy ids %#x-%#x: %u bytes written, %d veneers\n", f->name, f->ai, f->first_id,
		f->first_id + XV_VERSIONS - 1, (unsigned)tx.used, c.pool.n);
	return true;
}

void xvirus_install(int which) {
	static bool done;
	if (done || !emu_ready()) return;
	done = true;
	if (which == XVIRUS_CONTROL) xvirus_install_family(&xvirus_control);
	else if (which == XVIRUS_FAMILIES && XR[XROM_BN5_COLONEL_US].data) xvirus_install_family(&xvirus_cangard);
}
