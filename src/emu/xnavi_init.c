/* xnavi_init.h: a Navi's init's work past BN6's actor init, as shims of
 * its home. BN6 calls both places by mov lr,pc and bx r0 (0x0800F430, its
 * actor init's companion start at 0x08017064), lr without its Thumb bit:
 * a shim returns by pop {pc}, which stays in Thumb on the GBA's ARMv4T. */
#include "xnavi_init.h"

#include <stdbool.h>

#include "bn6.h"
#include "bn6_families.h"
#include "bn6_panels.h"
#include "bytes.h"
#include "xvirus.h"
#include "xvirus_move.h"
#include "xvirus_tx.h"

#define SHIM_HALVES 64   /* a shim's instructions at most (BN4's Navis' longest, WindMan's two companions and his update: 34) */
#define SHIM_LITS   8
#define COLL_ARG6   3u   /* BN6's actor init's r3 (movs r3,#3 at 0x08016FE8) */
#define BL_REACH    0x400000

typedef struct {
	uint32_t at;                   /* where it goes (a word's start) */
	uint16_t h[SHIM_HALVES];
	int k;
	uint32_t lit[SHIM_LITS];       /* its pool's words, after its code */
	int nlit;
	int ld_k[SHIM_HALVES], ld_n[SHIM_HALVES], nld;   /* each ldr's place and its word's */
	bool bad;
} Shim;

static void op(Shim *s, uint32_t x) {
	if (s->k < SHIM_HALVES) s->h[s->k++] = (uint16_t)x;
	else s->bad = true;
}

/* ldr rd,=v (its offset given as the pool is laid out) */
static void lit(Shim *s, int rd, uint32_t v) {
	int n = 0;
	while (n < s->nlit && s->lit[n] != v) ++n;
	if (n == s->nlit && s->nlit < SHIM_LITS) s->lit[s->nlit++] = v;
	if (n >= SHIM_LITS || s->nld >= SHIM_HALVES) { s->bad = true; return; }
	s->ld_k[s->nld] = s->k, s->ld_n[s->nld++] = n;
	op(s, 0x4800u | (unsigned)rd << 8);
}

/* bl target (its own code, a bl's reach away) */
static void bl(Shim *s, uint32_t target) {
	const int32_t off = (int32_t)((target & ~1u) - (s->at + 2u * (uint32_t)s->k + 4u));
	if (!target || off < -BL_REACH || off >= BL_REACH) s->bad = true;
	op(s, 0xF000u | ((uint32_t)off >> 12 & 0x7FFu));
	op(s, 0xF800u | ((uint32_t)off >> 1 & 0x7FFu));
}

/* a call of BN6's own code past a bl's reach: ldr r0,=target; mov r1,pc; adds r1,#5; mov lr,r1; bx r0 (lr with its Thumb
 * bit, as a bl gives) */
static void far_call(Shim *s, uint32_t target) {
	lit(s, 0, target | 1u);
	op(s, 0x4679), op(s, 0x3105), op(s, 0x468E), op(s, 0x4700);
}

/* one spawn of its init's (XnSpawn): its registers, the call, the words kept */
static void spawn(Shim *s, const XNavi *n, const XnSpawn *c) {
	if (c->r4 != XN_R4_ANY) {
		if (c->r4 <= 0xFFu) op(s, 0x2400u | c->r4);
		else lit(s, 4, c->r4);
	}
	if (c->table) {
		const XvBlock *b = xvirus_block_of(&n->code, c->table);
		if (!b) { s->bad = true; return; }
		op(s, 0x7929), lit(s, 3, b->at + (c->table - b->start)), op(s, 0x5C5A);   /* ldrb r1,[r5,#4]; ldr r3,=its table; ldrb r2,[r3,r1] */
		op(s, 0x2000u | c->hi), op(s, 0x0400), op(s, 0x4310);                     /* movs r0,#hi; lsls r0,r0,#16; orrs r0,r2 */
	}
	bl(s, xv_move_ref(c->spawner));
	const int stores = (c->link_at ? 3 : 0) + (c->keep_at ? 1 : 0);
	if ((c->link_at | c->keep_at) & 3u || c->link_at > 0x7Cu || c->keep_at > 0x7Cu) s->bad = true;
	if (!stores) return;
	op(s, 0x2800), op(s, 0xD000u | (unsigned)(stores - 1));   /* cmp r0,#0; beq past the stores */
	if (c->link_at) op(s, 0x2100u | c->link_from), op(s, 0x1949), op(s, 0x6001u | (c->link_at / 4u) << 6);
	if (c->keep_at) op(s, 0x6028u | (c->keep_at / 4u) << 6);
}

