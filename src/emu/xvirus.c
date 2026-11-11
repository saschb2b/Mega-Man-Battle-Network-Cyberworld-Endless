/* xvirus.h: a family's recipe (xvirus_families.c) carried out on the
 * core's ROM copy. Every write is staged first (xvirus_tx.h) and made only
 * once all of the family's checks have passed; its entries go into BN6's
 * tables as xvirus_grow.c grew them, each slot checked free first. */
#include "xvirus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "bytes.h"
#include "claims.h"
#include "data.h"
#include "rom.h"
#include "xattack_grow.h"
#include "xchips.h"
#include "xfields.h"
#include "xnavi.h"
#include "xnavi_bn4_room.h"
#include "xvirus_bn4.h"
#include "xvirus_bn5.h"
#include "xvirus_dispatch.h"
#include "xvirus_families.h"
#include "xvirus_grow.h"
#include "xvirus_move.h"
#include "xvirus_shim.h"
#include "xvirus_songs.h"
#include "xvirus_tx.h"

#define BUS        0x08000000u
#define HELD_LISTS 0xC0     /* a family's six held-chip lists, 0x20 bytes each */
#define TEXT_END   0xE6     /* a text archive's entry ends with it */
#define NAME_BYTES   32       /* a name's bytes at most, its end among them */

_Static_assert(XV_IDS <= ENEMY_IDS_MAX, "data.c is told of every id a family may take");

/* ---- the family's game: its ROM, its per-family tables and its viruses' names ---- */

typedef struct {
	const uint8_t *rom;
	uint32_t tables, count, step;   /* its per-family tables: the first, entries each, bytes apart */
	uint32_t names;                 /* its viruses' names, a text archive (a ROM offset) */
	uint32_t ice, ice_count;        /* the sizes of the ice blocks its viruses are frozen in, a byte per AI index (0: none) */
	uint32_t sprite_lists;          /* its sprite lists (a ROM offset, as RomLayout.sprite_lists) */
	uint32_t effects, neffects;     /* its effects, 4 bytes each (as BN6_EFFECTS) */
	uint32_t types[XV_DISPATCH_KINDS], ntypes[XV_DISPATCH_KINDS];   /* the first, third and fourth kinds' routines by type (as BN6_T1_TYPES,
	                                                                   BN6_T3_TYPES, BN6_T4_TYPES), how many */
	uint32_t drops;                 /* its enemies' reward rows (as BN6_DROP_ROWS) */
	bool bn5;                       /* its reward rows' chips BN5's, given as BN6's (xchips.h) */
	bool bn4;                       /* BN4's: no per-family tables (a family its own object type), its records its enemy records,
	                                   its reward rows' chips BN4's (xchips.h) */
	uint8_t text_end;               /* its text's end code where not BN6's TEXT_END (BN4's 0xE5) */
} Game;

static bool game_of(int game, Game *g) {
	memset(g, 0, sizeof *g);
	if (!R.data || !R.layout) return false;
	if (game == XV_GAME_BN6) {
		/* (the sizes as BN6 reads them for its 32 AI indexes: AI 30's and 31's past its viruses' 30, in the Navis') */
		*g = (Game){ R.data, BN6_FAMILY_TABLES, BN6_FAMILIES, BN6_FAMILY_STEP, R.layout->enemy_names[0], BN6_ICE_SIZES, BN6_FAMILIES,
			R.layout->sprite_lists, BN6_EFFECTS, BN6_EFFECT_COUNT, { BN6_T1_TYPES, BN6_T3_TYPES, BN6_T4_TYPES },
			{ BN6_TYPES_T1_COUNT, BN6_TYPES_T3_COUNT, BN6_TYPES_T4_COUNT }, BN6_DROP_ROWS, false, false };
		return true;
	}
	if (game == XROM_BN5_COLONEL_US && XR[game].data && XR[game].layout) {
		/* (BN5 freezes no virus: no ice blocks' sizes) */
		*g = (Game){ XR[game].data, BN5_FAMILY_TABLES, BN5_FAMILIES, BN5_FAMILY_STEP, BN5_ENEMY_NAMES - BUS, 0, 0,
			XR[game].layout->sprite_lists, BN5_EFFECTS, BN5_EFFECT_COUNT, { BN5_T1_TYPES, BN5_T3_TYPES, BN5_T4_TYPES },
			{ BN5_TYPES_T1_COUNT, BN5_TYPES_T3_COUNT, BN5_TYPES_T4_COUNT }, BN5_REWARD_ROWS, true, false };
		return true;
	}
	if (game == XROM_BN4_RED_SUN_US && XR[game].data && XR[game].layout) {
		/* (no per-family tables, no ice; its reward rows its own, BN6's format, their chips given as BN6's: xchips_reward_bn4) */
		*g = (Game){ XR[game].data, 0, 0, 0, BN4_ENEMY_NAMES - BUS, 0, 0, XR[game].layout->sprite_lists, BN4_EFFECTS, BN4_EFFECT_COUNT,
			{ BN4_T1_TYPES, BN4_T3_TYPES, BN4_T4_TYPES }, { BN4_TYPES_T1_COUNT, BN4_TYPES_T3_COUNT, BN4_TYPES_T4_COUNT }, BN4_REWARD_ROWS, false,
			true, BN4_TEXT_END };
		return true;
	}
	return false;
}

static bool in_rom(uint32_t a, uint32_t n) { return a >= BUS && a - BUS <= ROM_SIZE && n <= ROM_SIZE - (a - BUS); }
static uint32_t rom32(const uint8_t *rom, uint32_t a) { return get32(rom + (a - BUS)); }

/* ---- a family's code ---- */

typedef struct {
	const XVirusFamily *f;
	const uint8_t *rom;
	ThumbVeneers pool;
	uint8_t veneers[XV_HOME_SIZE - XV_HOME_VENEERS];
	uint32_t missing;   /* a call the recipe does not name */
	const XvClone *clone;   /* the clone being moved: its call aimed otherwise */
	uint32_t shims;     /* the next free byte of its home's shims (XV_HOME_SHIMS on) */
	const XvRegion *region; /* (code alone, xvirus_code_stage) its installer's region: its calls may go to its stubs */
} Ctx;

/* where an address of a copied block is in BN6 (its Thumb bit kept); 0 for none */
static uint32_t moved_to(const XVirusFamily *f, uint32_t a) {
	const XvBlock *b = xvirus_block_of(f, a & ~1u);
	return b ? a - b->start + b->at : 0;
}

/* a pointer into the blocks, moved: a Thumb one names code, any other the start of a range of data or a byte of raw data
 * (a byte table's place, odd as well as even: a raw range may start at an odd address; a table of pointers to byte lists,
 * CrakOut's crack patterns) */
static bool pointer_ok(const XVirusFamily *f, uint32_t v) {
	if ((v & 1) && xvirus_is_code(f, v & ~1u)) return true;
	const XvData *d = xvirus_data_at(f, v);
	if (!d && (v & 1)) d = xvirus_data_at(f, v & ~1u);
	return d && (d->kind == XV_RAW || d->start == v);
}

static uint32_t resolve(uint32_t target, void *user) {
	Ctx *c = user;
	if (c->clone && target == c->clone->call) return in_rom(c->clone->to, 2) ? thumb_veneer_get(&c->pool, c->clone->to) : 0;
	/* (its own code; a routine of its game's whose bytes a block holds as dead data goes to its twin as any other) */
	if (xvirus_is_code(c->f, target)) return moved_to(c->f, target);
	for (int i = 0; i < c->f->ncalls; ++i)
		if (c->f->calls[i].from == target) {
			uint32_t to = c->f->calls[i].to & ~1u;
			/* (one of BN6's routines, one of the second-level dispatch's spawners, one of the Navis' stubs in their code's room,
			 * xnavi_install.c, or the moved objects'; a BN4 Navi's shim in its code's room, its summon's (xnavi_init.c); or a stub or a helper of the installer's region, xvirus_code_stage's: the
			 * chips', xchips_code.h) */
			bool own = c->region && to >= c->region->stubs && to < c->region->stubs_end;
			if (!in_rom(to, 2) && !xvirus_dispatch_is_spawner(to, NULL, NULL) && to != XV_CHIP_WRITER && to != XV_FIGHT_FLAGS &&
				!(to >= FREE_NAVI_CODE && to < FREE_NAVI_ART) && !(to >= FREE_BN4_NAVI_CODE && to < FREE_BN4_NAVI_ART) &&
				!own && !xvirus_bn4_copy_routine(to | 1)) break;
			return thumb_veneer_get(&c->pool, to);
		}
	c->missing = target;
	return 0;
}

