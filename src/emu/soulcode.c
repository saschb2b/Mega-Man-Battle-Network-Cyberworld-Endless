/* The code BN5's Souls run, moved into BN6's battle engine (soulcode.h). */
#include "soulcode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "soullife.h"
#include "thumb_reloc.h"
#include "xattack_grow.h"
#include "xnavi.h"
#include "xvirus_dispatch.h"
#include "xvirus_grow.h"
#include "xvirus_tx.h"

#define BUS 0x08000000u
#define THUMB_BX_LR 0x4770u
#define COUNTER_SIZE 36     /* counter_code's bytes */
#define BLS_MAX 128

static const SoulCode *code;   /* the recipe installed, NULL none */

uint32_t soulcode_moved(uint32_t bn5) {
	for (int i = 0; code && i < code->nblocks; ++i) {
		const XvBlock *b = &code->blocks[i];
		if ((bn5 & ~1u) >= b->start && (bn5 & ~1u) < b->end) return b->at + (bn5 - b->start);
	}
	return 0;
}

int soulcode_family(int bn5) {
	for (int i = 0; code && i < code->nfamilies; ++i)
		if (code->families[i].from == bn5) return code->families[i].to;
	return -1;
}

int soulcode_power(int bn5_id) {
	for (int i = 0; code && i < code->npowers; ++i)
		if (code->powers[i].id == bn5_id) return SOUL_POWER_FIRST + i;
	return -1;
}

/* ---- the stubs: what BN6 answers otherwise for the moved code ---- */

static int side_of(uint32_t obj) { return hook_read8(obj + BN6_T1_ALLIANCE) & 1; }

static uint8_t navi8(int side, uint32_t field) { return hook_read8(BN6_BATTLE_NAVI + (uint32_t)side * BN6_BATTLE_NAVI_SIZE + field); }

/* (r0 a side) BN5's pending Soul (0x080123FA): the Soul committed, its chaos, ColonelSoul's chip, its turns */
static HookAct pending(HookRegs *r, void *user) {
	(void)user;
	int side = (int)(r->r[0] & 1);
	uint8_t next = hook_read8(soul_field(side, BN6_SOUL_NEXT));
	r->r[0] = next ? next : hook_read8(soul_field(side, BN6_SOUL_ID));
	r->r[1] = hook_read8(soul_field(side, next ? BN6_SOUL_NEXT_CHAOS : BN6_SOUL_CHAOS));
	r->r[2] = hook_read16(soul_field(side, BN6_SOUL_CHIP));
	r->r[3] = hook_read8(soul_field(side, next ? BN6_SOUL_NEXT_TURNS : BN6_SOUL_TURNS));
	return HOOK_RETURN;
}

/* (a spawn the engine leaves out) no object */
static HookAct none(HookRegs *r, void *user) {
	(void)user;
	r->r[0] = 0;
	return HOOK_RETURN;
}

/* the byte BN5's getter answers: the battle NaviStats' field r1 of a side, MegaMan's form the Soul's number where he
 * stands in the proxy united (BN5 keeps the Soul there) */
static void navi_byte(HookRegs *r, int side) {
	uint32_t field = r->r[1] & 0xFF;
	uint8_t v = navi8(side, field);
	if (field == BN6_NAVI_FORM && v == BN6_FORM_PROXY && soul_united(side)) v = hook_read8(soul_field(side, BN6_SOUL_ID));
	r->r[0] = v;
	r->r[1] = BN6_BATTLE_NAVI;   /* (BN6's twin leaves the table's base in r1) */
}

static HookAct form(HookRegs *r, void *user) {
	(void)user;
	navi_byte(r, side_of(r->r[5]));
	return HOOK_RETURN;
}

static HookAct form_side(HookRegs *r, void *user) {
	(void)user;
	navi_byte(r, (int)(r->r[0] & 1));
	return HOOK_RETURN;
}

/* (a moved read of BN5's AI data's Soul field) its value from BN6_SOULS, on to the next instruction */
static HookAct field_read(HookRegs *r, void *user) {
	const SoulField *f = user;
	uint32_t obj = r->r[5], at = soul_field(side_of(obj), f->field);
	r->r[f->rd] = f->size == 2 ? hook_read16(at) : f->size == 4 ? hook_read32(at) : hook_read8(at);
	r->r[12] = (r->pc + 2) | 1;
	return HOOK_JUMP;
}

/* ---- the code ---- */

typedef struct {
	const SoulCode *c;
	ThumbVeneers pool;
	uint8_t veneers[SOULS_VENEERS_END - SOULS_VENEERS];
} Mover;

