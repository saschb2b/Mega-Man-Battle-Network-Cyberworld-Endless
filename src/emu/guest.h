/* Another game's battles on a second core beside BN6's: BN5's own battles
 * in BN5's own engine, the reference the differential compares BN6's with
 * (src/dev/xdiff.c, build.py xdiff; docs/MULTIROM.md, Guest battles). A
 * run's battles are all BN6's since issue #82 (One net): this core takes
 * no part in runs. Every build but the 3DS's, whose heap holds no second
 * core. */
#ifndef CW_GUEST_H
#define CW_GUEST_H

#include <stdbool.h>
#include <stdint.h>

#include "hook.h"

/* Makes the guest core for extra ROM `xrom` (rom.h, XR) and boots it to a
 * playable state (its title, NEW GAME and intro, pressed through once and
 * kept as a state in the data directory, xdiff-bn5.state; that state
 * loaded later), at once, on this thread. True once it is ready; false
 * where it cannot be. */
bool guest_start(int xrom);

/* Its memory, bus addresses (its battle as bn5.h names it); 0 before it
 * is ready */
uint8_t guest_read8(uint32_t addr);
uint16_t guest_read16(uint32_t addr);
uint32_t guest_read32(uint32_t addr);
void guest_write16(uint32_t addr, uint16_t v);
void guest_write32(uint32_t addr, uint32_t v);

/* A battle of its game's as its game has it: `record` a BattleSettings of
 * `len` bytes, its entities from +0x10 (its pointer at +0xC set to them),
 * MegaMan at `hp` HP and max HP, its own buster, folder, mood and dark
 * meter, its RNG words from `seed` as it begins (battle_rng_word). From
 * the next guest frame it runs. False where it cannot be. */
bool guest_dev_battle(const uint8_t *record, uint32_t len, int hp, uint32_t seed);
/* A hook on its core's code (hook.h: its own table, its BKPT handler put
 * before its board's at the first); false where it is not ready, or the
 * hook does not fit. */
bool guest_hook(uint32_t addr, EmuHook fn, void *user);
/* One frame of the guest with these keys held (GBA key bits), its sound
 * dropped. */
void guest_frame_quiet(uint32_t keys);
/* The guest's last frame, 240x160, as emu_video's. */
const uint32_t *guest_video(void);

#endif
