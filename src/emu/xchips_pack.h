/* The chip pack's routines in the core's copy, and its entries in the
 * cart's save RAM (docs/rom/chips-code.md, The pack in the save RAM).
 *
 * BN6's pack holds 12 bytes a chip id to 319 (four counts, a byte a code
 * of the record's four, then four u16 order stamps). The engine's routine
 * answers every reader of an entry's address (bn6_chips.h's
 * bn6_pack_entry, the same mapping): BN6's own pack, the extension in the
 * work RAM (ids 411-602), and from id 635 the cart's save RAM, 32 KB of
 * which BN6's save library addresses four blocks and nothing between the
 * first and the rest. mGBA serves that RAM a byte at a time (a halfword
 * read there is one byte twice), so the three routines of BN6's that reach
 * an order stamp by a halfword go on to the engine's, which read and write
 * it as two bytes, wherever the entry lies. Every state the engine writes
 * (a checkpoint's, the boot's, a .cwsave's) holds the save RAM whole; a
 * NetBattle's rollback, whose snapshots are the core's raw state, takes
 * the entries beside it (xchips_pack_keep, xchips_pack_put). A state from
 * before the entries holds 0xFF there, as mGBA fills a save RAM nothing
 * wrote: xchips_pack_ready zeroes them wherever a core is made or a state
 * loaded, told by 16 bytes before them. */
#ifndef CW_XCHIPS_PACK_H
#define CW_XCHIPS_PACK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bn6_chips.h"
#include "hook.h"

struct mCore;

/* The routines' block, written at CHIPS_CODE: where each begins in it, and
 * its bytes with its literals */
enum {
	XPACK_OFFSET = 0x00,       /* getOffsetToQuantityOfChipCode's tail */
	XPACK_TOTAL = 0x0A,        /* GetTotalChipCount's */
	XPACK_COUNT = 0x1E,        /* the pack totals' count */
	XPACK_CLEAR = 0x2C,        /* the pack's clear: BN6's pack, the extension, the save RAM's entries */
	XPACK_CLEAR_HOOK = 0x3E,   /* ... its nop before the save RAM's loop: xchips_pack_cleared's place */
	XPACK_PSEUDO = 0x4C,       /* the pseudo-chips' test */
	XPACK_ENTRY = 0x5A,        /* a chip's pack entry: bn6_pack_entry */
	XPACK_STAMP_SET = 0x9A,    /* BN6_PACK_STAMP_SET's work by bytes, */
	XPACK_STAMP_GET = 0xB2,    /* ... BN6_PACK_STAMP_GET's */
	XPACK_STAMP_CLEAR = 0xC8,  /* ... and BN6_PACK_STAMP_CLEAR's */
	XPACK_SIZE = 0x11C,
};
void xchips_pack_code(uint8_t out[XPACK_SIZE]);

/* The clear's hook (an answer hook on CHIPS_CODE + XPACK_CLEAR_HOOK): the
 * save RAM's entries zeroed here and the routine's loop left one byte to
 * write, so a clear costs the game's frame nothing. Without it the loop
 * zeroes them all: the hook saves time, the clear does not rest on it. */
HookAct xchips_pack_cleared(HookRegs *r, void *user);

/* The save RAM's entries made on `core`: its save found to be SRAM (as
 * BN6's first read of it finds it), and where the 16 bytes before the
 * entries are not the mark, the entries zeroed and the mark written. After
 * a core is made and after every state it loads with its save RAM; false
 * where the core's save is no SRAM. */
bool xchips_pack_ready(struct mCore *core);

/* The mark and the entries, for a snapshot that keeps the core's raw state
 * alone: copied out, and back (nothing where the core's save is no SRAM) */
#define XPACK_KEPT (BN6_PACK_SAVE_MARKED + BN6_PACK_SAVE_BYTES)
void xchips_pack_keep(struct mCore *core, uint8_t out[XPACK_KEPT]);
void xchips_pack_put(struct mCore *core, const uint8_t in[XPACK_KEPT]);

#endif
