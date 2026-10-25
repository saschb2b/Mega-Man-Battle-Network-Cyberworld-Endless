/* A liberation's board (docs/LIBERATION.md, Building it: stage 1): the
 * dark layer's own floor taken at 32 world units a panel, one layer cell a
 * panel (the map's, as built), its kinds placed by BN5's rules as
 * tools/liberation.py prototypes them, from the layer's seed and floor
 * alone (no ROM, no game RAM: a checkpoint rebuilds it as it rebuilds the
 * layer and its map). Each act brings one
 * rule of BN5's more (the staircase), and the board is kept only where the
 * planner (liberation_plan.h) finds it a set of real decisions
 * (docs/LIBERATION.md, The quality bar). */
#ifndef CW_LIBERATION_BOARD_H
#define CW_LIBERATION_BOARD_H

#include <stdbool.h>
#include <stdint.h>

#include "net.h"

/* A panel's kind (BN5's, src/emu/bn5_liberation.h's BN5_PANEL_*: the
 * same few, numbered here), LIB_VOID off the board. LIB_BAR is a
 * BarrierPanel (its number in `item`), LIB_TRAP a dark panel that hurts
 * as it is liberated, shown as such (BN5 hides its traps in item panels;
 * the quality bar's honest telegraph shows them). */
enum { LIB_VOID, LIB_CLEAN, LIB_DARK, LIB_ITEM, LIB_BONUS, LIB_HOLE, LIB_RING, LIB_BOSS, LIB_BAR, LIB_TRAP, LIB_KINDS };
/* What an item panel holds (BN5's item kinds), its amount in the high
 * nibble: Order Points (2-3), a heart, a barrier's key (its number),
 * zenny (hundreds), a chip (its loot index, layer_objs.c's script), the
 * sealed region's route */
enum { LIB_GOT_OP, LIB_GOT_HEART, LIB_GOT_KEY, LIB_GOT_ZENNY, LIB_GOT_CHIP, LIB_GOT_ROUTE };
/* The staircase: the rules an act's board asks about, one more an act
 * (docs/LIBERATION.md, The quality bar, 3). Act 2 has the basics (dark
 * panels, DarkHoles, the Darkloid, Order Points and hearts). */
enum {
	LIB_RULE_BARRIERS = 1,   /* act 3: a BarrierPanel and its key, the sealed region's route behind it */
	LIB_RULE_REACH = 2,      /* act 4: roaming guardians and their reach; barriers across the way */
	LIB_RULE_TOLLS = 4,      /* act 5: the board fights back: trap panels, Bladia darkening */
	LIB_RULE_MAZE = 8,       /* act 6: barriers in a chain, each key behind the barrier before */
};
/* The units BN5 sets on a board: the Darkloid, and the guardians of its
 * DarkHoles (BN5's BigBrute, TinHawk and Bladia: their rules, not their
 * looks, docs/LIBERATION.md) */
enum { UNIT_DARKLOID, UNIT_BRUTE, UNIT_HAWK, UNIT_BLADIA };
#define LIB_UNITS 8
#define LIB_HOLES 5
#define LIB_BARS  3
#define LIB_LOOT  6      /* chip and zenny items a board at most */
#define LIB_BONUS_CHIPS 4   /* the chips a board's BonusPanels give, one of them each (BN5's four a mission) */
/* Order Points: BN5's, 3 at the start and 8 at most (BN5_LIB_OP_ADD) */
#define LIB_OP_START 3
#define LIB_OP_MAX   8
/* BN5's Pass gives a Navi this part of his max HP back (romlab, mission 2:
 * MegaMan 32 to 52 of 100, Colonel 32 to 72 of 200) */
#define LIB_REST_PART 5

/* The team (docs/LIBERATION.md, The team): MegaMan first, then the Navis
 * whose BN5 Souls the run holds, at most BN5's team of five. Each by BN5's
 * id (a team record's +5, bn5_liberation.h's BN5_LIB_NAVI_*), which is his
 * Soul's number too (souls.h: bit k the Soul of id 7 + k) */
#define LIB_TEAM 5
enum { MATE_MEGAMAN = 0, MATE_COLONEL = 7, MATE_SHADOWMAN, MATE_NUMBERMAN, MATE_TOMAHAWKMAN, MATE_KNIGHTMAN, MATE_TOADMAN };
typedef struct {
	int n;                      /* 1 to LIB_TEAM: MegaMan and his teammates */
	uint8_t navi[LIB_TEAM];     /* MATE_*, MegaMan first */
	int16_t hp[LIB_TEAM];       /* each one's max HP on the board (MegaMan's 0: the act's, liberation_plan.c's) */
} LibTeam;
/* A Navi of the team where the board stands: his panel, and whether he
 * is out (deleted in a battle: BN5's Navi leaves the mission) */
