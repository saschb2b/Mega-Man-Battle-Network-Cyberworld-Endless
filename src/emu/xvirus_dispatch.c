/* xvirus_dispatch.h: the second-level dispatch's numbers and its code,
 * made here as the project's own Thumb, each instruction encoded from its
 * fields. Nothing here reads a ROM or the core: xvirus_grow.c writes the
 * code and the tables, xvirus.c a family's entries. */
#include "xvirus_dispatch.h"

#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "rom.h"

static const int KINDS[XV_DISPATCH_KINDS] = { 1, 3, 4 };
static const int TYPES[XV_DISPATCH_KINDS] = { XV_T1_DISPATCH, XV_T3_DISPATCH, XV_T4_DISPATCH };

/* the dispatch's spawners, in their order at XV_SPAWN_1 on: each its kind and BN6's spawner, and BN5's that go to it */
static const struct { int kind; uint32_t bn6, bn5[2]; } SPAWNERS[XV_SPAWNERS] = {
	{ 1, BN6_SPAWN_1, { BN5_SPAWN_1, 0 } },
	{ 1, BN6_SPAWN_1_LAST, { BN5_SPAWN_1_LAST, BN5_SPAWN_1_LAST2 } },
	{ 3, BN6_SPAWN_3, { BN5_SPAWN_3, 0 } },
	{ 3, BN6_SPAWN_3_LAST, { BN5_SPAWN_3_LAST, 0 } },
	{ 3, BN6_SPAWN_3_FIRST, { BN5_SPAWN_3_FIRST, 0 } },
	{ 4, BN6_SPAWN_4, { BN5_SPAWN_4, 0 } },
};

int xvirus_dispatch_slot(int kind) {
	for (int k = 0; k < XV_DISPATCH_KINDS; ++k)
		if (KINDS[k] == kind) return k;
	return -1;
}

int xvirus_dispatch_type(int kind) {
	int k = xvirus_dispatch_slot(kind);
	return k < 0 ? -1 : TYPES[k];
}

uint32_t xvirus_dispatch_moved_at(int kind, int n) {
	int k = xvirus_dispatch_slot(kind);
	return k < 0 || n < 0 || n >= XV_TYPES_MAX ? 0 : XV_MOVED_TABLES + (uint32_t)k * XV_STEP + 4u * (uint32_t)n;
}

uint32_t xvirus_dispatch_twin_at(int kind, int n) {
	int k = xvirus_dispatch_slot(kind);
	return k < 0 || n < 0 || n >= XV_TYPES_MAX ? 0 : XV_TWIN_TABLES + 0x100u * (uint32_t)k + (uint32_t)n;
}

uint32_t xvirus_dispatch_side(int kind) {
	int k = xvirus_dispatch_slot(kind);
	return k < 0 ? 0 : BN6_DISPATCH_SIDE + BN6_DISPATCH_SIDE_STEP * (uint32_t)k;
}

uint32_t xvirus_dispatch_routine(int kind) {
	int k = xvirus_dispatch_slot(kind);
	return k < 0 ? 0 : XV_STUB_DISPATCH + XV_STUB_DISPATCH_STEP * (uint32_t)k;
}

uint32_t xvirus_dispatch_spawner(int game, uint32_t from) {
	if (game != XROM_BN5_COLONEL_US) return 0;
	for (int i = 0; i < XV_SPAWNERS; ++i)
		if (from == SPAWNERS[i].bn5[0] || (SPAWNERS[i].bn5[1] && from == SPAWNERS[i].bn5[1])) return XV_SPAWN_1 + XV_SPAWN_STEP * (uint32_t)i;
	return 0;
}

bool xvirus_dispatch_is_spawner(uint32_t to, int *kind, uint32_t *bn6) {
	uint32_t at = to & ~1u;
	if (at < XV_SPAWN_1 || (at - XV_SPAWN_1) % XV_SPAWN_STEP || (at - XV_SPAWN_1) / XV_SPAWN_STEP >= XV_SPAWNERS) return false;
	int i = (int)((at - XV_SPAWN_1) / XV_SPAWN_STEP);
	if (kind) *kind = SPAWNERS[i].kind;
	if (bn6) *bn6 = SPAWNERS[i].bn6;
	return true;
}

/* ---- the code: Thumb encoded from its fields ---- */

typedef struct {
	uint8_t *out;   /* the stubs' bytes, XV_STUBS at out[0] */
	uint32_t at;    /* the next instruction's offset */
} Asm;

static void op(Asm *a, uint32_t v) {
	put16(a->out + a->at, v);
	a->at += 2;
}

static void word(uint8_t *out, uint32_t off, uint32_t v) { put32(out + off, v); }

