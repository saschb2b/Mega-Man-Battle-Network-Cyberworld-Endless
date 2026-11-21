/* A moved chip's name in BN6's eight cells (issue #732; docs/multirom/chip-registry.md, A name in eight cells): BN6's
 * lists draw a name a glyph a cell and never stop at the eighth (bn6_chips.h, BN6_LIST_NAME_DRAW), and BN5's DS mark,
 * one glyph there, is two letters in BN6's font (rom.h, xrom_text_bn6), so BN5's twelve DS Navi chips came to nine. A
 * name past eight loses the letter the series itself dropped where it spelt the chip short (BN4's NumbMnDS, ShadMnDS
 * and ProtoMDS, BN6's TmhkMan), else the smallest cut; the pairing by name reads the games' own names, not these. */
#ifndef CW_XCHIPS_CELLS_H
#define CW_XCHIPS_CELLS_H

#include <stddef.h>
#include <stdint.h>

/* the cells a name of BN6's text codes takes: a glyph each, to its first command (0xE5 on) */
int xchips_cells(const uint8_t *s, size_t n);

/* the cell the series' own shortening drops from chip `index` of game `game` (5 or 4) as BN6's codes spell it, -1 where
 * the chip has none listed */
int xchips_cells_cut(int game, int index);

/* name `s` (`n` bytes of BN6's codes) of chip `index` of game `game` cut to BN6_CHIP_NAME_CELLS: its listed cut first,
 * then the last small vowel past its first letter, then the last small letter, then its end; its bytes now (an end code
 * after the name kept after it) */
size_t xchips_cells_fit(int game, int index, uint8_t *s, size_t n);

#endif
