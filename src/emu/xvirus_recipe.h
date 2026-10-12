/* A virus family's recipe (xvirus.h): its types, where its parts go in the
 * core's ROM copy, and what a recipe can say of itself before any ROM is
 * read (each block's ranges whole, its states and calls aimed at code). */
#ifndef CW_XVIRUS_RECIPE_H
#define CW_XVIRUS_RECIPE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "emu.h"
#include "thumb_reloc.h"

#define XV_TABLES   11   /* BN6's per-family tables (BN6_FAMILY_TABLES) */
#define XV_HOOKS    (XV_TABLES - 3)   /* ... past the state table, struct1 and struct2: per frame, spawn start and end, paralysis, freeze, bubble, flinch, drag */
#define XV_VERSIONS 6    /* a family's versions, its enemy ids one after another */
#define XV_GAME_BN6 (-1) /* XVirusFamily.game: BN6's own ROM (the control); else an XRomId */

/* What a range of a copied block holds that is not code */
enum {
	XV_POOL,    /* literal words: one that points into a copied block is moved with it; one in RAM, I/O or ROM
	               elsewhere must be one of the recipe's words; any other is a number, kept */
	XV_TABLE,   /* a jump table: Thumb pointers into a copied block, each moved */
	XV_RAW,     /* bytes as they are (tables of numbers, alignment): no word of them may point into a block */
	XV_ZERO,    /* bytes dead in the copy, zeroed (the family's state table in its game's layout) */
};

typedef struct { uint32_t start, end, at, crc; } XvBlock;   /* [start, end) in its game, copied to `at`; the CRC32 of its bytes there */
typedef struct { uint32_t start, end; int kind; } XvData;   /* [start, end) in its game, within a block */
typedef struct { uint32_t from, to; } XvPair;               /* an address of its game's, and BN6's in its place */
typedef struct { uint32_t addr; bool own; } XvRef;          /* a BN6 routine (its Thumb address), or the family's own (its address in its game) */
typedef struct {                                            /* an immediate changed in its instruction, checked first */
	uint32_t at;
	ThumbForm form;
	uint8_t rd, rn;
	uint16_t was, now;
} XvImm;

/* A sprite of its game's (list, number; LZ77 or not) copied whole,
 * decompressed, to `at` (`room` bytes at most) and listed at BN6's
 * (to_list, to_index): a number of BN6's that points at its placeholder
 * (BN6_SPRITE_PLACEHOLDER), or one past the end of list 4
 * (BN6_SPRITES_ATTACKS), which is grown */
typedef struct { uint8_t list, index, to_list, to_index; uint32_t at, room; } XvSprite;
typedef struct { uint8_t from, to; } XvId;                  /* an id of its game's (an effect, an attack's type) and BN6's in its place */
/* Its struct1 laid out BN6's way where its game lays it out otherwise
 * (BN5 keeps the shadow at +5, where BN6 keeps an element): BN6's record
 * made of its game's palette step and these, its battle sprite the
 * recipe's first, where its game's record names that sprite and these
 * (BN6_TRAITS_*) */
typedef struct {
	bool laid;                 /* false: its game's record as it is (BN6's own) */
	uint8_t mode;              /* BN6_TRAITS_MODE: under 32 (its game's own +4) */
	uint8_t element, weakness; /* BN6's second wheel: BN5's viruses have none */
	bool shadow;
} XvTraits;

