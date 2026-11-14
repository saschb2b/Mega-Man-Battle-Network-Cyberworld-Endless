#include "xchips_pack.h"

#include <string.h>

#include <mgba/core/core.h>
#include <mgba/internal/gba/gba.h>

#include "emu_gpsp.h"
#include "free_space.h"

/* The engine's own Thumb routines, at CHIPS_CODE (numbers: written for
 * this, each instruction encoded by hand and read back with a
 * disassembler; docs/engine/emulation/):
 * +0x00 getOffsetToQuantityOfChipCode's tail (r2 the chip, r3 its code's
 * place: the count's address; pop {r4,r7,pc}); +0x0A GetTotalChipCount's
 * (r0 the chip: its four counts summed; pop {r7,pc}); +0x1E the pack
 * totals' count (r2, r3: r1 the count, r0 kept; back by pop {pc}); +0x2C
 * the pack's clear (BN6's ZeroFillByWord on the pack, then on the
 * extension's 0x900 bytes, then the save RAM's entries a byte at a time,
 * after the nop the clear's hook stands on; pop {pc}); +0x4C the
 * pseudo-chips' test (r2 the chip: r1 1 for 0x170-0x19A, BN6's own
 * pseudo-chips, else 0; mov pc,lr); +0x5A a chip's pack entry (r0 the
 * chip: its 12 bytes' address, r1 and r7 its own; bn6_pack_entry: BN6's
 * pack below 320, the extension as if 320 on followed from 411, the ids
 * from 512 in the entries of 320 on, the ids from 635 in the save RAM, and
 * for any other the 12 zero bytes at +0xD8, in the ROM, where a write is
 * lost); +0x9A, +0xB2 and +0xC8 an entry's order stamp set, read and
 * zeroed as BN6's three routines do (r0 the count's address, r1 the code's
 * place; r0-r3 and the flags left as BN6's leave them), by two bytes for
 * BN6's halfword; from +0xE4 their literals (pool, written from
 * bn6_chips.h's numbers). As they read:
 *   00 movs r0,r2; bl 5a; adds r0,r0,r3; pop {r4,r7,pc}
 *   0a bl 5a; ldrb r1,[r0]; ldrb r7,[r0,#1]; adds r1,r1,r7; ldrb r7,[r0,#2]; adds r1,r1,r7;
 *      ldrb r7,[r0,#3]; adds r0,r1,r7; pop {r7,pc}
 *   1e push {r0,lr}; movs r0,r2; bl 5a; ldrb r1,[r0,r3]; pop {r0,pc}; nop
 *   2c bl 48; ldr r0,=ext; ldr r1,=0x900; bl 48; ldr r0,=save; ldr r1,=save's bytes; movs r2,#0
 *   3e nop
 *   40 subs r1,#1; strb r2,[r0,r1]; bne 40; pop {pc}
 *   48 ldr r3,=ZeroFillByWord; bx r3
 *   4c movs r1,#0; ldr r3,=0x170; subs r3,r2,r3; cmp r3,#0x2B; bcs 58; movs r1,#1
 *   58 mov pc,lr
 *   5a ldr r1,=320; cmp r0,r1; bge 66; mov r7,r10; ldr r7,[r7,#0x4C]; b 92
 *   66 ldr r7,=ext-320*12; ldr r1,=411; cmp r0,r1; blt 8e; ldr r1,=512; cmp r0,r1; blt 92;
 *      ldr r1,=603; cmp r0,r1; bge 7e; subs r0,#192; b 92
 *   7e ldr r1,=635; subs r0,r0,r1; blt 8e; ldr r1,=389; cmp r0,r1; bge 8e; ldr r7,=save; b 92
 *   8e adr r7,d8; movs r0,#0
 *   92 movs r1,#12; muls r0,r1; adds r0,r7,r0; bx lr
 *   9a subs r0,r0,r1; lsls r1,r1,#1; adds r0,r0,r1; ldr r2,=next; ldrh r3,[r2]; strb r3,[r0,#4];
 *      lsrs r2,r3,#8; strb r2,[r0,#5]; ldr r2,=next; subs r3,#1; strh r3,[r2]; mov pc,lr
 *   b2 ldr r2,=next; subs r0,r0,r1; lsls r1,r1,#1; adds r0,r0,r1; ldrb r3,[r0,#5]; ldrb r0,[r0,#4];
 *      lsls r3,r3,#8; orrs r0,r3; adds r3,r2,#0; ldrh r3,[r2]; mov pc,lr
 *   c8 subs r0,r0,r1; lsls r1,r1,#1; adds r0,r0,r1; movs r2,#0; strb r2,[r0,#4]; strb r2,[r0,#5];
 *      mov pc,lr; nop
 *   d8 .word 0, 0, 0 */
