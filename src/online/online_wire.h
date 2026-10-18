/* Byte order and validation shared by the native and browser transports. */
#ifndef CW_ONLINE_WIRE_H
#define CW_ONLINE_WIRE_H

#include "online_types.h"

void online_put16(uint8_t *out, uint16_t value);
void online_put32(uint8_t *out, uint32_t value);
bool online_wire_name(const char *name);
bool online_wire_message(const uint8_t *data, size_t size, uint32_t now, OnlineSnapshot *snapshot);
bool online_wire_profile(uint8_t out[ONLINE_PROFILE_BYTES + 1], const OnlineProfile *profile);

#endif