static bool in5(uint32_t a, uint32_t n) { return a >= BUS && a - BUS + n <= ROM_SIZE; }

static const XvData *data_at(const SoulCode *c, uint32_t a) {
	for (int i = 0; i < c->ndata; ++i)
		if (a >= c->data[i].start && a < c->data[i].end) return &c->data[i];
	return NULL;
}

static const XvNumber *listed(const XvNumber *t, int n, uint32_t at) {
	for (int i = 0; i < n; ++i)
		if (t[i].at == at) return &t[i];
	return NULL;
}

/* a call out of the block (its copy's bytes from bus address `base`, the BL at `off`) to `to`: straight where a BL
 * reaches, else through a veneer */
static bool call_to(Mover *m, uint8_t *bytes, uint32_t off, uint32_t base, uint32_t to) {
	to &= ~1u;
	if (!thumb_bl_write(bytes, off, base, to)) {
		uint32_t v = thumb_veneer_get(&m->pool, to);
		return v && thumb_bl_write(bytes, off, base, v);
	}
	return true;
}

/* a pool's or table's every word in block b's copy p listed and fixed (BN5's value checked); raw bytes kept */
static void block_pool(const SoulCode *c, const XvBlock *b, uint8_t *p, const XvData *d) {
	for (uint32_t w = (d->start + 3) & ~3u; d->kind == XV_POOL && w + 4 <= d->end && !xv_failed(); w += 4) {
		const XvNumber *n = listed(c->words, c->nwords, w);
		if (!n || get32(p + (w - b->start)) != n->was) xv_fail("the word at %08x is not the recipe's", (unsigned)w);
		else put32(p + (w - b->start), n->now);
	}
}

/* a run of code's calls (BN5's run to end): each out of the block listed, BN5's routine checked, aimed at BN6's in its
 * place; one within the block holds as the block moves */
static void block_calls(Mover *m, const XvBlock *b, uint8_t *p, uint32_t run, uint32_t end) {
	const SoulCode *c = m->c;
	uint32_t offs[BLS_MAX];
	int n = thumb_bl_scan(p + (run - b->start), end - run, offs, BLS_MAX);
	if (n < 0 || n >= BLS_MAX) { xv_fail("half a BL, or too many, in %08x-%08x", (unsigned)run, (unsigned)end); return; }
	for (int i = 0; i < n && !xv_failed(); ++i) {
		uint32_t off = run - b->start + offs[i], site = b->start + off, t;
		thumb_bl_read(p, off, b->start, &t);
		const XvNumber *call = listed(c->calls, c->ncalls, site);
		if (!call && t >= b->start && t < b->end) continue;
		if (!call || call->was != t) xv_fail("the call at %08x (to %08x) is not the recipe's", (unsigned)site, (unsigned)t);
		else if (!call_to(m, p, off, b->at, call->now)) xv_fail("the call at %08x cannot reach %08x", (unsigned)site, (unsigned)call->now);
	}
}

/* the immediates and raw bytes in block b's copy p that BN6 numbers otherwise, each BN5's checked */
static void block_numbers(const SoulCode *c, const XvBlock *b, uint8_t *p) {
	for (int i = 0; i < c->nimms && !xv_failed(); ++i) {
		const XvImm *im = &c->imms[i];
		if (im->at < b->start || im->at >= b->end) continue;
		uint16_t insn = get16(p + (im->at - b->start));
		ThumbImm was = { im->form, im->rd, im->rn, im->was };
		if (!thumb_imm_patch(&insn, &was, im->now)) xv_fail("the immediate at %08x is not the one expected", (unsigned)im->at);
		else put16(p + (im->at - b->start), insn);
	}
	for (int i = 0; i < c->nbytes && !xv_failed(); ++i) {
		const XvByte *y = &c->bytes[i];
		if (y->at < b->start || y->at >= b->end) continue;
		if (p[y->at - b->start] != y->was) xv_fail("the byte at %08x is not the one expected", (unsigned)y->at);
		else p[y->at - b->start] = y->now;
	}
}

/* One block copied and fixed: its code's calls (each listed, or within the block), its words (each listed, BN5's value
 * checked), its immediates and raw bytes; false (the step failed) where any is not as the recipe says */