/* ldr rd,[pc,#..]: the word at offset `lit` (word-aligned, after the instruction) */
static void ldr_pc(Asm *a, int rd, uint32_t lit) {
	uint32_t pc = (XV_STUBS + a->at + 4) & ~3u;
	op(a, 0x4800u | (uint32_t)rd << 8 | ((XV_STUBS + lit - pc) / 4 & 0xFF));
}

static uint32_t ldrb_imm(int rd, int rn, int imm) { return 0x7800u | (uint32_t)imm << 6 | (uint32_t)rn << 3 | (uint32_t)rd; }
static uint32_t ldrb_reg(int rd, int rn, int rm) { return 0x5C00u | (uint32_t)rm << 6 | (uint32_t)rn << 3 | (uint32_t)rd; }
static uint32_t ldr_reg(int rd, int rn, int rm) { return 0x5800u | (uint32_t)rm << 6 | (uint32_t)rn << 3 | (uint32_t)rd; }
static uint32_t lsls(int rd, int rm, int imm) { return 0x0000u | (uint32_t)imm << 6 | (uint32_t)rm << 3 | (uint32_t)rd; }
static uint32_t lsrs(int rd, int rm, int imm) { return 0x0800u | (uint32_t)imm << 6 | (uint32_t)rm << 3 | (uint32_t)rd; }
static uint32_t bx(int rm) { return 0x4700u | (uint32_t)rm << 3; }
/* b<cond> from offset `from` to offset `to` */
static uint32_t b_cond(int cond, uint32_t from, uint32_t to) { return 0xD000u | (uint32_t)cond << 8 | ((to - from - 4) / 2 & 0xFF); }
#define NE 1
#define EQ 0

#define PUSH_R0_LR  0xB501u   /* push {r0,lr} */
#define PUSH_R0_R1  0xB403u   /* push {r0,r1} */
#define POP_R0_R1   0xBC03u   /* pop {r0,r1} */
#define POP_PC      0xBD00u   /* pop {pc} */
#define ADD_SP_4    0xB001u   /* add sp,#4 */
#define MOV_LR_PC   0x46FEu   /* mov lr,pc: lr the instruction after the next (pop {pc} returns there in Thumb, as ARMv4T's does) */
#define NOP         0x46C0u   /* mov r8,r8 */

/* XV_STUB_NONE: a MOVED entry no family set, for an object of a dispatch
 * type spawned as none of the moved: let go as it first runs (a tail call:
 * object_freeMemory returns to the runner) */
static void none(uint8_t *out) {
	Asm a = { out, XV_STUB_NONE - XV_STUBS };
	ldr_pc(&a, 0, a.at + 4);
	op(&a, bx(0));
	word(out, a.at, BN6_OBJECT_FREE | 1);
}

/* the dispatch type's routine (r5 the object, lr the runner's way back):
 *   ldrb r0,[r5,#3]   its slot
 *   ldr  r1,=side     the kind's side table
 *   ldrb r1,[r1,r0]   its game's type
 *   lsls r1,r1,#2
 *   ldr  r0,=moved    the kind's MOVED
 *   ldr  r0,[r0,r1]   the routine
 *   bx   r0           on to it, r1 its type times four as its game's runner leaves it */
static void dispatch(uint8_t *out, int k) {
	uint32_t base = XV_STUB_DISPATCH - XV_STUBS + XV_STUB_DISPATCH_STEP * (uint32_t)k, lit = base + 0x10;
	Asm a = { out, base };
	op(&a, ldrb_imm(0, 5, BN6_T1_SLOT));
	ldr_pc(&a, 1, lit);
	op(&a, ldrb_reg(1, 1, 0));
	op(&a, lsls(1, 1, 2));
	ldr_pc(&a, 0, lit + 4);
	op(&a, ldr_reg(0, 0, 1));
	op(&a, bx(0));
	op(&a, NOP);
	word(out, lit, xvirus_dispatch_side(KINDS[k]));
	word(out, lit + 4, XV_MOVED_TABLES + XV_STEP * (uint32_t)k);
}

/* a spawner (r0 its game's type, r1-r4 the object's words, called by BL
 * from the moved code; back r5 the object or 0, r0-r4 as BN6's spawner left
 * them):
 *   push {r0,lr}
 *   ldr  r5,=twin      (r5 free: the spawn sets it)
 *   lsls r0,r0,#24; lsrs r0,r0,#24   its low byte, as the spawn stores it
 *   ldrb r0,[r5,r0]    BN6's twin, or XV_TYPE_MOVED
 *   cmp  r0,#0xFF; bne 1f
 *   movs r0,#dispatch  the kind's dispatch type
 * 1:ldr  r5,=spawner; mov lr,pc; bx r5   BN6's (its frame over ours: our r0 and lr at its five words + 0x1C and + 0x20)
 *   cmp  r5,#0; beq 2f
 *   push {r0,r1}; ldrb r1,[r5,#3]; ldr r0,=side; adds r1,r1,r0
 *   ldr  r0,[sp,#8]    its game's type; strb r0,[r1]   the side table by its slot
 *   pop  {r0,r1}
 * 2:add  sp,#4; pop {pc} */