typedef struct { uint8_t x, y; bool out; } LibMate;

typedef struct {
	uint8_t kind;      /* UNIT_* */
	uint8_t x, y;      /* its panel */
	int8_t hole;       /* the DarkHole it guards, -1 a roamer or the Darkloid */
	bool gone;         /* deleted */
} LibUnit;

/* What the planner found playing the board (liberation_plan.c), the
 * quality bar's measures: over its plays */
typedef struct {
	int plays, wins;
	int median, p25, p75;            /* phases to liberate it, the wins' */
	int phases, dead, strikes;       /* the plays' phases; those with one sensible move on the way; strikes at a DarkHole or the Darkloid */
	int moves;                       /* sensible moves over every phase (decisions per phase: moves / phases) */
	int best[4];                     /* the best move by command: Liberation, a Navi's command (LongSwrd ...), Pass, a fight */
	int longswrd;                    /* the Navis' commands taken (LongSwrd and the teammates'), over the plays */
	int out;                         /* teammates deleted, over the plays */
	int orders;                      /* distinct orders the DarkHoles were taken in */
	int sealed, route;               /* the route's plays that took it, and their median phases */
	int bare_dead, bare_phases;      /* ... the board with its rewards stripped */
	int hurt;                        /* HP the board's units and traps took, over the plays */
} LibMeasure;

typedef struct {
	int act;                        /* 0-based, the layer's */
	int rules;                      /* LIB_RULE_*: the act's staircase */
	uint8_t kind[MAP_H][MAP_W];     /* LIB_* */
	uint8_t item[MAP_H][MAP_W];     /* an item panel's: LIB_GOT_* | amount << 4; a barrier's number */
	int start_x, start_y;           /* MegaMan's first panel (the arrival) */
	LibTeam team;                   /* the team it was made for */
	LibMate mate[LIB_TEAM];         /* ... where each stands (their first panels by the start, the course's after) */
	int boss_x, boss_y;             /* the Darkloid's panel, the middle of his 3x3 */
	int nholes;
	uint8_t hole_x[LIB_HOLES], hole_y[LIB_HOLES];
	int nunits;
	LibUnit unit[LIB_UNITS];        /* the Darkloid first */
	int nbars;                      /* barriers, numbered 1 to nbars */
	int sealed;                     /* the sealed region's barrier's number, 0 none */
	int route_x, route_y;           /* its route's item panel, -1 none */
	int nloot;                      /* chip and zenny items: their loot indexes 0 to nloot - 1 */
	int traps;
	int panels, dark;               /* the board's panels, and those dark as it begins (items and rings among them) */
	int par;                        /* phases: the planner's (lib_plan) */
	int tries;                      /* the seeds it took (a board rejected tries the next) */
	int why;                        /* the last try's rejection (LIB_WHY_*) */
	uint8_t rejected[16];           /* ... each kind's tries rejected (LIB_WHY_*: the measures') */
	LibMeasure m;
} LibBoard;

/* Why a try was rejected (the measures count them) */
enum { LIB_WHY_NONE, LIB_WHY_FLOOR, LIB_WHY_DARKLOID, LIB_WHY_HOLES, LIB_WHY_BARRIER, LIB_WHY_TRAPS, LIB_WHY_FAIR, LIB_WHY_UNWON,
	LIB_WHY_DEAD, LIB_WHY_BARE, LIB_WHY_ROUTE, LIB_WHY_LONGSWRD, LIB_WHYS };

/* The guardians' and the Darkloid's rules on the board, by act (BN5's
 * guides' numbers for its missions, tools/liberation.py's GUARDS): steps
 * over the dark a phase, reach, whether it hits every Navi in reach, its
 * damage at act `act` (0-based); a trap's */
typedef struct { int move, reach; bool all; } LibGuardRule;
const LibGuardRule *lib_guard_rule(int kind);
int lib_harm(int kind, int act);
int lib_darkloid_reach(int act);
int lib_trap_harm(int act);
/* A zenny item's zenny in act `act` (BN5's amount times 100, the act's more) */
int lib_zenny(int act);
/* A board cleared over par: its zenny at `depth` (BN5's 1000 at the
 * second act, 250 more an act, by the plan's zenny curve) */
