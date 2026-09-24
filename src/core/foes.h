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

typedef struct {
	Foe foes[MAX_FOES];
	int nfoes;
	int biome;
	bool boss;
	bool no_escape;
	int field;       /* the BattleSettings battlefield: the panels' layout (0 plain) */
} Encounter;

#endif
