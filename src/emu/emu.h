/* The embedded GBA core that runs the game's own code (docs/EMULATION.md). */
#ifndef CW_EMU_H
#define CW_EMU_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hook.h"

#define EMU_W 240
#define EMU_H 160
#define EMU_ROM_SIZE 0x1000000u  /* the in-memory ROM: the game, then free space */
#define EMU_FREE 0x08800000u      /* first free bus address past an 8 MB game */

/* GBA key bits */
enum {
	KEY_A = 1 << 0, KEY_B = 1 << 1, KEY_SELECT = 1 << 2, KEY_START = 1 << 3,
	KEY_RIGHT = 1 << 4, KEY_LEFT = 1 << 5, KEY_UP = 1 << 6, KEY_DOWN = 1 << 7,
	KEY_R = 1 << 8, KEY_L = 1 << 9,
};

/* Starts the core on a private copy of the ROM (writes to ROM space patch
 * only that copy) and resets it to power-on. */
bool emu_init(const uint8_t *rom, size_t len);
/* mGBA's log for every core, quiet (emu_init sets it; the guest's core,
 * made first in the browser, guest.c, too): its debug lines cost the
 * page a console line each */
void emu_log_quiet(void);
bool emu_ready(void);
void emu_reset(void);

/* Runs one frame with these keys held. Where the core has a thread of
 * its own (emu_threaded), starts it: the next access to the core waits for
 * it, and emu_video shows the frame before until then. */
void emu_frame(uint32_t keys);
bool emu_threaded(void);
/* Waits for a frame the core runs on its own thread: what its hooks left
 * is the main thread's to read after it. */
void emu_sync(void);
/* Set while a frame is drawn: a read of the game then, with the core on a
 * thread of its own, waits for the next frame (counted in emu_draw_waits,
 * which the frame log shows). */
extern bool emu_drawing;
extern int emu_draw_waits;
/* Ends the core's thread, before the program ends. */
void emu_quit(void);
/* The last frame, 240x160 pixels, R in the low byte (SDL ABGR8888). */
const uint32_t *emu_video(void);
/* Another core's sound (a struct mCore: the guest's, guest.c) into the
 * same ring BN6's plays from, and the rate it is played at. */
void emu_audio_from(void *core);
/* ... or its sound let go, for frames run ahead unseen */
void emu_audio_drop_from(void *core);
int emu_audio_out_rate(void);

/* Bus access (ROM, EWRAM, IWRAM, IO, palette, VRAM, OAM). */
uint8_t emu_read8(uint32_t addr);
uint16_t emu_read16(uint32_t addr);
uint32_t emu_read32(uint32_t addr);
void emu_write8(uint32_t addr, uint8_t v);
void emu_write32(uint32_t addr, uint32_t v);
void emu_write(uint32_t addr, const void *data, size_t len);

/* A hook on the game's code (hook.h; docs/EMULATION.md, Hooks), set
 * between frames: on the Thumb instruction at addr, an answer hook or an
 * event hook queuing `kind`; and taken off. */
bool emu_hook(uint32_t addr, EmuHook fn, void *user);
bool emu_hook_event(uint32_t addr, int kind);
void emu_unhook(uint32_t addr);
/* The events the hooks queued in the frames run since the last call,
 * oldest first: how many. */
int emu_hook_events(HookEvent *out, int max);

/* Save states in the data directory (made on the device, never shipped). */
bool emu_save_state(const char *path);
bool emu_load_state(const char *path);

/* Stereo 16-bit samples at the given rate, pulled by the audio device.
 * Returns the frames written; silence fills the rest. */
int emu_audio_read(int16_t *out, int frames);
void emu_audio_rate(int rate);

#endif
