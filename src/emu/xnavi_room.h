/* BN6's battle engine given the room other games' Navis take (issue #82;
 * docs/ROM_DATA.md, The Navis' tables; docs/EMULATION.md, the free-space
 * map): the thirteen tables a Navi is found through by its AI index, the
 * Navi words of the four triples by actor type (its ice block's size, its
 * held chips, its companion's start and end), the companion overlay's
 * kinds, each copied past FREE_NAVI_ROOM with BN6's entries and a fill
 * after them and the one word that names it pointed at the copy; and the
 * moved Navis' names, an archive of their own by an id's low byte, which a
 * hook on BN6's name lookup hands out for their ids (BN6's own Navis read
 * BN6's archive as before). Made once a core, all or nothing,
 * after the families' room (xvirus_grow.h, whose grown ids, reward rows,
 * sprite lists, effects and types a Navi takes too) and before any Navi's
 * install (xnavi_install.c). BN6's own Navis read the copies as BN6's. */
#ifndef CW_XNAVI_ROOM_H
#define CW_XNAVI_ROOM_H

#include <stdbool.h>
#include <stdint.h>

#include "bn6.h"
#include "free_space.h"

#define XN_AIS        256   /* Navi AI indexes in the grown tables (AIData +1, a byte) */
#define XN_KINDS      64    /* the companion overlay's kinds grown */
#define XN_NAMES_MAX  256   /* the Navis' names' entries (an id's low byte) */
/* Where they go, from FREE_NAVI_ROOM */
#define XN_TABLES     (FREE_NAVI_ROOM + 0x0000u)   /* the thirteen tables, XN_AIS words each, XN_STEP apart, in BN6_NAVI_TABLES' order */
#define XN_STEP       0x400u
#define XN_ICE        (FREE_NAVI_ROOM + 0x3400u)   /* the ice block's size, a byte an AI */
#define XN_HELD       (FREE_NAVI_ROOM + 0x3500u)   /* the held chips' words, a word an AI */
#define XN_COMPANION  (FREE_NAVI_ROOM + 0x3900u)   /* the companion's start, a word an AI */
#define XN_COMPANION_END (FREE_NAVI_ROOM + 0x3D00u) /* ... its end */
#define XN_OVL_MASKS  (FREE_NAVI_ROOM + 0x4100u)   /* the overlay's masks, a word a kind */
#define XN_OVL_SPRITES (FREE_NAVI_ROOM + 0x4200u)  /* the overlay's sprites, two bytes a kind */
#define XN_NAMES      (FREE_NAVI_ROOM + 0x4400u)   /* the moved Navis' names: XN_NAMES_MAX offsets, the empty name, then theirs */
#define XN_NAMES_ROOM 0x3C00u
#define XN_ROOM_END   (FREE_NAVI_ROOM + 0x8000u)

/* The grown tables, by what a Navi's install names them */
enum {
	XN_TABLE,                                  /* the thirteen: XN_TABLE + k for BN6_NAVI_TABLES' table k, a word an AI */
	XN_T_ICE = XN_TABLE + BN6_NAVI_TABLE_COUNT, /* a byte an AI */
	XN_T_HELD,                                 /* a word an AI */
	XN_T_COMPANION,
	XN_T_COMPANION_END,
	XN_T_OVL_MASKS,                            /* a word a kind */
	XN_T_OVL_SPRITES,                          /* two bytes a kind */
	XN_GROWN
};
/* The thirteen tables, by their place in BN6_NAVI_TABLES */
enum {
	XN_STATES, XN_PRESTEP, XN_HOOK_A, XN_STRUCT1, XN_STRUCT2, XN_POST, XN_SPAWN, XN_SPAWNED, XN_PARALYSIS, XN_FREEZE, XN_BUBBLE,
	XN_FLINCH, XN_DRAG,
};

/* Who the Navis' room's and the Navis' bytes are (claims.h) */
#define XN_CLAIMS "the Navis and their room (xnavi_room.c, xnavi_install.c)"

/* BN6's tables grown (the families' room first): true once they are, now
 * or before; false, nothing written, where a check failed (the log says
 * which). */
bool xn_grow(void);
/* Entry i of grown table t (XN_*) given a Navi's bytes (the table's entry
 * size), staged (xvirus_tx.h): the core's copy must still hold the fill
 * there. */
void xn_set(int t, uint32_t i, const uint8_t *entry);
void xn_set_word(int t, uint32_t i, uint32_t v);
/* The names' archive's bytes in use in the core's copy: where a Navi's
 * names go on */
uint32_t xn_names_end(void);
/* A Navi's name at entry i (its id's low byte), staged at the archive's
 * offset `off` (from xn_names_end on): BN6's text codes, `len` bytes with
 * its end; false where it has no room */
bool xn_name(uint32_t i, uint32_t off, const uint8_t *text, uint32_t len);
/* The writer of the bytes at `at` as a Navi's install stages them: the
 * families' room's (its grown ids, rows, lists, effects, types) or the
 * Navis' own (claims.h) */
const char *xn_writer(uint32_t at);

#endif
