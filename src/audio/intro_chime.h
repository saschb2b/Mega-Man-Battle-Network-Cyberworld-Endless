/* The start's chime: "UI Success Chime" by SoundShelfStudio, from Pixabay
 * (pixabay.com/sound-effects/ui-success-chime-513565/, the Pixabay Content
 * License), 48 kHz mono; written by tools/intro_chime.py.
 * Its two tones begin INTRO_CHIME_TONE1 and INTRO_CHIME_TONE2 samples in. */
#ifndef CW_INTRO_CHIME_H
#define CW_INTRO_CHIME_H

#include <stdint.h>

#define INTRO_CHIME_RATE 48000
#define INTRO_CHIME_LEN 34036
#define INTRO_CHIME_TONE1 1203
#define INTRO_CHIME_TONE2 12004

extern const int16_t intro_chime[INTRO_CHIME_LEN];

#endif
