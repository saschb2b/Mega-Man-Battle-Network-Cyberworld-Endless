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

typedef struct {
	uint16_t result;
	uint8_t mode;
	uint16_t parts[3];
} ProgramAdvance;
extern const ProgramAdvance program_advances[];
extern const int program_advance_count;
void chip_info(int rom_id, ChipInfo *out);
/* The game's own three-line description, lines separated by '\n'. */
void chip_desc(int rom_id, char *out, int len);

/* Virus families and navis. */
typedef enum {
	AI_METTAUR,   /* follow row, send a shockwave */
	AI_SHOOTER,   /* follow row, fire a projectile */
	AI_SWORDY,    /* warp in front of MegaMan and slash */
	AI_BEAM,      /* wait until aligned, beam the whole row */
	AI_PUNCHER,   /* warp adjacent and punch */
	AI_LOBBER,    /* lob a shell at MegaMan's panel */
	AI_GUNNER,    /* crosshair tracks MegaMan, then rapid fire */
	AI_ROLLER,    /* roll across the row */
	AI_QUAKER,    /* jump and shake the field, cracking panels */
	AI_NAVI,      /* boss pattern, see battle.c */
} AiKind;

typedef struct {
	uint8_t ai_index;   /* ROM virus family = battle sprite index */
	uint8_t ai;
	int8_t anim_idle, anim_move, anim_attack, anim_hit;
	int8_t fx_cat, fx_idx, fx_anim; /* projectile or effect sprite */
	uint8_t biome_mask; /* biomes where the family appears */
	uint8_t first_depth;
} VirusDef;

extern const VirusDef virus_defs[];
extern const int virus_def_count;

typedef struct {
	uint8_t ai_index;   /* navi index = sprite index = name index */
	int8_t anim_idle, anim_move, anim_hit;
	int8_t attack_anim[3];
	uint8_t attack_kind[3];
	int8_t fx_cat[3], fx_idx[3], fx_anim[3];
	uint16_t chip_reward;
	const char *cross;  /* style gained for the run, or NULL */
} NaviDef;

enum { NA_SHOT, NA_TARGET, NA_DASH, NA_COLUMN, NA_WAVE, NA_SUMMON, NA_ROWBLAST };

extern const NaviDef navi_defs[];
extern const int navi_def_count;
const NaviDef *navi_def(int ai_index);

/* ROM enemy table: enemy id for (actor type, family, version). */
int enemy_id(int actor_type, int family, int version);
int enemy_hp(int id);
int enemy_element(int id);
int enemy_attack(int id);
void enemy_name(int id, char *out, int len);
void navi_name(int navi, char *out, int len);

#endif
