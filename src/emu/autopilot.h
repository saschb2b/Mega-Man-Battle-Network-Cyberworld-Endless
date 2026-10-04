/* Test autopilot: walks MegaMan to the layer's exit with the pad
 * (CYBERWORLD_AUTOPILOT=1), fighting its battles and pressing A through
 * messages. */
#ifndef CW_AUTOPILOT_H
#define CW_AUTOPILOT_H

#include <stdbool.h>
#include <stdint.h>

bool autopilot_on(void);
/* GBA keys for this frame. */
uint32_t autopilot_keys(void);
/* ... and for this frame of a battle on the guest core (guest.h), from its
 * own game's memory; none before its battle is on its screen. */
uint32_t autopilot_guest_keys(void);

#endif
