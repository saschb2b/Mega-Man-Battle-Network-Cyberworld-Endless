/* What the GBA core on gpSP (emu_gpsp.c, CW_GPSP) gives beside emu.h. */
#ifndef CW_EMU_GPSP_H
#define CW_EMU_GPSP_H

#include <stddef.h>
#include <stdint.h>

/* The cart's save RAM (*size bytes), NULL before the game is loaded */
uint8_t *emu_gpsp_save_ram(size_t *size);
/* The ROM's pages the engine copied to write: the ROM's memory beyond the
 * player's own bytes */
size_t emu_gpsp_rom_owned(void);

#endif
