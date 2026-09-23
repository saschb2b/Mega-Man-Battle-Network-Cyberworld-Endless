/* Chip and enemy definitions. Names, power, codes, element, HP and art come
 * from the ROM; behavior is implemented by the engine and chosen here. */
#ifndef DATA_H
#define DATA_H

#include <stdbool.h>
#include <stdint.h>

enum { ELEM_NULL, ELEM_FIRE, ELEM_AQUA, ELEM_ELEC, ELEM_WOOD };

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
	int chip_element; /* ROM icon element, used for colours */
	char codes[5];    /* letters, '*' wildcard */
	int ncodes;
} ChipInfo;

extern const ChipDef chip_defs[];
extern const int chip_def_count;
const ChipDef *chip_def(int rom_id);

void chip_info(int rom_id, ChipInfo *out);

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

#endif
