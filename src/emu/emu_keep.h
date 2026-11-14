/* RAM kept patched (emu_keep.c): the patches emu.h's emu_keep makes the
 * engine's, kept for both cores (emu.c's mGBA, emu_gpsp.c's gpSP), each
 * reading and writing its memory raw. */
#ifndef CW_EMU_KEEP_H
#define CW_EMU_KEEP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t (*EmuRawRead)(uint32_t addr);
typedef void (*EmuRawWrite)(uint32_t addr, uint8_t v);

/* A patch kept: `len` bytes at `addr`, the game's own `was`, the engine's
 * `now`; false where there is no room for one more */
bool emu_keep_add(uint32_t addr, const void *was, const void *now, size_t len);
/* Each patch's bytes made the engine's, whatever they read: the game's
 * own, or another build's patch, which a state that build saved may hold
 * (a checkpoint from before the game's own were saved there, its hit
 * gate's calls going where that build's code lay: issue #482, the game
 * stood still at the first hit). `own`: the game's again where they read
 * the engine's, for a state's save; bytes that read neither stay. */
void emu_keep_put(bool own, EmuRawRead rd, EmuRawWrite wr);

#endif
