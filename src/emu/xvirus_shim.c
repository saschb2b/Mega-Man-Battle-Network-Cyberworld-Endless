/* xvirus_shim.h: the shims' Thumb, encoded field by field (ARMv4T), each
 * branch and literal load resolved within the shim. */
#include "xvirus_shim.h"

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "thumb_reloc.h"

#define LABELS 8
#define FIXES  16

/* A shim as it is written: halfwords from `at` (its bus address), labels by
 * their offset, branches and literal loads fixed once all is written */
typedef struct {
	uint8_t *out;
	int max, n;
	uint32_t at;
	bool bad;
	int label[LABELS];
	struct { int at, label; char kind; } fix[FIXES];   /* (kind: 'b' a branch, 'c' a conditional one, 'l' a literal load) */
	int nfix;
} Asm;

static void half(Asm *a, uint16_t v) {
	if (a->n + 2 > a->max) { a->bad = true; return; }
	put16(a->out + a->n, v);
	a->n += 2;
}

static void here(Asm *a, int label) { a->label[label] = a->n; }

/* a literal word, labelled, the word aligned first (a nop: mov r8,r8) */
static void word(Asm *a, int label, uint32_t v) {
	if (a->n & 2) half(a, 0x46C0);
	here(a, label);
	half(a, (uint16_t)v);
	half(a, (uint16_t)(v >> 16));
}

static void to_label(Asm *a, uint16_t op, int label, char kind) {
	if (a->nfix >= FIXES) { a->bad = true; return; }
	a->fix[a->nfix++] = (__typeof__(a->fix[0])){ a->n, label, kind };
	half(a, op);
}

static void bl(Asm *a, uint32_t target) {
	uint16_t hi, lo;
	if (!thumb_bl_encode(a->at + (uint32_t)a->n, target, &hi, &lo)) { a->bad = true; return; }
	half(a, hi);
	half(a, lo);
}

/* the labels resolved: false where a branch or a literal is out of reach */
static bool done(Asm *a) {
	for (int i = 0; i < a->nfix && !a->bad; ++i) {
		int at = a->fix[i].at, to = a->label[a->fix[i].label];
		uint16_t op = get16(a->out + at);
		int32_t off = to - (at + 4);
		if (a->fix[i].kind == 'l') {
			int32_t lit = to - ((at + 4) & ~3);
			if (lit < 0 || lit > 1020 || (lit & 3) || (to & 3)) a->bad = true;
			op = (uint16_t)(op | lit / 4);
		} else if (a->fix[i].kind == 'c') {
			if (off < -256 || off > 254) a->bad = true;
			op = (uint16_t)(op | ((off >> 1) & 0xFF));
		} else {
			if (off < -2048 || off > 2046) a->bad = true;
			op = (uint16_t)(op | ((off >> 1) & 0x7FF));
		}
		put16(a->out + at, op);
	}
	return !a->bad;
}

/* ---- the forms the shims use (rd, rn, rm low registers) ---- */

#define MOVS(rd, imm)       ((uint16_t)(0x2000 | (rd) << 8 | (imm)))
#define CMP_I(rd, imm)      ((uint16_t)(0x2800 | (rd) << 8 | (imm)))
#define ADDS_I(rd, imm)     ((uint16_t)(0x3000 | (rd) << 8 | (imm)))
#define CMP_R(rn, rm)       ((uint16_t)(0x4280 | (rm) << 3 | (rn)))
#define TST(rn, rm)         ((uint16_t)(0x4200 | (rm) << 3 | (rn)))
#define SUBS_R(rd, rn, rm)  ((uint16_t)(0x1A00 | (rm) << 6 | (rn) << 3 | (rd)))
#define LDR_I(rd, rn, off)  ((uint16_t)(0x6800 | ((off) / 4) << 6 | (rn) << 3 | (rd)))
#define STR_I(rd, rn, off)  ((uint16_t)(0x6000 | ((off) / 4) << 6 | (rn) << 3 | (rd)))
#define LDRH_R(rd, rn, rm)  ((uint16_t)(0x5A00 | (rm) << 6 | (rn) << 3 | (rd)))
#define STRH_R(rd, rn, rm)  ((uint16_t)(0x5200 | (rm) << 6 | (rn) << 3 | (rd)))
#define STR_R(rd, rn, rm)   ((uint16_t)(0x5000 | (rm) << 6 | (rn) << 3 | (rd)))
#define LDR_PC(rd)          ((uint16_t)(0x4800 | (rd) << 8))   /* (its offset fixed) */
#define BCC(cond)           ((uint16_t)(0xD000 | (cond) << 8))
#define B_                  ((uint16_t)0xE000)
#define BX(rm)              ((uint16_t)(0x4700 | (rm) << 3))
#define PUSH_R(r)           ((uint16_t)(0xB400 | 1u << (r)))
#define POP_R(r)            ((uint16_t)(0xBC00 | 1u << (r)))
#define MOV_LR(rm)          ((uint16_t)(0x4686 | (rm) << 3))   /* mov lr, rm */
#define BX_LR               ((uint16_t)0x4770)
enum { EQ = 0, NE = 1, MI = 4 };

