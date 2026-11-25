/* The core's sound on its way to the device (emu_sound.c): the ring both
 * cores write (emu.c's mGBA, emu_gpsp.c's gpSP) and emu.h's emu_audio_*
 * read. */
#ifndef CW_EMU_SOUND_H
#define CW_EMU_SOUND_H

#include <stdbool.h>
#include <stdint.h>

/* Whether the ring is made (emu_audio_open) */
bool emu_sound_ready(void);
/* `n` stereo frames into the ring, past those of a rollback's sound heard
 * already; the rest dropped where it is full */
void emu_sound_put(const int16_t *frames, int n);
/* A step that adds to each frame's sound heard of BN6's own core before it
 * goes into the ring (fast forward's music, music_fast.c: the engine's
 * MP2K playing the game's song at the GBA's pace, in step with the
 * effects); NULL none. On the core's thread where it has one. */
void emu_sound_mixer(void (*mix)(int16_t *frames, int n));
/* ... run on `n` stereo frames heard (emu.c's, emu_gpsp.c's) */
void emu_sound_mix(int16_t *frames, int n);
/* After a frame's sound: the ring cut back where it ran too far ahead, and
 * the nudge to the resampler's output rate that holds its fill near the
 * target (audio_pace.h) */
double emu_sound_settle(void);

#endif
