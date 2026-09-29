/* What a battle holds: the foes on the field and the field itself. */
#ifndef CW_FOES_H
#define CW_FOES_H

#include <stdbool.h>

enum { FOE_VIRUS, FOE_NAVI, FOE_ROCK };

typedef struct {
	int kind;
	int family;  /* virus family / navi index */
	int version; /* 0 V1, 1 V2, 2 V3, 3 SP, 4-5 rare */
	int col, row;
	int id;      /* the ROM's enemy id when known (rocks and cubes too), else -1 */
} Foe;

#define MAX_FOES 6
#define MAX_FIELD_OBJS 4

/* A field object's kind, the entity list's high nibble (docs/ROM_DATA.md) */
enum { FIELD_GEM = 2, FIELD_ROCK = 3, FIELD_CUBE = 8, FIELD_STATUE = 9, FIELD_METAL = 10 };

typedef struct {
	Foe foes[MAX_FOES];
	int nfoes;
	int biome;
	bool boss;
	bool held;       /* no running from it: a guardian's, as BN6's story bosses */
	int field;       /* the BattleSettings battlefield: the panels' layout (0 plain) */
	int player;      /* MegaMan's panel on it (row << 4 | column, from 1); 0 column 2 row 2 */
	/* its objects as the area's battle has them: the entity's kind byte,
	 * its panel (row << 4 | column, from 1) and its two bytes (a cube's
	 * kind, a Mystery Data's reward row) */
	struct { int kind, panel, arg; } obj[MAX_FIELD_OBJS];
	int nobj;
	/* the area's battle it was rolled from, kept in mind once fought
	 * (loot_battle_fought): the area, the battle (-1 none), its virus
	 * families and its viruses */
	int from_biome, from_pick;
	unsigned from_families, from_viruses;
} Encounter;

#endif
