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
} Foe;

typedef struct {
	Foe foes[4];
	int nfoes;
	int biome;
	bool boss;
	bool no_escape;
	int field;       /* panel preset: 0 plain, 1 cracked, 2 grass, 3 poison, 4 holy, 5 ice */
} Encounter;

#endif
