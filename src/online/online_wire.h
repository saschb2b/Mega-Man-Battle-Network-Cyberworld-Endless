/* Byte order and validation shared by the native and browser transports. */
#ifndef CW_ONLINE_WIRE_H
#define CW_ONLINE_WIRE_H

#include "online_types.h"

void online_put16(uint8_t *out, uint16_t value);
void online_put32(uint8_t *out, uint32_t value);
bool online_wire_name(const char *name);
bool online_wire_message(const uint8_t *data, size_t size, uint32_t now, OnlineSnapshot *snapshot);
/* Whether a look's six bytes (v30: three palettes, then three colours) are
 * within the wire's bounds: each channel ONLINE_LOOK_NONE twice, or a
 * palette and a colour in range (src/core/looks.h's look_valid, by the
 * wire's own numbers). */
bool online_wire_look(const uint8_t look[2 * ONLINE_LOOK_CHANNELS]);
bool online_wire_profile(uint8_t out[ONLINE_PROFILE_BYTES + 1], const OnlineProfile *profile);

#endif
