/* A BonusPanel as BN5's (docs/LIBERATION.md, BonusPanels as BN5's): the
 * Liberation command on it frees it without a battle and without the
 * Navi's action; three white flashes, then BN5's roulette on the phase
 * banner's band, its five slots in BN5's order, stopped by A, and the slot
 * stopped on given as BN5 gives it (liberation_bonus.c). Its picture is
 * liberation_roulette.c's. */
#ifndef CW_LIBERATION_BONUS_H
#define CW_LIBERATION_BONUS_H

#include <stdbool.h>
#include <stdint.h>

/* The roulette's slots, BN5's order (BN5_LIB_BONUS_*) */
enum { LIB_BONUS_SHIELD, LIB_BONUS_HP, LIB_BONUS_OP, LIB_BONUS_CHIP, LIB_BONUS_HIT, LIB_BONUS_SLOTS };

/* (the course's hooks: liberation_state.h's lib_bonus_begin, _update and
 * _keys) The roulette spinning, A awaited (the autopilot's) */
bool lib_bonus_spinning(void);

/* What its picture shows this frame: a flash's whiteness (16ths, 0 none);
 * the band's rows (0 none); the icon's slot and its x on the picture
 * (`icon` false: none); the words' animation (-1 none) */
typedef struct {
	int white;
	int band_top, band_rows;
	bool icon;
	int slot, icon_x;
	int words;
} LibBonusView;
void lib_bonus_view(LibBonusView *v);

#endif
