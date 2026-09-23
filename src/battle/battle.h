/* Real-time grid battle: MegaMan against viruses or a navi on a 6x3 field. */
#ifndef BATTLE_H
#define BATTLE_H

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

enum { BATTLE_WIN, BATTLE_LOSE, BATTLE_ESCAPE };

typedef struct {
	int chip;         /* -1 for zenny */
	char code;
	int zenny;
} RewardOption;

typedef struct {
	int outcome;
	int busting;      /* 1-10 */
	int frames;
	int hp_left;
	bool took_reward;
	RewardOption reward;
} BattleResult;

/* Fills up to 4 options for the RESULT window's GET DATA slot. */
typedef void (*RewardMaker)(int busting, RewardOption *opts, int *n);

void battle_begin(const Encounter *e, void (*done)(const BattleResult *r));
void battle_set_rewards(RewardMaker make);

/* The HP box and the 8x16 battle font, which the net's HUD shares. */
void battle_hp_box(int x, int y, int hp);
void battle_area_name(int x, int y, const char *s);

#endif