typedef struct {
	const char *name;          /* for the log */
	int game;                  /* XV_GAME_BN6, or the XRomId of the ROM it comes from */
	int src_ai;                /* its AI index in its game's per-family tables (its records are checked against them) */
	int ai;                    /* its AI index in BN6's grown tables (BN6_FAMILIES on) */
	int first_id;              /* its first enemy id; its versions' follow it */
	uint32_t home;             /* its records in BN6 (XV_HOME_*): the state table, struct1, struct2, held chips, veneers */
	const XvBlock *blocks;
	int nblocks;
	const XvData *data;        /* each block's ranges that are not code; the rest of a block is code */
	int ndata;
	const XvPair *calls;       /* the routines its BLs call outside the blocks: its game's, BN6's in their place */
	int ncalls;
	const XvPair *words;       /* the literal words that name RAM, I/O or ROM outside the blocks: its game's, BN6's */
	int nwords;
	const XvImm *imms;
	int nimms;
	const XvRef *states;       /* its state table, BN6's layout: generic states 0-7, its own from 8 */
	int nstates;
	XvRef hooks[XV_HOOKS];     /* its entries of tables 3-10 */
	uint32_t struct1, struct2; /* its records in its game (8 bytes: sprites, element; 36: HP and damage per version) */
	uint16_t names[XV_VERSIONS];   /* its versions' enemy ids in its game: its names' indexes there, and its reward rows' (another
	                                  game's: xvirus.c, drops_set) */
	uint32_t held;             /* its viruses' held chips, lists in BN6's ROM (the control's: its own); 0 none */
	int ice;                   /* the size of the ice block it is frozen in (BN6_ICE_SIZES: 0 to XV_ICE_MAX), or XV_ICE_OWN, its
	                              game's own for its AI index there (BN6's for the control); 0 where a recipe says none */
	XvTraits traits;
	const XvSprite *sprites;   /* what it draws, its battle sprite first where `traits` are laid */
	int nsprites;
	const XvId *effects;       /* its game's effects it draws, each given BN6's id past BN6_EFFECT_COUNT (the table grown), its sprite
	                              one of `sprites` */
	int neffects;
	const XvId *types;         /* its attacks: its game's types, whose routine is its own code, each given an empty type of BN6's */
	int ntypes;
} XVirusFamily;

#define XV_ICE_OWN (-1)
#define XV_ICE_MAX 2

/* Where it all goes, at fixed addresses (pointers into them stay good in
 * the states a session saves): EMU_FREE + 0x330000 to + 0x350000
 * (docs/EMULATION.md) */
#define XV_AT     (EMU_FREE + 0x330000u)
#define XV_GROWN  XV_AT                  /* BN6's 11 per-family tables grown to XV_FAMILIES entries, 0x100 apart */
#define XV_ICE    (XV_AT + 0xB00u)       /* the ice blocks' sizes (BN6_ICE_SIZES) grown to XV_FAMILIES bytes; on to XV_LIST free */
#define XV_LIST   (XV_AT + 0xC00u)       /* sprite list 4 (BN6_SPRITES_ATTACKS) grown to XV_LIST_MAX numbers */
#define XV_EFFECTS (XV_AT + 0xE00u)      /* the effects (BN6_EFFECTS) grown to XV_EFFECTS_MAX ids */
#define XV_NAMES  (XV_AT + 0x1000u)      /* the viruses' names, BN6's archive rebuilt with the families' */
#define XV_HOMES  (XV_AT + 0x2000u)      /* the families' homes, XV_HOME_SIZE each */
#define XV_CODE   (XV_AT + 0x8000u)      /* their code, block by block */
#define XV_ART    (XV_AT + 0x10000u)     /* what they draw: their sprites */
#define XV_END    (XV_AT + 0x20000u)
#define XV_LIST_MAX    128
#define XV_EFFECTS_MAX 128
#define XV_FAMILIES 64                   /* AI indexes in the grown tables */

/* A family's records in BN6, from its home */
#define XV_HOME_STATES  0x000   /* its state table, 16 entries at most */
#define XV_HOME_STRUCT1 0x040
#define XV_HOME_STRUCT2 0x048
#define XV_HOME_HELD    0x070   /* six empty held-chip lists (0x20 bytes each, 0xFFFF), where it holds none */
#define XV_HOME_VENEERS 0x200   /* its veneers, to the end of its home */
#define XV_HOME_SIZE    0x800
#define XV_STATES_MAX   16

#define XV_HELD_AIS     35      /* the AI indexes BN6_HELD_CHIPS holds (and two more 35-word tables): a fourth family needs them moved */

/* The CRC32 (zlib's) of bytes: a recipe's check on the ranges it copies. */
uint32_t xvirus_crc32(const uint8_t *p, uint32_t n);
/* Whether the recipe holds together (AI index, ids, home, its blocks, their
 * data, states, hooks, calls and immediates); where not, why, into `why`. */
bool xvirus_recipe_ok(const XVirusFamily *f, char *why, size_t n);
/* The block an address of the family's game lies in (NULL: none), the data
 * range (NULL: code, or outside the blocks), and whether it is code. */
const XvBlock *xvirus_block_of(const XVirusFamily *f, uint32_t a);
const XvData *xvirus_data_at(const XVirusFamily *f, uint32_t a);
bool xvirus_is_code(const XVirusFamily *f, uint32_t a);

#endif
