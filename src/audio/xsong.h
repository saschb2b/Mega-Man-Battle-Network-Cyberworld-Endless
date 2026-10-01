/* Another Battle Network game's songs in BN6 (docs/MULTIROM.md): a song's
 * MP2K data copied from that game's ROM into BN6's in-memory free space,
 * its pointers moved, and entered in one of BN6's empty song slots, where
 * BN6's own sound engine plays it as one of its own. */
#ifndef CW_XSONG_H
#define CW_XSONG_H

#include <stdbool.h>
#include <stdint.h>

/* A song's sequence (`n` bytes from its first track to its header's end,
 * `base` its address in its own ROM) walked from each track's start
 * (`starts`, offsets in it) as the sequencer would: the offsets of the
 * pointers GOTO, PATT and REPT carry (`ptrs`, `maxptrs` at most), and the
 * voices VOICE selects, a bit per program in `programs`. False where a
 * pointer leaves the sequence or the lists are full. */
bool xsong_walk(const uint8_t *seq, uint32_t n, uint32_t base, const uint32_t *starts, int nstarts, uint32_t *ptrs, int *nptrs, int maxptrs,
                uint8_t programs[16]);

/* Song `song` of extra ROM `xrom` (XRomId) in BN6: the BN6 song id that
 * plays it, or `fallback` where it cannot be. The first call copies in
 * every song the other games' areas play (docs/MULTIROM.md), in their
 * order, so each has the same place in every session; again where the
 * core's ROM copy was made anew. */
int xsong_install(int xrom, int song, int fallback);

#endif
