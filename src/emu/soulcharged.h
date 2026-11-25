/* A BN4 Soul's own attack for a chip he charged, in BN6's engine
 * (docs/multirom/bn4-souls.md, FireSoul in BN6's engine): BN4's chip use runs
 * FireSoul's flame (BN4_FIRE_CHARGED) where the chip is charged and his the
 * form, in place of the chip's setup. BN6 uses a charged chip through its
 * setup (0x080127C0) and then sets the family it returns
 * (BN6_CHARGED_CHIP_SET); here, after that setup, the copy of his attack
 * runs (soulbn4_charged_copy) and its family is the one set. */
#ifndef CW_SOULCHARGED_H
#define CW_SOULCHARGED_H

#include <stdbool.h>

/* The stub and the hook (SOULS_BN4_CHARGED, BN6_CHARGED_CHIP_SET), where a
 * moved BN4 Soul has such an attack; true where nothing failed */
bool soulcharged_install(void);

#endif
