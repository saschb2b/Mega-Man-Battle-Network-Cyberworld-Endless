/* What a battle holds: the foes on the field and the field itself. */
#ifndef CW_FOES_H
#define CW_FOES_H

#include <stdbool.h>
#include <stdint.h>

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

/* A field object's kind, the entity list's high nibble (docs/rom/) */
enum { FIELD_GEM = 2, FIELD_ROCK = 3, FIELD_CUBE = 8, FIELD_STATUE = 9, FIELD_METAL = 10 };

typedef struct {
	Foe foes[MAX_FOES];
	int nfoes;
	int biome;
	bool boss;
	bool held;       /* no running from it: a guardian's, as BN6's story bosses */
	int song;        /* its music where BN6's own record names one (a super boss's), 0 the battle's or boss's theme */
	bool map_backdrop;   /* the background of the map it is fought on, as BN6's story records ask (0xFF), not its area's roll */
	int field;       /* the BattleSettings battlefield: the panels' layout (0 plain) */
	int player;      /* MegaMan's panel on it (row << 4 | column, from 1); 0 column 2 row 2 */
	int sides;       /* whose each column is (the BattleSettings' +6, BN6_SIDES_*): 0 the usual three and three */
	/* its objects as the area's battle has them: the entity's kind byte,
	 * its panel (row << 4 | column, from 1) and its two bytes (a cube's
	 * kind, a Mystery Data's reward row) */
	struct { int kind, panel, arg; } obj[MAX_FIELD_OBJS];
	int nobj;
	/* the area's battle it was rolled from, kept in mind once fought
	 * (loot_battle_fought): the area, the battle (-1 none), its virus
	 * families (a bit each, by BN6's AI index: the families moved into the
	 * engine from 32) and its viruses */
	int from_biome, from_pick;
	uint64_t from_families;
	unsigned from_viruses;
	/* another game's battle fought in BN6's engine (xbattles.h, issue #82):
	 * its record there (0: BN6's own), and the foe that came from the
	 * other net into it (1 + its index, 0 none: docs/multirom/one-net.md) */
	uint32_t xrecord;
	int visitor;
	/* its Mystery Data's finds its own (its game's rows, as BN6's names
	 * them: xbattles.c), not the run's roll (loot_gem_rewards): the eight
	 * entries the battle draws one of (bits 14-15: 0 a chip, its id and
	 * code << 9; 1 zenny; 3 BugFrags) */
	bool gem_own;
	uint16_t gem_row[8];
} Encounter;

#endif
