/* What BN6's PET changed in the Net Square, carried home (docs/online/square/pet.md).
 * The Square runs on the game's own core and its RAM goes on the way home,
 * the gate's checkpoint loaded again (online_visit.c). So each time the PET
 * closes there, the bytes it changed in the game's kept data (the folders,
 * the Regular and TagChips, the NaviCust's board and MegaMan's stats, the
 * SubChips, the mail read) are noted, byte by byte with what stood there
 * before; at home they are written onto the checkpoint, where every one of
 * them still stands as the PET found it, or none is. No game here: the
 * regions are read and written by the caller (square_pet.c), and the record
 * is the file kept beside the checkpoint until home has it. */
#ifndef CW_PET_CARRY_H
#define CW_PET_CARRY_H

#include <stdbool.h>
#include <stdint.h>

#include "bn6.h"
#include "bn6_bbs.h"
#include "bn6_navicust.h"

/* The kept data a PET screen writes, one after another in the record: the
 * event flags to the request BBS's state, where the map's own begins (the
 * folders, the pack, the key items' counts and their checks, the NaviCust,
 * MegaMan's stats and the chips' marks between: bn6f ewram.s); the
 * programs on the NaviCust's command line; and the pack past BN6's own ids */
typedef struct { uint32_t at, bytes; } PetCarryRegion;
#define PET_CARRY_REGIONS 4
#define PET_CARRY_MAIN  (BN6_BBS_STATE - BN6_EVENT_FLAGS)
#define PET_CARRY_EXT   ((BN6_PACK_EXT_IDS - BN6_PACK_IDS) * BN6_PACK_ENTRY)
#define PET_CARRY_BYTES (PET_CARRY_MAIN + BN6_NAVICUST_LINE_BYTES + PET_CARRY_EXT + BN6_PACK_SAVE_BYTES)
extern const PetCarryRegion pet_carry_regions[PET_CARRY_REGIONS];

#define PET_CARRY_MAGIC 0x54455043u   /* "CPET" */

typedef struct {
	uint32_t seed;    /* the run it belongs to, and the layer it was at */
	int32_t depth;
	uint32_t bits;    /* the record's first bytes that are event flags: carried a bit at a time */
	uint32_t count;   /* the bytes it changes, 0 none */
	uint32_t traded;  /* the visit's trade is among them: the Pack's chips swapped (online_trade.h) */
	uint8_t old[PET_CARRY_BYTES];    /* each changed byte as the PET first found it, */
	uint8_t now[PET_CARRY_BYTES];    /* ... as it last left it, */
	uint8_t mask[PET_CARRY_BYTES];   /* ... and its bits changed: 0 a byte left alone */
} PetCarry;

/* A record of nothing, for run `seed` at `depth`; `bits` as above */
void pet_carry_clear(PetCarry *c, uint32_t seed, int32_t depth, uint32_t bits);
/* A PET session's changes into the record: the regions as it opened and as
 * it closed (or a trade's swap: before it and after). The bytes of this
 * session that differ. */
unsigned pet_carry_note(PetCarry *c, const uint8_t *open, const uint8_t *shut);
/* Whether the regions `home` (the checkpoint's) stand as the PET found
 * them wherever the record changes a byte */
bool pet_carry_fits(const PetCarry *c, const uint8_t *home);
/* The record written into `home`: each changed byte, a flag's a bit at a time */
void pet_carry_apply(const PetCarry *c, uint8_t *home);
/* A record read from a file: its counts as its bytes say, nothing past the regions */
bool pet_carry_sound(const PetCarry *c);

#endif
