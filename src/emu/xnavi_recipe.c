/* xnavi_recipe.h: what a Navi's recipe says of itself, before any ROM is
 * read. */
#include "xnavi_recipe.h"

#include <stdarg.h>
#include <stdio.h>

#include "bn6.h"
#include "rom.h"
#include "xattack_grow.h"
#include "xnavi_room.h"
#include "xvirus_dispatch.h"

#define BUS 0x08000000u

static bool no(char *why, size_t len, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
static bool no(char *why, size_t len, const char *fmt, ...) {
	if (why && len) {
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(why, len, fmt, ap);
		va_end(ap);
	}
	return false;
}

static bool span_in(uint32_t at, uint32_t len, uint32_t lo, uint32_t hi) { return at >= lo && at <= hi && len <= hi - at; }
static bool spans_meet(uint32_t a, uint32_t alen, uint32_t b, uint32_t blen) { return a < b + blen && b < a + alen; }

/* a routine of BN6's ROM, of the Navis' stubs or the moved objects (XN_STUBS to XN_OBJECTS_END), or of its own code (its
 * blocks or a clone's) */
static bool ref_ok(const XVirusFamily *f, XvRef r) {
	if (!(r.addr & 1)) return false;
	if (r.clone) return r.own && r.clone <= f->nclones && (r.addr & ~1u) >= f->clones[r.clone - 1].start && (r.addr & ~1u) < f->clones[r.clone - 1].end;
	if (r.own) return xvirus_is_code(f, r.addr & ~1u);
	return (r.addr < BUS + ROM_SIZE && r.addr > BUS) || span_in(r.addr & ~1u, 2, XN_STUBS, XN_OBJECTS_END);
}

/* its block or clone i (its blocks first): where it starts and ends in its game, where it goes */
static void piece_of(const XVirusFamily *f, int i, uint32_t *start, uint32_t *end, uint32_t *at) {
	bool block = i < f->nblocks;
	*start = block ? f->blocks[i].start : f->clones[i - f->nblocks].start;
	*end = block ? f->blocks[i].end : f->clones[i - f->nblocks].end;
	*at = block ? f->blocks[i].at : f->clones[i - f->nblocks].at;
}

/* its data in its blocks, words on a word's boundary */
static bool data_ok(const XVirusFamily *f, char *why, size_t len) {
	for (int i = 0; i < f->ndata; ++i) {
		const XvData *d = &f->data[i];
		const XvBlock *b = xvirus_block_of(f, d->start);
		if (d->start >= d->end || !b || d->end > b->end || d->kind < XV_POOL || d->kind > XV_ZERO) return no(why, len, "data %08x", (unsigned)d->start);
		if ((d->kind == XV_POOL || d->kind == XV_TABLE) && ((d->start | d->end) & 3)) return no(why, len, "words at %08x off a boundary", (unsigned)d->start);
	}
	return true;
}

/* its blocks and clones in the Navis' code room, apart, each the same distance from a word boundary as in its game */
static bool code_ok(const XVirusFamily *f, char *why, size_t len) {
	if (f->nblocks < 1) return no(why, len, "no blocks");
	for (int i = 0; i < f->nblocks + f->nclones; ++i) {
		bool block = i < f->nblocks;
		uint32_t start, end, at;
		piece_of(f, i, &start, &end, &at);
		if (start >= end || start < BUS || ((start | end) & 1) || ((at ^ start) & 3) || !span_in(at, end - start, XN_CODE, XN_ART))
			return no(why, len, "%s %d at %08x", block ? "block" : "clone", block ? i : i - f->nblocks, (unsigned)at);
		for (int k = 0; k < i; ++k) {
			uint32_t ks, ke, ka;
			piece_of(f, k, &ks, &ke, &ka);
			if (spans_meet(at, end - start, ka, ke - ks) || (block && k < f->nblocks && spans_meet(start, end - start, ks, ke - ks)))
				return no(why, len, "its code %d and %d overlap", k, i);
		}
	}
	return data_ok(f, why, len);
}

/* its states and hooks, calls (BN6's routines, the dispatch's spawners, the Navis' stubs, the moved objects) and immediates */
static bool refs_ok(const XNavi *n, char *why, size_t len) {
	const XVirusFamily *f = &n->code;
	for (int i = 0; i < f->nstates; ++i)
		if (!ref_ok(f, f->states[i])) return no(why, len, "state %d (%08x)", i, (unsigned)f->states[i].addr);
	for (int i = 0; i < XV_HOOKS; ++i)
		if (!ref_ok(f, f->hooks[i])) return no(why, len, "hook %d (%08x)", i, (unsigned)f->hooks[i].addr);
	const XvRef more[5] = { n->prestep, n->hook_a, n->companion, n->companion_end, n->hide };
	for (int i = 0; i < 5; ++i)
		if (!ref_ok(f, more[i])) return no(why, len, "its pre-step, hook A or companion (%08x)", (unsigned)more[i].addr);
	for (int i = 0; i < f->ncalls; ++i) {
		const XvPair *c = &f->calls[i];
		uint32_t spawner = xvirus_dispatch_spawner(f->game, c->from);
		bool stub = span_in(c->to, 2, XN_STUBS, XN_OBJECTS_END);
		if (xvirus_block_of(f, c->from) || (c->to & 1) || (spawner ? c->to != spawner : !stub && !(c->to > BUS && c->to < BUS + ROM_SIZE)))
			return no(why, len, "call %08x to %08x", (unsigned)c->from, (unsigned)c->to);
	}
	for (int i = 0; i < f->nimms; ++i)
		if ((f->imms[i].at & 1) || !xvirus_is_code(f, f->imms[i].at) || (unsigned)f->imms[i].form >= THUMB_FORMS)
			return no(why, len, "immediate at %08x", (unsigned)f->imms[i].at);
	return true;
}

/* what its attacks draw: each sprite at its place in the shared art, its effects past BN6's, its types the dispatch's kinds */
static bool art_ok(const XVirusFamily *f, char *why, size_t len) {
	for (int i = 0; i < f->nsprites; ++i) {
		const XvSprite *s = &f->sprites[i];
		if (xvirus_list_slot(s->to_list) < 0 || (s->at & 3) || !s->room || !span_in(s->at, s->room, XN_SHARED_ART, XN_END))
			return no(why, len, "sprite %d at %08x", i, (unsigned)s->at);
		for (int k = 0; k < i; ++k)
			if (spans_meet(s->at, s->room, f->sprites[k].at, f->sprites[k].room)) return no(why, len, "sprites %d and %d meet", k, i);
	}
	for (int i = 0; i < f->neffects; ++i)
		if (f->effects[i].to < BN6_EFFECT_COUNT) return no(why, len, "effect %#x, one of BN6's", f->effects[i].to);
	for (int i = 0; i < f->ntypes; ++i)
		if (xvirus_dispatch_slot(f->types[i].kind) < 0 || f->types[i].to == xvirus_dispatch_type(f->types[i].kind))
			return no(why, len, "object type %d:%#x", f->types[i].kind, f->types[i].from);
	return true;
}

/* its overlay of its own kind: a kind past BN6's in the grown tables, its masks in the art's rooms (before the shared art), its
 * sprite in a list BN6 grows (or BN6's own list 4) */
static bool overlay_ok(const XNavi *n, char *why, size_t len) {
	const XnOverlay *o = n->overlay;
	const XVirusFamily *f = &n->code;
	if (!o) return true;
	if (o->kind != XN_KIND_OF(f->src_ai) || o->kind >= XN_KINDS) return no(why, len, "its overlay's kind %d (its own is %d)", o->kind, XN_KIND_OF(f->src_ai));
	if (!o->masks_len || o->masks < BUS || !span_in(o->masks_at, o->masks_len, XN_ART, XN_SHARED_ART) || xvirus_list_slot(o->list) < 0)
		return no(why, len, "its overlay's masks at %08x or its sprite's list %d", (unsigned)o->masks_at, o->list);
	return true;
}

/* the moved objects' types it needs: each of a kind the dispatch moves, its type moved (xnavi_objects.h); and BN5's
 * attack families it moves: each at BN6's number for it, in the moved families' range, its routine its own code */
static bool needs_ok(const XNavi *n, char *why, size_t len) {
	for (int i = 0; i < n->nneeds; ++i)
		if (xvirus_dispatch_slot(n->needs[i].kind) < 0 || n->needs[i].to != XV_TYPE_MOVED)
			return no(why, len, "its need %d:%#x", n->needs[i].kind, n->needs[i].from);
	for (int i = 0; i < n->nfamilies; ++i) {
		const XnFamily *x = &n->families[i];
		if (x->bn6 != XN_FAMILY_OF(x->bn5) || x->bn6 < XATTACK_CHIPS || x->bn6 >= XATTACK_CHIPS_END || !x->at.own || !ref_ok(&n->code, x->at))
			return no(why, len, "its family %#x as %#x (%08x)", x->bn5, x->bn6, (unsigned)x->at.addr);
	}
	return true;
}

/* its AI index, ids and home the Navis' */
static bool numbers_ok(const XNavi *n, char *why, size_t len) {
	const XVirusFamily *f = &n->code;
	if (f->ai < XN_FIRST_AI || f->ai >= XN_AIS) return no(why, len, "AI index %d (the Navis' are %d-%d)", f->ai, XN_FIRST_AI, XN_AIS - 1);
	if (n->versions < 1 || n->versions > XN_VERSIONS_MAX || f->first_id < XN_FIRST_ID || f->first_id + n->versions > XN_IDS_END)
		return no(why, len, "ids from %#x, %d versions (the Navis' are %#x-%#x)", f->first_id, n->versions, XN_FIRST_ID, XN_IDS_END - 1);
	if (f->home < XN_HOMES || (f->home - XN_HOMES) % XV_HOME_SIZE || (f->home - XN_HOMES) / XV_HOME_SIZE >= XN_HOMES_MAX)
		return no(why, len, "home %08x", (unsigned)f->home);
	return true;
}

bool xnavi_recipe_ok(const XNavi *n, char *why, size_t len) {
	const XVirusFamily *f = &n->code;
	if (!f->name) return no(why, len, "no name");
	if (!numbers_ok(n, why, len)) return false;
	if (f->nstates < 8 || f->nstates > XV_STATES_MAX) return no(why, len, "%d states", f->nstates);
	if (!f->struct1 || !f->struct2 || (f->held && (f->held < BUS || f->held >= BUS + ROM_SIZE)) || (f->held && f->held_chips))
		return no(why, len, "its records");
	if (f->ice < 0 || f->ice > XV_ICE_MAX) return no(why, len, "an ice block of size %d", f->ice);
	return code_ok(f, why, len) && refs_ok(n, why, len) && art_ok(f, why, len) && overlay_ok(n, why, len) && needs_ok(n, why, len);
}

/* the moved objects' blocks in XN_OBJECTS, their data and bytes in them */
static bool objects_code_ok(const XVirusFamily *f, char *why, size_t len) {
	for (int i = 0; i < f->nblocks; ++i) {
		const XvBlock *b = &f->blocks[i];
		if (b->start >= b->end || b->start < BUS || ((b->start | b->end) & 1) || ((b->at ^ b->start) & 3) ||
			!span_in(b->at, b->end - b->start, XN_OBJECTS, XN_OBJECTS_END))
			return no(why, len, "block %d at %08x", i, (unsigned)b->at);
	}
	if (!data_ok(f, why, len)) return false;
	for (int i = 0; i < f->nbytes; ++i) {
		const XvData *d = xvirus_data_at(f, f->bytes[i].at);
		if (!d || d->kind != XV_RAW) return no(why, len, "byte at %08x, in no raw data", (unsigned)f->bytes[i].at);
	}
	return true;
}

/* their calls (BN6's routines, the dispatch's spawners), sprites (the shared art) and types (moved) */
static bool objects_refs_ok(const XVirusFamily *f, char *why, size_t len) {
	for (int i = 0; i < f->ncalls; ++i) {
		const XvPair *c = &f->calls[i];
		uint32_t spawner = xvirus_dispatch_spawner(f->game, c->from);
		if (xvirus_block_of(f, c->from) || (c->to & 1) || (spawner ? c->to != spawner : !(c->to > BUS && c->to < BUS + ROM_SIZE)))
			return no(why, len, "call %08x to %08x", (unsigned)c->from, (unsigned)c->to);
	}
	for (int i = 0; i < f->nsprites; ++i) {
		const XvSprite *s = &f->sprites[i];
		if (xvirus_list_slot(s->to_list) < 0 || (s->at & 3) || !s->room || !span_in(s->at, s->room, XN_SHARED_ART, XN_END))
			return no(why, len, "sprite %d at %08x", i, (unsigned)s->at);
	}
	for (int i = 0; i < f->ntypes; ++i)
		if (xvirus_dispatch_slot(f->types[i].kind) < 0 || f->types[i].to != XV_TYPE_MOVED)
			return no(why, len, "object type %d:%#x, not moved", f->types[i].kind, f->types[i].from);
	return true;
}

bool xnavi_objects_ok(const XVirusFamily *f, char *why, size_t len) {
	if (!f->name || f->home != XN_OBJECTS_HOME || f->nblocks < 1 || f->nclones) return no(why, len, "its name, home or blocks");
	if (f->nstates || f->ntypes < 1) return no(why, len, "states of its own, or no type");
	for (int i = 0; i < f->neffects; ++i)
		if (f->effects[i].to < BN6_EFFECT_COUNT) return no(why, len, "effect %#x, one of BN6's", f->effects[i].to);
	return objects_code_ok(f, why, len) && objects_refs_ok(f, why, len);
}