/* ---- a NameID range ---- */

bool xvirus_shim_range(uint8_t out[XV_SHIM_RANGE_SIZE], uint32_t at, uint16_t lo, uint16_t hi, uint32_t veneer) {
	enum { LO, HI, ON };
	Asm a = { out, XV_SHIM_RANGE_SIZE, 0, at, (at & 3) != 0, { 0 }, { { 0 } }, 0 };
	to_label(&a, LDR_PC(1), LO, 'l');
	to_label(&a, LDR_PC(2), HI, 'l');
	if (!a.bad) {
		int32_t off = (int32_t)(veneer - (at + 4 + 4));
		if (off < -2048 || off > 2046 || (veneer & 3)) a.bad = true;
		else half(&a, (uint16_t)(B_ | ((off >> 1) & 0x7FF)));
	}
	word(&a, LO, lo);
	word(&a, HI, hi);
	(void)ON;
	return done(&a) && a.n == XV_SHIM_RANGE_SIZE;
}

/* ---- a chip compare ---- */

bool xvirus_shim_cmp(uint8_t out[XV_SHIM_CMP_SIZE], uint32_t at, int reg, uint16_t now, bool eq, uint32_t to) {
	enum { RET, NOW, TO };
	Asm a = { out, XV_SHIM_CMP_SIZE, 0, at, (at & 3) != 0 || reg < 0 || reg > 7 || (to & 1), { 0 }, { { 0 } }, 0 };
	int s = reg ? 0 : 1;   /* (a scratch register other than the one compared, kept on the stack) */
	half(&a, PUSH_R(s));
	to_label(&a, LDR_PC(s), NOW, 'l');
	half(&a, CMP_R(reg, s));
	half(&a, POP_R(s));
	to_label(&a, BCC(eq ? NE : EQ), RET, 'c');   /* (the branch not taken: back to the caller) */
	half(&a, PUSH_R(s));
	to_label(&a, LDR_PC(s), TO, 'l');
	half(&a, MOV_LR(s));
	half(&a, POP_R(s));
	here(&a, RET);
	half(&a, BX_LR);
	word(&a, NOW, now);
	word(&a, TO, to | 1);
	return done(&a) && a.n == XV_SHIM_CMP_SIZE;
}

/* ---- a pre-step: BN5's case, its collision data's offsets BN6's (BN5's +0x68 the hit's flags BN6's +0x70, its +0x74 on
 * BN6's +0x7C: tests/xdiff/legit.json's collision layout) ---- */

#define COLL         0x54   /* an object's collision data (BN6_T1_COLLISION) */
#define C_DAMAGE     0x80   /* the pending hit's damage (BN5's +0x78) */
#define C_PARTS      0x82   /* ... its parts, five halfwords (BN5's +0x7A-+0x82), */
#define C_PARTS_END  0x8C   /* ... to here (BN5's +0x84, a word Appley's case clears) */
#define C_EXTRA      0x8E   /* ... and a halfword Dominerd's clears (BN5's +0x86) */
#define C_HELD       0x84   /* a halfword Appley's case waits on (BN5's +0x7C) */
#define C_FLAGS      0x70   /* what struck it (BN5's +0x68) */

/* the labels take() places and branches to: the mask's literal, its loop, the one left, the way out */
typedef struct { int lit, loop, one, out; } TakeLabels;

/* BN5_PRESTEP_TAKE and BN5_PRESTEP_TAKE_MIN: the hit's statuses cleared (`mask`), its damage less its parts left, the
 * parts cleared; `least` the second's: +C_EXTRA cleared, at least 1 left */
