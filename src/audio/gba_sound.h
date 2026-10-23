/* The GBA's sound, after MP2K (mp2k.h): its registers (0x04000060-0x040000AF)
 * as the game reads and writes them, the PSG (mGBA's own, GBAudio, run on a
 * clock of its own), DirectSound's two FIFOs played at the mix's rate, and
 * the output mixed as mGBA 0.10.5's GBAAudioSample mixes it (the PSG's
 * ratio, the FIFOs at 100%, the bias, 10 bits, its master volume) into
 * blip_buf at the GBA's clock, read at 48 kHz: what mGBA plays of BN6. */
#ifndef CW_GBA_SOUND_H
#define CW_GBA_SOUND_H

#include <stdint.h>

/* The registers as BN6's start leaves them (SoundInit, MPlayExtender) */
void gba_sound_reset(void);
/* A register's byte written or read, as the game does (`reg` from 0x60);
 * a word of the wave RAM (0x90-0x9C) */
void gba_sound_write8(int reg, uint8_t value);
void gba_sound_write32(int reg, uint32_t value);
uint8_t gba_sound_read8(int reg);
/* One GBA frame (280896 cycles) of output, DirectSound playing `n` samples
 * of each side (the ring's part MP2K mixed the frame before) */
void gba_sound_frame(const int8_t *right, const int8_t *left, int n);
/* 48 kHz stereo frames ready, and up to `frames` of them taken */
int gba_sound_avail(void);
int gba_sound_read(int16_t *out, int frames);

#endif
