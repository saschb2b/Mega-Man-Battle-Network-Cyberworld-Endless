/* Headless runs' input and pictures (capture.c), for main.c's command line
 * and frame. */
#ifndef CW_CAPTURE_H
#define CW_CAPTURE_H

#include <stdbool.h>

/* A headless run's option and its value (--input, --taps, --pad, --shot,
 * --screen-shot, --second-shot, --shot-range, --bot, --remote): true where
 * it was one */
bool capture_option(const char *a, const char *v);
/* Remote play's pipes opened where --remote asked; false where they
 * could not be */
bool capture_start(void);
/* The frame's scripted input (the bot's, remote play's batches, --input's
 * steps), before the platform's poll; the fingers --taps puts down, after */
void capture_input(void);
void capture_taps(void);
/* The pictures due this frame, once it is drawn */
void capture_shots(void);
/* --pad's virtual controller, or NULL */
const char *capture_pad_kind(void);

#endif
