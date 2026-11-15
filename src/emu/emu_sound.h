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
/* After a frame's sound: the ring cut back where it ran too far ahead, and
 * the nudge to the resampler's output rate that holds its fill near the
 * target (audio_pace.h) */
double emu_sound_settle(void);

#endif
