/* BN5's own pieces of a liberation's board, drawn from BN5's ROM as the
 * game runs (liberation_pieces.c): the Order Points gauge, the L view's
 * phases, the command window, the banners after a battle. Each draws
 * nothing where BN5's ROM is not there (liberation_draw.c's PET look then
 * stands in). Draws only. */
#ifndef CW_LIBERATION_PIECES_H
#define CW_LIBERATION_PIECES_H

#include <stdbool.h>

/* Whether BN5's pieces can be drawn */
bool lib_pieces_on(void);
/* The Order Points gauge, at the picture's top right */
void lib_piece_gauge(int op);
/* The L view's "PHASE" and "TRGT PHASE" with their numbers */
void lib_piece_phases(int phase, int par);
/* The command window: its `n` lines (dim where `can` is false), the cursor on `cursor` */
void lib_piece_window(const char *const *lines, const bool *can, int n, int cursor);
/* A banner (liberation_state.h's LIB_CHEER_*), `t` frames since it began */
void lib_piece_cheer(int cheer, int t);

#endif
