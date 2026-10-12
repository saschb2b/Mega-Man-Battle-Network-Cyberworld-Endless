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
 * (to_list, to_index) in the grown copy of that list (lists 1, 2, 4 and 5,
 * XV_LISTS): a number that points at BN6's placeholder there
 * (BN6_SPRITE_PLACEHOLDER), one of BN6's own unused or one past its end */
typedef struct { uint8_t list, index, to_list, to_index; uint32_t at, room; } XvSprite;
typedef struct { uint8_t from, to; } XvId;                  /* an id of its game's (an effect, an attack's type) and BN6's in its place */
/* Its struct1 laid out BN6's way where its game lays it out otherwise
 * (BN5 keeps the shadow at +5, where BN6 keeps an element): BN6's record
 * made of its game's palette step and these, its battle sprite the
 * recipe's first, where its game's record names that sprite and these
 * (BN6_TRAITS_*) */
typedef struct {
	bool laid;                 /* false: its game's record as it is (BN6's own) */
	uint8_t mode;              /* its game's own +4 (BN6_TRAITS_MODE): BN6's record takes it where BN6's table has it (under
	                              BN6_TRAITS_MODE_COUNT), else XV_MODE_PAST */
	uint8_t element, weakness; /* BN6's second wheel: BN5's viruses have none */
	bool shadow;
} XvTraits;

typedef struct {
	const char *name;          /* for the log */
	int game;                  /* XV_GAME_BN6, or the XRomId of the ROM it comes from */
	int src_ai;                /* its AI index in its game's per-family tables (its records are checked against them) */
	int ai;                    /* its AI index in BN6's grown tables (BN6_FAMILIES to XV_AIS) */
	int first_id;              /* its first enemy id (XV_FIRST_ID on, a fixed place each family: the static model, issue #82);
	                              its versions' follow it */
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
	uint16_t names[XV_VERSIONS];   /* its versions' enemy ids in its game: its names' indexes there, and its reward rows' */
	uint32_t held;             /* its viruses' held chips, lists in BN6's ROM (the control's: its own); 0 none */
	int ice;                   /* the size of the ice block it is frozen in (BN6_ICE_SIZES: 0 to XV_ICE_MAX), or XV_ICE_OWN, its
	                              game's own for its AI index there (BN6's for the control); 0 where a recipe says none */
	XvTraits traits;
	const XvSprite *sprites;   /* what it draws, its battle sprite first where `traits` are laid */
	int nsprites;
	const XvId *effects;       /* its game's effects it draws, each given BN6's id past BN6_EFFECT_COUNT (the table grown), its sprite
	                              one of `sprites` */
	int neffects;
	const XvId *types;         /* its attacks: its game's types of the third kind, whose routine is its own code, each given a type of
	                              the grown table whose entry is empty (BN6's 0xD1, or one past BN6's 0xD4; never XV_T3_DISPATCH) */
	int ntypes;
} XVirusFamily;

#define XV_ICE_OWN (-1)
#define XV_ICE_MAX 2
/* The types kept for the second-level dispatch that lifts the byte-wide
 * type limit (a later step of issue #82: a type per kind whose routine
 * reads the moved routine from a table by the object's slot), each dead in
 * BN6 (no spawn names it): never a family's */
#define XV_T1_DISPATCH 0x5C
#define XV_T3_DISPATCH 0xD2
#define XV_T4_DISPATCH 0x91
#define XV_MODE_PAST 0   /* BN6_TRAITS_MODE given a family whose game's mode is past BN6's table: its byte 0, as most of BN6's families' */

/* Where it all goes, at fixed addresses (pointers into them stay good in
 * the states a session saves): EMU_FREE + 0x330000 to + 0x400000
 * (docs/EMULATION.md). First BN6's own tables, grown once a core
 * (xvirus_grow.h): each holds BN6's entries as the player's ROM has them,
 * the rest its fill, and the one word that names it points here */
#define XV_AT       (EMU_FREE + 0x330000u)
#define XV_GROWN    XV_AT                  /* BN6's 11 per-family tables grown to XV_AIS words, XV_STEP apart */
#define XV_STEP     0x400u
#define XV_ICE      (XV_AT + 0x2C00u)      /* the ice blocks' sizes (BN6_ICE_SIZES) grown to XV_AIS bytes */
#define XV_PER_AI   (XV_AT + 0x2D00u)      /* the three 35-word tables by AI index grown to XV_AIS words, XV_STEP apart: BN6_AI_HOOKS,
                                              BN6_AI_HOOKS2, BN6_HELD_CHIPS */
#define XV_LISTS    (XV_AT + 0x4000u)      /* sprite lists 1, 2, 4 and 5 grown to XV_LIST_MAX numbers, XV_STEP apart */
#define XV_EFFECTS  (XV_AT + 0x5000u)      /* the effects (BN6_EFFECTS) grown to XV_EFFECTS_MAX ids */
#define XV_TYPES    (XV_AT + 0x5400u)      /* the first, third and fourth kinds' types grown to XV_TYPES_MAX, XV_STEP apart */
#define XV_IDS_AT   (XV_AT + 0x6000u)      /* the enemy ids' triples grown to XV_IDS */
#define XV_ROWS     (XV_AT + 0x7000u)      /* the reward rows (BN6_DROP_ROWS) grown to XV_IDS */
#define XV_NAMES    (XV_AT + 0x11000u)     /* the names of ids from XV_FIRST_ID: a text archive of XV_IDS - XV_FIRST_ID entries (BN6_NAME_OF's
                                              third, by a hook) */
#define XV_NAMES_ROOM 0x4000u
/* then the families' own */
#define XV_HOMES    (XV_AT + 0x18000u)     /* the families' homes, XV_HOME_SIZE each, XV_FAMILIES of them */
#define XV_CODE     (XV_AT + 0x38000u)     /* their code, block by block */
#define XV_ART      (XV_AT + 0x50000u)     /* what they draw: their sprites */
#define XV_END      (XV_AT + 0xD0000u)
#define XV_AIS      256                    /* AI indexes in the grown tables (AIData +1, a byte) */
#define XV_LIST_MAX 256                    /* a grown sprite list's numbers (struct1's and sprite_load's, a byte) */
#define XV_EFFECTS_MAX 256                 /* effect ids (an effect object's +4, a byte) */
#define XV_TYPES_MAX 256                   /* a kind's types (an object's +1, a byte) */
#define XV_FIRST_ID 0x200                  /* the families' enemy ids: from here, six one after another a family, */
#define XV_IDS      0x400                  /* ... to here (the battle's own read of an id takes 12 bits, its preload's patched to as many) */
#define XV_FAMILIES 64                     /* the families' homes */

/* A family's records in BN6, from its home */
#define XV_HOME_STATES  0x000   /* its state table, 16 entries at most */
#define XV_HOME_STRUCT1 0x040
#define XV_HOME_STRUCT2 0x048
#define XV_HOME_HELD    0x070   /* six empty held-chip lists (0x20 bytes each, 0xFFFF), where it holds none */
#define XV_HOME_VENEERS 0x200   /* its veneers, to the end of its home */
#define XV_HOME_SIZE    0x800
#define XV_STATES_MAX   16

#define XV_LISTS_GROWN 4   /* the sprite lists grown (XV_LISTS): 1, 2, 4 and 5 */
/* A sprite list's place among the grown (0 to XV_LISTS_GROWN - 1), -1 for
 * one that is not grown. */
int xvirus_list_slot(int list);
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
