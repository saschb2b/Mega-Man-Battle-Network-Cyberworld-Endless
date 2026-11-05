/* The faces of BN6's and BN5's Navis on BN4's moved bracket
 * (docs/multirom/bn4-tournament.md, Faces for every entrant): each
 * Navi's own face, from the player's ROMs as the bracket installs, made
 * a plate as BN4 draws one (12 tiles, four palettes) by
 * tools/bn4_faces.py's conversion, its oracle; the plates in two copies of
 * BN4's plate sheets, the moved copy's plate tables grown so that
 * XBR_PLATE_NAVI(n) draws Navi n's (xbracket.h). Numbers only here: the
 * pixels are the player's ROMs'. */
#ifndef CW_XBRACKET_FACES_H
#define CW_XBRACKET_FACES_H

#include <stdbool.h>
#include <stdint.h>

#include "bn4_tournament.h"
#include "free_space.h"

#define XBF_FACES      22      /* BN6's sixteen (navis 1-10, 12-16, 18), then BN5's six (24-29) */
#define XBF_FACES_BN6  16      /* ... the first sheet's: its animations 0-15, as BN4's ids 0x00-0x0F take them */
#define XBF_IDS        0x40    /* the grown id tables' entries: BN4's 0x00-0x1E, then XBR_PLATE_NAVI's, the rest BN4_PLATE_EMPTY_ID's */
#define XBF_PLATE_BYTES (BN4_PLATE_TILES * 32 + BN4_PLATE_VARIANTS * 32)   /* a plate: its tiles, then its four palettes */

/* Its room (FREE_BRACKET_FACES) */
#define XBF_FRAMES     (FREE_BRACKET_FACES + 0x000u)   /* BN4_PLATE_FRAMES grown to XBF_IDS */
#define XBF_PALETTES   (FREE_BRACKET_FACES + 0x040u)   /* BN4_PLATE_PALETTES grown */
#define XBF_SHEETS     (FREE_BRACKET_FACES + 0x080u)   /* BN4_PLATE_SHEETS grown: XBF_SHEET_VALUE_A or _B for a face */
#define XBF_SPRITES    (FREE_BRACKET_FACES + 0x0C0u)   /* BN4_PLATE_SPRITES grown: its own, then the two sheets' sprite indexes */
#define XBF_PTRS       (FREE_BRACKET_FACES + 0x0E0u)   /* BN4_MENU_SPRITE_PTRS' first six as moved, then the two sheets */
#define XBF_TABLES_END (FREE_BRACKET_FACES + 0x100u)
#define XBF_SHEET_A    (FREE_BRACKET_FACES + 0x100u + (BN4_PLATE_SHEET_A & 0x3Fu))   /* BN6's sixteen: a copy of BN4_PLATE_SHEET_A */
#define XBF_SHEET_B    (FREE_BRACKET_FACES + 0x2500u + (BN4_PLATE_SHEET_B & 0x3Fu))  /* BN5's six: a copy of BN4_PLATE_SHEET_B */
#define XBF_END        (FREE_BRACKET_FACES + 0x3500u)
#define XBF_SHEET_VALUE_A BN4_PLATE_SHEET_VALUES          /* the r6 BN4_PLATE_SET takes for the first copy: 0x12, */
#define XBF_SHEET_VALUE_B (BN4_PLATE_SHEET_VALUES + 1)    /* ... the second's */
#define XBF_SPRITE_A   BN4_MENU_SPRITE_PTRS_USED          /* their entries in XBF_PTRS: 6, */
#define XBF_SPRITE_B   (BN4_MENU_SPRITE_PTRS_USED + 1)    /* ... 7 */

/* The faces' writes staged into the bracket's step (xvirus_tx.h), after its
 * code: the tables, the sheets with every face its game's ROM gives, the
 * moved literals aimed at them; false (the step failed) where BN4's plates
 * or a Navi's face is not as the conversion reads it. */
bool xbracket_faces_stage(void);
/* After the step's commit: with CYBERWORLD_XBRACKET_FACES=PATH, each plate
 * made written to PATH as a line ("navi" and its XBF_PLATE_BYTES in hex),
 * read back from the core's copy as the bracket reads it, through the
 * moved literals and the grown tables (tools/bn4_faces.py check). */
void xbracket_faces_dump(void);

#endif
