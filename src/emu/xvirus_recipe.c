/* xvirus_recipe.h: what a recipe says of itself, before any ROM is read. */
#include "xvirus_recipe.h"

#include <stdarg.h>
#include <stdio.h>

#include "bn5.h"
#include "bn6.h"
#include "rom.h"
#include "xnavi_recipe.h"
#include "xvirus_bn4_copy.h"
#include "xvirus_dispatch.h"
#include "xvirus_shim.h"

#define BUS     0x08000000u
#define BN6_END (BUS + ROM_SIZE)

uint32_t xvirus_crc32(const uint8_t *p, uint32_t n) {
	uint32_t c = 0xFFFFFFFFu;
	for (uint32_t i = 0; i < n; ++i) {
		c ^= p[i];
		for (int k = 0; k < 8; ++k) c = c >> 1 ^ (0xEDB88320u & (0u - (c & 1)));
	}
	return ~c;
}

const XvBlock *xvirus_block_of(const XVirusFamily *f, uint32_t a) {
	for (int i = 0; i < f->nblocks; ++i)
		if (a >= f->blocks[i].start && a < f->blocks[i].end) return &f->blocks[i];
	return NULL;
}

const XvData *xvirus_data_at(const XVirusFamily *f, uint32_t a) {
	for (int i = 0; i < f->ndata; ++i)
		if (a >= f->data[i].start && a < f->data[i].end) return &f->data[i];
	return NULL;
}

bool xvirus_is_code(const XVirusFamily *f, uint32_t a) { return xvirus_block_of(f, a) && !xvirus_data_at(f, a); }