/* its code and pool written at s->at, `room` bytes at most, their size into *size; its address with the Thumb bit, 0 where
 * it failed */
static uint32_t shim_put(Shim *s, uint32_t room, const char *what, uint32_t *size_out) {
	if (s->k & 1) op(s, 0x46C0);
	const uint32_t pool = s->at + 2u * (uint32_t)s->k, size = pool + 4u * (uint32_t)s->nlit - s->at;
	if (s->bad || size > room) { xv_fail("its %s's shim does not fit its home or reach its calls", what); return 0; }
	for (int i = 0; i < s->nld; ++i) {
		const int k = s->ld_k[i];
		s->h[k] = (uint16_t)(s->h[k] | (pool + 4u * (uint32_t)s->ld_n[i] - ((s->at + 2u * (uint32_t)k + 4u) & ~3u)) / 4u);
	}
	*size_out = size;
	uint8_t *p = xv_put(s->at, size, true);
	for (int i = 0; p && i < s->k; ++i) put16(p + 2 * i, s->h[i]);
	for (int i = 0; p && i < s->nlit; ++i) put32(p + (pool - s->at) + 4u * (uint32_t)i, s->lit[i]);
	return p ? s->at | 1u : 0;
}

/* its spawn's end where its init needs one: its collision's r3, +0x11, its panel at +6, its spawns where at_end, its sprite,
 * its update */
static uint32_t end_set(const XNavi *n, uint32_t at, uint32_t room, uint32_t *size) {
	const XnInit *in = n->init;
	Shim s = { .at = at };
	op(&s, 0xB510);   /* push {r4, lr} */
	if (n->coll_arg) op(&s, 0x6D68), op(&s, 0x2100u | n->coll_arg), op(&s, 0x7001u | BN6_COLL_SETUP_ARG << 6);   /* [[r5,#0x54]+0xE] = arg */
	if (in->anim) op(&s, 0x7C28), op(&s, 0x7468);   /* ldrb r0,[r5,#0x10]; strb r0,[r5,#0x11] */
	if (in->home) op(&s, 0x8A68), op(&s, 0x80E8);   /* ldrh r0,[r5,#0x12]; strh r0,[r5,#6] */
	for (int i = 0; in->at_end && i < in->nspawns; ++i) spawn(&s, n, &in->spawns[i]);
	if (in->sprite) far_call(&s, BN6_SPRITE_UPDATE);
	if (in->frame) far_call(&s, BN6_NAVI_FRAME);
	op(&s, 0xBD10);   /* pop {r4, pc} */
	return shim_put(&s, room, "spawn end", size);
}

/* its companion start: its spawns, where not at_end */
static uint32_t start_set(const XNavi *n, uint32_t at, uint32_t room) {
	uint32_t size;
	Shim s = { .at = at };
	op(&s, 0xB510);
	for (int i = 0; i < n->init->nspawns; ++i) spawn(&s, n, &n->init->spawns[i]);
	op(&s, 0xBD10);
	return shim_put(&s, room, "companion start", &size);
}

/* its collision setup's own r3 alone (GutsMan's: no init past it): ldr r0,[r5,#0x54]; movs r1,#arg; strb r1,[r0,#0xE];
 * mov pc,lr */
static uint32_t coll_arg_set(const XNavi *n, uint32_t at) {
	const uint16_t shim[4] = { 0x6D68, (uint16_t)(0x2100u | n->coll_arg), (uint16_t)(0x7001u | BN6_COLL_SETUP_ARG << 6), 0x46F7 };
	uint8_t *p = xv_put(at, sizeof shim, true);
	for (int i = 0; p && i < 4; ++i) put16(p + 2 * i, shim[i]);
	return p ? at | 1u : 0;
}

