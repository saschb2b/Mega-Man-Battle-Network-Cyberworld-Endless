/* The NaviCust as a run's second build axis (docs/NAVICUST.md): the
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
	uint8_t program;   /* BN6's program number, 1-46 (given as program * 4) */
	uint8_t color;     /* 1-6; 0 until navicust_color picks one */
} NaviProgram;

/* The draft of a guardian at `depth`: three programs of three different
 * builds, each of a tier the act has reached (ROM-free; the rng decides). */
int navicust_draft(int depth, NaviProgram out[NAVICUST_DRAFT]);
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
 * 1-6 in out[k].color); fewer than three where not enough fit. Without a
 * ROM as navicust_draft, colours left 0. */
int navicust_draft_fitting(int depth, NaviProgram out[NAVICUST_DRAFT], const uint8_t *have, int nhave, int w, int h);
/* The shape of variant `variant` from the ROM; false without one. As it
 * stands once compressed by its code (`compressed`), the record's smaller
 * shape (issue #54); navicust_shape, as it comes. */
bool navicust_shape_as(int variant, bool compressed, NaviShape *out);
bool navicust_shape(int variant, NaviShape *out);
/* Whether `n` shapes fit a `w` x `h` board together without a bug: all on
 * it, none over another, the command line (the third row) under every
 * program part and no plus part, no two program parts of one colour side
 * by side; each turned as the NaviCust's L and R turn it. */
bool navicust_pack(const NaviShape *shapes, int n, int w, int h);
/* The colours whose programs turn: a bit per colour 1-6 (bit c - 1), the
 * NaviCust's Spins (key item 0x4F + colour, found in the net: docs/META.md).
 * navicust_pack turns only those; the others lie as their records draw
 * them. All six until set. */
void navicust_set_spins(unsigned mask);
/* The board's size after `expmemry` ExpMemry (0-2): 4x4, 5x4, 5x5. */
void navicust_board(int expmemry, int *w, int *h);
/* BugFrags a guardian's draft pays when none is taken, at `depth`. */
int navicust_skip_frags(int depth);
/* ExpMemry a guardian at `depth` gives (the board grows 4x4, 5x4, 5x5):
 * the second and fourth acts' guardians of the first cycle. */
bool navicust_expmemry(int depth);
/* MegaMan's words for the bug counts (one per type, as the game keeps
 * them): speaker-marked boxes for talk_start; "" for none. `after_run`:
 * said after the NaviCust's RUN, whose "OK" they answer; `cause`: why,
 * as navicust_bug_cause has it, or NULL for the rules in general. */
const char *navicust_bug_words(const uint8_t counts[NAVICUST_BUGS], bool after_run, const char *cause);
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
/* Which programs L and R turn, in MegaMan's words (one sentence): those of
 * the colours whose Spins are held (navicust_set_spins); for `variant`
 * (program * 4 + v, 0 for any), whether that one turns. */
const char *navicust_turn_words(int variant);
/* The same for a program of colour c (1-6): whether L and R turn it, by
 * the Spins held; "" for none. */
const char *navicust_color_turns(int c);
/* Colour `c`'s name (1-6: white, yellow, pink, red, blue, green), "" for
 * none. */
const char *navicust_color_name(int c);

#endif
