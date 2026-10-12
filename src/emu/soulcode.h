/* The code BN5's Team Colonel Souls run, moved into BN6's battle engine
 * (issue #82, the Souls track's S3-S5; docs/MULTIROM.md, Transplanted
 * Souls; docs/ROM_DATA.md, The Souls' code in BN6's engine): each Soul's
 * charged shot's power attack and attack family, the objects they spawn,
 * and BN5's phase of the unison that shows the Soul's image, copied from
 * BN5's ROM into the Souls' room (soul_room.h) as the core starts, their
 * calls aimed at BN6's twins, their literals at BN6's RAM and tables, the
 * numbers BN6 has otherwise given BN6's. The recipe is numbers alone,
 * written from the two ROMs by tools/xsoul.py (soulcode_bn5.c), the
 * install all or nothing, every byte it stands on checked first. */
#ifndef CW_SOULCODE_H
#define CW_SOULCODE_H

#include <stdbool.h>
#include <stdint.h>

#include "xvirus_recipe.h"

/* an object type its code spawns: BN6's twin `to`, or moved (XV_TYPE_MOVED: BN5's routine `routine`, moved with the code) */
typedef struct { uint8_t kind, type, to; uint32_t routine; } SoulType;
/* an effect BN6 has none of: at BN6's id `to`, BN5's record (its list, number, animation, palette) with its sprite as BN6 lists it */
typedef struct { uint8_t from, to, list, index, anim, pal; } SoulEffect;
/* a read of BN5's AI data's Soul field at `at`, `size` bytes into rd, which BN6's AI data holds otherwise: answered from BN6_SOULS */
typedef struct { uint32_t at; uint8_t rd, size; uint32_t field; } SoulField;
/* a power attack a Soul's loadout names: BN5's id, its routine (BN5's, moved) */
typedef struct { uint8_t id; uint32_t entry; } SoulPower;
/* a song its code plays that Gregar has otherwise: BN5's, copied into BN6's empty slot `to` (soulpower.c) */
typedef struct { uint16_t from; uint8_t to; } SoulSong;
/* an attack family moved: BN5's CurAction, BN6's (xattack_grow.h), its routine (BN5's, moved) */
typedef struct { uint8_t from, to; uint32_t entry; } SoulFamily;

typedef struct {
	const XvBlock *blocks;      /* BN5's code and data, each to its place in SOULS_CODE (its distance from a word kept) */
	int nblocks;
	const XvData *data;         /* each block's ranges that are no code: XV_POOL (words, each in `words`), XV_RAW (as they are) */
	int ndata;
	const XvNumber *words;      /* every word of its pools and tables: BN5's, and BN6's in its place */
	int nwords;
	const XvNumber *calls;      /* every call out of its block: its site, BN5's routine, BN6's in its place */
	int ncalls;
	const XvImm *imms;          /* immediates given BN6's numbers */
	int nimms;
	const SoulType *types;
	int ntypes;
	const XvSprite *sprites;    /* BN5's sprites it draws that BN6 has none of: copied into SOULS_ART, listed in a grown list */
	int nsprites;
	const SoulEffect *effects;
	int neffects;
	const XvByte *bytes;        /* bytes of its raw tables BN6 numbers otherwise */
	int nbytes;
	const SoulField *fields;
	int nfields;
	const SoulPower *powers;
	int npowers;
	const SoulSong *songs;
	int nsongs;
	const SoulFamily *families;
	int nfamilies;
} SoulCode;

extern const SoulCode soulcode_bn5;

/* Who the Souls' moved code's bytes are (claims.h) */
#define SOULCODE_CLAIMS "the Souls' moved code (soulcode.c)"

/* The recipe carried out on the core's ROM copy, once a core, BN6's
 * attack families and the families' room grown first: its code (BN5's
 * arm flash object and its kinds table among it), veneers and sprites in
 * the Souls' room; its effects and object types in the families' room's
 * grown tables; its families in the
 * grown attack families; its stubs and the Soul fields its code reads
 * hooked. False, nothing of it placed, where BN5's ROM is not read or a
 * byte it stands on is not as the recipe says (the log says which). */
bool soulcode_install(const SoulCode *c);
/* Where BN5's address `bn5` is in the moved code (its Thumb bit kept), 0
 * where it is none of it; and BN6's id of a Soul's power attack by BN5's
 * (SOUL_POWER_FIRST on, the recipe's order), -1 for none. */
uint32_t soulcode_moved(uint32_t bn5);
int soulcode_power(int bn5_id);

#endif