static const uint8_t code[0xE4] = {
	0x10, 0x00, 0x00, 0xf0, 0x2a, 0xf8, 0xc0, 0x18, 0x90, 0xbd, 0x00, 0xf0, 0x26, 0xf8, 0x01, 0x78, 0x47, 0x78, 0xc9, 0x19, 0x87, 0x78,
	0xc9, 0x19, 0xc7, 0x78, 0xc8, 0x19, 0x80, 0xbd, 0x01, 0xb5, 0x10, 0x00, 0x00, 0xf0, 0x1a, 0xf8, 0xc1, 0x5c, 0x01, 0xbd, 0xc0, 0x46,
	0x00, 0xf0, 0x0c, 0xf8, 0x2c, 0x48, 0x2d, 0x49, 0x00, 0xf0, 0x08, 0xf8, 0x33, 0x48, 0x34, 0x49, 0x00, 0x22, 0xc0, 0x46, 0x01, 0x39,
	0x42, 0x54, 0xfc, 0xd1, 0x00, 0xbd, 0x28, 0x4b, 0x18, 0x47, 0x00, 0x21, 0x28, 0x4b, 0xd3, 0x1a, 0x2b, 0x2b, 0x00, 0xd2, 0x01, 0x21,
	0xf7, 0x46, 0x26, 0x49, 0x88, 0x42, 0x02, 0xda, 0x57, 0x46, 0xff, 0x6c, 0x15, 0xe0, 0x24, 0x4f, 0x24, 0x49, 0x88, 0x42, 0x0f, 0xdb,
	0x24, 0x49, 0x88, 0x42, 0x0e, 0xdb, 0x23, 0x49, 0x88, 0x42, 0x01, 0xda, 0xc0, 0x38, 0x09, 0xe0, 0x24, 0x49, 0x40, 0x1a, 0x04, 0xdb,
	0x23, 0x49, 0x88, 0x42, 0x01, 0xda, 0x1f, 0x4f, 0x01, 0xe0, 0x12, 0xa7, 0x00, 0x20, 0x0c, 0x21, 0x48, 0x43, 0x38, 0x18, 0x70, 0x47,
	0x40, 0x1a, 0x49, 0x00, 0x40, 0x18, 0x1d, 0x4a, 0x13, 0x88, 0x03, 0x71, 0x1a, 0x0a, 0x42, 0x71, 0x1b, 0x4a, 0x01, 0x3b, 0x13, 0x80,
	0xf7, 0x46, 0x19, 0x4a, 0x40, 0x1a, 0x49, 0x00, 0x40, 0x18, 0x43, 0x79, 0x00, 0x79, 0x1b, 0x02, 0x18, 0x43, 0x13, 0x1c, 0x13, 0x88,
	0xf7, 0x46, 0x40, 0x1a, 0x49, 0x00, 0x40, 0x18, 0x00, 0x22, 0x02, 0x71, 0x42, 0x71, 0xf7, 0x46, 0xc0, 0x46, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
/* ... their literals, after them */
static const uint32_t pool[] = {
	BN6_PACK_EXT, (BN6_PACK_EXT_IDS - BN6_PACK_IDS) * BN6_PACK_ENTRY, 0x080008E1u, 0x170,
	BN6_PACK_IDS, BN6_PACK_EXT - BN6_PACK_IDS * BN6_PACK_ENTRY, BN6_PACK_EXT_FIRST, BN6_PACK_WIDE_FIRST, BN6_PACK_WIDE_END,
	BN6_PACK_SAVE, BN6_PACK_SAVE_BYTES, BN6_PACK_SAVE_FIRST, BN6_PACK_SAVE_END - BN6_PACK_SAVE_FIRST, BN6_PACK_STAMP_NEXT,
};
_Static_assert(sizeof code + sizeof pool == XPACK_SIZE, "the block's size");
_Static_assert(BN6_PACK_WIDE_FIRST - BN6_PACK_IDS == 192, "the routine's subs r0,#192 (+0x7A)");
_Static_assert(BN6_PACK_ENTRY == 12, "the routine's movs r1,#12 (+0x92), and an entry's stamps from its +4");
_Static_assert(BN6_PACK_SAVE_BYTES % 4 == 0 && BN6_PACK_SAVE_BYTES > 0, "the clear's hook zeroes the entries by words; its loop wants a byte at least");
_Static_assert(BN6_PACK_SAVE - BN6_PACK_SAVE_MARK == BN6_PACK_SAVE_MARKED && BN6_PACK_SAVE_MARK == SRAM_PACK &&
	BN6_PACK_SAVE + BN6_PACK_SAVE_BYTES <= SRAM_PACK_END, "the entries in their region of the save RAM (free_space.plan)");
_Static_assert(BN6_SAVE_MAIN + BN6_SAVE_MAIN_SIZE <= SRAM_PACK && SRAM_PACK_END <= BN6_SAVE_BLOCK_1, "the entries between BN6's own blocks");

void xchips_pack_code(uint8_t out[XPACK_SIZE]) {
	memcpy(out, code, sizeof code);
	for (size_t i = 0; i < sizeof pool / sizeof *pool; ++i) {
		uint8_t *b = out + sizeof code + 4 * i;
		b[0] = (uint8_t)pool[i]; b[1] = (uint8_t)(pool[i] >> 8); b[2] = (uint8_t)(pool[i] >> 16); b[3] = (uint8_t)(pool[i] >> 24);
	}
}

HookAct xchips_pack_cleared(HookRegs *r, void *user) {
	(void)user;
	/* (r0 the entries, r1 their bytes, as the routine loaded them: one byte left to its loop, the first, zero already) */
	for (uint32_t i = 0; i + 4 <= r->r[1]; i += 4) hook_write32(r->r[0] + i, 0);
	r->r[1] = 1;
	return HOOK_CONTINUE;
}

/* ---- the save RAM's entries, from outside the game ---- */

static const uint8_t mark[BN6_PACK_SAVE_MARKED] = { 'C', 'W', ' ', 'p', 'a', 'c', 'k', ' ', 'i', 'd', 's', ' ', '6', '3', '5', 1 };

/* the mark's place in the core's save RAM, NULL where its save is no SRAM */
#ifdef CW_GPSP
/* (on gpSP its cart's save RAM, the one core's whatever core is named) */
static uint8_t *kept_at(struct mCore *core) {
	(void)core;
	size_t n = 0;
	uint8_t *save = emu_gpsp_save_ram(&n);
	return save && n >= BN6_PACK_SAVE_MARK - BN6_SAVE_RAM + XPACK_KEPT ? save + (BN6_PACK_SAVE_MARK - BN6_SAVE_RAM) : NULL;
}
#else
static uint8_t *kept_at(struct mCore *core) {
	struct GBA *gba = core ? core->board : NULL;
	if (!gba || gba->memory.savedata.type != SAVEDATA_SRAM || !gba->memory.savedata.data) return NULL;
	return gba->memory.savedata.data + (BN6_PACK_SAVE_MARK - BN6_SAVE_RAM);
}
#endif

bool xchips_pack_ready(struct mCore *core) {
#ifndef CW_GPSP
	struct GBA *gba = core ? core->board : NULL;
	/* (mGBA finds the save's kind at the game's first use of it: a read finds SRAM, as BN6's own first read does) */
	if (gba && gba->memory.savedata.type == SAVEDATA_AUTODETECT) (void)core->busRead8(core, BN6_PACK_SAVE_MARK);
#endif
	uint8_t *at = kept_at(core);
	if (!at) return false;
	if (memcmp(at, mark, sizeof mark)) {
		memset(at, 0, XPACK_KEPT);
		memcpy(at, mark, sizeof mark);
	}
	return true;
}

void xchips_pack_keep(struct mCore *core, uint8_t out[XPACK_KEPT]) {
	const uint8_t *at = kept_at(core);
	if (at) memcpy(out, at, XPACK_KEPT);
	else memset(out, 0, XPACK_KEPT);
}

void xchips_pack_put(struct mCore *core, const uint8_t in[XPACK_KEPT]) {
	uint8_t *at = kept_at(core);
	if (at) memcpy(at, in, XPACK_KEPT);
}
