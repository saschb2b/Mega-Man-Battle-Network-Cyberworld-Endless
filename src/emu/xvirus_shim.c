/* xvirus_shim.h: the shims' Thumb, encoded field by field (ARMv4T), each
 * branch and literal load resolved within the shim. */
#include "xvirus_shim.h"

#include "bn4.h"
#include "bn4_battle.h"
#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "thumb_reloc.h"

#define LABELS 16
#define FIXES  24

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
#define PUSH_LR             ((uint16_t)0xB500)
#define POP_PC              ((uint16_t)0xBD00)
#define MOV_LR(rm)          ((uint16_t)(0x4686 | (rm) << 3))   /* mov lr, rm */
#define BX_LR               ((uint16_t)0x4770)
#define LDRB_I(rd, rn, off) ((uint16_t)(0x7800 | (off) << 6 | (rn) << 3 | (rd)))
#define EORS(rd, rm)        ((uint16_t)(0x4040 | (rm) << 3 | (rd)))
#define NOP                 ((uint16_t)0x46C0)   /* mov r8, r8 */
enum { EQ = 0, NE = 1, MI = 4, LT = 0xB };

/* ---- a NameID range ---- */

bool xvirus_shim_range(uint8_t out[XV_SHIM_RANGE_SIZE], uint32_t at, uint16_t lo, uint16_t hi, uint32_t veneer, int side) {
	enum { LO, HI, ON };
	Asm a = { out, XV_SHIM_RANGE_SIZE, 0, at, (at & 3) != 0 || side > 0xFF, { 0 }, { { 0 } }, 0 };
	to_label(&a, LDR_PC(1), LO, 'l');
	to_label(&a, LDR_PC(2), HI, 'l');
	if (side >= 0) half(&a, (uint16_t)(0x2300 | side));   /* movs r3, #side */
	if (!a.bad) {
		int32_t off = (int32_t)(veneer - (at + 4 + (side >= 0 ? 6 : 4)));
		if (off < -2048 || off > 2046 || (veneer & 3)) a.bad = true;
		else half(&a, (uint16_t)(B_ | ((off >> 1) & 0x7FF)));
	}
	word(&a, LO, lo);
	word(&a, HI, hi);
	(void)ON;
	return done(&a) && a.n == XV_SHIM_RANGE_SIZE;
}

/* ---- a sound past a byte, or a queue's number ---- */

uint32_t xvirus_shim_r0_to(uint32_t was) {
	switch (was) {
	case BN4_PLAY_SOUND: return BN6_PLAY_SOUND;
	case BN4_QUEUE_HEAD: return BN6_QUEUE_HEAD;
	case BN4_QUEUE_ENTER: return BN6_QUEUE_ENTER;
	case BN4_QUEUE_LEAVE: return BN6_QUEUE_LEAVE;
	default: return 0;
	}
}

bool xvirus_shim_sound(uint8_t out[XV_SHIM_SOUND_SIZE], uint32_t at, uint16_t slot, uint32_t veneer) {
	Asm a = { out, XV_SHIM_SOUND_SIZE, 0, at, (at & 1) != 0, { 0 }, { { 0 } }, 0 };
	half(&a, MOVS(0, slot >> 8));
	half(&a, (uint16_t)(8 << 6));   /* lsls r0, r0, #8 */
	half(&a, ADDS_I(0, slot & 0xFF));
	int32_t off = (int32_t)(veneer - (at + 6 + 4));
	if (off < -2048 || off > 2046 || (veneer & 1)) a.bad = true;
	else half(&a, (uint16_t)(B_ | ((off >> 1) & 0x7FF)));
	return done(&a) && a.n == XV_SHIM_SOUND_SIZE;
}

/* ---- an effect's id no immediate names ---- */

uint32_t xvirus_shim_r4_to(uint32_t was) { return was == BN4_SPAWN_EFFECT ? BN6_SPAWN_EFFECT : 0; }

int xvirus_shim_effect_size(int n) { return n < 1 || n > 4 ? 0 : (2 * (3 * n + 1) + 3) & ~3; }

int xvirus_shim_effect(uint8_t out[XV_SHIM_EFFECT_MAX], uint32_t at, const uint8_t pairs[][2], int n, uint32_t veneer) {
	int size = xvirus_shim_effect_size(n);
	if (!size) return 0;
	Asm a = { out, size, 0, at, (at & 1) != 0, { 0 }, { { 0 } }, 0 };
	for (int i = 0; i < n; ++i) {
		for (int k = 0; k < n; ++k) a.bad = a.bad || pairs[k][0] == pairs[i][1];   /* (an id made is never one looked for) */
		half(&a, (uint16_t)(0x2C00 | pairs[i][0]));   /* cmp r4, #from */
		half(&a, 0xD100);                             /* bne past the movs */
		half(&a, MOVS(4, pairs[i][1]));
	}
	int32_t off = (int32_t)(veneer - (at + 6u * (uint32_t)n + 4));
	if (off < -2048 || off > 2046 || (veneer & 1)) a.bad = true;
	else half(&a, (uint16_t)(B_ | ((off >> 1) & 0x7FF)));
	while (!a.bad && a.n < size) half(&a, NOP);
	return done(&a) && a.n == size ? size : 0;
}

