/* The chips' data as the core's ROM copy holds it (issue #82; docs/
 * MULTIROM.md, The chip registry): the one place the run's code reads a
 * chip's record, name, description and art from, not the player's file.
 *
 * BN6's chip tables, grown from 411 records to 512 and their names and
 * descriptions to ids 511, with the registry's chips of Battle Network 5
 * in them (xchips_registry.h): made here from the player's ROMs, BN5's
 * records translated field by field (xchips_record.h), its names and
 * descriptions in BN6's text codes, its icons and pictures copied; where
 * BN5's ROM is not read, each of them a placeholder that keeps its codes,
 * type and MB. The same bytes go into the core's copy at fixed places
 * past EMU_FREE (src/emu/xchips_install.c; docs/EMULATION.md), the same at
 * every boot with the same ROMs, so a saved folder never names garbage. */
#ifndef CW_CHIPS_H
#define CW_CHIPS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "emu.h"
#include "xchips_registry.h"

#define CHIPS_IDS 512                          /* the records: ids 0-511 (nine bits) */
#define CHIPS_BN6_RECORDS 411                  /* BN6's own, 0-410 */
#define CHIPS_FREE (EMU_FREE + 0x400000u)      /* the chips' place in the core's copy (docs/EMULATION.md), to: */
#define CHIPS_FREE_END (EMU_FREE + 0x480000u)
#define CHIPS_RECORDS (CHIPS_FREE + 0x0000u)   /* 512 records, 0x2C each */
#define CHIPS_NAMES0 (CHIPS_FREE + 0x6000u)    /* the names' text archives: ids 0-255, */
#define CHIPS_NAMES1 (CHIPS_FREE + 0x7000u)    /* ... 256-511, 0x1000 each */
#define CHIPS_DESCS0 (CHIPS_FREE + 0x8000u)    /* the descriptions' archives, 0x4000 each */
#define CHIPS_DESCS1 (CHIPS_FREE + 0xC000u)
#define CHIPS_CODE (CHIPS_FREE + 0x10000u)     /* the engine's own Thumb routines (xchips_install.c), 0x400 */
#define CHIPS_ART (CHIPS_FREE + 0x11000u)      /* BN5's icons, pictures and palettes, to CHIPS_FREE_END */
#define CHIPS_NAMES_ROOM 0x1000u
#define CHIPS_DESCS_ROOM 0x4000u

/* Chip `id`'s record (0x2C bytes, BN6's layout) as the core's copy holds
 * it, NULL without BN6's ROM or past 511 */
const uint8_t *chips_record(int id);
/* ... its name ("WideSht1"; a placeholder's "BN5-9"), "" for none */
void chips_name(int id, char *out, size_t n);
/* ... its description, its lines joined by spaces */
void chips_desc(int id, char *out, size_t n);
/* The bytes the core's copy holds at bus address `bus` (`n` of them): the
 * player's ROM below 8 MB, the chips' place past EMU_FREE (their art);
 * NULL elsewhere */
const uint8_t *chips_bytes(uint32_t bus, uint32_t n);
/* ... as a ROM offset (0x800000 on past the player's ROM), for gfx.c's
 * tiles, which take offsets */
const uint8_t *chips_offset_bytes(uint32_t off, uint32_t n);

/* Whether chip `id` sits out of BN6's battles (taken out of the battle's
 * deck as it is dealt: xchips_install.c): a chip of BN5's whose code is
 * not moved yet, a DarkChip of BN5's until its rule is, every one of BN5's
 * where its ROM is not read */
bool chips_sits_out(int id);
/* ... the same as a bit per id, for the hook that deals the deck */
const uint8_t *chips_sit_bits(void);
/* Whether chip `id` is one of BN5's (the registry's own ids) */
bool chips_bn5(int id);
/* Whether the registry's chips are BN5's own (its ROM read), not placeholders */
bool chips_from_bn5(void);

/* What the core's copy takes (xchips_install.c): the region's bytes from
 * CHIPS_FREE, `*len` of them, made now where the ROMs read changed since */
const uint8_t *chips_image(uint32_t *len);
/* Where the game reads the records: CHIPS_RECORDS once the core's copy
 * holds them (chips_installed), BN6's own table until then. A write into
 * the core's records (the All * helper's codes, BN6's DarkChips' power)
 * goes there. */
uint32_t chips_table_bus(void);
void chips_installed(bool on);
/* Made again at the next read, from the ROMs read then (the tests' made-up
 * ROMs, which may take a freed one's memory) */
void chips_forget(void);

#endif
