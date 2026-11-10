/* Another game's battles on a second core beside BN6's: BN5's own battles
 * in BN5's own engine, the reference the differential compares BN6's with
 * (src/dev/xdiff.c, build.py xdiff; docs/multirom/guest-battles.md), or
 * BN4 Red Sun's in its own (xdiff's game=bn4, src/dev/xdiff_bn4.c). A
 * run's battles are all BN6's since issue #82 (One net): this core takes
 * no part in runs. Every build but the 3DS's, whose heap holds no second
 * core. */
#ifndef CW_GUEST_H
#define CW_GUEST_H

#include <stdbool.h>
#include <stdint.h>

#include "hook.h"

/* Makes the guest core for extra ROM `xrom` (rom.h, XR: BN5 Team Colonel
 * or BN4 Red Sun, whose ROM it reads beside BN6's, xrom_research) and boots
 * it to a playable state (its title, NEW GAME and intro, pressed through
 * once and kept as a state in the data directory, xdiff-bn5.state or
 * xdiff-bn4.state; that state loaded later), at once, on this thread. One
 * game a process. True once it is ready; false where it cannot be. */
bool guest_start(int xrom);

/* Its memory, bus addresses (its battle as bn5.h or bn4.h names it); 0
 * before it is ready */
uint8_t guest_read8(uint32_t addr);
uint16_t guest_read16(uint32_t addr);
uint32_t guest_read32(uint32_t addr);
void guest_write16(uint32_t addr, uint16_t v);
void guest_write32(uint32_t addr, uint32_t v);

/* A battle of its game's as its game has it: `record` of `len` bytes, BN5's
 * a BattleSettings, its entities from +0x10 (its pointer at +0xC set to
 * them), BN4's a battle record, its entities from +0xC (its pointer at +8;
 * docs/rom/bn4.md, Battle records), MegaMan at `hp` HP and max HP, its own
 * buster, folder, mood and dark meter, its RNG words from `seed` as it
 * begins (battle_rng_word). From the next guest frame it runs. False where
 * it cannot be. */
bool guest_dev_battle(const uint8_t *record, uint32_t len, int hp, uint32_t seed);
/* A routine of BN5's that takes no arguments, run once on its map before a
 * battle is asked: from its main loop's next pass (BN5_MAIN_CALL), which
 * goes on as it was once the routine has returned; its frames run here.
 * False where it is not BN5 on its map, or the routine never returned. */
bool guest_call(uint32_t fn);
/* True from guest_dev_battle until its battle is over (back on its map, or
 * a GAME OVER) or never began */
bool guest_battling(void);
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
