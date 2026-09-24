/* How hard a run is at each depth (docs/PROGRESSION.md): the act a depth
 * belongs to, what a battle there may hold, which areas and guardians each
 * act draws from. Numbers only; the ROM's HP values reach it through the
 * callers, so it builds and tests without a ROM. */
#ifndef CW_PACING_H
#define CW_PACING_H

#include <stdbool.h>
#include <stdint.h>

#define PACING_ACTS 7       /* six acts of three layers, then the Nest */

/* The act of a depth, 0-5, and 6 for the Nest; the cycle, 0 on the first. */
int pacing_act(int depth);
int pacing_loop(int depth);

/* What one battle may hold: its viruses' HP together (lo is the aim, hi the
 * limit) and the strongest one's damage. A challenge is held to the next
 * act's; `easy` keeps to the lower half. */
typedef struct { int lo, hi, cap; } PacingBand;
PacingBand pacing_band(int depth, bool challenge, bool easy);
/* The same band one act further, for an area that has nothing inside it. */
PacingBand pacing_band_wider(PacingBand b);

/* The highest virus version (0 V1 .. 3 SP) a battle at `depth` aims for. */
int pacing_virus_version(int depth, bool challenge);
/* Whether a battle there may turn one virus rare (from the third act on,
 * more often on later cycles), on a roll of 0-99. */
bool pacing_rare(int depth, int roll);

/* Acts 1-4 take four areas by their own battles: the first from the
 * opening areas, the second from those or the middle ones, the third from
 * the middle or late ones, the fourth from the late ones (BIOME_* values). */
void pacing_area_order(uint8_t out[4]);

/* A guardian's HP band in an act (the Nest's is open). */
void pacing_guardian_band(int act, int *lo, int *hi);
/* The version (0 V1, 1 EX, 2 SP) navi is fought at in an act: SP in the
 * Nest, the Secret Area and later cycles; else the first allowed version
 * whose HP (hp(navi, version), -1 unknown) lies in the act's band, or the
 * one nearest to it. Acts 1-3 fight V1 only. */
int pacing_guardian_version(int navi, int act, int loop, bool always_sp, int (*hp)(int navi, int version));
/* How far navi's HP at that version lies outside the act's band (0 inside). */
int pacing_guardian_miss(int navi, int act, int loop, bool always_sp, int (*hp)(int navi, int version));
/* The guardian of an area in an act: from `pool` (4 slots, repeats weigh
 * more) the navis whose HP fits, else one of `others` that fits, else the
 * pool's nearest. */
int pacing_guardian_pick(const uint8_t pool[4], const uint8_t *others, int nothers, int act, int loop,
                         bool always_sp, int (*hp)(int navi, int version));

/* The layer of an act (0-2) that always has a heal and the Net Dealer, and
 * whether the heal is still certain on this cycle. */
bool pacing_heal_certain(int depth);

#endif
