/* CROSSSELECT's rows for Falzar's five Crosses, which Gregar has no art
 * of (src/emu/crossselect.h; docs/game/fidelity.md). */
#ifndef CW_CROSS_ROWS_H
#define CW_CROSS_ROWS_H

#include <stdbool.h>
#include <stdint.h>

/* Falzar's five rows made from the player's ROM, laid out as Gregar's
 * (CROSSSELECT_ART_BYTES of `art`: the five, then the five grey;
 * CROSSSELECT_PAL_BYTES of `pals`: the five, then the five chosen): each
 * Gregar's SlashCross row, its frame, band and text box, with the Navi's
 * face from his battle sprite (his portrait's window, portrait.c) in the
 * left and the Cross's name in the engine's own letters. False where the
 * ROM is not as expected. */
bool cross_rows_make(uint8_t *art, uint8_t *pals);

#endif
