/* Cards the second screen's panels share (second_card.c): a chip's and a
 * NaviCust program's shape. */
#ifndef CW_SECOND_CARD_H
#define CW_SECOND_CARD_H

#include <stdint.h>

#include "gfx.h"
#include "navicust.h"

/* A chip as a card: its picture twice as large, its name, code, element
 * and power beside it, its text under them; `e` chip | code << 9 */
#define SECOND_CARD_H (48 * 2 + 4)
void second_chip_card(uint16_t e, int x, int y, int w);
/* A program's shape on its own 7 x 7 grid in its colour, from (x, y),
 * SECOND_SHAPE_W square */
#define SECOND_SHAPE_CELL 8
#define SECOND_SHAPE_W (7 * SECOND_SHAPE_CELL + 4)
void second_program_shape(const NaviShape *s, int x, int y);
/* ... a colour's swatch, `size` square, from (x, y) */
void second_color_swatch(int color, int x, int y, int size);

#endif
