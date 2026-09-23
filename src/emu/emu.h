/* The embedded GBA core that runs the game's own code (docs/EMULATION.md). */
#ifndef CW_EMU_H
#define CW_EMU_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
bool emu_ready(void);
void emu_reset(void);

/* Runs one frame with these keys held. */
void emu_frame(uint32_t keys);
/* The last frame, 240x160 pixels, R in the low byte (SDL ABGR8888). */
const uint32_t *emu_video(void);

/* Bus access (ROM, EWRAM, IWRAM, IO, palette, VRAM, OAM). */
uint8_t emu_read8(uint32_t addr);
uint16_t emu_read16(uint32_t addr);
uint32_t emu_read32(uint32_t addr);
void emu_write8(uint32_t addr, uint8_t v);
void emu_write32(uint32_t addr, uint32_t v);
void emu_write(uint32_t addr, const void *data, size_t len);

/* Save states in the data directory (made on the device, never shipped). */
bool emu_save_state(const char *path);
bool emu_load_state(const char *path);

/* Stereo 16-bit samples at the given rate, pulled by the audio device.
 * Returns the frames written; silence fills the rest. */
int emu_audio_read(int16_t *out, int frames);
void emu_audio_rate(int rate);

#endif
