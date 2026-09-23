/* Test autopilot: walks MegaMan to the layer's exit with the pad
 * (CYBERWORLD_AUTOPILOT=1), pressing A through battles and messages. */
#ifndef CW_AUTOPILOT_H
#define CW_AUTOPILOT_H

#include <stdbool.h>
#include <stdint.h>

bool autopilot_on(void);
/* GBA keys for this frame. */
uint32_t autopilot_keys(void);

#endif
