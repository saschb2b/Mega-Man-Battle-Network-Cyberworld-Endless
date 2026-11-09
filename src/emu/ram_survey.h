/* The RAM survey: which bytes of the GBA's work RAM the game ever holds
 * anything in, and a canary over a stretch thought free
 * (docs/devtools/ram-survey.md). A dev tool, off without its variables:
 * - CYBERWORLD_RAM_SURVEY=FILE: at exit, FILE holds a byte for each byte of
 *   EWRAM (0x40000) then IWRAM (0x8000), 1 where a frame ended with that
 *   byte not zero; a FILE already there is read first, so runs add up.
 * - CYBERWORLD_RAM_CANARY=ADDR:LEN[,ADDR:LEN...]: each stretch, found all
 *   zero, is filled with a pattern; a byte of it changed by a frame's end is
 *   told on stderr (the first few a stretch) and counted, and the stretch
 *   filled again. A stretch the engine's own regions hold cannot be one. */
#ifndef CW_RAM_SURVEY_H
#define CW_RAM_SURVEY_H

#include <stdbool.h>
#include <stdint.h>

/* The variables read: true where either is set. */
bool ram_survey_open(void);
/* After a frame: `ewram` the 0x40000 bytes from 0x02000000, `iwram` the
 * 0x8000 from 0x03000000. */
void ram_survey_frame(uint8_t *ewram, uint8_t *iwram);
#endif
