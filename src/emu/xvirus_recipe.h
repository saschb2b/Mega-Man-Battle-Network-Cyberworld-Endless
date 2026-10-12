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
	XV_TABLE,   /* a jump table: Thumb pointers into a copied block, each moved, or to a routine of its game's whose twin of BN6's the
	               recipe's words name */
	XV_RAW,     /* bytes as they are (tables of numbers, alignment): no word of them may point into a block */
	XV_ZERO,    /* bytes dead in the copy, zeroed (the family's state table in its game's layout) */
};

typedef struct { uint32_t start, end, at, crc; } XvBlock;   /* [start, end) in its game, copied to `at`; the CRC32 of its bytes there */
typedef struct { uint32_t start, end; int kind; } XvData;   /* [start, end) in its game, within a block */
typedef struct { uint32_t from, to; } XvPair;               /* an address of its game's, and BN6's in its place */
/* a BN6 routine (its Thumb address), or the family's own (its address in its game: in its blocks, or in clone k - 1 where
 * `clone` is k) */
typedef struct { uint32_t addr; bool own; uint8_t clone; } XvRef;
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
typedef struct { uint8_t from, to; } XvId;                  /* an id of its game's (an effect) and BN6's in its place */
/* An object type its code spawns through one of its game's spawners (the
 * first, third or fourth kind's), its number in its game left in the code:
 * the second-level dispatch (xvirus_dispatch.h) spawns it as BN6's twin
 * `to`, or, where `to` is XV_TYPE_MOVED, as the kind's dispatch type, which
 * runs its game's routine for `from` copied with the family's code */
typedef struct { uint8_t kind, from, to; } XvType;
#define XV_TYPE_MOVED 0xFF
/* A word of a block's literal pool that is a number, not an address (a mask, a spawn's words, an enemy id's bound): its
 * game's `was`, BN6's `now` (the same where it is kept), each checked first */
typedef struct { uint32_t at, was, now; } XvNumber;
/* A byte of a block's raw data (a table of ids its code reads: effects, sprites) given BN6's number, checked first */
typedef struct { uint32_t at; uint8_t was, now; } XvByte;
/* A routine of its code (code alone, no literal load) copied once more to `at` (the same distance from a word boundary),
 * its call to `call` (its game's) aimed at BN6's `to` instead of the recipe's twin, and an immediate otherwise
 * (imm.at 0: none): a status wrapper of its own given for BN6's states BN5 has no twin of (frozen, bubbled) */
typedef struct {
	uint32_t start, end, at;
	uint32_t call, to;
	XvImm imm;
} XvClone;
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
	const XvPair *words;       /* the literal words that name RAM, I/O or ROM outside the blocks, and a jump table's entries out of
	                              them: its game's, BN6's */
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
	const XvType *types;       /* the object types its code spawns by its game's spawners: moved (its game's routine for the type is
	                              its own code) or BN6's twin, through the second-level dispatch; each spawner its code calls is
	                              aimed at the dispatch's (calls: XV_SPAWN_*) */
	int ntypes;
	const XvNumber *numbers;   /* its pools' words that are numbers (kept, or BN6's) */
	int nnumbers;
	const XvByte *bytes;       /* its raw data's bytes that are ids BN6 numbers otherwise */
	int nbytes;
	const XvClone *clones;     /* its routines copied once more, a call aimed otherwise (its states and hooks may name them) */
	int nclones;
	const uint16_t *held_chips; /* its viruses' held chips as BN6's ids, six lists of 16 (0xFFFF ending one), written into its home
	                              (`held` 0); a chip of its game's BN6 has none of left out (the generator's hold-outs) */
} XVirusFamily;

#define XV_ICE_OWN (-1)
#define XV_ICE_MAX 2
/* The second-level dispatch's types (xvirus_dispatch.h): a type per kind,
 * dead in BN6 (no spawn names it), whose routine runs the moved routine its
 * object was spawned as, from a side table by the object's slot */
#define XV_T1_DISPATCH 0x5C
#define XV_T3_DISPATCH 0xD2
#define XV_T4_DISPATCH 0x91
#define XV_MODE_PAST 0   /* BN6_TRAITS_MODE given a family whose game's mode is past BN6's table: its byte 0, as most of BN6's families' */

/* Where it all goes, at fixed addresses (pointers into them stay good in
 * the states a session saves): the plan's FREE_VIRUS_ROOM and FREE_FAMILIES,
 * to FREE_CHIPS (free_space.h; docs/EMULATION.md). First BN6's own tables,
 * grown once a core (xvirus_grow.h): each holds BN6's entries as the
 * player's ROM has them, the rest its fill, and the one word that names it
 * points here */
#define XV_AT       FREE_VIRUS_ROOM
#define XV_GROWN    XV_AT                  /* BN6's 11 per-family tables grown to XV_AIS words, XV_STEP apart */
#define XV_STEP     0x400u
#define XV_ICE      (XV_AT + 0x2C00u)      /* the ice blocks' sizes (BN6_ICE_SIZES) grown to XV_AIS bytes */
#define XV_PER_AI   (XV_AT + 0x2D00u)      /* the three 35-word tables by AI index grown to XV_AIS words, XV_STEP apart: BN6_AI_HOOKS,
                                              BN6_AI_HOOKS2, BN6_HELD_CHIPS */