/* (the region of the code alone being staged, xvirus_code_stage's: a screen's reads no object's type) */
static const XvRegion *move_ctx_region;

/* One instruction of a block's code checked: a load from its literal pool
 * must find a word of the recipe's pools, a branch stay in the block, and
 * no read of its object's type (ldrb rX,[r5,#1]: a moved object's is the
 * dispatch's, its game's number in the side table, xvirus_dispatch.h). */
static bool insn_ok(const XVirusFamily *f, const XvBlock *b, uint32_t a, uint16_t op) {
	uint32_t pc4 = (a + 4) & ~3u;
	if ((op & 0xFFF8) == 0x7868 && !(move_ctx_region && move_ctx_region->screen))
		return xv_fail("its object's type read at %08x (the dispatch's, its number in the side table)", (unsigned)a);
	if ((op & 0xF800) == 0x4800) {   /* ldr rd,[pc,#imm]: a pool of its own block (a clone has none) */
		uint32_t t = pc4 + (op & 0xFFu) * 4;
		const XvData *d = xvirus_data_at(f, t);
		return (d && d->kind == XV_POOL && t + 4 <= d->end && t >= b->start && t < b->end) ||
			xv_fail("the load at %08x reads %08x, in no pool of its block", (unsigned)a, (unsigned)t);
	}
	if ((op & 0xF800) == 0xA000) {   /* add rd,pc,#imm */
		uint32_t t = pc4 + (op & 0xFFu) * 4;
		return (t >= b->start && t < b->end) || xv_fail("the address at %08x is out of its block", (unsigned)a);
	}
	int32_t off = 0;
	if ((op & 0xF800) == 0xE000) off = (int32_t)((uint32_t)(op & 0x7FF) << 21) >> 20;           /* b */
	else if ((op & 0xF000) == 0xD000 && (op & 0x0F00) < 0x0E00) off = (int8_t)(op & 0xFF) * 2;  /* b<cond> */
	else if (op == 0x4778) return xv_fail("ARM code at %08x", (unsigned)a);                       /* bx pc */
	else return true;
	uint32_t t = a + 4 + (uint32_t)off;
	return (t >= b->start && t < b->end && xvirus_is_code(f, t)) || xv_fail("the branch at %08x leaves its code", (unsigned)a);
}