int lib_par_zenny(int depth);
/* Guardian u's step toward (x, y) in the Darkloids' phase (Bladia's
 * darkening the clean panel it lands on): false where none brings it
 * nearer */
bool lib_guardian_step(LibBoard *b, int u, int x, int y);
/* A deleted guardian's return as BN5's (BN5_LIB_RETURN), at the
 * Darkloids' phase's end: DarkHole h still standing, MegaMan at (mx, my)
 * within three panels of it on both axes, fewer than eight units standing
 * and fewer than three of the hole's own: one of its deleted guardians
 * (BN5's pick at random among them; a board's hole has one), on the free
 * dark panel of the eight round the hole nearest MegaMan, BN5's order
 * breaking a tie. The guardian, its panel into (*x, *y); -1 none. */
int lib_guardian_return(const LibBoard *b, int h, int mx, int my, int *x, int *y);

/* Whether panel (x, y) is floor MegaMan walks at the ground's height: the
 * director's reads the layer's map as built (netmap's floor, after legal.c
 * made it drawable), so every way of the board is a way of the map's */
typedef bool (*LibFloor)(int x, int y);
/* The board for the layer just generated (net.h's `layer`, a dark one) in
 * act `act` (0-based), for `team` (NULL: MegaMan alone), from `seed`, on
 * the panels `floor` says (NULL: the layer's flat floor cells as
 * generated, a test's): its kinds, DarkHoles, units, the team's first
 * panels and par; false where no seed of a few gives one that keeps BN5's
 * guarantees and the quality bar (the layer is then a plain one). */
bool lib_board_make(LibBoard *b, uint32_t seed, int act, const LibTeam *team, LibFloor floor);
/* The Navi of the team on panel (x, y), -1 none (an out one is none) */
int lib_mate_at(const LibBoard *b, int x, int y);
/* A Navi's command (BN5's: a team record's command by his id), and its
 * panels: those a command taken on panel (x, y), facing grid way `dir`
 * from (sx, sy), frees or searches, into xs/ys (at most 6); how many */
enum { LIB_DO_NONE, LIB_DO_LONGSWRD, LIB_DO_SCRENDIV, LIB_DO_TOMAHAWK, LIB_DO_NUMBER, LIB_DO_SNEAK, LIB_DO_MELODY };
int lib_command_of(int navi);
int lib_command_panels(const LibBoard *b, int command, int x, int y, int dir, int xs[6], int ys[6]);
/* The shield of KnightMan's StoneBody: no harm on the board to him or a
 * Navi beside him (BN5's guides) */
bool lib_mate_shielded(const LibBoard *b, int k);
/* Whether panel (x, y) is on the board and walked (clean: BN5's units
 * move over clean panels alone). */
bool lib_walkable(const LibBoard *b, int x, int y);
/* Whether panel (x, y) can be liberated as the board stands: dark, an
 * item's, a trap, a DarkHole; the Darkloid's ring once every DarkHole is
 * gone. */
bool lib_liberable(const LibBoard *b, int x, int y);
/* Whether kind k is dark (counts round a panel, guardians walk it) */
bool lib_dark_kind(int k);
/* The dark panels around (x, y) (its eight), which make its battle harder. */
int lib_dark_around(const LibBoard *b, int x, int y);
/* The Darkloid's HP after his heal in his phase, as BN5's (its ROM,
 * BN5_LIB_HEAL): below his max, a tenth of it for each dark panel round
 * him (lib_dark_kind's, BN5's weights), his max at most */
int lib_darkloid_heal(const LibBoard *b, int hp, int max);
/* The DarkHoles still standing. */
int lib_holes_left(const LibBoard *b);
/* A key taken: barrier n's panels open (clean); how many */
int lib_open_barrier(LibBoard *b, int n);
/* Per panel, the dark panels still to liberate from it to the nearest
 * target (the DarkHoles standing, else the Darkloid; with `keys`, the keys
 * of barriers that wall a target off as well): a 0-1 walk, barriers shut,
 * into cost (0x7F7F unreached) */
void lib_cost_map(const LibBoard *b, int16_t cost[MAP_H][MAP_W], bool keys);
/* The HP the board's units would take off a Navi standing on (x, y) in the
 * Darkloids' phase next, as their rules move them (the telegraph: their
 * reach and next step, docs/LIBERATION.md, The quality bar, 4) */
int lib_threat(const LibBoard *b, int x, int y);

#endif
