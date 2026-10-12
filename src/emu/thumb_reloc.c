#include "thumb_reloc.h"

#include <stdlib.h>

#include "bytes.h"

/* ---- BL ---- */

bool thumb_is_bl(uint16_t hi, uint16_t lo) { return (hi & 0xF800) == 0xF000 && (lo & 0xF800) == 0xF800; }

bool thumb_bl_decode(uint32_t at, uint16_t hi, uint16_t lo, uint32_t *target) {
	if (!thumb_is_bl(hi, lo)) return false;
	uint32_t off = (uint32_t)(hi & 0x7FF) << 12 | (uint32_t)(lo & 0x7FF) << 1;
	if (off & 0x400000) off |= 0xFF800000u;   /* (its sign, bit 22, carried up) */
	*target = at + 4 + off;
	return true;
}

bool thumb_bl_encode(uint32_t at, uint32_t target, uint16_t *hi, uint16_t *lo) {
	int64_t off = (int64_t)target - ((int64_t)at + 4);
	if (((target | at) & 1) || off < -THUMB_BL_REACH || off >= THUMB_BL_REACH) return false;
	uint32_t u = (uint32_t)off;
	*hi = (uint16_t)(0xF000 | (u >> 12 & 0x7FF));
	*lo = (uint16_t)(0xF800 | (u >> 1 & 0x7FF));
	return true;
}

bool thumb_bl_read(const uint8_t *code, uint32_t off, uint32_t base, uint32_t *target) {
	return thumb_bl_decode(base + off, get16(code + off), get16(code + off + 2), target);
}

bool thumb_bl_write(uint8_t *code, uint32_t off, uint32_t base, uint32_t target) {
	uint16_t hi, lo;
	if (!thumb_bl_encode(base + off, target, &hi, &lo)) return false;
	put16(code + off, hi);
	put16(code + off + 2, lo);
	return true;
}

int thumb_bl_scan(const uint8_t *code, uint32_t size, uint32_t *offs, int max) {
	int n = 0;
	for (uint32_t i = 0; i + 2 <= size;) {
		uint16_t op = get16(code + i);
		if ((op & 0xF000) != 0xF000) {
			i += 2;
			continue;
		}
		/* (0xF000-0xFFFF is a BL's half and nothing else in ARMv4T's Thumb) */
		if (i + 4 > size || !thumb_is_bl(op, get16(code + i + 2))) return -1;
		if (n < max) offs[n] = i;
		++n;
		i += 4;
	}
	return n;
}

/* ---- a block moved ---- */

bool thumb_block_rebase(const ThumbBlock *b, const uint32_t *offs, int n, ThumbResolve resolve, void *user) {
	if ((b->from ^ b->to) & 3) return false;
	if (n <= 0) return n == 0;
	/* first every call's new halves (0 for a call within: left alone), then
	 * written only when all of them are good */
	uint16_t *halves = calloc((size_t)n * 2, sizeof *halves);
	if (!halves) return false;
	bool ok = true;
	for (int i = 0; ok && i < n; ++i) {
		uint32_t off = offs[i], old;
		ok = !(off & 1) && off + 4 <= b->size && thumb_bl_read(b->code, off, b->from, &old);
		if (!ok || (old >= b->from && old - b->from < b->size)) continue;
		uint32_t to = resolve(old, user);
		ok = to && thumb_bl_encode(b->to + off, to, &halves[2 * i], &halves[2 * i + 1]);
	}
	for (int i = 0; ok && i < n; ++i) {
		if (!halves[2 * i]) continue;
		put16(b->code + offs[i], halves[2 * i]);
		put16(b->code + offs[i] + 2, halves[2 * i + 1]);
	}
	free(halves);
	return ok;
}

/* ---- immediates ---- */

/* Each form: its opcode (the top five bits), where its registers and its
 * immediate sit, the immediate's width and the bytes one step of it is. */
typedef struct {
	uint16_t op;
	uint8_t rd_at;
	int8_t rn_at;   /* -1: no rn */
	uint8_t imm_at, imm_bits, scale;
} FormBits;

