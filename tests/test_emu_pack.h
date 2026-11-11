/* The chip pack's routines and its entries in the cart's save RAM, run on
 * mGBA (tests/test_emu_pack.c): after test_emu.c's emu_init. The checks
 * that failed. */
#ifndef CW_TEST_EMU_PACK_H
#define CW_TEST_EMU_PACK_H

#include <stdint.h>

int pack_checks(const uint8_t *rom, uint32_t rom_size, uint32_t *video);

#endif
