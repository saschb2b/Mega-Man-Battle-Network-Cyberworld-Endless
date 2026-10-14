/* Local block/mute choices use a server's stable public identity. */
#ifndef CW_ONLINE_PEOPLE_H
#define CW_ONLINE_PEOPLE_H

#include "online_types.h"

enum { ONLINE_MUTED = 1, ONLINE_BLOCKED = 2 };
void online_people_load(void);
unsigned online_people_flags(const uint8_t identity[ONLINE_IDENTITY_BYTES]);
bool online_people_toggle(const uint8_t identity[ONLINE_IDENTITY_BYTES], unsigned flag);

#endif