/* its states each entered through a stub (ldr r0,=the state; b the shared part) whose shared part runs its hide step first:
 * push {r0, lr}; ldr r1,=hide_step; mov lr,pc; bx r1; pop {r0, r1}; mov lr,r1; bx r0. BN6's status dispatcher calls a state
 * by mov lr,pc and bx r0 (0x0801AF44), so the state returns to it as before, r5-r7 as it gave them (the hide step keeps
 * them). Its state table's words aimed at the stubs (staged after the table's own, so written over them) */
#define XN_HIDE_SHARED 20u
#define XN_HIDE_STUB   8u
#define XN_HIDE_SIZE(f) (XN_HIDE_SHARED + XN_HIDE_STUB * (uint32_t)(f)->nstates)
static bool hide_set(const XNavi *n, uint32_t at, uint32_t room) {
	static const uint16_t shared[7] = { 0xB501, 0x4903, 0x46FE, 0x4708, 0xBC03, 0x468E, 0x4700 };   /* (its ldr's word at +16) */
	const XVirusFamily *f = &n->code;
	if (at & 3u || XN_HIDE_SIZE(f) > room) { xv_fail("no room in its home for its hide step's stubs"); return false; }
	uint8_t *p = xv_put(at, XN_HIDE_SIZE(f), true), *st = xv_put(f->home + XV_HOME_STATES, 4u * (uint32_t)f->nstates, false);
	if (!p || !st) return false;
	for (int i = 0; i < 7; ++i) put16(p + 2 * i, shared[i]);
	put16(p + 14, 0x46C0);   /* (nop: its word aligned) */
	put32(p + 16, xv_move_ref(n->hide_step));
	for (int i = 0; i < f->nstates; ++i) {
		const uint32_t off = XN_HIDE_SHARED + XN_HIDE_STUB * (uint32_t)i;
		const int32_t back = -(int32_t)(off + 2u + 4u);   /* (the b at off + 2 back to the shared part, pc 4 past it) */
		put16(p + off, 0x4800);                           /* ldr r0,[pc,#0]: the word past the b */
		put16(p + off + 2, (uint16_t)(0xE000u | ((uint32_t)back >> 1 & 0x7FFu)));
		put32(p + off + 4, xv_move_ref(f->states[i]));
		put32(st + 4 * i, (at + off) | 1u);
	}
	return true;
}

uint32_t xnavi_init_set(const XNavi *n, uint32_t table[BN6_NAVI_TABLE_COUNT]) {
	const XVirusFamily *f = &n->code;
	const XnInit *in = n->init;
	const uint32_t companion = xv_move_ref(n->companion);
	const bool end = in ? in->at_end || in->anim || in->sprite || in->frame || in->home : n->coll_arg != 0;
	const bool start = in && !in->at_end && in->nspawns;
	uint32_t place = XV_HOME_SHIMS + xvirus_shims_room(f);
	if (end && table[7] != BN6_NAVI_NULLSUB) { xv_fail("its spawn end's hook is its own: no room for its init's work"); return 0; }
	if (start && companion != BN6_NAVI_NO_COMPANION) { xv_fail("its companion start is its own: no room for its init's spawns"); return 0; }
	if (n->hide_step.addr) {   /* (its hide step's stubs first: a Navi that hides none leaves its shims where they were) */
		place = (place + 3u) & ~3u;
		if (!hide_set(n, f->home + place, XV_HOME_VENEERS > place ? XV_HOME_VENEERS - place : 0)) return 0;
		place += XN_HIDE_SIZE(f);
	}
	if (end && !in) {
		if (place + 8u > XV_HOME_VENEERS) { xv_fail("no room in its home for its collision's r3 past its shims"); return 0; }
		table[7] = coll_arg_set(n, f->home + place);
		return companion;
	}
	place = (place + 3u) & ~3u;
	if (end) {
		uint32_t size = 0;
		const uint32_t at = end_set(n, f->home + place, XV_HOME_VENEERS > place ? XV_HOME_VENEERS - place : 0, &size);
		if (!at) return 0;
		table[7] = at;
		place = (place + size + 3u) & ~3u;
	}
	return start ? start_set(n, f->home + place, XV_HOME_VENEERS > place ? XV_HOME_VENEERS - place : 0) : companion;
}
