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
	LIB_STEP_CLEAR,    /* the Darkloid deleted: BN5's SUCCESS! and its song, then the tally (its value after the
	                    * others', as a CONTINUE's note keeps the step) */
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
#define LIB_FINDS 4
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
	/* the Darkloids' phase: the units in their order (the Darkloid first),
	 * and the step of the one acting (liberation_turn.c) and its frames */
	int enemy_unit, enemy_t, enemy_steps;
	bool shield;           /* a BonusPanel's: no harm on the board this phase */
	/* ... as BN5 shows it, not kept: */
	bool hidden[LIB_UNITS];        /* a guardian between its warp out and in */
	uint8_t anim[LIB_UNITS];       /* each unit's animation (its object's byte, LayerObjs.lib_anim) */
	uint8_t reach[MAP_H][MAP_W];   /* 1: in the acting unit's reach, */
	bool reach_lit;                /* ... lit in BN5's red (liberation_look.c) */
	struct { int sprite, anim, x, y, n; } effect;   /* an effect over the board: BN5's list-7 sprite (-1 none) and its animation
	                                             * at world (x, y), the n-th begun */
	int harm, harm_t;              /* a blow's harm in BN5's digits over MegaMan (0 none), frames shown */
	/* where each unit's sprite is (its object), and where it shows */
	uint8_t fence[MAP_H][MAP_W];   /* 1: the panel has walls of its own (liberation_fence.c), shut or open as it is walked */
	int slot[LIB_UNITS];
	int back_in[LIB_UNITS];  /* a deleted guardian's Darkloids' phases till his DarkHole sends him back */
	int ux[LIB_UNITS], uy[LIB_UNITS];   /* world units, gliding */
	int from_x[LIB_UNITS], from_y[LIB_UNITS];   /* ... and where the map puts its object */
	int darkloid;          /* the Darkloid's navi (guardians.h) */
	int tour, tour_t;      /* the opening's look at the Darkloid (1 there, 2 back) and its frames there */
	bool briefed;          /* MegaMan's word on the board, said */
	bool met;              /* the Darkloid's word at his first turn, said (not kept: phase 1's alone) */
	bool holes_told;       /* ... that every DarkHole is gone */
	char words[512];       /* words due ("" none) */
	int cheer, cheer_t;    /* the banner after a won battle (LIB_CHEER_*), frames since */
	struct { int8_t x, y, kind; int t; } pop[LIB_POPS];   /* items freed: BN5's kind (BN5_ITEM_*), shown over their panel */
	int npops;             /* ... while the words of them run */
	int prize;             /* the par's prize: 0 two under, 1 at par, 2 over */
	bool prize_given;
	int finds[LIB_FINDS];  /* chips' and zenny's finds due: their scripts (layer_objs.c's) */
	int nfinds;
	bool route_found;      /* the sealed region's route taken (lib_route_found) */
} LibState;

extern LibState LB;

/* (liberation.c) The panel MegaMan faces, and the unit on panel (x, y), -1 none */
void lib_faced(int *x, int *y);
/* ... nothing else showing (no chat box, card or scene); MegaMan's HP,
 * and set (1 at least, his max at most) */
bool lib_quiet(void);
int lib_hp(void);
void lib_hp_set(int hp);
/* (liberation_turn.c) Unit u's turn in the Darkloids' phase as BN5 plays
 * it, a frame at a time: true once over */
bool lib_turn(int u);
/* ... and whether A there takes a command */
bool lib_faced_takes(void);
int lib_unit_at(int x, int y);

#endif