static void block_move(Mover *m, const XvBlock *b, const uint8_t *rom5) {
	const SoulCode *c = m->c;
	uint32_t len = b->end - b->start;
	if (!in5(b->start, len) || xvirus_crc32(rom5 + (b->start - BUS), len) != b->crc) {
		xv_fail("block %08x-%08x is not the one expected (its CRC32)", (unsigned)b->start, (unsigned)b->end);
		return;
	}
	uint8_t *p = xv_put(b->at, len, true);
	if (!p) return;
	memcpy(p, rom5 + (b->start - BUS), len);
	for (uint32_t a = b->start; a < b->end && !xv_failed();) {
		const XvData *d = data_at(c, a);
		if (d) {
			block_pool(c, b, p, d);
			a = d->end;
			continue;
		}
		uint32_t run = a;
		while (a < b->end && !data_at(c, a)) a += 2;
		block_calls(m, b, p, run, a);
	}
	block_numbers(c, b, p);
}

/* BN5's counter's start (0x0800E338) as the moved code calls it: BN6's twin (0x080105F2: the same steps, BN6's families
 * 0x47 and 0x48 for BN5's 0x2C and 0x2D), then the family it set run once, r5 him, r7 his AttackVars, as BN5's runs it
 * at once (0x0800E39A, 0x0800E3A6) where BN6's leaves it to the next frame */
static void counter_code(uint8_t out[COUNTER_SIZE]) {
	static const uint16_t thumb[] = {
		0xB510,           /* push {r4,lr} */
		0x4C06,           /* ldr r4,[pc,#24]: BN6's twin */
		0, 0,             /* bl the bx r4 below */
		0x7A68,           /* ldrb r0,[r5,#9]: the family it set (his CurAction) */
		0x3810,           /* subs r0,#0x10 */
		0x0080,           /* lsls r0,r0,#2 */
		0x4904,           /* ldr r1,[pc,#16]: the word naming the attack families (grown) */
		0x6809,           /* ldr r1,[r1] */
		0x580C,           /* ldr r4,[r1,r0]: the family's routine */
		0, 0,             /* bl the bx r4 below */
		0xBD10,           /* pop {r4,pc} */
		0x4720,           /* bx r4 */
	};
	for (size_t i = 0; i < sizeof thumb / sizeof *thumb; ++i) put16(out + 2 * i, thumb[i]);
	put32(out + 28, BN6_COUNTER_START | 1);
	put32(out + 32, BN6_ATTACK_FAMILIES_REF);
	thumb_bl_write(out, 4, SOUL_STUB_COUNTER, SOUL_STUB_COUNTER + 26);
	thumb_bl_write(out, 20, SOUL_STUB_COUNTER, SOUL_STUB_COUNTER + 26);
}

/* its sprites into SOULS_ART and FREE_SOULS_ART (the list words with the families' room's, below) */
static void art_move(const SoulCode *c, const XRom *x) {
	for (int i = 0; i < c->nsprites && !xv_failed(); ++i) {
		const XvSprite *s = &c->sprites[i];
		uint32_t len;
		uint8_t *d = xnavi_sprite_bytes(x->data, x->layout->sprite_lists, s->list, s->index, &len);
		bool room = (s->at >= SOULS_ART && s->at + s->room <= SOULS_ART_END) ||
			(s->at >= FREE_SOULS_ART && s->at + s->room <= FREE_SOULS_ART_END);
		if (!d || len > s->room || !room) xv_fail("its sprite %d:%#x", s->list, s->index);
		uint8_t *p = d ? xv_put(s->at, len, true) : NULL;
		if (p) memcpy(p, d, len);
		free(d);
	}
	/* (BN5's counter's start, the Thumb its moved call goes to: BN6's twin, then the family it set once, as BN5's runs it) */
	uint8_t *k = xv_put(SOUL_STUB_COUNTER, COUNTER_SIZE, true);
	if (k) counter_code(k);
	/* (the stubs, each a bx lr its hook stands on) */
	uint8_t *s = xv_put(SOULS_STUBS, 4 * SOUL_STUBS_COUNT, true);
	for (int i = 0; s && i < SOUL_STUBS_COUNT; ++i) put16(s + 4 * i, THUMB_BX_LR);
}

