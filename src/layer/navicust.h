/* The NaviCust as a run's second build axis (docs/game/navicust.md): the
 * programs a run can be offered, by tier and build, the guardian's draft,
 * the ExpMemry milestones, and MegaMan's words for the bugs a layout
 * causes. Programs are BN6's: a program n is given as id n * 4 (EF 1B)
 * in one of its colours (1 White, 2 Yellow, 3 Pink, 4 Red, 5 Blue,
 * 6 Green). */
#ifndef CW_NAVICUST_H
#define CW_NAVICUST_H

#include <stdbool.h>
#include <stdint.h>

enum { BUILD_BUSTER, BUILD_HAND, BUILD_GUARD, BUILD_FIELD, BUILD_HP, BUILD_COUNT };

#define NAVICUST_DRAFT 3      /* programs a guardian's draft offers */
#define NAVICUST_BUGS  16     /* bug types the game counts */

typedef struct {
	uint8_t program;   /* BN6's program number, 1-46, or 48, BN5's SoulT+1 (given as program * 4) */
	uint8_t color;     /* 1-6; 0 until navicust_color picks one */
} NaviProgram;

/* The draft of a guardian at `depth`: three programs of three different
 * builds, each of a tier the act has reached (ROM-free; the rng decides). */
int navicust_draft(int depth, NaviProgram out[NAVICUST_DRAFT]);
/* Whether the programs held (`have`, `nhave` variants: program * 4 + v,
 * as run.programs) already set all that `program` would, so that it adds
 * nothing: a second SuprArmr, UnderSht or Shield, whose effects are set
 * and not added to; SuprArmr beside BodyPack; Attack+1 beside AttckMAX
 * (bn6f's compile). A Custom1 or an HP+50 always adds. */
bool navicust_adds_nothing(int program, const uint8_t *have, int nhave);
/* How many of `program` `have` holds, in any colour. */
int navicust_held(int program, const uint8_t *have, int nhave);
/* One of the program's colours, as the ROM's program records have them
 * (the rng picks among a plus part's three); 0 when the ROM has none. */
int navicust_color(int program);
/* What the program does, in MegaMan's words ("Custom1: one more chip each
 * turn."); NULL for a program outside the pool. */
const char *navicust_about(int program);
/* Whether `program` may ever be offered (the inert and the harmful are not). */
bool navicust_in_pool(int program);
/* Whether it may be offered at `depth`: in the pool, of a tier reached. */
bool navicust_offerable(int program, int depth);
/* The act a program may be offered from (the pool's tier), -1 outside it. */
int navicust_tier(int program);
/* The build a program belongs to (BUILD_*), -1 outside the pool. */
int navicust_build(int program);
/* A program's shape (one colour variant, program * 4 + v) as its record
 * draws it on a 7x7 grid, its colour and its kind: a program part needs a
 * block on the command line, a plus part keeps off it, either may. */
enum { NAVI_PART, NAVI_PLUS, NAVI_EITHER };
typedef struct {
	uint8_t cell[7][7];
	uint8_t kind;      /* NAVI_* */
	uint8_t color;     /* 1-6 */
} NaviShape;
/* The draft, of programs that fit a `w` x `h` board beside the `nhave`
 * variants on it (`have`), each in a colour whose variant fits (colour
 * 1-6 in out[k].color), none that adds nothing to them
 * (navicust_adds_nothing); fewer than three where not enough fit. Without
 * a ROM as navicust_draft, colours left 0. */
int navicust_draft_fitting(int depth, NaviProgram out[NAVICUST_DRAFT], const uint8_t *have, int nhave, int w, int h);
/* The shape of variant `variant` from the ROM; false without one. As it
 * stands once compressed by its code (`compressed`), the record's smaller
 * shape (issue #54); navicust_shape, as it comes. */
bool navicust_shape_as(int variant, bool compressed, NaviShape *out);
bool navicust_shape(int variant, NaviShape *out);
/* Variant `variant`'s part and colour as its record has them (0 a program
 * part, 1 a plus part, 2 either; 1-6): false for none, or one of no
 * colour. Program 48's from BN5's ROM (xprograms.h). */
