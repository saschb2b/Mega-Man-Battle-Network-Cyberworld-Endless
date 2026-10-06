/* ProtoMan's cross, mended (issue #55; docs/FIDELITY.md): BN6 can end it
 * with nothing able to touch him for the rest of the battle; a hook gives
 * him his panel back as he lands on it. */
#ifndef CW_PROTOMAN_CROSS_H
#define CW_PROTOMAN_CROSS_H

#include <stdint.h>

/* Its hook, once the core is up. */
void protoman_cross_install(void);
/* The crosses that ended with him untouchable and were mended, so far
 * (CYBERWORLD_EMU_DEBUG says each). */
extern uint32_t protoman_cross_mended;

#endif