/* its entries in the families' room: sprites in grown lists, effects, object types through the dispatch */
static void room_set(const SoulCode *c) {
	/* (list 0's the Souls' own: soullook.c lists them as it grows it) */
	for (int i = 0; i < c->nsprites && !xv_failed(); ++i)
		if (c->sprites[i].to_list != BN6_SPRITES_MEGAMAN)
			xg_set_word(XG_LIST + xvirus_list_slot(c->sprites[i].to_list), c->sprites[i].to_index, c->sprites[i].at);
	for (int i = 0; i < c->neffects && !xv_failed(); ++i) {
		const SoulEffect *e = &c->effects[i];
		xg_set_word(XG_EFFECTS, e->to, (uint32_t)(4u * e->list) | (uint32_t)e->index << 8 | (uint32_t)e->anim << 16 | (uint32_t)e->pal << 24);
	}
	for (int i = 0; i < c->ntypes && !xv_failed(); ++i) {
		const SoulType *t = &c->types[i];
		uint32_t routine = t->to == XV_TYPE_MOVED ? soulcode_moved(t->routine) : 0;
		if (t->to == XV_TYPE_MOVED && !routine) { xv_fail("its object %d:%#x's routine %08x is none of its code", t->kind, t->type, (unsigned)t->routine); return; }
		/* (a type another's install moved already, the same BN5 routine: its copy serves) */
		uint32_t twin = xvirus_dispatch_twin_at(t->kind, t->type), moved = xvirus_dispatch_moved_at(t->kind, t->type);
		if (t->to == XV_TYPE_MOVED && twin && emu_read8(twin) == XV_TYPE_MOVED && moved && emu_read32(moved) != (XV_STUB_NONE | 1)) {
			fprintf(stderr, "soulcode: the object %d:%#x is moved already (%08x): that copy serves\n", t->kind, t->type, (unsigned)emu_read32(moved));
			continue;
		}
		xg_type(t->kind, t->type, t->to, routine);
	}
}

static bool hooks_set(const SoulCode *c) {
	static const EmuHook stub[SOUL_STUBS_COUNT] = { pending, none, form, form_side };
	bool ok = true;
	for (int i = 0; i < SOUL_STUBS_COUNT; ++i) ok = ok && emu_hook(SOUL_STUB(i), stub[i], NULL);
	for (int i = 0; ok && i < c->nfields; ++i) {
		uint32_t at = soulcode_moved(c->fields[i].at);
		ok = at && emu_hook(at, field_read, (void *)(uintptr_t)&c->fields[i]);
	}
	for (int i = 0; ok && i < c->nfamilies; ++i) {
		uint32_t at = soulcode_moved(c->families[i].entry);
		ok = at && xattack_set(c->families[i].to, at);
	}
	return ok;
}

bool soulcode_install(const SoulCode *c) {
	if (code) return code == c;
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	if (!R.data || !emu_ready() || !x->data || !x->layout || !xattack_grow() || !xg_grow()) return false;
	static Mover m;
	memset(&m, 0, sizeof m);
	m.c = c;
	m.pool = (ThumbVeneers){ m.veneers, SOULS_VENEERS, (int)(sizeof m.veneers / THUMB_VENEER_SIZE), 0 };
	code = c;   /* (soulcode_moved answers from here on: the types' routines) */
	/* (its own bytes first, then its entries in the families' room: each step all or nothing) */
	xv_tx_begin();
	for (int i = 0; i < c->nblocks && !xv_failed(); ++i) block_move(&m, &c->blocks[i], x->data);
	if (!xv_failed()) art_move(c, x);
	uint8_t *v = !xv_failed() && m.pool.n ? xv_put(SOULS_VENEERS, (uint32_t)m.pool.n * THUMB_VENEER_SIZE, true) : NULL;
	if (v) memcpy(v, m.veneers, (size_t)m.pool.n * THUMB_VENEER_SIZE);
	if (xv_failed()) {
		fprintf(stderr, "soulcode: the Souls' code not moved: %s\n", xv_why());
		xv_tx_end();
		code = NULL;
		return false;
	}
	uint32_t used = xv_used();
	claims_begin(SOULCODE_CLAIMS);
	xv_commit();
	claims_end();
	xv_tx_end();
	xv_tx_begin();
	room_set(c);
	if (xv_failed()) {
		fprintf(stderr, "soulcode: the Souls' entries in BN6's grown tables not made: %s\n", xv_why());
		xv_tx_end();
		code = NULL;
		return false;
	}
	claims_begin(XV_CLAIMS);
	xv_commit();
	claims_end();
	xv_tx_end();
	claims_begin(SOULCODE_CLAIMS);
	bool hooked = hooks_set(c);
	claims_end();
	for (int i = 0; i < c->nblocks; ++i) xv_moved_add(c->blocks[i].start, c->blocks[i].at, c->blocks[i].end - c->blocks[i].start);
	printf("soulcode: the Souls' code moved: %d blocks, %u bytes, %d veneers%s\n", c->nblocks, (unsigned)used, m.pool.n,
		hooked ? "" : ", a hook NOT placed");
	return hooked;
}