bool navicust_part(int variant, int *kind, int *color);
/* Program `program`'s compression code, its ten buttons as the NaviCust
 * takes them (held RIGHT on it), five and five: "LBBRB AALRR" (issue #50);
 * false where it has none. navicust_code_text spells the table's ten bytes
 * so (ROM-free). */
#define NAVICUST_PROGRAMS 48   /* the code table's rows: programs 0-47 */
#define NAVICUST_VARIANTS (49 * 4)   /* the colour variants a board or the PET can hold: BN6's programs 1-46, and 48, BN5's SoulT+1 where its ROM is read (xprograms.h) */
bool navicust_code(int program, char out[12]);
bool navicust_code_text(const uint8_t raw[10], char out[12]);
/* Whether `n` shapes fit a `w` x `h` board together without a bug: all on
 * it, none over another, the command line (the third row) under every
 * program part and no plus part, no two program parts of one colour side
 * by side; each turned as the NaviCust's L and R turn it. */
bool navicust_pack(const NaviShape *shapes, int n, int w, int h);
/* The colours whose programs turn: a bit per colour 1-6 (bit c - 1), the
 * NaviCust's Spins (key item 0x4F + colour, found in the net: docs/game/meta/).
 * navicust_pack turns only those; the others lie as their records draw
 * them. All six until set. */
void navicust_set_spins(unsigned mask);
/* ... and the Spins set so (navicust_words.c, what turns) */
unsigned navicust_spins(void);
/* The older Net's program (docs/game/navicust.md, 11): SoulT+1, program 48,
 * which BN5's ROM brings (xprograms.h). In the pool only while `held`: the
 * run holds a Soul as its layer is made, or the layer's guardian gives
 * his, and the core has the program; `lead`: the layer's guardian gives a
 * Soul the run has none of yet, and his draft offers SoulT+1 where it fits
 * and is not held, in the place of its build's pick. Neither until set. */
void navicust_set_soul(bool held, bool lead);
/* The board's size after `expmemry` ExpMemry (0-2): 4x4, 5x4, 5x5. */
void navicust_board(int expmemry, int *w, int *h);
/* BugFrags a guardian's draft pays when none is taken, at `depth`. */
int navicust_skip_frags(int depth);
/* ExpMemry a guardian at `depth` gives (the board grows 4x4, 5x4, 5x5):
 * the second and fourth acts' guardians of the first cycle. */
bool navicust_expmemry(int depth);
/* MegaMan's words for the bug counts (one per type, as the game keeps
 * them): speaker-marked boxes for talk_start; "" for none. */
const char *navicust_bug_words(const uint8_t counts[NAVICUST_BUGS]);
/* A program on BN6's board: its name, kind (0 a program part, 1 a plus
 * part, as its record has them) and colour (1-6). */
typedef struct { const char *name; int kind, color; } NaviPart;
#define NAVICUST_GRID 7   /* BN6's board: a 7x7 grid, the board in it from (1, 1), the command line its row 3 */
/* Why the board bugs, in MegaMan's words ("HP+100 is a plus part on the
 * command line."), from BN6's grid (each cell its program's index in
 * `parts` + 1) on a `w` x `h` board: a plus part on the command line, a
 * program part off it, a part past the board's edge, two programs of one
 * colour side by side; NULL where none of them is. */
const char *navicust_bug_cause(const uint8_t grid[NAVICUST_GRID * NAVICUST_GRID], const NaviPart *parts, int n, int w, int h);
/* Whether `shape` fits the board's free cells as its programs stand (BN6's
 * grid, as navicust_bug_cause reads it): on the board, over none, a
 * program part on the command line and a plus part off it, beside no
 * program part of its colour, turned only with its colour's Spin. (A
 * playtester's Guardian Data program fitted only after he moved two, and
 * nothing had said so.) */
bool navicust_fits_free(const uint8_t grid[NAVICUST_GRID * NAVICUST_GRID], const NaviPart *parts, int n, const NaviShape *shape, int w, int h);
/* Colour `c`'s name (1-6: white, yellow, pink, red, blue, green), "" for
 * none. */
const char *navicust_color_name(int c);

#endif
