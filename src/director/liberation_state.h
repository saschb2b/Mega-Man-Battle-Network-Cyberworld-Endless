/* A liberation's state (liberation.c), shared by its drawing
 * (liberation_draw.c) and its words (liberation_words.c). */
#ifndef CW_LIBERATION_STATE_H
#define CW_LIBERATION_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "liberation_board.h"

/* The board's course */
enum {
	LIB_STEP_WAIT,     /* the arrival's card and words first */
	LIB_STEP_BANNER,   /* a phase's banner */
	LIB_STEP_MOVE,     /* MegaMan's move: he walks the clean panels as BN6 walks him */
	LIB_STEP_MENU,     /* ... and his command */
	LIB_STEP_BATTLE,   /* a liberation's battle */
	LIB_STEP_ENEMY,    /* the guardians' and the Darkloid's phase */
	LIB_STEP_TALK,     /* words said, then `after` */
	LIB_STEP_PRIZE,    /* the par's prize given */
	LIB_STEP_DONE,     /* liberated: the exit open, MegaMan walks as ever */
};
/* The commands (BN5's names) */
enum { LIB_CMD_LIBERATE, LIB_CMD_LONGSWRD, LIB_CMD_PASS, LIB_CMDS };
/* The banners after a liberation's battle (liberation_pieces.c), and the
 * frames from its end: the board faded back in first (BN5's 27), then the
 * banner squashed in and out (BN5's 58, traced) */
enum { LIB_CHEER_NONE, LIB_CHEER_SUCCESS, LIB_CHEER_ONE_TURN };
#define LIB_CHEER_WAIT   27
#define LIB_CHEER_FRAMES 58
#define LIB_POPS 4
/* What a battle liberates */
enum { LIB_FIGHT_PANEL, LIB_FIGHT_GUARDIAN, LIB_FIGHT_HOLE, LIB_FIGHT_DARKLOID };

typedef struct {
	bool on;               /* a dark layer's board, made */
	LibBoard b;
	int step, after;       /* LIB_STEP_*; after a talk, the step next */
	int phase;             /* 1 on */
	int op;                /* Order Points */
	int mx, my;            /* MegaMan's panel */
	int face;              /* the grid way he faces (0 +x, 1 +y, 2 -x, 3 -y) */
	int tx, ty;            /* the panel beside his he faces, -1 none */
	uint32_t held;         /* the pad last frame */
	int menu;              /* the command under the cursor */
	bool can[LIB_CMDS];    /* ... which can be taken as he stands */
	int banner_t;          /* frames the banner has shown */
	bool enemy_banner;     /* ... it is the Darkloid's phase's */
	/* the battle under way */
	int fight, fight_unit, fight_cmd;
	int fx, fy;            /* the panel fought for */
	bool fight_seen;       /* the battle began */
	/* the enemy's phase */
	int enemy_unit, enemy_t, enemy_steps;
	int hurt;              /* HP the board took this phase */
	char hurt_by[24];      /* ... the last who hit */
	bool shield;           /* a BonusPanel's: no harm on the board this phase */
	/* where each unit's sprite is (its object), and where it shows */
	uint8_t fence[MAP_H][MAP_W];   /* 1: the panel has walls of its own (liberation_fence.c), shut or open as it is walked */
	int slot[LIB_UNITS];
	int back_in[LIB_UNITS];  /* a deleted guardian's Darkloids' phases till his DarkHole sends him back */
	int ux[LIB_UNITS], uy[LIB_UNITS];   /* world units, gliding */
	int from_x[LIB_UNITS], from_y[LIB_UNITS];   /* ... and where the map puts its object */
	int darkloid;          /* the Darkloid's navi (guardians.h) */
	int tour, tour_t;      /* the opening's look at the Darkloid (1 there, 2 back) and its frames there */
	bool briefed;          /* MegaMan's word on the board, said */
	bool holes_told;       /* ... that every DarkHole is gone */
	char words[512];       /* words due ("" none) */
	int cheer, cheer_t;    /* the banner after a won battle (LIB_CHEER_*), frames since */
	struct { int8_t x, y, kind; int t; } pop[LIB_POPS];   /* items freed: BN5's kind (BN5_ITEM_*), shown over their panel */
	int npops;             /* ... while the words of them run */
	int prize;             /* the par's prize: 0 two under, 1 at par, 2 over */
	bool prize_given;
} LibState;

extern LibState LB;

/* (liberation.c) The panel MegaMan faces, and the unit on panel (x, y), -1 none */
void lib_faced(int *x, int *y);
/* ... and whether A there takes a command */
bool lib_faced_takes(void);
int lib_unit_at(int x, int y);

#endif
