/* BN5's chips' code as units moved into BN6's battle engine (issue #82's
 * C3; docs/multirom/chips-code.md): a unit is one attack family of
 * BN5's (the routine a chip's CurAction runs), or one support or Navi
 * routine (the routine BN6's support family, or its Navi object, runs by a
 * chip's subfamily), with every routine and object it reaches that BN6 has
 * no twin of. Numbers alone, written by tools/xchiprecipe.py from the two
 * ROMs into xchips_bn5.c; src/emu/xchips_code.c installs them. */
#ifndef CW_XCHIPS_BN5_H
#define CW_XCHIPS_BN5_H

#include <stddef.h>
#include <stdint.h>

#include "xvirus_recipe.h"

/* What a unit is in BN6's tables */
enum {
	XCHIP_ATTACK,    /* an attack family: its entry of the grown attack families (xattack_grow.h), a chip's family */
	XCHIP_SUPPORT,   /* a support routine: its entry of the grown support routines, a chip's subfamily under family 0x15 */
	XCHIP_NAVI,      /* a Navi chip's routine: its entry of the grown Navi routines, a chip's subfamily under family 0x1B */
	XCHIP_HELPER,    /* a helper: BN5's code several units call whose object one copy alone may move (the shake), or a
	                    routine of theirs (the buster arm, a computed power), at XC_HELPERS, installed before the units; no
	                    entry of a table, its `entry` the routine they call */
	XCHIP_MISC,      /* a routine of the misc family's (BN4's units, xchips_bn4.h): its entry of the grown misc routines, a chip's
	                    subfamily under BN6's family 0x1C */
};

/* A sound its code starts that BN6 has another song at: BN5's song copied
 * into BN6's slot fixed for it (xsong.h, xsong_install_at), the immediates
 * that name it given the slot */
typedef struct { uint16_t song; uint16_t slot; } XcSong;   /* (a slot past a byte: the grown song table's, song_table.h) */

typedef struct {
	uint8_t kind;          /* XCHIP_* */
	uint8_t bn5;           /* BN5's family (an attack) or subfamily (a support or Navi routine); a helper's 0 */
	uint8_t number;        /* BN6's: its family (XATTACK_CHIPS on), or its entry in the grown routines' table; a helper's 0 */
	uint32_t entry;        /* the routine the table names, BN5's address (Thumb): moved with the code */
	uint32_t home;         /* its veneers' place (XC_HOMES, XC_HOME_SIZE each) */
	XVirusFamily code;     /* its code alone: blocks, data, calls, words, immediates, numbers, the objects it spawns, what it draws */
	const XcSong *songs;   /* its sounds copied in (NULL none) */
	int nsongs;
	uint32_t navis;        /* the Navis whose battle sprites its code draws, bit XN_SEAT(n) BN5's Navi of AI n (xnavi_recipe.h: Team
	                          Colonel's six n - 7, the Darkloids n - 6, Nebula's LarkMan and GridMan n; xnavi_sprites.h: list 2's
	                          0x20 + its seat, installed before its step) */
	uint8_t share;         /* (BN4's units) where not 0, its routine is one of unit `share` - 1's code, a block that unit moved: no code
	                          of its own, its entry alone (WhitWeb2 and 3 in WhitWeb1's object's code) */
	uint8_t wait;          /* (BN4's misc units) where not 0, the frames BN4's misc run waits after its routine before it ends the
	                          attack (bn4_chips.h BN4_MISC_WAIT: subfamily 5's 0x28, SidBmbo's), which BN6's run does for its own
	                          0x14 alone: its entry in the grown table is a wrapper of the installer's in its home (XC4_HOME_WAIT),
	                          the run's routine every frame of the wait */
	uint16_t step_in;      /* (BN4's Navi chips' units) where not 0, the slot of the sound BN4's object of the chip plays as it steps
	                          MegaMan in, which BN6's Navi object does not (GutPnch's 0xB0): BN6's object's last state is aimed at a
	                          stub of the installer's in its home (XC4_HOME_WAIT) that plays it for this unit's number alone */
} XChipUnit;

extern const XChipUnit xchips_bn5[];
extern const int xchips_bn5_count;

#endif
