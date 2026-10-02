/* BN6's wait for the next frame, halted instead of spun (docs/EMULATION.md,
 * Hooks; issue #35). */
#ifndef CW_IDLE_H
#define CW_IDLE_H

/* Its hook, once the core is up. */
void idle_install(void);

#endif