static void take(Asm *a, uint32_t mask, bool least, uint32_t clear_veneer, TakeLabels l) {
	const int lit = l.lit, loop = l.loop, one = l.one, out = l.out;
	if (mask <= 0xFF) half(a, MOVS(0, mask));
	else to_label(a, LDR_PC(0), lit, 'l');
	bl(a, clear_veneer);
	half(a, LDR_I(2, 5, COLL));
	half(a, MOVS(1, C_DAMAGE));
	half(a, LDRH_R(1, 2, 1));
	half(a, MOVS(3, C_PARTS));
	here(a, loop);
	half(a, LDRH_R(0, 2, 3));
	half(a, SUBS_R(1, 1, 0));
	half(a, MOVS(0, 0));
	half(a, STRH_R(0, 2, 3));
	half(a, ADDS_I(3, 2));
	half(a, CMP_I(3, C_PARTS_END));
	to_label(a, BCC(NE), loop, 'c');
	if (!least) {
		half(a, STR_R(0, 2, 3));
		half(a, MOVS(3, C_DAMAGE));
		half(a, STRH_R(1, 2, 3));
		return;
	}
	half(a, MOVS(3, C_EXTRA));
	half(a, STRH_R(0, 2, 3));
	half(a, CMP_I(1, 0));
	to_label(a, BCC(EQ), one, 'c');
	to_label(a, BCC(MI), one, 'c');
	half(a, MOVS(3, C_DAMAGE));
	half(a, STRH_R(1, 2, 3));
	to_label(a, B_, out, 'b');
	here(a, one);
	half(a, LDR_I(0, 5, COLL));
	half(a, MOVS(1, C_DAMAGE));
	half(a, MOVS(2, 1));
	half(a, STRH_R(2, 0, 1));
	half(a, MOVS(1, C_PARTS));
	half(a, STRH_R(2, 0, 1));
}

int xvirus_shim_prestep(uint8_t *out, int max, uint32_t at, uint32_t which, uint32_t clear_veneer, uint32_t sound_veneer) {
	enum { DONE, TAKE, LOOP, ONE, SET, MASK, STEP };
	Asm a = { out, max, 0, at, (at & 3) != 0, { 0 }, { { 0 } }, 0 };
	if (which == BN5_PRESTEP_APPLEY) {
		/* (while its +0x60 and its collision data's held count are 0: a sound where a hit is pending, the hit taken) */
		half(&a, LDR_I(0, 5, 0x60));
		half(&a, CMP_I(0, 0));
		to_label(&a, BCC(NE), DONE, 'c');
		half(&a, LDR_I(0, 5, COLL));
		half(&a, MOVS(1, C_HELD));
		half(&a, LDRH_R(1, 0, 1));
		half(&a, CMP_I(1, 0));
		to_label(&a, BCC(NE), DONE, 'c');
		half(&a, MOVS(1, C_DAMAGE));
		half(&a, LDRH_R(1, 0, 1));
		half(&a, CMP_I(1, 0));
		to_label(&a, BCC(EQ), TAKE, 'c');
		half(&a, MOVS(0, 0x6E));
		bl(&a, sound_veneer);
		here(&a, TAKE);
		take(&a, 0xFE, false, clear_veneer, (TakeLabels){ MASK, LOOP, ONE, DONE });
	} else if (which == BN5_PRESTEP_DOMINERD) {
		/* (while its +0x68 is 0 and a hit is pending: the hit taken unless its flags hold 2; its +0x64 set) */
		half(&a, LDR_I(0, 5, 0x68));
		half(&a, CMP_I(0, 0));
		to_label(&a, BCC(NE), DONE, 'c');
		half(&a, LDR_I(0, 5, COLL));
		half(&a, MOVS(1, C_DAMAGE));
		half(&a, LDRH_R(1, 0, 1));
		half(&a, CMP_I(1, 0));
		to_label(&a, BCC(EQ), DONE, 'c');
		half(&a, LDR_I(0, 0, C_FLAGS));
		half(&a, MOVS(1, 2));
		half(&a, TST(0, 1));
		to_label(&a, BCC(NE), SET, 'c');
		take(&a, 0x1FE, true, clear_veneer, (TakeLabels){ MASK, LOOP, ONE, SET });
		here(&a, SET);
		half(&a, MOVS(0, 1));
		half(&a, STR_I(0, 5, 0x64));
	} else return 0;
	here(&a, DONE);
	to_label(&a, LDR_PC(0), STEP, 'l');
	half(&a, BX(0));
	word(&a, STEP, BN6_VIRUS_STATE_STEP | 1);
	word(&a, MASK, 0x1FE);
	return done(&a) ? a.n : 0;
}
