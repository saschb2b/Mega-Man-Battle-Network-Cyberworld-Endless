/* The second screen's words (second_text.c): what the PET says of the
 * screen beside it, from the run's facts and BN6's own tutorials. */
#ifndef CW_SECOND_TEXT_H
#define CW_SECOND_TEXT_H

#include <stdbool.h>
#include <stddef.h>

#include "director.h"   /* DirectorWay */
#include "second_state.h"

/* A Cross's card in CROSSSELECT (navi 1-5): what hits him twice as hard,
 * what it gives, what it does to Navis; lines apart by '|', into `out` */
void second_cross_lines(int navi, char *out, size_t n);
/* Beast Out's card on its emblem: the turns the EmotionCounter leaves, or
 * MegaMan tired (`turns` 0), and what the Cybeast gives; lines apart by
 * '|', into `out` */
void second_beast_lines(int turns, char *out, size_t n);

/* One of MegaMan's states while he fights: its name and a line */
typedef struct { char name[24], line[40]; } SecondStateLine;
/* His Cross (`form` as BN6_BATTLE_FORM), Beast Out with the turns left,
 * BeastOver, tired (`beast` his and the EmotionCounter, `turns`, at 0),
 * Full Synchro (`synchro`): how many of `most` it wrote into `out` */
int second_state_lines(int form, int turns, bool beast, bool synchro, SecondStateLine *out, int most);

/* The NaviCust program the NaviCustomizer's cursor is on (s->nc_*): its
 * colour and kind and where that may go, the copies left or where it is,
 * whether it fits the free cells as the board stands, whether L and R
 * turn it; lines apart by '|', into `out` */
void second_program_lines(const SecondState *s, char *out, size_t n);
/* What a program does, in MegaMan's words (navicust_about's, its name
 * left off), "" for none */
const char *second_program_does(int program);
/* RUN as the board stands: no bug, or a bug (its cause beside it) */
const char *second_run_line(bool bug);
/* The board's rules, in a line */
const char *second_board_rules(void);

/* Home's next step where Lan or MegaMan stands (docs/HOME.md): in Lan's
 * HP the pink pad, in his room his PC, in his house and home's town the PC
 * up in his room, in a town not home its port (`landmark_at`, "bird statue
 * on the plaza"); into `out` */
enum { SECOND_HOME_HP, SECOND_HOME_ROOM, SECOND_HOME_UP, SECOND_HOME_PORT };
void second_home_next(int where, const char *landmark_at, char *out, size_t n);
/* The run's setup in lines ("Short net, threat 2|Standard folder,
 * HeatCross"), into `out` */
void second_home_setup(char *out, size_t n);
/* The Net's clock (docs/HOME.md, going back): its notches and what they
 * cost, empty before the first trip back; into `out` */
void second_home_clock(char *out, size_t n);
/* The ways lit at home (director_home_ways), a line each: the pink pad's
 * area and guardian, a link's, an older portal's back to an area won (a
 * guardian named where MegaMan knows him); into `out` */
void second_home_ways(const DirectorWay *ways, int n, char *out, size_t size);
/* The request held (run.job, jobs.h): who asked, what, and how far along;
 * empty for none; into `out` */
void second_home_job(char *out, size_t n);
/* The town's hour (director_town_hour: 0 morning .. 3 night) as a word. */
const char *second_hour_name(int hour);
/* A row of the PET's: its name and its value */
typedef struct { const char *name; char value[24]; } SecondRow;
/* The profile's record: how many rows of `most` */
int second_record(SecondRow *out, int most);
/* The run's record for MegaMan's status: the layer, the guardians
 * beaten, the viruses deleted, the max HP and its parts (`max_hp`,
 * `base_hp`: HPMemory counts into the base, programs on top), the Cross
 * brought: how many rows of `most` */
int second_status(SecondRow *out, int most, int max_hp, int base_hp);
/* The Library's classes against what a run can hold, and the chips new
 * this run: how many rows of `most` */
int second_library(SecondRow *out, int most);
/* What the Library is to a run, a line */
const char *second_library_line(void);
/* What the run holds of a chip in any code, the Library's card's: its
 * copies in the folder and in the pack, as rows; how many of `most` */
int second_copies(SecondRow *out, int most, int folder, int pack);
/* A layer's next step: the guardian its exit waits on (navi, 0 none) or
 * the exit pad, into `out` */
void second_layer_next(int guardian, char *out, size_t n);
/* What the run holds of the shop's entry under the cursor (s->sh_*):
 * lines apart by '|', into `out` */
void second_held_lines(const SecondState *s, char *out, size_t n);
/* What a key item a shop sells does in the run, a line; "" where the
 * PET's own description says it */
const char *second_item_does(int id);
/* A trader of `kind` (TraderKind): its name, and what it takes and gives
 * (and the chips the pack holds, `pack_chips`), lines apart by '|' */
const char *second_trader_name(int kind);
void second_trader_lines(int kind, int pack_chips, char *out, size_t n);
/* Whether BN5's ROM was found beside BN6's, in a line */
const char *second_bn5_line(bool found);

#endif