static void spawner(uint8_t *out, int i) {
	int kind = SPAWNERS[i].kind, k = xvirus_dispatch_slot(kind);
	uint32_t base = XV_SPAWN_1 - XV_STUBS + XV_SPAWN_STEP * (uint32_t)i, lit = base + 0x2C;
	Asm a = { out, base };
	op(&a, PUSH_R0_LR);
	ldr_pc(&a, 5, lit);
	op(&a, lsls(0, 0, 24));
	op(&a, lsrs(0, 0, 24));
	op(&a, ldrb_reg(0, 5, 0));
	op(&a, 0x2800u | XV_TYPE_MOVED);
	uint32_t bne = a.at;
	op(&a, 0);
	op(&a, 0x2000u | (uint32_t)TYPES[k]);
	put16(out + bne, b_cond(NE, bne, a.at));
	ldr_pc(&a, 5, lit + 4);
	op(&a, MOV_LR_PC);
	op(&a, bx(5));
	op(&a, 0x2D00u);   /* cmp r5,#0 */
	uint32_t beq = a.at;
	op(&a, 0);
	op(&a, PUSH_R0_R1);
	op(&a, ldrb_imm(1, 5, BN6_T1_SLOT));
	ldr_pc(&a, 0, lit + 8);
	op(&a, 0x1809u);   /* adds r1,r1,r0 */
	op(&a, 0x9802u);   /* ldr r0,[sp,#8] */
	op(&a, 0x7008u);   /* strb r0,[r1] */
	op(&a, POP_R0_R1);
	put16(out + beq, b_cond(EQ, beq, a.at));
	op(&a, ADD_SP_4);
	op(&a, POP_PC);
	word(out, lit, XV_TWIN_TABLES + 0x100u * (uint32_t)k);
	word(out, lit + 4, SPAWNERS[i].bn6 | 1);
	word(out, lit + 8, xvirus_dispatch_side(kind));
}

/* XV_SUPPORT (r5 the virus, r7 its attack's variables): BN5's support family (0x080EC400) ends a virus's support chip in the
 * frame its routine runs, BN6's (0x080ED0DC) a frame later (phase 1): BN6's run twice in the one frame, +1 back to 0 as BN5's
 * leaves it (BN6's family itself as it is, which MegaMan's own support chips run):
 *   push {lr}; ldr r3,=family; mov lr,pc; bx r3; ldr r3,=family; mov lr,pc; bx r3; movs r0,#0; strb r0,[r7,#1]; pop {pc} */
static void support(uint8_t *out) {
	uint32_t base = XV_SUPPORT - XV_STUBS, lit = base + 0x14;
	Asm a = { out, base };
	op(&a, 0xB500u);   /* push {lr} */
	for (int k = 0; k < 2; ++k) {
		ldr_pc(&a, 3, lit);
		op(&a, MOV_LR_PC);
		op(&a, bx(3));
	}
	op(&a, 0x2000u);   /* movs r0,#0 */
	op(&a, 0x7078u);   /* strb r0,[r7,#1] */
	op(&a, POP_PC);
	word(out, lit, BN6_SUPPORT_RUN | 1);
}

/* XV_CHIP_WRITER (BN5's 0x0800FE6A from a moved family's code): BN6's (0x080126E4), a support chip's family (0x15) BN5's
 * timing's (XV_SUPPORT_FAMILY), for setAttack0 after it:
 *   push {lr}; ldr r3,=writer; mov lr,pc; bx r3; cmp r0,#0x15; bne 1f; movs r0,#XV_SUPPORT_FAMILY; 1: pop {pc} */
static void chip_writer(uint8_t *out) {
	uint32_t base = XV_CHIP_WRITER - XV_STUBS, lit = base + 0x10;
	Asm a = { out, base };
	op(&a, 0xB500u);
	ldr_pc(&a, 3, lit);
	op(&a, MOV_LR_PC);
	op(&a, bx(3));
	op(&a, 0x2800u | BN6_CHIP_SUPPORT_FAMILY);   /* cmp r0,#0x15 */
	uint32_t bne = a.at;
	op(&a, 0);
	op(&a, 0x2000u | XV_SUPPORT_FAMILY);
	put16(out + bne, b_cond(NE, bne, a.at));
	op(&a, POP_PC);
	word(out, lit, BN6_CHIP_WRITER | 1);
}

void xvirus_dispatch_code(uint8_t out[XV_STUBS_SIZE]) {
	memset(out, 0, XV_STUBS_SIZE);
	none(out);
	for (int k = 0; k < XV_DISPATCH_KINDS; ++k) dispatch(out, k);
	for (int i = 0; i < XV_SPAWNERS; ++i) spawner(out, i);
	support(out);
	chip_writer(out);
}
