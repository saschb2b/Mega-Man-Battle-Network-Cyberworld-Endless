/* BN4's chips' code as units moved into BN6's battle engine
 * (docs/multirom/bn4-chips.md, The build plan's units 3 and 5;
 * docs/devtools/chip-recipe-generator.md, The BN4 path): a unit is one
 * routine of BN4's support state's table (BN4_CHIP_ROUTINES, its support
 * and Navi chips' by subfamily), a spawner of a fourth-kind object, with
 * that object's type and every routine and object it reaches that BN6 has
 * no twin of. Numbers alone, written by tools/xchiprecipe_bn4.py from the
 * two ROMs into xchips_bn4.c; src/emu/xchips_code.c installs them where
 * BN4's ROM is read, each at BN6's grown support routines' XC4_FIRST_SUPPORT
 * + BN4's subfamily (a routine of BN4's misc state's table, BN4_MISC_ROUTINES,
 * likewise in BN6's misc routines grown, XC4_MISC), which its chips' records take under BN6's support
 * family (the registry's XREG_MOVED rows, src/core/xchips_registry_bn4.c). */
#ifndef CW_XCHIPS_BN4_H
#define CW_XCHIPS_BN4_H

#include "free_space.h"
#include "xattack_grow.h"
#include "xchips_bn5.h"

/* Their places, FREE_BN4_CHIPS (free_space.plan): */
#define XC4_HOMES     FREE_BN4_CHIPS       /* each unit's home, XC4_HOME_SIZE apart: its veneers (XC4_HOME_VENEERS bytes), then */
#define XC4_HOME_SIZE 0x400u
#define XC4_HOME_VENEERS 0x300u            /* ... its shims (xvirus_shim.h: a sound past a byte played through one), which the installer
                                              puts at its code's `home` + XV_HOME_SHIMS: */
#define XC4_SHIMS_HOME(home) ((home) + XC4_HOME_VENEERS - XV_HOME_SHIMS)
#define XC4_WAIT_SIZE 0x40u               /* ... and, where the unit waits (XChipUnit.wait), its wrapper at its home's end, the
                                              project's own Thumb (xchips_code.c wait_stage): the misc run's routine each frame
                                              of the wait, which calls the unit's routine once and counts as BN4's run does */
#define XC4_HOME_WAIT (XC4_HOME_SIZE - XC4_WAIT_SIZE)
#define XC4_HOME_STATES (XC4_HOME_WAIT - 0x40u)   /* ... and, where its moved object is an actor (its code's `states`: an
                                                     object the actors' runner runs, TimeBomb's bomb), its state table in
                                                     BN6's layout, XV_STATES_MAX entries at most, which the object's update's
                                                     pool word names (a number row of its code) */
#define XC4_UNITS     96                   /* ... this many units (BN4's 48 of its chips and 18 of its Navi chips) */
#define XC4_CODE      (XC4_HOMES + XC4_HOME_SIZE * XC4_UNITS)   /* the units' code, block by block, to: */
#define XC4_ART       (FREE_BN4_CHIPS + 0x30000u)                /* the room what they draw is put into as an object loads it
                                                                   * (xchips_bn4_art.h; the code's room 96 KB), to: */
#define XC4_END       XC4_PAGE
#define XC4_PAGE      (FREE_BN4_CHIPS_END - 0xA00u)   /* BN4's chips' page of the second-level dispatch (xvirus_dispatch.h): */
#define XC4_PAGE_MOVED XC4_PAGE            /* ... its MOVED table, 256 words by BN4's own third-kind type, */
#define XC4_PAGE_CODE (XC4_PAGE + 0x400u)  /* ... its code, the project's own Thumb: its dispatch type's routine, */
#define XC4_PAGE_CODE_SIZE 0x100u
#define XV_SPAWN4_3   (XC4_PAGE_CODE + 0x20u)   /* ... its spawners, as XV_SPAWN_3, XV_SPAWN_3_LAST and XV_SPAWN_3_FIRST (BN4's 0x080032F0, */
#define XV_SPAWN4_3_LAST (XC4_PAGE_CODE + 0x60u)   /* ... 0x0800330C, */
#define XV_SPAWN4_3_FIRST (XC4_PAGE_CODE + 0xA0u)  /* ... 0x08003328) */
#define XC4_MISC      (FREE_BN4_CHIPS_END - 0x400u)   /* BN6's misc family's routines grown, 256 (BN6's own 7, a unit's at XC4_FIRST_SUPPORT + BN4's subfamily) */
#define XC4_FIRST_SUPPORT 0x80             /* BN4's subfamily 0 in BN6's grown support routines (BN6's 0-0x29, BN5's from 0x2A) */
/* An attack family of BN4's (a state of its MegaMan's, BN4_PLAYER_STATES by a chip's record's +0xB: the spread chips' 0x1E)
 * is a unit too, XCHIP_ATTACK: the state's code whole, every subfamily of it, at a number of BN6's grown attack families
 * the chips of BN4 have for their own (xattack_grow.h XATTACK_BN4 on, in the units' order: tools/xchiprecipe_bn4.py
 * ATTACK_NUMBERS), `bn5` BN4's family. Its chips' records take that number as their family and keep BN4's subfamily; a
 * registry row names it in `sub` (XC4_FIRST_ATTACK on: past every support and misc routine's number) */
#define XC4_FIRST_ATTACK XATTACK_BN4

/* The units (XChipUnit, xchips_bn5.h: `bn5` BN4's subfamily, `navis` 0) */
extern const XChipUnit xchips_bn4[];
extern const int xchips_bn4_count;
/* BN4's computed powers by its table's entry (bn4_chips.h BN4_CHIP_POWERS, power less 1000): the routine its power unit
 * moved (a block of an XCHIP_HELPER's), 0 for one it did not (BN6's stub, power 0); src/emu/xchips_code.c powers4_stage */
extern const uint32_t xchips_bn4_powers[];
extern const int xchips_bn4_powers_count;

#endif