static bool no(char *why, size_t n, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
static bool no(char *why, size_t n, const char *fmt, ...) {
	if (why && n) {
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(why, n, fmt, ap);
		va_end(ap);
	}
	return false;
}

static bool bn6_routine(uint32_t a) { return (a & 1) && a >= BUS && a < BN6_END; }
/* a routine of BN5's engine objects moved once for every track, at its copy (xnavi_objects.h, installed before any family) */
static bool objects_routine(uint32_t a) { return (a & 1) && a >= XN_OBJECTS && a < XN_OBJECTS_END; }
/* a routine of a Navi's copy of a type it moves, which another track's code runs as that type's (a chip's unit) */
static bool navis_routine(uint32_t a) { return (a & 1) && xn_code_at(a); }
bool xvirus_bn4_copy_routine(uint32_t a) {
	const XVirusFamily *f = &xvirus_bn4_bn6copy;
	for (int i = 0; (a & 1) && i < f->nblocks; ++i) {
		const XvBlock *b = &f->blocks[i];
		uint32_t at = a & ~1u;
		if (at >= b->at && at < b->at + (b->end - b->start)) return xvirus_is_code(f, at - b->at + b->start);
	}
	return false;
}

/* a state or a hook: a routine of BN6's, or code of the family's own (in its blocks, or in one of its clones) */
static bool ref_ok(const XVirusFamily *f, XvRef r) {
	if (r.clone) {
		if (!r.own || r.clone > f->nclones || !(r.addr & 1)) return false;
		const XvClone *c = &f->clones[r.clone - 1];
		return (r.addr & ~1u) >= c->start && (r.addr & ~1u) < c->end;
	}
	return r.own ? (r.addr & 1) && xvirus_is_code(f, r.addr & ~1u) : bn6_routine(r.addr);
}

/* the families' region: their code from XV_CODE, their art from XV_ART; their one stub the chip writer's (XV_CHIP_WRITER,
 * their support chips' family BN5's: xvirus_dispatch.c) */
static const XvRegion families = { XV_CODE, XV_ART, XV_ART, XV_ART_END, XV_CHIP_WRITER, XV_STUBS_END, false, XV_LATE_ART, XV_LATE_ART_END,
	XV_LATE_CODE, XV_LATE_CODE_END };

static bool blocks_ok(const XVirusFamily *f, const XvRegion *r, char *why, size_t n) {
	if (f->nblocks < 1) return no(why, n, "no blocks");
	for (int i = 0; i < f->nblocks; ++i) {
		const XvBlock *b = &f->blocks[i];
		if (b->start >= b->end || b->start < BUS || (b->start & 1) || (b->end & 1)) return no(why, n, "block %d's range", i);
		uint32_t len = b->end - b->start;
		bool placed = (b->at >= r->code && b->at + len <= r->code_end) || (r->code2 && b->at >= r->code2 && b->at + len <= r->code2_end);
		if (!placed || ((b->at ^ b->start) & 3)) return no(why, n, "block %d placed at %08x", i, (unsigned)b->at);
		for (int k = 0; k < i; ++k) {
			const XvBlock *o = &f->blocks[k];
			bool src = b->start < o->end && o->start < b->end;
			bool dst = b->at < o->at + (o->end - o->start) && o->at < b->at + (b->end - b->start);
			if (src || dst) return no(why, n, "blocks %d and %d overlap", k, i);
		}
	}
	return true;
}

static bool spans_meet(uint32_t a, uint32_t alen, uint32_t b, uint32_t blen) { return a < b + blen && b < a + alen; }

/* its clones: code of one block each, copied past its blocks' copies, a call aimed at a routine of BN6's */
static bool clones_ok(const XVirusFamily *f, const XvRegion *r, char *why, size_t n) {
	for (int i = 0; i < f->nclones; ++i) {
		const XvClone *c = &f->clones[i];
		uint32_t len = c->end - c->start;
		const XvBlock *b = xvirus_block_of(f, c->start);
		if (c->start >= c->end || ((c->start | c->end) & 1) || !b || c->end > b->end) return no(why, n, "clone %d's range", i);
		for (uint32_t a = c->start; a < c->end; a += 2)
			if (!xvirus_is_code(f, a)) return no(why, n, "clone %d holds data at %08x", i, (unsigned)a);
		if (c->at < r->code || c->at + len > r->code_end || ((c->at ^ c->start) & 3)) return no(why, n, "clone %d placed at %08x", i, (unsigned)c->at);
		for (int k = 0; k < f->nblocks; ++k)
			if (spans_meet(c->at, len, f->blocks[k].at, f->blocks[k].end - f->blocks[k].start)) return no(why, n, "clone %d over block %d", i, k);
		for (int k = 0; k < i; ++k)
			if (spans_meet(c->at, len, f->clones[k].at, f->clones[k].end - f->clones[k].start)) return no(why, n, "clones %d and %d overlap", k, i);
		if ((c->call & 1) || (c->to & 1) || !bn6_routine(c->to | 1)) return no(why, n, "clone %d's call %08x to %08x", i, (unsigned)c->call, (unsigned)c->to);
		if (c->imm.at && (c->imm.at < c->start || c->imm.at >= c->end || (c->imm.at & 1))) return no(why, n, "clone %d's immediate", i);
	}
	return true;
}

/* its numbers in its pools and bytes in its raw data, each in such a range and named once */
static bool fixes_ok(const XVirusFamily *f, char *why, size_t n) {
	for (int i = 0; i < f->nnumbers; ++i) {
		const XvData *d = xvirus_data_at(f, f->numbers[i].at);
		if ((f->numbers[i].at & 3) || !d || d->kind != XV_POOL) return no(why, n, "number at %08x, in no pool", (unsigned)f->numbers[i].at);
		for (int k = 0; k < i; ++k)
			if (f->numbers[k].at == f->numbers[i].at) return no(why, n, "number at %08x twice", (unsigned)f->numbers[i].at);
	}
	for (int i = 0; i < f->nbytes; ++i) {
		const XvData *d = xvirus_data_at(f, f->bytes[i].at);
		if (!d || d->kind != XV_RAW) return no(why, n, "byte at %08x, in no raw data", (unsigned)f->bytes[i].at);
		for (int k = 0; k < i; ++k)
			if (f->bytes[k].at == f->bytes[i].at) return no(why, n, "byte at %08x twice", (unsigned)f->bytes[i].at);
	}
	if (f->held && f->held_chips) return no(why, n, "held chips twice (a list of BN6's and its own)");
	return true;
}

static bool data_ok(const XVirusFamily *f, char *why, size_t n) {
	for (int i = 0; i < f->ndata; ++i) {
		const XvData *d = &f->data[i];
		const XvBlock *b = xvirus_block_of(f, d->start);
		if (d->start >= d->end || !b || d->end > b->end) return no(why, n, "data %08x-%08x not within a block", (unsigned)d->start, (unsigned)d->end);
		if (d->kind < XV_POOL || d->kind > XV_ZERO) return no(why, n, "data %08x's kind", (unsigned)d->start);
		if ((d->kind == XV_POOL || d->kind == XV_TABLE) && ((d->start | d->end) & 3)) return no(why, n, "words at %08x off a word boundary", (unsigned)d->start);
		for (int k = 0; k < i; ++k)
			if (d->start < f->data[k].end && f->data[k].start < d->end) return no(why, n, "data %08x and %08x overlap", (unsigned)f->data[k].start, (unsigned)d->start);
	}
	return true;
}

/* (a hook entered past its routine's push is code of its own blocks; a state never is) */
static bool refs_ok(const XVirusFamily *f, char *why, size_t n) {
	for (int i = 0; i < f->nstates; ++i)
		if (!ref_ok(f, f->states[i]) || f->states[i].entered) return no(why, n, "state %d (%08x)", i, (unsigned)f->states[i].addr);
	for (int i = 0; i < XV_HOOKS; ++i)
		if (!ref_ok(f, f->hooks[i]) || (f->hooks[i].entered && (!f->hooks[i].own || f->hooks[i].clone)))
			return no(why, n, "hook %d (%08x)", i, (unsigned)f->hooks[i].addr);
	return true;
}

/* its calls out (each to one of BN6's routines, one of its region's stubs, or the dispatch's spawner for one of its
 * game's) and its immediates */
static bool calls_ok(const XVirusFamily *f, const XvRegion *r, char *why, size_t n) {
	for (int i = 0; i < f->ncalls; ++i) {
		const XvPair *c = &f->calls[i];
		if (xvirus_is_code(f, c->from)) return no(why, n, "call %08x is to its own code", (unsigned)c->from);
		/* (one of BN6's routines, of the moved objects' or its region's stubs, or the second-level dispatch's spawner where it
		 * calls one of its game's; a chip's unit, a routine of a Navi's copy of a type it runs as theirs: never another) */
		uint32_t spawner = xvirus_dispatch_spawner(f->game, c->from);
		bool stub = c->to >= r->stubs && c->to < r->stubs_end;
		if ((c->to & 1) || (spawner ? c->to != spawner : !stub && !bn6_routine(c->to | 1) && !objects_routine(c->to | 1) && !xvirus_bn4_copy_routine(c->to | 1) &&
			!(r != &families && navis_routine(c->to | 1))))
			return no(why, n, "call %08x goes to %08x%s", (unsigned)c->from, (unsigned)c->to, spawner ? ", not the dispatch's spawner" : "");
		for (int k = 0; k < i; ++k)
			if (f->calls[k].from == c->from) return no(why, n, "call %08x twice", (unsigned)c->from);
	}
	for (int i = 0; i < f->nimms; ++i) {
		const XvImm *m = &f->imms[i];
		if ((m->at & 1) || !xvirus_is_code(f, m->at) || (unsigned)m->form >= THUMB_FORMS) return no(why, n, "immediate at %08x", (unsigned)m->at);
	}
	return true;
}

#define SPRITE_LISTS 10   /* (RomLayout.sprite_lists) */

int xvirus_list_slot(int list) {
	static const int grown[XV_LISTS_GROWN] = { BN6_SPRITES_VIRUSES, BN6_SPRITES_NAVIS, BN6_SPRITES_ATTACKS, BN6_SPRITES_LIST5, BN6_SPRITES_LIST3 };
	for (int i = 0; i < XV_LISTS_GROWN; ++i)
		if (grown[i] == list) return i;
	return -1;
}

uint32_t xvirus_list_entry(int list, uint32_t index) {
	int slot = xvirus_list_slot(list);
	if (slot < 0 || index >= XV_LIST_MAX) return 0;
	return (list == BN6_SPRITES_LIST3 ? XV_LIST3 : XV_LISTS + (uint32_t)slot * XV_STEP) + 4u * index;
}

/* a span wholly within lo to hi */
static bool span_in(uint32_t at, uint32_t room, uint32_t lo, uint32_t hi) {
	return at >= lo && at < hi && room <= hi - at;
}

/* its sprites each in its own room past XV_ART (or in the region's second room), at a number of its own in a grown list */
static bool sprites_ok(const XVirusFamily *f, const XvRegion *r, char *why, size_t n) {
	for (int i = 0; i < f->nsprites; ++i) {
		const XvSprite *s = &f->sprites[i];
		if (s->list >= SPRITE_LISTS || xvirus_list_slot(s->to_list) < 0) return no(why, n, "sprite %d's lists", i);
		bool placed = span_in(s->at, s->room, r->art, r->art_end) || (r->art2 && span_in(s->at, s->room, r->art2, r->art2_end));
		if ((s->at & 3) || !s->room || !placed) return no(why, n, "sprite %d placed at %08x", i, (unsigned)s->at);
		/* (a row of a room filled on demand: its place its name, apart from every other's; its room no place of its own) */
		bool row = r->rows && s->at >= r->rows && s->at < r->rows_end;
		for (int k = 0; k < i; ++k) {
			const XvSprite *o = &f->sprites[k];
			bool meet = row ? s->at == o->at : s->at < o->at + o->room && o->at < s->at + s->room;
			if (meet || (s->to_list == o->to_list && s->to_index == o->to_index))
				return no(why, n, "sprites %d and %d in one place", k, i);
		}
	}
	return true;
}

/* its game's after-state its own code, BN4's alone (xvirus_shim.h's paused present, after its state 0 and hook 5), or none */
static bool present_ok(const XVirusFamily *f) {
	return !f->present || (f->game == XROM_BN4_RED_SUN_US && !(f->present & 1) && xvirus_is_code(f, f->present));
}

/* the bytes its shims take in its home (XV_HOME_SHIMS on): a pre-step's at most, its ranges', chip compares' and
 * directions', two paused presents (its state 0's and its hook 5's) and its entries' */
uint32_t xvirus_shims_room(const XVirusFamily *f) {
	uint32_t room = (f->prestep ? XV_SHIM_PRESTEP_MAX : 0) + (uint32_t)f->nranges * XV_SHIM_RANGE_SIZE + (uint32_t)f->ncmps * XV_SHIM_CMP_SIZE +
	                (uint32_t)f->nsides * XV_SHIM_SIDE_SIZE + (uint32_t)f->nsound_calls * XV_SHIM_SOUND_SIZE;
	room += 2u * XV_SHIM_PAUSED_SIZE * (f->present != 0);
	for (int i = 0; i < XV_HOOKS; ++i) room += f->hooks[i].entered ? XV_SHIM_ENTRY_SIZE : 0;
	return room;
}

/* its shims' sites (each in its code) and room in its home, its pre-step one of the ports */
static bool shims_ok(const XVirusFamily *f, char *why, size_t n) {
	for (int i = 0; i < f->nranges; ++i) {
		const XvRange *r = &f->ranges[i];
		if ((r->at & 1) || !xvirus_is_code(f, r->at) || r->lo > r->hi || r->hi >= XV_IDS) return no(why, n, "NameID range %d at %08x", i, (unsigned)r->at);
	}
	for (int i = 0; i < f->ncmps; ++i) {
		const XvCmp *k = &f->cmps[i];
		if ((k->at & 1) || !xvirus_is_code(f, k->at) || !xvirus_is_code(f, k->at + 2) || k->reg > 7 || k->was > 0xFF || k->now <= 0xFF)
			return no(why, n, "chip compare %d at %08x", i, (unsigned)k->at);
	}
	for (int i = 0; i < f->nsides; ++i)
		if ((f->sides[i] & 1) || !xvirus_is_code(f, f->sides[i])) return no(why, n, "direction %d at %08x", i, (unsigned)f->sides[i]);
	/* (BN5's a port of its dispatcher's case; BN4's its own code) */
	if (f->prestep && (f->game == XROM_BN4_RED_SUN_US ? !xvirus_is_code(f, f->prestep) :
		f->game != XROM_BN5_COLONEL_US || (f->prestep != BN5_PRESTEP_APPLEY && f->prestep != BN5_PRESTEP_DOMINERD)))
		return no(why, n, "a pre-step %08x no port makes", (unsigned)f->prestep);
	if (!present_ok(f)) return no(why, n, "an after-state %08x none of its code", (unsigned)f->present);
	uint32_t room = xvirus_shims_room(f);
	if (room > XV_HOME_VENEERS - XV_HOME_SHIMS) return no(why, n, "its shims past their room in its home (%u bytes)", (unsigned)room);
	return true;
}

/* (BN6's empty song slots, each its shared header of no tracks in the player's ROM, that no other track takes: the themes'
 * 0x26-0x31 and the families' 0x34-0x3F below and above the first two, BN5's 0x40-0x62 left alone; the five to 0xFF a movs
 * or a byte names, then those past it; docs/rom/sound.md) */
const uint16_t xv_bn4_pool[XV_BN4_POOL_SLOTS] = { 0x32, 0x33, 0x63, 0xD0, 0xE0, 0x104, 0x117, 0x148, 0x150, 0x155, 0x159, 0x15D,
	0x167, 0x16B, 0x16E, 0x172 };

bool xv_bn4_pool_slot(int slot) {
	for (int i = 0; i < XV_BN4_POOL_SLOTS; ++i)
		if (xv_bn4_pool[i] == slot) return true;
	return false;
}

/* its songs in BN6's empty slots the families take (a BN4 family's fixed one, or the pool's for a late one) and in the art's
 * room or the late families', apart from its sprites and each other */
static bool songs_ok(const XVirusFamily *f, char *why, size_t n) {
	for (int i = 0; i < f->nsongs; ++i) {
		const XvSong *g = &f->songs[i];
		bool bn4 = f->game == XROM_BN4_RED_SUN_US;
		bool slot = bn4 ? (g->slot >= XV_BN4_SONG_SLOT_FIRST && g->slot <= XV_BN4_SONG_SLOT_LAST) || xv_bn4_pool_slot(g->slot) :
		                  g->slot >= XV_SONG_SLOT_FIRST && g->slot <= BN6_SONGS_EMPTY_LAST;
		bool room = span_in(g->at, g->room, XV_ART, XV_ART_END) || span_in(g->at, g->room, XV_LATE_ART, XV_LATE_ART_END);
		if (!slot || !room || (g->at & 3) || !g->room)
			return no(why, n, "song %d (slot %#x at %08x)", i, g->slot, (unsigned)g->at);
		for (int k = 0; k < f->nsprites; ++k)
			if (spans_meet(g->at, g->room, f->sprites[k].at, f->sprites[k].room)) return no(why, n, "song %d over sprite %d", i, k);
		for (int k = 0; k < i; ++k)
			if (spans_meet(g->at, g->room, f->songs[k].at, f->songs[k].room) || g->slot == f->songs[k].slot) return no(why, n, "songs %d and %d in one place", k, i);
	}
	return true;
}

/* what it draws (its sprites, its effects past BN6's, its songs in the families' room), its attacks' types one each */
static bool art_ok(const XVirusFamily *f, const XvRegion *r, char *why, size_t n) {
	if (!sprites_ok(f, r, why, n)) return false;
	for (int i = 0; i < f->neffects; ++i) {
		if (f->effects[i].to < BN6_EFFECT_COUNT) return no(why, n, "effect %#x, one of BN6's", f->effects[i].to);
		for (int k = 0; k < i; ++k)
			if (f->effects[k].to == f->effects[i].to) return no(why, n, "effect %#x twice", f->effects[i].to);
	}
	for (int i = 0; i < f->ntypes; ++i) {
		const XvType *t = &f->types[i];
		if (xvirus_dispatch_slot(t->kind) < 0) return no(why, n, "object type %#x of kind %d, which the dispatch has none of", t->from, t->kind);
		if (t->to == xvirus_dispatch_type(t->kind) && t->to != XV_TYPE_SHARED) return no(why, n, "object type %#x as the dispatch's own", t->from);
		for (int k = 0; k < i; ++k)
			if (f->types[k].kind == t->kind && f->types[k].from == t->from) return no(why, n, "object type %d:%#x twice", t->kind, t->from);
	}
	if (f->traits.laid && (!f->nsprites || f->sprites[0].to_list != BN6_SPRITES_VIRUSES))
		return no(why, n, "its struct1 laid out with no battle sprite of list 1");
	return songs_ok(f, why, n);
}

/* its code in its region: its blocks, data, clones, fixes and calls out */
static bool code_ok(const XVirusFamily *f, const XvRegion *r, char *why, size_t n) {
	return blocks_ok(f, r, why, n) && data_ok(f, why, n) && clones_ok(f, r, why, n) && fixes_ok(f, why, n) && calls_ok(f, r, why, n);
}

bool xvirus_recipe_ok(const XVirusFamily *f, char *why, size_t n) {
	if (!f->name) return no(why, n, "no name");
	if (f->ai < BN6_FAMILIES || f->ai >= XV_AIS) return no(why, n, "AI index %d (BN6's tables end at %d, the grown ones at %d)", f->ai, BN6_FAMILIES, XV_AIS);
	if (f->first_id < XV_FIRST_ID || f->first_id + XV_VERSIONS > XV_IDS) return no(why, n, "enemy ids from %#x (the families' are %#x-%#x)", f->first_id,
		XV_FIRST_ID, XV_IDS - 1);
	if (f->nstates < 8 || f->nstates > XV_STATES_MAX) return no(why, n, "%d states", f->nstates);
	if (f->home < XV_HOMES || f->home + XV_HOME_SIZE > XV_CODE || (f->home - XV_HOMES) % XV_HOME_SIZE) return no(why, n, "home %08x", (unsigned)f->home);
	if (!f->struct1 || !f->struct2 || (f->held && (f->held < BUS || f->held >= BN6_END))) return no(why, n, "its records");
	if (f->ice != XV_ICE_OWN && (f->ice < 0 || f->ice > XV_ICE_MAX)) return no(why, n, "an ice block of size %d", f->ice);
	return code_ok(f, &families, why, n) && refs_ok(f, why, n) && art_ok(f, &families, why, n) && shims_ok(f, why, n);
}

bool xvirus_code_ok(const XVirusFamily *f, const XvRegion *r, char *why, size_t n) {
	if (!f->name) return no(why, n, "no name");
	if (f->nstates || f->held || f->held_chips || f->traits.laid) return no(why, n, "states, records or held chips: a family's, not code alone");
	return code_ok(f, r, why, n) && art_ok(f, r, why, n);
}
