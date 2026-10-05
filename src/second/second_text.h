/* The second screen's words (second_text.c): what the PET says of the
 * screen beside it, from the run's facts and BN6's own tutorials. */
#ifndef CW_SECOND_TEXT_H
#define CW_SECOND_TEXT_H

#include <stdbool.h>
#include <stddef.h>

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

#endif