/* The block's code checked, run by run, and its BLs found (offsets in the block) */
static int code_walk(const XVirusFamily *f, const XvBlock *b, const uint8_t *code, uint32_t *offs, int max) {
	int n = 0;
	for (uint32_t a = b->start; a < b->end && !xv_failed();) {
		if (!xvirus_is_code(f, a)) { a += 2; continue; }
		uint32_t run = a;
		while (a < b->end && xvirus_is_code(f, a)) a += 2;
		int got = thumb_bl_scan(code + (run - b->start), a - run, offs + n, max - n);
		if (got < 0 || n + got > max) {
			xv_fail("half a BL, or too many, in %08x-%08x", (unsigned)run, (unsigned)a);
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
	return xv_failed() ? -1 : n;
}

/* One word of a block's data: moved where it points into the blocks,
 * named anew where it is one of the recipe's words, kept where it is a
 * number; false (the family failed) where none of these may be */
static bool word_fix(const XVirusFamily *f, int kind, uint32_t a, uint8_t *w) {
	uint32_t v = get32(w);
	if (xvirus_block_of(f, v & ~1u)) {
		if (kind == XV_RAW) return xv_fail("raw data at %08x points into the code (%08x)", (unsigned)a, (unsigned)v);
		if (!pointer_ok(f, v) || (kind == XV_TABLE && !(v & 1))) return xv_fail("the word at %08x points at %08x, neither code nor data's start", (unsigned)a, (unsigned)v);
		put32(w, moved_to(f, v));
		return true;
	}
	/* (a jump table's entry out of the blocks: a routine of its game's whose twin the recipe names, a Thumb pointer) */
	if (kind == XV_TABLE && !(v & 1)) return xv_fail("the jump table's word at %08x points out (%08x)", (unsigned)a, (unsigned)v);
	/* (RAM, I/O and ROM: what the code names there is its game's) */
	if (kind == XV_RAW || (kind == XV_POOL && (v < 0x02000000u || v >= 0x0E000000u))) return true;
	for (int k = 0; k < f->nwords; ++k)
		if (f->words[k].from == v) {
			put32(w, f->words[k].to);
			return true;
		}
	if (kind == XV_TABLE) return xv_fail("the jump table's word at %08x points out (%08x), at no routine the recipe names", (unsigned)a, (unsigned)v);
	return xv_fail("the literal at %08x names %08x, which the recipe does not", (unsigned)a, (unsigned)v);
}

/* a word of its pools the recipe names a number (XvNumber): BN6's in its place, its game's checked first; false where
 * the recipe names none there */
static bool number_fix(const XVirusFamily *f, uint32_t a, uint8_t *w) {
	for (int k = 0; k < f->nnumbers; ++k)
		if (f->numbers[k].at == a) {
			if (get32(w) != f->numbers[k].was) xv_fail("the number at %08x is %08x, not %08x", (unsigned)a, (unsigned)get32(w), (unsigned)f->numbers[k].was);
			else put32(w, f->numbers[k].now);
			return true;
		}
	return false;
}

/* A block's data: pools' words and tables moved (or the numbers the recipe names), raw bytes checked and the ids the
 * recipe names given BN6's, dead ones zeroed */
static void data_fix(const XVirusFamily *f, const XvBlock *b, uint8_t *code) {
	for (int i = 0; i < f->ndata && !xv_failed(); ++i) {
		const XvData *d = &f->data[i];
		if (d->start < b->start || d->start >= b->end) continue;
		if (d->kind == XV_ZERO) {
			memset(code + (d->start - b->start), 0, d->end - d->start);
			continue;
		}
		for (uint32_t a = (d->start + 3) & ~3u; a + 4 <= d->end && !xv_failed(); a += 4)
			if (d->kind != XV_POOL || !number_fix(f, a, code + (a - b->start))) word_fix(f, d->kind, a, code + (a - b->start));
	}
	for (int i = 0; i < f->nbytes && !xv_failed(); ++i) {
		const XvByte *y = &f->bytes[i];
		if (y->at < b->start || y->at >= b->end) continue;
		uint8_t *p = code + (y->at - b->start);
		if (*p != y->was) xv_fail("the byte at %08x is %02x, not %02x", (unsigned)y->at, *p, y->was);
		else *p = y->now;
	}
}

static void imms_patch(const XVirusFamily *f, const XvBlock *b, uint8_t *code) {
	for (int i = 0; i < f->nimms && !xv_failed(); ++i) {
		const XvImm *m = &f->imms[i];
		if (m->at < b->start || m->at >= b->end) continue;
		uint8_t *p = code + (m->at - b->start);
		uint16_t insn = get16(p);
		ThumbImm was = { m->form, m->rd, m->rn, m->was };
		if (!xvirus_is_code(f, m->at) || (insn & 0xF000) == 0xF000 || !thumb_imm_patch(&insn, &was, m->now)) {
			xv_fail("the immediate at %08x is not the one expected", (unsigned)m->at);
			return;
		}
		put16(p, insn);
	}
}

#define BLS_MAX 256

/* ---- its shims (xvirus_shim.h), in its home after its held chips ---- */

/* room for n bytes of shim in its home, word-aligned: the address, 0 where its shims are full */
static uint32_t shim_room(Ctx *c, uint32_t n) {
	uint32_t at = c->f->home + XV_HOME_SHIMS + c->shims;
	if (c->shims + n > XV_HOME_VENEERS - XV_HOME_SHIMS) { xv_fail("no room in its home for its shims"); return 0; }
	c->shims += (n + 3) & ~3u;
	return at;
}

/* A call to look enemies up by a NameID range (XvRange): its game's to object_getEnemyByNameRange or to the shared virus
 * code's count by one, aimed at a shim that loads BN6's bounds and goes on to BN6's twin through the family's veneer for it,
 * the call's own target after the rebase */
static void range_set(Ctx *c, const XvBlock *b, uint8_t *code, const XvRange *r) {
	uint32_t t = 0, was = 0, at = 0;
	const uint8_t *orig = c->rom + (r->at - BUS);
	bool ok = thumb_bl_decode(r->at, get16(orig), get16(orig + 2), &was);
	bool bn4 = ok && c->f->game == XROM_BN4_RED_SUN_US && was == BN4_BY_NAME_RANGE;
	if (!ok || (was != BN5_BY_NAME_RANGE && was != BN5_COUNT_BY_NAME && !bn4) ||
		!thumb_bl_read(code, r->at - b->start, b->at, &t) || t != thumb_veneer_get(&c->pool, was == BN5_COUNT_BY_NAME ? BN6_COUNT_BY_NAME : BN6_BY_NAME_RANGE)) {
		xv_fail("its NameID range's call at %08x", (unsigned)r->at);
		return;
	}
	uint8_t shim[XV_SHIM_RANGE_SIZE];
	if (!(at = shim_room(c, sizeof shim)) || !xvirus_shim_range(shim, at, r->lo, r->hi, t, bn4 ? 1 : -1)) { xv_fail("its NameID range's shim for %08x", (unsigned)r->at); return; }
	uint8_t *p = xv_put(at, sizeof shim, true);
	if (p) memcpy(p, shim, sizeof shim);
	if (!thumb_bl_write(code, r->at - b->start, b->at, at)) xv_fail("its call at %08x aimed at its shim", (unsigned)r->at);
}

/* A held chip's compare past an immediate (XvCmp): `cmp rN,#was; beq/bne L` made a call to a shim that compares with BN6's
 * id and goes on to L's copy where the branch would */
static void cmp_set(Ctx *c, const XvBlock *b, uint8_t *code, const XvCmp *k) {
	uint32_t off = k->at - b->start;
	uint16_t op = get16(code + off), br = get16(code + off + 2);
	uint32_t to = k->at + 2 + 4 + (uint32_t)((int32_t)(int8_t)(br & 0xFF) * 2), at = 0;
	if (op != (0x2800 | k->reg << 8 | k->was) || (br & 0xFE00) != 0xD000 || !xvirus_is_code(c->f, to) || xvirus_block_of(c->f, to) != b) {
		xv_fail("its chip compare at %08x is not the one expected", (unsigned)k->at);
		return;
	}
	uint8_t shim[XV_SHIM_CMP_SIZE];
	if (!(at = shim_room(c, sizeof shim)) || !xvirus_shim_cmp(shim, at, k->reg, k->now, (br & 0x0100) == 0, moved_to(c->f, to))) {
		xv_fail("its chip compare's shim for %08x", (unsigned)k->at);
		return;
	}
	uint8_t *p = xv_put(at, sizeof shim, true);
	if (p) memcpy(p, shim, sizeof shim);
	if (!thumb_bl_write(code, off, b->at, at)) xv_fail("its compare at %08x made a call to its shim", (unsigned)k->at);
}

/* A call to its game's direction rule (XVirusFamily.sides), the rule's copy its target: aimed at a shim that calls it and
 * numbers left and right by the object's side, BN6's way */
static void side_set(Ctx *c, const XvBlock *b, uint8_t *code, uint32_t site) {
	uint32_t was = 0, t = 0, at = 0;
	const uint8_t *orig = c->rom + (site - BUS);
	uint8_t shim[XV_SHIM_SIDE_SIZE];
	if (!thumb_bl_decode(site, get16(orig), get16(orig + 2), &was) || was != BN4_DIRECTION || !thumb_bl_read(code, site - b->start, b->at, &t) ||
		t != moved_to(c->f, BN4_DIRECTION) || !(at = shim_room(c, sizeof shim)) || !xvirus_shim_side(shim, at, t | 1)) {
		xv_fail("its direction's call at %08x", (unsigned)site);
		return;
	}
	uint8_t *p = xv_put(at, sizeof shim, true);
	if (p) memcpy(p, shim, sizeof shim);
	if (!thumb_bl_write(code, site - b->start, b->at, at)) xv_fail("its direction's call at %08x aimed at its shim", (unsigned)site);
}

/* A call that takes r0 BN6's way (XvSoundCall: a sound past a byte, the owner's call of 5 November 2026, docs/multirom/bn4.md
 * call 11; the appear queue's number): its game's call aimed at a shim that makes r0 the slot and goes on to BN6's twin
 * (xvirus_shim_r0_to) through the family's veneer for it, the call's own target after the rebase */
static void sound_set(Ctx *c, const XvBlock *b, uint8_t *code, const XvSoundCall *k) {
	uint32_t was = 0, t = 0, at = 0, to = 0;
	const uint8_t *orig = c->rom + (k->at - BUS);
	uint8_t shim[XV_SHIM_SOUND_SIZE];
	if (!thumb_bl_decode(k->at, get16(orig), get16(orig + 2), &was) || !(to = xvirus_shim_r0_to(was)) || !thumb_bl_read(code, k->at - b->start, b->at, &t) ||
		t != thumb_veneer_get(&c->pool, to) || !(at = shim_room(c, sizeof shim)) || !xvirus_shim_sound(shim, at, k->slot, t)) {
		xv_fail("its sound's call at %08x", (unsigned)k->at);
		return;
	}
	uint8_t *p = xv_put(at, sizeof shim, true);
	if (p) memcpy(p, shim, sizeof shim);
	if (!thumb_bl_write(code, k->at - b->start, b->at, at)) xv_fail("its sound's call at %08x aimed at its shim", (unsigned)k->at);
}

static void shims_set(Ctx *c, const XvBlock *b, uint8_t *code) {
	for (int i = 0; i < c->f->nsides && !xv_failed(); ++i)
		if (xvirus_block_of(c->f, c->f->sides[i]) == b) side_set(c, b, code, c->f->sides[i]);
	for (int i = 0; i < c->f->nranges && !xv_failed(); ++i)
		if (xvirus_block_of(c->f, c->f->ranges[i].at) == b) range_set(c, b, code, &c->f->ranges[i]);
	for (int i = 0; i < c->f->ncmps && !xv_failed(); ++i)
		if (xvirus_block_of(c->f, c->f->cmps[i].at) == b) cmp_set(c, b, code, &c->f->cmps[i]);
	for (int i = 0; i < c->f->nsound_calls && !xv_failed(); ++i)
		if (xvirus_block_of(c->f, c->f->sound_calls[i].at) == b) sound_set(c, b, code, &c->f->sound_calls[i]);
}

/* Its pre-step (xvirus_shim.h): the port of its game's dispatcher's case, whose source (BN5's dispatcher and the routines
 * it calls) is checked by its CRC32s first, in its home; its entry in the pre-steps by AI index */
static void prestep_set(Ctx *c) {
	const XVirusFamily *f = c->f;
	if (!f->prestep) return;
	if (f->game == XROM_BN4_RED_SUN_US) {
		/* (BN4's: its update's own work, its copy, which goes on to BN6's state step itself) */
		uint32_t at = moved_to(f, f->prestep);
		if (!at) xv_fail("its pre-step %08x is none of its code", (unsigned)f->prestep);
		else xv_word(XV_PRESTEP + 4u * (uint32_t)f->ai, 0, at | 1);
		return;
	}
	if (f->game != XROM_BN5_COLONEL_US || xvirus_crc32(c->rom + (BN5_VIRUS_DISPATCH - BUS), BN5_VIRUS_DISPATCH_END - BN5_VIRUS_DISPATCH) != BN5_VIRUS_DISPATCH_CRC ||
		xvirus_crc32(c->rom + (BN5_PRESTEP_TAKE - BUS), BN5_PRESTEP_TAKE_END - BN5_PRESTEP_TAKE) != BN5_PRESTEP_TAKE_CRC) {
		xv_fail("its pre-step's source in its game is not the one ported");
		return;
	}
	uint32_t clear = thumb_veneer_get(&c->pool, BN6_CLEAR_FLAG2), sound = thumb_veneer_get(&c->pool, BN6_PLAY_SOUND);
	uint8_t code[XV_SHIM_PRESTEP_MAX];
	uint32_t at = c->f->home + XV_HOME_SHIMS + c->shims;
	int n = clear && sound ? xvirus_shim_prestep(code, sizeof code, at, f->prestep, clear, sound) : 0;
	if (!n || !shim_room(c, (uint32_t)n)) { xv_fail("its pre-step %08x", (unsigned)f->prestep); return; }
	uint8_t *p = xv_put(at, (uint32_t)n, true);
	if (p) memcpy(p, code, (size_t)n);
	xv_word(XV_PRESTEP + 4u * (uint32_t)f->ai, 0, at | 1);
}

/* Its hooks entered past their routine's push (XvRef.entered): each a shim of its home that pushes lr and calls the
 * tail, in the hook's entry of its tables in place of the tail itself */
static void entries_set(Ctx *c, uint32_t entry[XV_TABLES]) {
	for (int k = 0; k < XV_HOOKS && !xv_failed(); ++k) {
		if (!c->f->hooks[k].entered) continue;
		uint8_t shim[XV_SHIM_ENTRY_SIZE];
		uint32_t at = shim_room(c, sizeof shim);
		if (!at || !xvirus_shim_entry(shim, at, entry[3 + k])) { xv_fail("its hook %d's entry (%08x)", k, (unsigned)c->f->hooks[k].addr); return; }
		uint8_t *p = xv_put(at, sizeof shim, true);
		if (p) memcpy(p, shim, sizeof shim);
		entry[3 + k] = at | 1;
	}
}

/* Its game's after-state while BN6's battle is paused (XVirusFamily.present): a paused present (xvirus_shim.h) of its home
 * that calls `own` first (Thumb; 0 none); its address (Thumb), 0 where it failed */
static uint32_t paused_put(Ctx *c, uint32_t own) {
	uint8_t shim[XV_SHIM_PAUSED_SIZE];
	uint32_t at = shim_room(c, sizeof shim), present = moved_to(c->f, c->f->present | 1);
	uint32_t mode = thumb_veneer_get(&c->pool, BN6_SETTINGS_MODE_READ), paused = thumb_veneer_get(&c->pool, BN6_PAUSE_TEST);
	uint8_t *p = at && present && mode && paused && xvirus_shim_paused(shim, at, own, mode, paused, present) ? xv_put(at, sizeof shim, true) : NULL;
	if (!p) { xv_fail("its after-state %08x while paused", (unsigned)c->f->present); return 0; }
	memcpy(p, shim, sizeof shim);
	return at | 1;
}

/* A call within the block to bytes it holds as dead (a routine of its game's
 * whose twin BN6 has, zeroed in the copy between code that shares a pool):
 * aimed at the twin here, as thumb_block_rebase leaves a call within alone.
 * The block's other calls are left in offs for it; their count, -1 where one
 * cannot be made. */
static int dead_calls(Ctx *c, const XvBlock *b, uint8_t *code, uint32_t *offs, int n) {
	int kept = 0;
	for (int i = 0; i < n; ++i) {
		uint32_t t = 0;
		if (!thumb_bl_read(code, offs[i], b->start, &t) || t < b->start || t >= b->end || xvirus_is_code(c->f, t)) {
			offs[kept++] = offs[i];
			continue;
		}
		c->missing = 0;
		uint32_t to = resolve(t, c);
		if (!to || !thumb_bl_write(code, offs[i], b->at, to)) {
			xv_fail("the call at %08x into its dead bytes (%08x) has no twin to go to", (unsigned)(b->start + offs[i]), (unsigned)t);
			return -1;
		}
	}
	return kept;
}

static void block_move(Ctx *c, const XvBlock *b) {
	const XVirusFamily *f = c->f;
	uint32_t len = b->end - b->start;
	if (!len || !in_rom(b->start, len)) { xv_fail("block %08x-%08x outside its ROM", (unsigned)b->start, (unsigned)b->end); return; }
	if (xvirus_crc32(c->rom + (b->start - BUS), len) != b->crc) { xv_fail("block %08x-%08x is not the one expected (its CRC32)", (unsigned)b->start, (unsigned)b->end); return; }
	uint8_t *code = xv_put(b->at, len, true);
	if (!code) return;
	memcpy(code, c->rom + (b->start - BUS), len);
	static uint32_t offs[BLS_MAX];
	int n = code_walk(f, b, code, offs, BLS_MAX);
	if (n < 0) return;
	data_fix(f, b, code);
	imms_patch(f, b, code);
	if (xv_failed()) return;
	n = dead_calls(c, b, code, offs, n);
	if (n < 0) return;
	ThumbBlock tb = { code, len, b->start, b->at };
	c->missing = 0;
	if (!thumb_block_rebase(&tb, offs, n, resolve, c)) {
		if (c->missing) xv_fail("a call to %08x the recipe does not name", (unsigned)c->missing);
		else xv_fail("a call out of %08x-%08x cannot be made (veneers full, or out of reach)", (unsigned)b->start, (unsigned)b->end);
	}
	if (!xv_failed()) shims_set(c, b, code);
}

/* A clone of a routine of its code (XvClone): copied once more, its call to `call` aimed at `to` and its immediate
 * changed; the bytes are its block's, whose CRC32 was checked as it moved */
static void clone_move(Ctx *c, const XvClone *k) {
	const XVirusFamily *f = c->f;
	uint32_t len = k->end - k->start;
	const XvBlock b = { k->start, k->end, k->at, 0 };
	uint8_t *code = in_rom(k->start, len) ? xv_put(k->at, len, true) : NULL;
	if (!code) { xv_fail("its clone of %08x-%08x", (unsigned)k->start, (unsigned)k->end); return; }
	memcpy(code, c->rom + (k->start - BUS), len);
	static uint32_t offs[BLS_MAX];
	int n = code_walk(f, &b, code, offs, BLS_MAX);
	if (n < 0) return;
	if (k->imm.at) {
		uint8_t *p = code + (k->imm.at - k->start);
		uint16_t insn = get16(p);
		ThumbImm was = { k->imm.form, k->imm.rd, k->imm.rn, k->imm.was };
		if (!thumb_imm_patch(&insn, &was, k->imm.now)) { xv_fail("the clone's immediate at %08x is not the one expected", (unsigned)k->imm.at); return; }
		put16(p, insn);
	}
	ThumbBlock tb = { code, len, k->start, k->at };
	bool called = false;
	for (int i = 0; i < n && !called; ++i) {
		uint32_t t;
		called = thumb_bl_read(code, offs[i], k->start, &t) && t == k->call;
	}
	c->missing = 0;
	c->clone = k;
	if (!called) xv_fail("its clone of %08x-%08x calls no %08x", (unsigned)k->start, (unsigned)k->end, (unsigned)k->call);
	else if (!thumb_block_rebase(&tb, offs, n, resolve, c))
		xv_fail("a call out of its clone of %08x-%08x cannot be made (%08x)", (unsigned)k->start, (unsigned)k->end, (unsigned)c->missing);
	c->clone = NULL;
}

/* ---- its records and its entries in BN6's grown tables ---- */

static uint32_t ref_to(const XVirusFamily *f, XvRef r) {
	if (r.clone) {
		const XvClone *k = r.clone <= f->nclones ? &f->clones[r.clone - 1] : NULL;
		if (!k || (r.addr & ~1u) < k->start || (r.addr & ~1u) >= k->end) { xv_fail("its clone's %08x", (unsigned)r.addr); return 0; }
		return (k->at + ((r.addr & ~1u) - k->start)) | 1;
	}
	if (r.own) {
		uint32_t to = (r.addr & 1) && xvirus_is_code(f, r.addr & ~1u) ? moved_to(f, r.addr) : 0;
		if (!to) xv_fail("its own %08x is no code of its blocks", (unsigned)r.addr);
		return to;
	}
	if (!(r.addr & 1) || !in_rom(r.addr & ~1u, 2)) xv_fail("%08x is no routine of BN6's", (unsigned)r.addr);
	return r.addr;
}

/* its versions' enemy ids: (version, a virus, its AI index), each where
 * the grown ids give BN6's filler */
static void ids_set(const XVirusFamily *f) {
	for (int v = 0; v < XV_VERSIONS && !xv_failed(); ++v) {
		const uint8_t t[3] = { (uint8_t)v, 0, (uint8_t)f->ai };
		xg_set(XG_IDS, (uint32_t)(f->first_id + v), t);
	}
}

/* the size of the ice block it is frozen in: the recipe's, or its game's
 * for its AI index there */
static void ice_set(const XVirusFamily *f, const Game *g) {
	int size = f->ice;
	if (f->ice == XV_ICE_OWN) {
		if (!g->ice || f->src_ai < 0 || (uint32_t)f->src_ai >= g->ice_count) { xv_fail("no ice block size in its game for AI %d", f->src_ai); return; }
		size = g->rom[g->ice - BUS + (uint32_t)f->src_ai];
	}
	const uint8_t b = (uint8_t)size;
	xg_set(XG_ICE, (uint32_t)f->ai, &b);
}

/* ---- the names of its ids, after those already in the families' archive ---- */

static struct {
	uint8_t text[XV_VERSIONS][NAME_BYTES];   /* (BN6's codes, each ended) */
	uint32_t len[XV_VERSIONS];
	XvMoved moved[XV_VERSIONS];   /* each from its game's archive to the families' */
} names;

/* an archive's entry `i` in `rom` (ending with `end`): its bytes and length, 0 for none */
static uint32_t entry_of(const uint8_t *rom, uint32_t archive, uint32_t i, uint8_t end, const uint8_t **out) {
	if (archive + 2 * i + 2 > ROM_SIZE) return 0;
	uint32_t p = archive + get16(rom + archive + 2 * i), n = 0;
	while (p + n < ROM_SIZE && n < NAME_BYTES && rom[p + n] != end) ++n;
	*out = rom + p;
	return p + n < ROM_SIZE && n < NAME_BYTES ? n + 1 : 0;
}

static void names_set(const XVirusFamily *f, const Game *g) {
	uint32_t end = xg_names_end();
	for (int v = 0; v < XV_VERSIONS && !xv_failed(); ++v) {
		const uint8_t *s;
		uint32_t n = entry_of(g->rom, g->names, f->names[v], g->text_end ? g->text_end : TEXT_END, &s), id = (uint32_t)(f->first_id + v);
		if (!n) { xv_fail("its name %d is no entry of its game's archive", v); return; }
		/* (another game's in BN6's text codes: rom.h, xrom_text_bn6) */
		uint32_t to = n - 1;
		if (f->game == XV_GAME_BN6) memcpy(names.text[v], s, to);
		else to = (uint32_t)xrom_text_bn6(f->game, s, to, names.text[v], NAME_BYTES - 1);
		names.text[v][to] = TEXT_END;
		names.len[v] = to + 1;
		if (end + names.len[v] > XV_NAMES_ROOM) { xv_fail("no room for its name %d", v); return; }
		uint8_t *p = xv_put(XV_NAMES + end, names.len[v], true);
		if (p) memcpy(p, names.text[v], names.len[v]);
		xv_half(XV_NAMES + 2 * (id - XV_FIRST_ID), (uint16_t)XG_NAME_NONE, (uint16_t)end);
		names.moved[v] = (XvMoved){ BUS + (uint32_t)(s - g->rom), XV_NAMES + end, names.len[v] };
		end += names.len[v];
	}
}

/* ---- what it draws: its sprites and effects, and its attacks' types ---- */

/* Its sprites copied (a compressed one decompressed: BN6 draws a battle
 * sprite whose pointer has bit 31 only once its battle's start put it in
 * a cache) and listed in their grown lists, each number one that BN6's
 * placeholder holds there; a sprite an install before placed at its place
 * and number (the Navis' shared art, xnavi_recipe.h) kept, its bytes
 * checked */
static void sprites_set(const XVirusFamily *f, const Game *g) {
	for (int i = 0; i < f->nsprites && !xv_failed(); ++i) {
		const XvSprite *s = &f->sprites[i];
		/* (a BN4 Navi's battle sprite or attacks' art: the rooms the battles share, filled for his battle, xnavi_bn4_room.h) */
		if (g->bn4 && xnavi_bn4_room_row(s, g->rom, g->sprite_lists)) continue;
		uint32_t len;
		uint8_t *d = xnavi_sprite_bytes(g->rom, g->sprite_lists, s->list, s->index, &len);
		if (!d || len > s->room) {
			xv_fail("its sprite %d:%#x (%u bytes, %u of room)", s->list, s->index, (unsigned)len, (unsigned)s->room);
			free(d);
			return;
		}
		if (xvirus_list_entry(s->to_list, s->to_index) && emu_read32(xvirus_list_entry(s->to_list, s->to_index)) == s->at) {
			for (uint32_t b = 0; b < len && !xv_failed(); ++b)
				if (emu_read8(s->at + b) != d[b]) xv_fail("its sprite %d:%#x at %08x is another's", s->list, s->index, (unsigned)s->at);
			free(d);
			continue;
		}
		uint8_t *p = xv_put(s->at, len, true);
		if (p) memcpy(p, d, len);
		free(d);
		xg_set_word(XG_LIST + xvirus_list_slot(s->to_list), s->to_index, s->at);
	}
}

/* Its arm points' record (BN6 finds an object's by its struct1's list and number, list 1's grown: XG_ARMS) at its battle
 * sprite's number: its game's record's 30 points, then BN6's four more as its viruses' first record has them (BN5 has
 * none of 30-33, which the fourth kind's 0x3C, the third's 0x95, 0x08010820 and a freeze's ice block read). A status's
 * mark and a deletion's flash and explosions stand where its game's would. */
static void arms_set(const XVirusFamily *f, const Game *g) {
	if (!g->bn5 || !f->traits.laid || !f->nsprites) return;
	const XvSprite *s = &f->sprites[0];
	uint32_t five = rom32(g->rom, BN5_ARM_VIRUSES_REF) + (uint32_t)s->index * BN5_ARM_VIRUS_SIZE;
	uint32_t six = get32(R.data + (BN6_ARM_VIRUSES_REF - BUS));
	if (s->list != 1 || s->to_list != 1 || s->index >= BN5_ARM_VIRUSES_COUNT || !in_rom(five, BN5_ARM_VIRUS_SIZE) ||
		!in_rom(six, BN6_ARM_VIRUS_SIZE)) {
		xv_fail("its arm points (its battle sprite %d:%#x)", s->list, s->index);
		return;
	}
	uint8_t rec[BN6_ARM_VIRUS_SIZE];
	memcpy(rec, g->rom + (five - BUS), BN5_ARM_VIRUS_SIZE);
	memcpy(rec + BN5_ARM_VIRUS_SIZE, R.data + (six - BUS) + BN5_ARM_VIRUS_SIZE, BN6_ARM_VIRUS_SIZE - BN5_ARM_VIRUS_SIZE);
	xg_set(XG_ARMS, s->to_index, rec);
}

static const XVirusFamily *installed[XV_FAMILIES];
static int ninstalled;

/* the sprite of its game's (list, number) as one of the recipe's, or a family's installed before it of the same game's,
 * lists it */
static const XvSprite *sprite_of(const XVirusFamily *f, uint8_t list, uint8_t index) {
	for (int k = 0; k < f->nsprites; ++k)
		if (f->sprites[k].list == list && f->sprites[k].index == index) return &f->sprites[k];
	for (int i = 0; i < ninstalled; ++i)
		for (int k = 0; installed[i]->game == f->game && k < installed[i]->nsprites; ++k)
			if (installed[i]->sprites[k].list == list && installed[i]->sprites[k].index == index) return &installed[i]->sprites[k];
	return NULL;
}

/* Its effects in the grown effects' table: its game's entry with its
 * sprite as BN6 lists it (one of the recipe's, or of a family's before it);
 * an entry an install before wrote the same kept (the Navis share their
 * effects' numbers) */
static void effects_set(const XVirusFamily *f, const Game *g) {
	for (int i = 0; i < f->neffects && !xv_failed(); ++i) {
		const XvId *e = &f->effects[i];
		const uint8_t *src = e->from < g->neffects ? g->rom + (g->effects - BUS) + 4u * e->from : NULL;
		const XvSprite *s = src && !(src[0] & 3) ? sprite_of(f, (uint8_t)(src[0] / 4), src[1]) : NULL;
		if (!s) { xv_fail("its effect %#x draws none of its sprites", e->from); return; }
		uint32_t word = (uint32_t)(4u * s->to_list) | (uint32_t)s->to_index << 8 | (uint32_t)src[2] << 16 | (uint32_t)src[3] << 24;
		/* (one an install before wrote, the same: the Navis' effects one number whichever of them spawns it) */
		if (emu_read32(XV_EFFECTS + 4u * e->to) == word) continue;
		xg_set_word(XG_EFFECTS, e->to, word);
	}
}

/* The object types its code spawns, for the second-level dispatch
 * (xvirus_dispatch.h): BN6's twin, or moved, its game's routine for the
 * type its own code */
static void types_set(const XVirusFamily *f, const Game *g) {
	for (int i = 0; i < f->ntypes && !xv_failed(); ++i) {
		const XvType *t = &f->types[i];
		int k = xvirus_dispatch_slot(t->kind);
		uint32_t routine = 0;
		if (t->to == XV_TYPE_SHARED) {
			/* (moved by a family installed before it, or a unit before it in this step, xv_read32: its MOVED entry must name
			 * that one's copy, its TWIN entry moved) */
			uint32_t moved = xvirus_dispatch_moved_at(t->kind, t->from), twin = xvirus_dispatch_twin_at(t->kind, t->from);
			if (!moved || xv_read32(moved) == (XV_STUB_NONE | 1) || xv_read8(twin) != XV_TYPE_MOVED)
				xv_fail("its object %d:%#x is moved by no family installed before it", t->kind, t->from);
			continue;
		}
		if (k >= 0 && t->to == XV_TYPE_MOVED) {
			uint32_t of = t->of ? t->of : t->from;   /* (its number in its game, where its code spawns it by another) */
			uint32_t own = of < g->ntypes[k] ? rom32(g->rom, g->types[k] + 4u * of) : 0;
			routine = (own & 1) && xvirus_is_code(f, own & ~1u) ? moved_to(f, own) : 0;
			if (!routine) { xv_fail("its object %d:%#x (%08x) is none of its code", t->kind, t->from, (unsigned)own); return; }
		}
		xg_type(t->kind, t->from, t->to, routine);
	}
}

/* ---- a family ---- */

/* the recipe whole, and its records and its state table where its game's
 * tables have them (the state table the range of its block it zeroes) */
static bool sane(const XVirusFamily *f, const Game *g) {
	char why[200];
	if (!xvirus_recipe_ok(f, why, sizeof why)) return xv_fail("%s", why);
	if (g->bn4) {
		/* (its records its first version's enemy record, which names its type; no state table of its game's to zero) */
		uint32_t rec = BN4_ENEMY_STATS + BN4_ENEMY_STAT_SIZE * f->names[0];
		if (f->struct1 != rec || f->struct2 != rec || g->rom[rec - BUS + BN4_ENEMY_TYPE] != f->src_ai)
			return xv_fail("its records are not its enemy records (%08x)", (unsigned)rec);
		return true;
	}
	if (f->src_ai < 0 || (uint32_t)f->src_ai >= g->count) return xv_fail("its AI index in its game, %d", f->src_ai);
	uint32_t at = g->tables + 4u * (uint32_t)f->src_ai, st = rom32(g->rom, at);
	uint32_t s1 = rom32(g->rom, at + g->step), s2 = rom32(g->rom, at + 2 * g->step);
	if (s1 != f->struct1 || s2 != f->struct2) return xv_fail("its records are at %08x and %08x in its game", (unsigned)s1, (unsigned)s2);
	const XvData *d = xvirus_data_at(f, st);
	if (!d || d->kind != XV_ZERO || d->start != st) return xv_fail("its state table is at %08x in its game", (unsigned)st);
	return true;
}

/* Its struct1 as BN6 reads it (BN6_TRAITS_*): its game's as it is, or
 * laid out BN6's way where the recipe's traits are laid, its game's
 * record (BN5's layout) naming the recipe's battle sprite, mode and
 * shadow first; its mode BN6's where BN6's table has it, else XV_MODE_PAST */
static void traits_of(const XVirusFamily *f, const uint8_t *src, uint8_t out[BN6_TRAITS_SIZE]) {
	memcpy(out, src, BN6_TRAITS_SIZE);
	if (!f->traits.laid) return;
	const XvSprite *s = &f->sprites[0];
	if (f->game == XROM_BN4_RED_SUN_US) {
		/* (BN4's enemy record names its sprite: its list's offset and number; the rest the recipe's, from its init) */
		if (src[BN4_ENEMY_KIND] != 4u * s->list || src[BN4_ENEMY_FAMILY] != s->index) { xv_fail("its enemy record names another sprite"); return; }
		memset(out, 0, BN6_TRAITS_SIZE);
		out[BN6_TRAITS_LIST] = (uint8_t)(4u * s->to_list);
		out[BN6_TRAITS_SPRITE] = s->to_index;
		out[BN6_TRAITS_PALETTE] = f->traits.palette;
		out[BN6_TRAITS_MODE] = f->traits.mode < BN6_TRAITS_MODE_COUNT ? f->traits.mode : XV_MODE_PAST;
		out[BN6_TRAITS_SHADOW] = f->traits.shadow;
		return;
	}
	if (src[BN6_TRAITS_LIST] != 4u * s->list || src[BN6_TRAITS_SPRITE] != s->index || src[BN6_TRAITS_MODE] != f->traits.mode ||
		!src[BN5_TRAITS_SHADOW] != !f->traits.shadow) {
		xv_fail("its struct1 is not the one its recipe names");
		return;
	}
	out[BN6_TRAITS_LIST] = (uint8_t)(4u * s->to_list);
	out[BN6_TRAITS_SPRITE] = s->to_index;
	out[BN6_TRAITS_MODE] = f->traits.mode < BN6_TRAITS_MODE_COUNT ? f->traits.mode : XV_MODE_PAST;
	out[BN6_TRAITS_ELEMENT] = f->traits.element;
	out[BN6_TRAITS_WEAKNESS] = f->traits.weakness;
	out[BN6_TRAITS_SHADOW] = f->traits.shadow;
}

/* its records as the engine reads them, kept for data.c's readers
 * (enemies_tell) once it is installed */
static struct { uint8_t traits[BN6_TRAITS_SIZE], stats[XV_VERSIONS * BN6_STATS_SIZE]; } rec;

/* BN4's six enemy records (its versions' ids) as BN6's struct2: element << 12 | HP, the version, the flags its init's
 * collision setup gives (the recipe's), element << 12 | its attack's damage (BN4's +0xA, which carries no element) */
static void stats_bn4(const XVirusFamily *f, const Game *g, uint8_t *out) {
	for (int v = 0; v < XV_VERSIONS; ++v) {
		const uint8_t *r = g->rom + (BN4_ENEMY_STATS - BUS) + BN4_ENEMY_STAT_SIZE * f->names[v];
		uint16_t hp = get16(r + BN4_ENEMY_HP);
		uint8_t *o = out + BN6_STATS_SIZE * v;
		put16(o, hp);
		o[2] = (uint8_t)v;
		o[3] = f->traits.flags;
		/* (its attack's word as BN4's, no element: BN6's collision setup,
		 * 0x08019F44, reads the word's top four bits as the hit's properties,
		 * 15 and 14 as BN4's own does, 0x08012860, 13 and 12 its own) */
		put16(o + 4, get16(r + BN4_ENEMY_ATTACK));
	}
}

/* its state table, struct1, struct2 and held chips in its home or BN6's
 * (their words in the grown table of held chips), and its entries of the
 * eleven per-family tables into `entry` (`state0`, not 0: its state 0) */
static void records_set(const XVirusFamily *f, const Game *g, uint32_t entry[XV_TABLES], uint32_t state0) {
	uint8_t *st = xv_put(f->home + XV_HOME_STATES, 4u * (uint32_t)f->nstates, true);
	for (int i = 0; st && i < f->nstates; ++i) put32(st + 4 * i, !i && state0 ? state0 : ref_to(f, f->states[i]));
	uint8_t *s1 = xv_put(f->home + XV_HOME_STRUCT1, BN6_TRAITS_SIZE, true), *s2 = xv_put(f->home + XV_HOME_STRUCT2, sizeof rec.stats, true);
	if (!s1 || !s2 || !in_rom(f->struct1, BN6_TRAITS_SIZE) || !in_rom(f->struct2, sizeof rec.stats)) { xv_fail("its records"); return; }
	traits_of(f, g->rom + (f->struct1 - BUS), rec.traits);
	if (g->bn4) stats_bn4(f, g, rec.stats);
	else memcpy(rec.stats, g->rom + (f->struct2 - BUS), sizeof rec.stats);
	memcpy(s1, rec.traits, BN6_TRAITS_SIZE);
	memcpy(s2, rec.stats, sizeof rec.stats);
	entry[0] = f->home + XV_HOME_STATES;
	entry[1] = f->home + XV_HOME_STRUCT1;
	entry[2] = f->home + XV_HOME_STRUCT2;
	for (int k = 0; k < XV_HOOKS; ++k) entry[3 + k] = ref_to(f, f->hooks[k]);
	uint32_t held = f->held;
	if (!held) {
		/* (its own lists, BN6's ids, or none) */
		uint8_t *h = xv_put(f->home + XV_HOME_HELD, HELD_LISTS, true);
		if (h) memset(h, 0xFF, HELD_LISTS);
		for (uint32_t k = 0; h && f->held_chips && k < HELD_LISTS / 2; ++k) put16(h + 2 * k, f->held_chips[k]);
		held = f->home + XV_HOME_HELD;
	} else if (!in_rom(held, HELD_LISTS)) xv_fail("its held chips at %08x", (unsigned)held);
	xg_set_word(XG_HELD, (uint32_t)f->ai, held);
}

/* ---- its rewards: its game's reward rows for its versions' ids, in the
 * rows of its ids (another game's entries as BN6's: xchips.h) ---- */

static uint16_t drops[XV_VERSIONS][ENEMY_DROPS];

static void drops_set(const XVirusFamily *f, const Game *g) {
	if (!g->drops) return;
	for (int v = 0; v < XV_VERSIONS && !xv_failed(); ++v) {
		uint32_t from = g->drops - BUS + 2u * ENEMY_DROPS * f->names[v];
		if (from + 2u * ENEMY_DROPS > ROM_SIZE) { xv_fail("its reward row %d", v); return; }
		uint8_t row[2 * ENEMY_DROPS];
		for (uint32_t k = 0; k < ENEMY_DROPS; ++k) {
			uint16_t e = get16(g->rom + from + 2 * k);
			drops[v][k] = g->bn5 ? xchips_reward_bn6(e) : g->bn4 ? xchips_reward_bn4(e) : e;
			put16(row + 2 * k, drops[v][k]);
		}
		xg_set(XG_ROWS, (uint32_t)(f->first_id + v), row);
	}
}

/* data.c's readers told of its ids (EnemyMoved), as the engine reads them now */
static void enemies_tell(const XVirusFamily *f) {
	for (int v = 0; v < XV_VERSIONS; ++v) {
		EnemyMoved e = { { (uint8_t)v, 0, (uint8_t)f->ai }, { 0 }, { 0 }, "", true, { 0 } };
		memcpy(e.drops, drops[v], sizeof e.drops);
		memcpy(e.stats, rec.stats + BN6_STATS_SIZE * v, BN6_STATS_SIZE);
		memcpy(e.traits, rec.traits, BN6_TRAITS_SIZE);
		rom_text_bytes(names.text[v], names.len[v], e.name, sizeof e.name);
		if (!enemy_moved_set(f->first_id + v, &e)) fprintf(stderr, "xvirus: enemy id %#x read as BN6's (no room)\n", f->first_id + v);
	}
}

/* ---- the pre-steps' hook: BN6's dispatcher, as it reads a virus's NameID for its own cases, goes on to a family's pre-step
 * by its AI index (XV_PRESTEP) where it has one (xvirus_shim.h); every other virus as before ---- */

static HookAct prestep_run(HookRegs *r, void *user) {
	(void)user;
	uint32_t ai = hook_read8(hook_read32(r->r[5] + BN6_T1_AI_DATA) + BN6_AI_INDEX);
	uint32_t to = ai >= BN6_FAMILIES && ai < XV_AIS ? hook_read32(XV_PRESTEP + 4u * ai) : 0;
	if (!to) return HOOK_CONTINUE;
	r->r[12] = to;
	return HOOK_JUMP;
}

static void prestep_hook(void) {
	static bool set;
	if (set) return;
	set = emu_hook(BN6_VIRUS_NAMEID_CASES, prestep_run, NULL);
	if (!set) fprintf(stderr, "xvirus: no hook at %08x: the families' pre-steps do not run\n", (unsigned)BN6_VIRUS_NAMEID_CASES);
}

#define INSTALLED_MAX XV_FAMILIES

/* its parts staged, each step only where all before it held */
static void family_stage(Ctx *c, const Game *g) {
	const XVirusFamily *f = c->f;
	if (f->prestep) prestep_set(c);
	for (int i = 0; i < f->nblocks && !xv_failed(); ++i) block_move(c, &f->blocks[i]);
	for (int i = 0; i < f->nclones && !xv_failed(); ++i) clone_move(c, &f->clones[i]);
	uint32_t entry[XV_TABLES] = { 0 };
	/* (its game's after-state while paused, as its game runs it from its spawn through its state 1: after its spawn's done,
	 * hook 5, and after its state 0, its appear, which BN6's state step runs in the paused opening; BN6's runs its state 1
	 * as the fight begins, a fight frame later, xdiff's lag) */
	uint32_t state0 = f->present ? paused_put(c, thumb_veneer_get(&c->pool, ref_to(f, f->states[0]) & ~1u) | 1) : 0;
	if (!xv_failed()) records_set(f, g, entry, state0);
	if (!xv_failed()) entries_set(c, entry);
	if (!xv_failed() && f->present) entry[5] = paused_put(c, entry[5] == BN6_FAMILY_NULLSUB ? 0 : entry[5]);
	uint8_t *v = c->pool.n ? xv_put(c->pool.base, (uint32_t)c->pool.n * THUMB_VENEER_SIZE, true) : NULL;
	if (v) memcpy(v, c->veneers, (size_t)c->pool.n * THUMB_VENEER_SIZE);
	for (int k = 0; k < XV_TABLES && !xv_failed(); ++k) xg_set_word(XG_FAMILY + k, (uint32_t)f->ai, entry[k]);
	if (!xv_failed()) ice_set(f, g);
	if (!xv_failed()) ids_set(f);
	if (!xv_failed()) names_set(f, g);
	if (!xv_failed()) sprites_set(f, g);
	if (!xv_failed()) arms_set(f, g);
	if (!xv_failed()) effects_set(f, g);
	if (!xv_failed()) types_set(f, g);
	if (!xv_failed()) xvirus_songs_set(f);
	if (!xv_failed()) drops_set(f, g);
}

bool xvirus_install_family(const XVirusFamily *f) {
	for (int i = 0; i < ninstalled; ++i)
		if (installed[i] == f) return true;
	if (!xg_grow()) {
		fprintf(stderr, "xvirus: %s not installed: BN6's tables not grown\n", f->name);
		return false;
	}
	xv_tx_begin();
	Game g = { 0 };
	static Ctx c;
	memset(&c, 0, sizeof c);
	c.f = f;
	c.pool = (ThumbVeneers){ c.veneers, f->home + XV_HOME_VENEERS, (int)(sizeof c.veneers / THUMB_VENEER_SIZE), 0 };
	if (ninstalled >= INSTALLED_MAX) xv_fail("too many families");
	else if (!emu_ready() || !game_of(f->game, &g)) xv_fail("its game's ROM is not read");
	else if (sane(f, &g)) {
		c.rom = g.rom;
		family_stage(&c, &g);
	}
	if (xv_failed()) {
		fprintf(stderr, "xvirus: %s not installed: %s\n", f->name, xv_why());
		xv_tx_end();
		return false;
	}
	uint32_t used = xv_used();
	xv_commit();
	xv_tx_end();
	xvirus_songs_staged(f);
	for (int i = 0; i < f->nblocks; ++i) xv_moved_add(f->blocks[i].start, f->blocks[i].at, f->blocks[i].end - f->blocks[i].start);
	for (int i = 0; i < f->nclones; ++i) xv_moved_add(f->clones[i].start, f->clones[i].at, f->clones[i].end - f->clones[i].start);
	xv_moved_add(f->struct1, f->home + XV_HOME_STRUCT1, BN6_TRAITS_SIZE);
	xv_moved_add(f->struct2, f->home + XV_HOME_STRUCT2, XV_VERSIONS * BN6_STATS_SIZE);
	if (g.tables) xv_moved_add(rom32(g.rom, g.tables + 4u * (uint32_t)f->src_ai), f->home + XV_HOME_STATES, 4u * (uint32_t)f->nstates);
	for (int v = 0; v < XV_VERSIONS; ++v) xv_moved_add(names.moved[v].from, names.moved[v].to, names.moved[v].len);
	installed[ninstalled++] = f;
	if (f->prestep) prestep_hook();
	enemies_tell(f);
	printf("xvirus: %s as AI %d, enemy ids %#x-%#x: %u bytes written, %d veneers\n", f->name, f->ai, f->first_id,
		f->first_id + XV_VERSIONS - 1, (unsigned)used, c.pool.n);
	return true;
}

/* ---- the code mover, for the Navis' install too (xvirus_move.h) ---- */

static Ctx move_ctx;
static Game move_game;

bool xv_move_begin(const XVirusFamily *f) {
	memset(&move_ctx, 0, sizeof move_ctx);
	move_ctx.f = f;
	move_ctx.pool = (ThumbVeneers){ move_ctx.veneers, f->home + XV_HOME_VENEERS, (int)(sizeof move_ctx.veneers / THUMB_VENEER_SIZE), 0 };
	if (!game_of(f->game, &move_game)) return xv_fail("%s: its game's ROM is not read", f->name);
	move_ctx.rom = move_game.rom;
	return true;
}

void xv_move_code(void) {
	const XVirusFamily *f = move_ctx.f;
	for (int i = 0; i < f->nblocks && !xv_failed(); ++i) block_move(&move_ctx, &f->blocks[i]);
	for (int i = 0; i < f->nclones && !xv_failed(); ++i) clone_move(&move_ctx, &f->clones[i]);
	uint8_t *v = move_ctx.pool.n ? xv_put(move_ctx.pool.base, (uint32_t)move_ctx.pool.n * THUMB_VENEER_SIZE, true) : NULL;
	if (v) memcpy(v, move_ctx.veneers, (size_t)move_ctx.pool.n * THUMB_VENEER_SIZE);
}

uint32_t xv_move_ref(XvRef r) { return ref_to(move_ctx.f, r); }

void xv_move_art(void) {
	if (!xv_failed()) sprites_set(move_ctx.f, &move_game);
	if (!xv_failed()) effects_set(move_ctx.f, &move_game);
	if (!xv_failed()) types_set(move_ctx.f, &move_game);
}

const uint8_t *xv_move_rom(void) { return move_ctx.rom; }
int xv_move_veneers(void) { return move_ctx.pool.n; }
bool xvirus_code_stage(const XVirusFamily *f, const XvRegion *r, uint32_t veneers, int nveneers) {
	char why[200];
	Game g = { 0 };
	if (!xvirus_code_ok(f, r, why, sizeof why)) return xv_fail("%s", why);
	if (!emu_ready() || !game_of(f->game, &g)) return xv_fail("its game's ROM is not read");
	static Ctx c;
	memset(&c, 0, sizeof c);
	c.f = f;
	c.rom = g.rom;
	c.region = move_ctx_region = r;
	int cap = (int)(sizeof c.veneers / THUMB_VENEER_SIZE);
	c.pool = (ThumbVeneers){ c.veneers, veneers, nveneers < cap ? nveneers : cap, 0 };
	for (int i = 0; i < f->nblocks && !xv_failed(); ++i) block_move(&c, &f->blocks[i]);
	for (int i = 0; i < f->nclones && !xv_failed(); ++i) clone_move(&c, &f->clones[i]);
	uint8_t *v = c.pool.n && !xv_failed() ? xv_put(c.pool.base, (uint32_t)c.pool.n * THUMB_VENEER_SIZE, true) : NULL;
	if (v) memcpy(v, c.veneers, (size_t)c.pool.n * THUMB_VENEER_SIZE);
	if (!xv_failed()) sprites_set(f, &g);
	if (!xv_failed()) effects_set(f, &g);
	if (!xv_failed()) types_set(f, &g);
	move_ctx_region = NULL;
	return !xv_failed();
}

uint32_t xvirus_code_at(const XVirusFamily *f, uint32_t a) { return moved_to(f, a); }

const XVirusFamily *xvirus_installed(int game, int src_ai) {
	for (int i = 0; i < ninstalled; ++i)
		if (installed[i]->game == game && installed[i]->src_ai == src_ai) return installed[i];
	return NULL;
}

void xvirus_install(int which) {
	static bool done;
	if (done || !emu_ready()) return;
	/* (BN5's families and battlefields where its ROM is read, BN4's where
	 * its is: none latched before one is, so a ROM put in on the ROMs
	 * screen within a session comes in as the next run's core starts) */
	bool bn5 = XR[XROM_BN5_COLONEL_US].data, bn4 = XR[XROM_BN4_RED_SUN_US].data;
	if (which == XVIRUS_FAMILIES && !bn5 && !bn4) return;
	done = true;
	/* (the families' room and the families claim what they write; the
	 * battlefields theirs, within: claims.h) */
	claims_begin(XV_CLAIMS);
	if (which == XVIRUS_CONTROL) xvirus_install_family(&xvirus_control);
	else if (which == XVIRUS_FAMILIES) {
		/* (a virus's support chip, BN5's support family's timing: its family's entry, which the dispatch's chip writer names, set
		 * before a family whose code calls it; none of them where the table cannot grow) */
		bool placed = bn5 && xg_grow() && xattack_grow() && xattack_set(XV_SUPPORT_FAMILY, XV_SUPPORT | 1u);
		if (bn5 && !placed) fprintf(stderr, "xvirus: BN5's support family's timing not placed: no family installed\n");
		for (int i = 0; placed && i < xvirus_bn5_count; ++i) xvirus_install_family(xvirus_bn5[i]);
		xfields_install(XROM_BN5_COLONEL_US);
		/* (BN4 Red Sun's, where its ROM is read, after BN5's: the visitors, docs/multirom/bn4.md) */
		if (bn4 && xvirus_bn4_copy_install())
			for (int i = 0; i < xvirus_bn4_count; ++i) xvirus_install_family(xvirus_bn4[i]);
		/* (BN4's Hole, after BN5's lava and sea where they came in: xpanels.h) */
		if (bn4) xfields_install(XROM_BN4_RED_SUN_US);
	}
	claims_end();
	/* (outside the families' claims: xsong.c's own copies, the areas' songs, are no one's) */
	for (int i = 0; i < ninstalled; ++i) xvirus_songs_noted(installed[i]);
}