/* ---- a hook entered past its routine's push ---- */

bool xvirus_shim_entry(uint8_t out[XV_SHIM_ENTRY_SIZE], uint32_t at, uint32_t to) {
	Asm a = { out, XV_SHIM_ENTRY_SIZE, 0, at, (at & 3) != 0 || !(to & 1), { 0 }, { { 0 } }, 0 };
	half(&a, PUSH_LR);
	bl(&a, to & ~1u);
	half(&a, POP_PC);   /* (where the tail returns by bx lr: its pop {pc} returns from the push itself) */
	return done(&a) && a.n == XV_SHIM_ENTRY_SIZE;
}

/* ---- a direction numbered by the side ---- */

bool xvirus_shim_side(uint8_t out[XV_SHIM_SIDE_SIZE], uint32_t at, uint32_t rule) {
	enum { RET };
	Asm a = { out, XV_SHIM_SIDE_SIZE, 0, at, (at & 3) != 0 || !(rule & 1), { 0 }, { { 0 } }, 0 };
	half(&a, PUSH_LR);
	bl(&a, rule & ~1u);
	half(&a, CMP_I(0, 3));
	to_label(&a, BCC(LT), RET, 'c');      /* (0-2, none, up and down: as they are) */
	half(&a, LDRB_I(1, 5, 0x16));         /* its side (BN6's rule's r4, 0x0801A05C) */
	half(&a, CMP_I(1, 0));
	to_label(&a, BCC(EQ), RET, 'c');
	half(&a, MOVS(1, 7));
	half(&a, EORS(0, 1));                 /* (3 and 4 swapped) */
	here(&a, RET);
	half(&a, POP_PC);
	half(&a, NOP);                        /* (a word's end) */
	return done(&a) && a.n == XV_SHIM_SIDE_SIZE;
}

/* ---- its game's after-state while BN6's battle is paused ---- */

