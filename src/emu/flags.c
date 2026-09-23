#include "flags.h"

#include <stdint.h>

#include "bn6.h"
#include "emu.h"

static uint32_t byte_of(int flag) { return BN6_EVENT_FLAGS + (uint32_t)flag / 8u; }
static uint8_t bit_of(int flag) { return (uint8_t)(0x80u >> (flag & 7)); }

bool flag_get(int flag) { return emu_read8(byte_of(flag)) & bit_of(flag); }
void flag_set(int flag) { emu_write8(byte_of(flag), emu_read8(byte_of(flag)) | bit_of(flag)); }
void flag_clear(int flag) { emu_write8(byte_of(flag), emu_read8(byte_of(flag)) & (uint8_t)~bit_of(flag)); }
