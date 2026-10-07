/* Thumb code moved (issue #82): a routine copied whole from one address to
 * another, its calls out aimed again from where it now stands, the
 * immediates a recipe changes checked against the instruction they sit in
 * before they are written, and veneers for the calls a BL no longer
 * reaches. The GBA's CPU is an ARM7TDMI (ARMv4T): a BL reaches 4 MiB either
 * way, and a pop into pc stays in Thumb.
 *
 * Pure functions on instruction words and byte buffers: what they make is
 * written into the core's copy of the ROM by the one who called them, and
 * nothing of the game's is here, only the encoding. Each says no rather
 * than guess: a form that is not the one expected, an immediate that does
 * not fit, a call out of reach. */
#ifndef CW_THUMB_RELOC_H
#define CW_THUMB_RELOC_H

#include <stdbool.h>
#include <stdint.h>

/* A BL's reach from its address + 4: back 0x400000 bytes, on 0x3FFFFE (a
 * signed 22-bit count of halfwords, in the two halves' 11 bits each). */
#define THUMB_BL_REACH 0x400000

/* ---- BL: hi (0xF000-0xF7FF, the offset's upper bits) then lo (0xF800-0xFFFF) ---- */

bool thumb_is_bl(uint16_t hi, uint16_t lo);
/* where the BL at bus address `at` with these halves calls; false where they
 * are no BL */
bool thumb_bl_decode(uint32_t at, uint16_t hi, uint16_t lo, uint32_t *target);
/* the halves of a BL at `at` calling `target`; false, the halves left as they
 * were, where either is odd or the target out of reach */
bool thumb_bl_encode(uint32_t at, uint32_t target, uint16_t *hi, uint16_t *lo);

/* A BL in a block's bytes: code[off] is the byte at bus address base + off. */
bool thumb_bl_read(const uint8_t *code, uint32_t off, uint32_t base, uint32_t *target);
bool thumb_bl_write(uint8_t *code, uint32_t off, uint32_t base, uint32_t target);

/* The BLs of a run of Thumb code, walked an instruction at a time from its
 * first: the offset of each in offs (the first max of them), and how many
 * there are; -1 at half a BL (one half without the other), which no code
 * the walk understands holds. The run is code alone: its literal pools and
 * jump tables are left out by the caller, as data may look like a BL. */
int thumb_bl_scan(const uint8_t *code, uint32_t size, uint32_t *offs, int max);

/* ---- a block moved ---- */

/* A block of Thumb code copied whole, its bytes as they were: from bus
 * address `from` to `to`, the same distance from a word boundary, so that
 * its loads from its own literal pool and its calls within itself hold. */
typedef struct {
	uint8_t *code;
	uint32_t size;
	uint32_t from, to;
} ThumbBlock;

/* Where a call out of a moved block goes now: the new target for the old
 * one (even, a routine's address), or 0 where there is none. */
typedef uint32_t (*ThumbResolve)(uint32_t target, void *user);

/* The block's BLs at offs (thumb_bl_scan's) aimed again from its new
 * address: a call within it is left alone, a call out goes where resolve
 * says. All or nothing: false, the code left as it was, where an offset
 * holds no BL, resolve has no answer, a new target is out of reach, or the
 * block moved off its alignment. resolve is asked once per call out. */
bool thumb_block_rebase(const ThumbBlock *b, const uint32_t *offs, int n, ThumbResolve resolve, void *user);

/* ---- immediates in their instruction ---- */

typedef enum {
	THUMB_MOVS,     /* movs rd,#imm         0-255 */
	THUMB_CMP,      /* cmp rd,#imm          0-255 */
	THUMB_ADDS,     /* adds rd,#imm         0-255 */
	THUMB_SUBS,     /* subs rd,#imm         0-255 */
	THUMB_LDR,      /* ldr rd,[rn,#imm]     0-124, a multiple of 4 */
	THUMB_STR,      /* str rd,[rn,#imm]     the same */
	THUMB_LDRB,     /* ldrb rd,[rn,#imm]    0-31 */
	THUMB_STRB,     /* strb rd,[rn,#imm]    0-31 */
	THUMB_LDRH,     /* ldrh rd,[rn,#imm]    0-62, even */
	THUMB_STRH,     /* strh rd,[rn,#imm]    the same */
	THUMB_LDR_PC,   /* ldr rd,[pc,#imm]     0-1020, a multiple of 4 */
	THUMB_FORMS
} ThumbForm;

/* One instruction of those forms: its low registers (rn 0 where the form
 * has none) and its immediate, in bytes for the loads and stores. */
typedef struct {
	ThumbForm form;
	uint8_t rd, rn;
	uint16_t imm;
} ThumbImm;

/* insn read as `form`; false where it is another */
bool thumb_imm_decode(uint16_t insn, ThumbForm form, ThumbImm *out);
/* the instruction; false where a register or the immediate does not fit */
bool thumb_imm_encode(const ThumbImm *in, uint16_t *insn);
/* *insn, which must be `was` exactly (form, registers, immediate), given the
 * immediate imm instead; false, *insn untouched, otherwise or where imm
 * does not fit */
bool thumb_imm_patch(uint16_t *insn, const ThumbImm *was, uint16_t imm);

/* ---- veneers ---- */

/* A call out of reach, through 12 bytes at a word-aligned address within
 * reach: push {r0,r1}; ldr r0,[pc,#4]; str r0,[sp,#4]; pop {r0,pc}; the
 * target's address with its Thumb bit. r0-r12, sp, lr and the flags reach
 * the target as the BL left them, so it returns straight to its caller. */
#define THUMB_VENEER_SIZE 12

/* the veneer at bus address `at` on to the Thumb routine at `target` (its
 * Thumb bit set or not); false where `at` is not word-aligned */
bool thumb_veneer(uint8_t out[THUMB_VENEER_SIZE], uint32_t at, uint32_t target);
/* the routine a veneer's bytes go on to (its address, even); false where
 * they are no veneer */
bool thumb_veneer_target(const uint8_t v[THUMB_VENEER_SIZE], uint32_t *target);

/* Veneers side by side, one per target. */
typedef struct {
	uint8_t *bytes;   /* room for cap veneers */
	uint32_t base;    /* the first's bus address, word-aligned */
	int cap, n;       /* veneers there is room for, and made */
} ThumbVeneers;

/* the bus address of the pool's veneer on to target, made after the others
 * where there is none yet; 0 where the pool is full or misaligned */
uint32_t thumb_veneer_get(ThumbVeneers *pool, uint32_t target);

#endif