bool xvirus_shim_paused(uint8_t out[XV_SHIM_PAUSED_SIZE], uint32_t at, uint32_t own, uint32_t mode, uint32_t paused, uint32_t present) {
	enum { OUT };
	Asm a = { out, XV_SHIM_PAUSED_SIZE, 0, at, (at & 3) != 0 || (own && !(own & 1)) || !(present & 1), { 0 }, { { 0 } }, 0 };
	half(&a, PUSH_LR);
	if (own) bl(&a, own & ~1u);
	else { half(&a, NOP); half(&a, NOP); }
	bl(&a, mode & ~1u);
	half(&a, CMP_I(0, 1));
	to_label(&a, BCC(EQ), OUT, 'c');   /* (mode 1: BN6's own after-state runs, paused or not) */
	bl(&a, paused & ~1u);
	to_label(&a, BCC(EQ), OUT, 'c');   /* (not paused: BN6's own runs) */
	bl(&a, present & ~1u);
	here(&a, OUT);
	half(&a, POP_PC);
	half(&a, NOP);
	return done(&a) && a.n == XV_SHIM_PAUSED_SIZE;
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

/* r1 its collision data's u16 at `off` (r0 the collision data), compared with 0 */
static void coll_zero(Asm *a, int off) {
	half(a, MOVS(1, off));
	half(a, LDRH_R(1, 0, 1));
	half(a, CMP_I(1, 0));
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
		coll_zero(&a, C_HELD);
		to_label(&a, BCC(NE), DONE, 'c');
		coll_zero(&a, C_DAMAGE);
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
		coll_zero(&a, C_DAMAGE);
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

/* ---- a summon (a BN4 Navi's: BN4_SPAWN_ENEMY's place) ---- */

/* a call of `label`'s word (Thumb) past a bl's reach through `reg`, lr made in `link` (with its Thumb bit, as a bl gives) */
static void far(Asm *a, int reg, int link, int label) {
	to_label(a, LDR_PC(reg), label, 'l');
	half(a, (uint16_t)(0x4678 | link));        /* mov link, pc */
	half(a, ADDS_I(link, 5));
	half(a, MOV_LR(link));
	half(a, BX(reg));
}

int xvirus_shim_summon(uint8_t *out, int max, uint32_t at, const XvSummonShim *s) {
	enum { M0, M1, M2, M3, RNG, SPAWN, SOUND, IDS, L1, L2, LOOP, NONE, END, SLOT, INIT };
	Asm a = { out, max, 0, at, (at & 3) != 0 || s->nids < 0, { 0 }, { { 0 } }, 0 };
	half(&a, 0xB590);                            /* push {r4, r7, lr} */
	half(&a, 0xB084);                            /* sub sp, #16 */
	half(&a, 0x9001);                            /* str r0, [sp, #4]: its game's id */
	half(&a, LDRB_I(0, 5, 0x16));                /* its side */
	to_label(&a, LDR_PC(2), M0, 'l');            /* the masks wanted and refused, by side */
	to_label(&a, LDR_PC(3), M1, 'l');
	half(&a, CMP_I(0, 0));
	to_label(&a, BCC(EQ), L1, 'c');
	to_label(&a, LDR_PC(2), M2, 'l');
	to_label(&a, LDR_PC(3), M3, 'l');
	here(&a, L1);
	half(&a, 0xAF00);                            /* add r7, sp, #0: the panels' buffer */
	bl(&a, s->panels & ~1u);
	half(&a, 0x1C07);                            /* adds r7, r0, #0: their count */
	to_label(&a, BCC(NE), L2, 'c');
	half(&a, MOVS(0, 0));
	to_label(&a, B_, END, 'b');
	here(&a, L2);
	far(&a, 0, 1, RNG);                          /* a draw */
	if (a.n != XV_SHIM_SUMMON_DRAW) a.bad = true;   /* (its return where the differential keys it) */
	half(&a, 0x1C39);                            /* adds r1, r7, #0 */
	half(&a, 0xDF06);                            /* swi 6: r1 the draw modulo the count */
	half(&a, 0xA800);                            /* add r0, sp, #0 */
	half(&a, 0x5C40);                            /* ldrb r0, [r0, r1]: column | row << 4 */
	half(&a, 0x0901);                            /* lsrs r1, r0, #4 */
	half(&a, 0x0740);                            /* lsls r0, r0, #29 */
	half(&a, 0x0F40);                            /* lsrs r0, r0, #29 */
	half(&a, 0x9002);                            /* str r0, [sp, #8] */
	half(&a, 0x9103);                            /* str r1, [sp, #12] */
	half(&a, 0x9801);                            /* ldr r0, [sp, #4] */
	to_label(&a, 0xA300, IDS, 'l');              /* add r3, pc, #ids: its game's id | BN6's << 16, 0 ending */
	here(&a, LOOP);
	half(&a, 0xCB04);                            /* ldmia r3!, {r2} */
	half(&a, CMP_I(2, 0));
	to_label(&a, BCC(EQ), NONE, 'c');
	half(&a, 0x0411);                            /* lsls r1, r2, #16 */
	half(&a, 0x0C09);                            /* lsrs r1, r1, #16 */
	half(&a, CMP_R(1, 0));
	to_label(&a, BCC(NE), LOOP, 'c');
	half(&a, 0x0C10);                            /* lsrs r0, r2, #16: BN6's id (0: none) */
	to_label(&a, BCC(EQ), NONE, 'c');
	half(&a, 0x9902);                            /* ldr r1, [sp, #8] */
	half(&a, 0x9A03);                            /* ldr r2, [sp, #12] */
	half(&a, LDRB_I(3, 5, 0x16));
	far(&a, 4, 7, SPAWN);
	half(&a, 0x1C04);                            /* adds r4, r0, #0 */
	to_label(&a, BCC(EQ), END, 'c');
	half(&a, MOVS(0, 2));
	half(&a, (uint16_t)(0x7000 | 5 << 6 | 4 << 3));   /* strb r0, [r4, #5]: BN4's summon's mark */
	half(&a, 0xB420);                            /* push {r5} */
	half(&a, 0x1C25);                            /* adds r5, r4, #0 */
	far(&a, 3, 7, INIT);                         /* its init now: its first state in its turn this frame */
	half(&a, 0xBC20);                            /* pop {r5} */
	if (s->sound <= 0xFF) half(&a, MOVS(0, s->sound));
	else to_label(&a, LDR_PC(0), SLOT, 'l');
	far(&a, 3, 7, SOUND);
	half(&a, 0x1C20);                            /* adds r0, r4, #0 */
	to_label(&a, B_, END, 'b');
	here(&a, NONE);
	half(&a, MOVS(0, 0));
	here(&a, END);
	half(&a, 0xB004);                            /* add sp, #16 */
	half(&a, 0xBD90);                            /* pop {r4, r7, pc} */
	word(&a, M0, s->masks[0]);
	word(&a, M1, s->masks[1]);
	word(&a, M2, s->masks[2]);
	word(&a, M3, s->masks[3]);
	word(&a, RNG, s->rng | 1u);
	word(&a, SPAWN, s->spawn | 1u);
	word(&a, SOUND, s->play | 1u);
	word(&a, SLOT, s->sound);
	word(&a, INIT, s->init | 1u);
	word(&a, IDS, s->nids ? s->ids[0] : 0);
	for (int k = 1; k <= s->nids; ++k) {
		const uint32_t v = k < s->nids ? s->ids[k] : 0;
		half(&a, (uint16_t)v);
		half(&a, (uint16_t)(v >> 16));
	}
	return done(&a) ? a.n : 0;
}
