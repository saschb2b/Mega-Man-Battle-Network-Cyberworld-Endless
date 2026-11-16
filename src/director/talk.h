/* Conversations the director starts (talk.c): the run's story beats and
 * MegaMan's answer to L. */
#ifndef CW_TALK_H
#define CW_TALK_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "text.h"

/* Runs `boxes` (ta_talk's, speaker marks and all; `face` for unmarked
 * boxes) now; false while another conversation or chat box is open. */
bool talk_start(const char *boxes, int face);
/* ... and then asks `question` (its end a '\n'), the cursor on No; Yes
 * sets event flag `yes_flag`. */
bool talk_ask(const char *boxes, int face, const char *question, int yes_flag);
/* Runs script `script` of archive `a`, built by the caller (a chat of
 * choices the director acts on), the same way. */
bool talk_archive(const TextArchive *a, int script);
/* Runs script `script` of text archive `archive` the same way. */
bool talk_script(uint32_t archive, int script);
/* Each frame: gives the keys back once the conversation has closed. */
void talk_update(void);
bool talk_busy(void);
/* No talk begins while on (a quit running the game on to MegaMan free: director_settle.c) */
void talk_refuse(bool on);
bool talk_refused(void);
/* A chat box's question, for a state (director_describe; tests/scenarios
 * reads it): `choice K/N`, the cursor on option K of the N shown, while
 * BN6's box shows a choice; no line else, nothing changed */
void talk_describe(FILE *f);
/* Forgets a conversation under way (a new run, a new layer). */
void talk_reset(void);

#endif