static const FormBits FORMS[THUMB_FORMS] = {
	[THUMB_MOVS] = { 0x2000, 8, -1, 0, 8, 1 },
	[THUMB_CMP] = { 0x2800, 8, -1, 0, 8, 1 },
	[THUMB_ADDS] = { 0x3000, 8, -1, 0, 8, 1 },
	[THUMB_SUBS] = { 0x3800, 8, -1, 0, 8, 1 },
	[THUMB_STR] = { 0x6000, 0, 3, 6, 5, 4 },
	[THUMB_LDR] = { 0x6800, 0, 3, 6, 5, 4 },
	[THUMB_STRB] = { 0x7000, 0, 3, 6, 5, 1 },
	[THUMB_LDRB] = { 0x7800, 0, 3, 6, 5, 1 },
	[THUMB_STRH] = { 0x8000, 0, 3, 6, 5, 2 },
	[THUMB_LDRH] = { 0x8800, 0, 3, 6, 5, 2 },
	[THUMB_LDR_PC] = { 0x4800, 8, -1, 0, 8, 4 },
	[THUMB_LSLS] = { 0x0000, 0, 3, 6, 5, 1 },
	[THUMB_LSRS] = { 0x0800, 0, 3, 6, 5, 1 },
	[THUMB_ASRS] = { 0x1000, 0, 3, 6, 5, 1 },
};

bool thumb_imm_decode(uint16_t insn, ThumbForm form, ThumbImm *out) {
	if ((unsigned)form >= THUMB_FORMS || (insn & 0xF800) != FORMS[form].op) return false;
	const FormBits *f = &FORMS[form];
	out->form = form;
	out->rd = (uint8_t)(insn >> f->rd_at & 7);
	out->rn = f->rn_at < 0 ? 0 : (uint8_t)(insn >> f->rn_at & 7);
	out->imm = (uint16_t)((insn >> f->imm_at & ((1u << f->imm_bits) - 1)) * f->scale);
	return true;
}

bool thumb_imm_encode(const ThumbImm *in, uint16_t *insn) {
	if ((unsigned)in->form >= THUMB_FORMS) return false;
	const FormBits *f = &FORMS[in->form];
	unsigned steps = in->imm / f->scale;
	if (in->rd > 7 || in->rn > (f->rn_at < 0 ? 0 : 7) || in->imm % f->scale || steps >> f->imm_bits) return false;
	*insn = (uint16_t)(f->op | in->rd << f->rd_at | (f->rn_at < 0 ? 0 : in->rn << f->rn_at) | steps << f->imm_at);
	return true;
}

bool thumb_imm_patch(uint16_t *insn, const ThumbImm *was, uint16_t imm) {
	ThumbImm now;
	if (!thumb_imm_decode(*insn, was->form, &now) || now.rd != was->rd || now.rn != was->rn || now.imm != was->imm) return false;
	now.imm = imm;
	return thumb_imm_encode(&now, insn);
}

/* ---- veneers ---- */

/* push {r0,r1}; ldr r0,[pc,#4]; str r0,[sp,#4]; pop {r0,pc} */
static const uint16_t VENEER[4] = { 0xB403, 0x4801, 0x9001, 0xBD01 };

bool thumb_veneer(uint8_t out[THUMB_VENEER_SIZE], uint32_t at, uint32_t target) {
	if (at & 3) return false;
	for (int i = 0; i < 4; ++i) put16(out + 2 * i, VENEER[i]);
	put32(out + 8, target | 1);
	return true;
}

bool thumb_veneer_target(const uint8_t v[THUMB_VENEER_SIZE], uint32_t *target) {
	for (int i = 0; i < 4; ++i)
		if (get16(v + 2 * i) != VENEER[i]) return false;
	*target = get32(v + 8) & ~1u;
	return true;
}

uint32_t thumb_veneer_get(ThumbVeneers *pool, uint32_t target) {
	if (pool->base & 3) return 0;
	target &= ~1u;
	for (int i = 0; i < pool->n; ++i) {
		uint32_t t;
		if (thumb_veneer_target(pool->bytes + THUMB_VENEER_SIZE * i, &t) && t == target) return pool->base + (uint32_t)(THUMB_VENEER_SIZE * i);
	}
	if (pool->n >= pool->cap) return 0;
	uint32_t at = pool->base + (uint32_t)(THUMB_VENEER_SIZE * pool->n);
	thumb_veneer(pool->bytes + THUMB_VENEER_SIZE * pool->n, at, target);
	++pool->n;
	return at;
}
