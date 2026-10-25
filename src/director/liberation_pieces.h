/* BN5's own pieces of a liberation's board, drawn from BN5's ROM as the
 * game runs (liberation_pieces.c): the Order Points gauge, the L view's
 * phases and keys, the command window, the banners after a battle and the
 * phases', the mission won's and its tally. Each draws
 * nothing where BN5's ROM is not there (liberation_draw.c's PET look then
 * stands in). Draws only. */
#ifndef CW_LIBERATION_PIECES_H
#define CW_LIBERATION_PIECES_H

#include <stdbool.h>
#include <stddef.h>

/* Whether BN5's pieces can be drawn */
bool lib_pieces_on(void);
/* The Order Points gauge, at the picture's top right */
void lib_piece_gauge(int op);
/* A blow's harm over its Navi in BN5's yellow digits, centred on (x, y),
 * his feet on the GBA's picture */
void lib_piece_harm(int harm, int x, int y);
/* A unit's HP under it in BN5's white digits, centred on (x, y), its place
 * on the floor on the GBA's picture */
void lib_piece_hp(int hp, int x, int y);
/* The L view: "PHASE" and "TRGT PHASE" with their numbers on their panel,
 * its arrows at the picture's edges and its cross at the middle, `t` frames on */
void lib_piece_phases(int phase, int par, int t);
/* The command window: its `n` lines (dim where `can` is false), the cursor on `cursor` */
void lib_piece_window(const char *const *lines, const bool *can, int n, int cursor);
/* A banner (liberation_state.h's LIB_CHEER_*), `t` frames since it began */
void lib_piece_cheer(int cheer, int t);

/* A phase's banner as BN5 shows it, `t` frames since its band opened:
 * DARKLOID PHASE, or PLAYER PHASE and the phase's number */
void lib_piece_phase_banner(bool darkloid, int phase, int t);
/* ... and its frames, 0 where BN5's ROM is not there */
int lib_piece_banner_frames(void);
/* The mission won's SUCCESS! in the same band, `t` frames since it opened */
void lib_piece_clear_banner(int t);
/* MegaMan's tally of the mission won in BN5's own words, read from its ROM
 * (BN5_LIB_TALLY_MEGAMAN + `tier`, 0 two under par or better, 1 at par, 2
 * over): its first `pages` pages as chat boxes split by '|', the
 * target `par` and the `phases` taken in its numbers' places; false where
 * BN5's ROM is not there */
bool lib_piece_tally(int tier, int phases, int par, int pages, char *out, size_t size);

#endif
