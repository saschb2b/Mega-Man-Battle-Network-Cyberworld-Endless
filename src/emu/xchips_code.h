/* BN5's chips' code in BN6's battle engine (issue #82's C3; docs/MULTIROM.md,
 * The chips' code; docs/EMULATION.md, its free space): the units of
 * xchips_bn5.h installed into the core's copy as the core starts, where
 * BN5's ROM is read, on xvirus's machinery (their blocks copied, their calls
 * through veneers, their objects through the second-level dispatch, each
 * step staged and checked), with BN6's attack families grown (xattack_grow.h)
 * and its support and Navi chips' routines grown here: BN6_CHIP_ROUTINES and
 * BN6_NAVI_ROUTINES copied to 256 entries each, BN6's own as the player's
 * ROM has them, the rest a stub, their four words pointed at the copies.
 * The chips that run a unit (src/core/xchips_moved.h) take its family or
 * subfamily in their records (chips.c). All or nothing: one unit's check
 * failing writes nothing. */
#ifndef CW_XCHIPS_CODE_H
#define CW_XCHIPS_CODE_H

#include <stdbool.h>
#include <stdint.h>

#include "bn6.h"
#include "free_space.h"

/* Its places, from FREE_CHIP_MOVED (the region's start past the grown attack families) */
#define XC_AT        FREE_CHIP_MOVED
#define XC_SUPPORT   (XC_AT + 0x000u)      /* the support routines grown: a Thumb routine per subfamily, 256 */
#define XC_NAVI      (XC_AT + 0x400u)      /* the Navi chips' routines grown, 256 */
#define XC_STUBS     (XC_AT + 0x800u)      /* their stubs: a support routine's (+0, nothing spawned: r0 0, a unit's call to
                                            * a panel claim's spawner too: tools/xchiprecipe.py, STUBS) and a Navi's (+4) */
#define XC_STUBS_END (XC_STUBS + 8u)
#define XC_HELPERS   (XC_STUBS + 0x10u)    /* the helpers' code (XCHIP_HELPER: BN5's code several units call, installed once), to: */
#define XC_HOMES     (XC_AT + 0x2000u)     /* each unit's veneers, XC_HOME_SIZE apart, */
#define XC_HOME_SIZE 0x400u
#define XC_UNITS     64                    /* ... this many units */
#define XC_CODE      (XC_HOMES + XC_HOME_SIZE * XC_UNITS)   /* the units' code, block by block, to: */
#define XC_ART       (FREE_CHIP_CODE + 0xA0000u)            /* what they draw, to XC_END */
#define XC_END       FREE_TAIL
#define XC_ROUTINES  256                   /* entries in each grown routines' table */
#define XC_FIRST_SUPPORT BN6_CHIP_ROUTINE_COUNT    /* the units' support routines from here, */
#define XC_FIRST_NAVI    BN6_NAVI_ROUTINE_COUNT    /* ... their Navi routines from here */

/* Who its bytes are (claims.h) */
#define XC_CLAIMS "the chips' moved code (xchips_code.c)"

/* The units installed, once a core (again it does nothing): true once they
 * are, now or before; false where BN5's ROM is not read yet, or a check
 * failed (nothing of them written, the log says which). */
bool xchips_code_install(void);

#endif
