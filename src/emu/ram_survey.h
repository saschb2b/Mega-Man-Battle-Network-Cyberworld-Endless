/* The RAM survey: which bytes of the GBA's work RAM the game ever holds
 * anything in, and a canary over a stretch thought free
 * (docs/devtools/ram-survey.md). A dev tool, off without its variables:
 * - CYBERWORLD_RAM_SURVEY=FILE: at exit, FILE holds a byte for each byte of
 *   EWRAM (0x40000) then IWRAM (0x8000), 1 where a frame ended with that
 *   byte not zero; a FILE already there is read first, so runs add up.
 * - CYBERWORLD_RAM_CANARY=ADDR:LEN[,ADDR:LEN...]: each stretch, found all
 *   zero, is filled with a pattern; a byte of it changed by a frame's end is
 *   told on stderr (the first few a stretch) and counted, and the stretch
 *   filled again. A stretch the engine's own regions hold cannot be one.
 * - CYBERWORLD_RAM_WATCH=ADDR:LEN[,ADDR:LEN...]: each read and write the
 *   game's code makes of a stretch (the work RAM's or the cart's save
 *   RAM's, 0x0E000000; a mirror's address counted as its first image's),
 *   through the calls the core's CPU reaches memory by (its loads, stores,
 *   DMA and the BIOS's copies; not an instruction fetched, nor the engine's
 *   own reads): told on stderr once a place (the instruction, read or
 *   written, the access's size), and summed at exit. Nothing is changed:
 *   the run is the run without it. */
#ifndef CW_RAM_SURVEY_H
#define CW_RAM_SURVEY_H

#include <stdbool.h>
#include <stdint.h>

struct mCore;

/* The variables read: true where any is set. */
bool ram_survey_open(void);
/* A core's memory calls put under the watch (nothing without
 * CYBERWORLD_RAM_WATCH): the game's, and a NetBattle's pair's. */
void ram_survey_watch(struct mCore *core);
/* After a frame: `ewram` the 0x40000 bytes from 0x02000000, `iwram` the
 * 0x8000 from 0x03000000. */
void ram_survey_frame(uint8_t *ewram, uint8_t *iwram);
#endif
