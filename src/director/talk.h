/* Conversations the director starts (talk.c): the run's story beats and
 * MegaMan's answer to L. */
#ifndef CW_TALK_H
#define CW_TALK_H

#include <stdbool.h>
#include <stdint.h>

/* Runs `boxes` (ta_talk's, speaker marks and all; `face` for unmarked
 * boxes) now; false while another conversation or chat box is open. */
bool talk_start(const char *boxes, int face);
/* Runs script `script` of text archive `archive` the same way. */
bool talk_script(uint32_t archive, int script);
/* Each frame: gives the keys back once the conversation has closed. */
void talk_update(void);
bool talk_busy(void);
/* Forgets a conversation under way (a new run, a new layer). */
void talk_reset(void);

#endif
