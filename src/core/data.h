/* Chip and enemy definitions. Names, power, codes, element, HP and art come
 * from the ROM; behavior is implemented by the engine and chosen here. */
#ifndef DATA_H
#define DATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Elements: BN6's first wheel (Aqua beats Fire, Elec Aqua, Wood Elec, Fire
 * Wood), an enemy's HP word and a chip's attack element; then its second
 * (Sword beats Wind, Wind Cursor, Cursor Breaker, Breaker Sword), an
 * enemy's traits and a chip's icon (docs/ROM_DATA.md). Either doubles a
 * hit on the element it beats. */
enum { ELEM_NULL, ELEM_FIRE, ELEM_AQUA, ELEM_ELEC, ELEM_WOOD, ELEM_SWORD, ELEM_WIND, ELEM_CURSOR, ELEM_BREAK, ELEM_COUNT };
/* An element by BN6's name for it ("Breaker" as its Cross lessons say), "" for none. */
const char *elem_name(int elem);

typedef enum {
	CK_CANNON,     /* hits the first enemy in the row */
	CK_AIRSHOT,    /* hits first enemy and pushes it back */
	CK_VULCAN,     /* param hits, first enemy, last hit splashes behind */
	CK_SPREADER,   /* first enemy plus the 8 panels around it */
	CK_SWORD,      /* panel ahead */
	CK_WIDESWORD,  /* column of 3 ahead */
	CK_LONGSWORD,  /* 2 panels ahead */
	CK_WAVE,       /* shockwave travelling along the row */
	CK_BOMB,       /* lobbed 3 panels ahead; param: 0 single, 1 cross, 2 3x3 */
	CK_RECOVER,    /* param HP */
	CK_AREAGRAB,   /* steal the enemy's front column */
	CK_PANELGRAB,  /* steal one panel */
	CK_BARRIER,    /* param absorb HP (10 = one hit) */
	CK_INVIS,      /* param frames */
	CK_FLAME,      /* fire line 3 panels ahead */
	CK_THUNDER,    /* slow ball that follows enemies and stuns */
	CK_TORNADO,    /* 8 hits on the panel two ahead */
	CK_CROSSGUN,   /* hits target and its diagonals */
	CK_BOOMER,     /* travels round the field edge */
	CK_GEDDON,     /* cracks all enemy panels */
	CK_HOLYPANEL,  /* makes own panel holy */
	CK_METEORS,    /* random meteor strikes on the enemy side */
	CK_ATKPLUS,    /* adds power to the next chip */
	CK_ROCKCUBE,   /* obstacle in front of MegaMan */
	CK_NAVI,       /* navi chip: power strike on every enemy */
	CK_COUNT
} ChipKind;

typedef struct {
	uint16_t rom_id;
	uint8_t kind;
	uint8_t param;
	uint8_t tier;   /* 0 common .. 4 legendary: drop and shop weight */
	uint8_t price;  /* in hundreds of zenny */
} ChipDef;

typedef struct {
	char name[20];
	int power;
	int element;
	int chip_element; /* ROM icon element (bn6f CHIP_ELEM_*): its colours, and its second-wheel element */
	char codes[5];    /* letters, '*' wildcard */
	int ncodes;
	int mb;           /* its memory, which caps its copies in a folder (loot_folder_full) */
} ChipInfo;

extern const ChipDef chip_defs[];
extern const int chip_def_count;
const ChipDef *chip_def(int rom_id);

/* Chip `rom_id`'s record: its name, power, elements and codes (with the All
 * * helper, run_all_star, its * alone, as the game's record then has it). */
void chip_info(int rom_id, ChipInfo *out);
/* Chip `rom_id`'s description in BN6's words ("Cannon to attack 1 enemy"). */
void chip_desc(int rom_id, char *out, size_t outlen);
/* A battle object's name by its NameID (BN6_T1_NAME_ID): "Mettaur" */
void enemy_name(int name_id, char *out, size_t outlen);

/* The All * helper's chips (docs/META.md), in BN6's own layouts: a folder
 * entry (chip | code << 9) in *, an empty one as it is; and a pack entry
 * (a count per code of the record's four, then four u16 order stamps), its
 * counts in the first code's, 99 at most: with * the record's one code,
 * the pack's every chip is counted there. */
#define CHIP_CODE_STAR 26
#define CHIP_PACK_ENTRY 12
uint16_t chip_entry_star(uint16_t entry);
void chip_pack_star(uint8_t e[CHIP_PACK_ENTRY]);
/* Whether a chip strikes an enemy outright: the record's lock-on setting
 * (0xF) is 1 for Cannon, the swords, bombs and the like, 0 for traps,
 * counters and the ones that need a stunned or paralysed enemy
 * (MchnSwrd), a key combination (VarSwrd) or a delay (GolmHit, TimeBom);
 * of those, the attack families (0xB) of the Navi chips, MachGun,
 * AquaNdl, WaveArm, AirSpin and the Dragons strike outright too. */
bool chip_direct(int rom_id);
/* A chip that strikes only beside MegaMan, by its attack family (0xB): the
 * swords (19), WindRack (63: "Blow enmy in front! Range 3", as WideSwrd
 * cuts) and MoonBld (64, the panels around him). */
bool chip_melee(int rom_id);
/* A chip's attack family (record 0xB): 50 AquaNdl, whose needles fall on
 * the panel it aimed at a moment later; 36 TankCan1-3 (chips 12-14), which
 * fire after a wind-up. */
#define CHIP_FAMILY_TANKCAN 36
#define CHIP_FAMILY_NAVI 27   /* the Navi chips (docs/ROM_DATA.md, chip records) */
int chip_family(int rom_id);
/* A Standard chip, by the record's library type (0x7; 1 Mega, 2 Giga, 3
 * Secret): a folder takes several of one (a Mega or Giga chip, one). */
bool chip_standard(int rom_id);

/* Virus families the encounters draw from. */
typedef struct {
	uint8_t family;     /* ROM virus family */
	uint8_t biome_mask; /* biomes where the family appears */
	uint8_t first_depth;
} VirusDef;

extern const VirusDef virus_defs[];
extern const int virus_def_count;

/* ROM enemy table: enemy id for (actor type, family, version). */
int enemy_id(int actor_type, int family, int version);
/* Enemy `id`'s element (ELEM_*: the top of its stats' HP word, else its
 * traits' second-wheel element), -1 when unknown. */
int enemy_element(int id);
/* The element that hits enemy `id` twice as hard (ELEM_*: its first-wheel
 * element's, else its traits' second-wheel weakness; 0 none), -1 when
 * unknown. */
int enemy_weakness(int id);
/* The element chip `rom_id` hits with (ELEM_*): its attack element, else
 * the second-wheel one its icon shows (a Sword did 160 of TenguMan's HP
 * for its 80); 0 none. */
int chip_hits_with(int rom_id);
/* A navi's chip at a version (0 V1, 1 EX, 2 SP); 0 for none. */
int navi_chip(int navi, int version);
/* The ROM's HP and attack damage of enemy `id` (bn6f enemy_getStruct2);
 * false when unknown. */
bool enemy_stats(int id, int *hp, int *damage);

#endif