#define XV_LIST3    (XV_AT + 0x3900u)      /* sprite list 3 grown to XV_LIST_MAX numbers (the chips' track's, once for every track) */
#define XV_LISTS    (XV_AT + 0x4000u)      /* sprite lists 1, 2, 4 and 5 grown to XV_LIST_MAX numbers, XV_STEP apart */
#define XV_EFFECTS  (XV_AT + 0x5000u)      /* the effects (BN6_EFFECTS) grown to XV_EFFECTS_MAX ids */
#define XV_TYPES    (XV_AT + 0x5400u)      /* the first, third and fourth kinds' types grown to XV_TYPES_MAX, XV_STEP apart */
#define XV_IDS_AT   (XV_AT + 0x6000u)      /* the enemy ids' triples grown to XV_IDS */
#define XV_ROWS     (XV_AT + 0x7000u)      /* the reward rows (BN6_DROP_ROWS) grown to XV_IDS */
#define XV_NAMES    (XV_AT + 0x11000u)     /* the names of ids from XV_FIRST_ID: a text archive of XV_IDS - XV_FIRST_ID entries (BN6_NAME_OF's
                                              third, by a hook) */
#define XV_NAMES_ROOM 0x4000u
/* the second-level dispatch (xvirus_dispatch.h), made with the tables */
#define XV_DISPATCH (XV_AT + 0x15000u)
#define XV_MOVED_TABLES XV_DISPATCH          /* MOVED: a routine (Thumb) by its game's type, 256 words a kind (1, 3, 4), XV_STEP apart */
#define XV_TWIN_TABLES (XV_DISPATCH + 0xC00u) /* TWIN: BN6's twin by its game's type, or XV_TYPE_MOVED: 256 bytes a kind, 0x100 apart */
#define XV_STUBS (XV_DISPATCH + 0xF00u)      /* the dispatch's code, the project's own Thumb: */
#define XV_STUB_NONE (XV_STUBS + 0x000u)     /* ... a MOVED entry no family set: the object freed as it first runs */
#define XV_STUB_DISPATCH (XV_STUBS + 0x010u) /* ... the dispatch types' routines, the first kind's, the third's, the fourth's, */
#define XV_STUB_DISPATCH_STEP 0x20u          /* ... this far apart */
#define XV_SPAWN_1 (XV_STUBS + 0x080u)       /* ... the spawners a moved family's code calls, each its game's type through TWIN to BN6's: the
                                                first kind's, the object run after the one running (BN6's 0x08003320), */
#define XV_SPAWN_1_LAST (XV_STUBS + 0x0C0u)  /* ... the first kind's at the list's end (0x0800333C), */
#define XV_SPAWN_3 (XV_STUBS + 0x100u)       /* ... the third kind's after the one running (0x08003358), */
#define XV_SPAWN_3_LAST (XV_STUBS + 0x140u)  /* ... at the end (0x08003374), */
#define XV_SPAWN_3_FIRST (XV_STUBS + 0x180u) /* ... at the start (0x08003390), */
#define XV_SPAWN_4 (XV_STUBS + 0x1C0u)       /* ... the fourth kind's after the one running (0x080033AC) */
#define XV_SPAWN_STEP 0x40u
#define XV_SPAWNERS 6
#define XV_STUBS_END (XV_STUBS + 0x200u)
/* then the families' own */
#define XV_HOMES    FREE_FAMILIES          /* the families' homes, XV_HOME_SIZE each, XV_FAMILIES of them */
#define XV_CODE     (XV_AT + 0x38000u)     /* their code, block by block */
#define XV_ART      (XV_AT + 0x50000u)     /* what they draw: their sprites */
#define XV_END      FREE_CHIPS
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

#define XV_LISTS_GROWN 5   /* the sprite lists grown: 1, 2, 4 and 5 (XV_LISTS), 3 (XV_LIST3) */
/* A sprite list's place among the grown (0 to XV_LISTS_GROWN - 1), -1 for
 * one that is not grown. */
int xvirus_list_slot(int list);
/* The CRC32 (zlib's) of bytes: a recipe's check on the ranges it copies. */
uint32_t xvirus_crc32(const uint8_t *p, uint32_t n);
/* Whether the recipe holds together (AI index, ids, home, its blocks, their
 * data, states, hooks, calls and immediates); where not, why, into `why`. */
bool xvirus_recipe_ok(const XVirusFamily *f, char *why, size_t n);
/* Where a recipe's code and art may go: the families' (XV_CODE to XV_ART,
 * XV_ART to XV_END), or another installer's region (a chip's code,
 * xchips_code.h), and the stubs of its own its calls may go to besides
 * BN6's routines (stubs to stubs_end; the families' none) */
typedef struct { uint32_t code, code_end, art, art_end, stubs, stubs_end; } XvRegion;
/* Whether a recipe of code alone holds together (no states, records or held
 * chips: a chip's moved code, xchips_code.h), its blocks, clones and sprites
 * within `r`; where not, why. */
bool xvirus_code_ok(const XVirusFamily *f, const XvRegion *r, char *why, size_t n);
/* The block an address of the family's game lies in (NULL: none), the data
 * range (NULL: code, or outside the blocks), and whether it is code. */
const XvBlock *xvirus_block_of(const XVirusFamily *f, uint32_t a);
const XvData *xvirus_data_at(const XVirusFamily *f, uint32_t a);
bool xvirus_is_code(const XVirusFamily *f, uint32_t a);

#endif
