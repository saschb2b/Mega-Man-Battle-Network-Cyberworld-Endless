/* CROSSSELECT offering Falzar's five Crosses beside Gregar's (issue #706;
 * docs/rom/souls.md, CROSSSELECT). Gregar's list reads Gregar's five by
 * flags BN6_FLAG_HEAT_CROSS on and commits Cross i as form i + 1, the
 * same code as Falzar's but for its flags; made to read ten (Falzar's
 * flags BN6_FLAG_SPOUT_CROSS on), it commits Falzar's as forms 6-10, the
 * forms Gregar keeps whole in every table by form. Five rows at most, as
 * either game lists. Gregar has no art of Falzar's rows: theirs are made
 * from the ROM's own (src/layer/cross_rows.c) and read from the free
 * space in Gregar's art's place. */
#ifndef CW_CROSSSELECT_H
#define CW_CROSSSELECT_H

#include <stdbool.h>
#include <stdint.h>

#include "bn6.h"

#define CROSSSELECT_CLAIMS "CROSSSELECT's ten Crosses (crossselect.c)"
#define CROSSSELECT_FALZAR (BN6_CROSSES - BN6_CROSS_ROWS) /* Falzar's five, Crosses 5-9 */
#define CROSSSELECT_ART_BYTES (2u * CROSSSELECT_FALZAR * BN6_CROSS_ART_SIZE) /* their rows' art, the five, then the five grey */
#define CROSSSELECT_PAL_BYTES (2u * CROSSSELECT_FALZAR * 0x20u)            /* their palettes, the five, then the five chosen */

/* CROSSSELECT made to list Falzar's five with their rows' `art` and
 * `pals` (laid out as Gregar's: CROSSSELECT_ART_BYTES and
 * CROSSSELECT_PAL_BYTES), its hooks set; false, nothing written, where
 * the ROM's code is not Gregar's as expected or no art is given (Falzar's
 * then never listed). */
bool crossselect_install(const uint8_t *art, const uint8_t *pals);
/* Whether it lists Falzar's five (installed in this core) */
bool crossselect_falzar(void);

#endif
