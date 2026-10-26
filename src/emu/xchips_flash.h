/* BN5's green flash in BN6's engine (issue #82's C3; docs/multirom/chips-code.md, The
 * chips' code; docs/rom/chips-code.md, DarkPlus's flash): DarkPlus's power
 * stored in the next chip of the hand flashes its user green 24 frames
 * (BN5's misc family's subfamily 3, 0x0800E1D0-0x0800E1D4, moved: xchips_
 * bn5.c's Misc), a timer BN5 keeps in his AI data's +0x3C, which BN6's
 * engine reads as his resting height (0x0801A2B0 puts it in his Z after a
 * flinching hit). The moved code's two stores of it go to XC_FLASH
 * instead, a u16 a side; BN5's count-down (0x0800E532: one a frame, none
 * while time stops) where BN6's twin (0x080107D4) counts its own, and its
 * tint (0x080136B0: the sprite's colour shader by BN5's table of eight,
 * 0x080136D0, by the timer's low bits) where BN5's chain of an object's
 * colours calls it, before the step BN6's 0x080168C8 is the twin of. */
#ifndef CW_XCHIPS_FLASH_H
#define CW_XCHIPS_FLASH_H

#include <stdbool.h>
#include <stdint.h>

#include "free_space.h"

#define XC_FLASH RAM_BATTLE   /* the flash's frames, a u16 a side (the RAM plan's moved state: zeroed as each battle is set up) */
#define XC_FLASH_STEP 2u

/* Who its hooks are (claims.h) */
#define XC_FLASH_CLAIMS "the chips' green flash (xchips_flash.c)"

/* Its hooks, once the chips' code is installed (xchips_code.c): false,
 * nothing hooked, where a byte it stands on is not as expected or the
 * moved code holds no store of the flash (the log says which). */
bool xchips_flash_install(void);

#endif
