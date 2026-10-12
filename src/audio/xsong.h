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
/* A sound of extra ROM `xrom` (a song its battles start, issue #82's tracks)
 * in BN6, played by the players its game names for it (BN6's 32 are
 * BN5's): the BN6 song id that plays it, its slot of its own (BN5's sounds
 * that are no BN6 song, each at a fixed slot whichever track plays it),
 * copied there by the first install that needs it, after the areas' songs
 * and their games' themes; `fallback` where it has no slot or cannot be. */
int xsong_install_sound(int xrom, int song, int fallback);
/* The slot a sound of extra ROM `xrom` that is no BN6 song takes (BN6's
 * song of its number another), -1 for any other: what the differential
 * tells BN6's own song of that number from the copy by. */
int xsong_sound_slot(int xrom, int song);
/* The song a BN6 slot plays a copy of: its number in its own game (the
 * game in *xrom where not NULL), -1 for a slot of BN6's own. For a reader
 * comparing the two games' sounds (the differential). */
int xsong_copy_of(int slot, int *xrom);

#endif
