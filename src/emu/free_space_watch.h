/* The free space's plan (free_space.h, made from free_space.plan) at run
 * time: which region a place of the ROM copy is in, every write into the
 * copy's free space that runs past its region told on stderr whoever
 * writes it (emu.c's writes; an installer's claim, claims.h, only sees
 * another's bytes), and what a run used of each region
 * (docs/engine/emulation/free-space.md). */
#ifndef CW_FREE_SPACE_WATCH_H
#define CW_FREE_SPACE_WATCH_H

#include <stdbool.h>
#include <stdint.h>

/* The plan's region the place `at` of the ROM copy is in, its innermost
 * (a part rather than its parent); NULL outside the free space or in a gap
 * no region holds. */
const char *free_space_region(uint32_t at);
/* A write of `len` bytes at `at`: true where it is outside the free space or
 * inside one region of it; false where it runs past its region's end, into
 * a gap or over the next, told on stderr (the first few) and counted. */
bool free_space_write(uint32_t at, uint32_t len);
/* The writes told so far; and the next told without a word (a test's own). */
int free_space_strays(void);
void free_space_quiet(bool on);
/* From here on every write into the free space is kept, 16 bytes a bit
 * (64 KB, a measured run's alone: CYBERWORLD_FREE_SPACE). */
void free_space_measure(void);
/* What each region of the ROM copy holds: the end of its last byte that is
 * not the copy's 0xFF fill, read with `read8`, written to `path` a line a
 * region ("NAME END"); then, of a measured run, every span written during
 * it ("~ START END": what a room filled again per battle or layer reached
 * at its most, and what lies unwritten before a table at a region's end).
 * tools/free_space.py --usage FILE... reads it, several runs' as one. */
bool free_space_usage(const char *path, uint8_t (*read8)(uint32_t));

#endif
