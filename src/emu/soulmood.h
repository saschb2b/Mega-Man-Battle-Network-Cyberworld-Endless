/* BN5's mood and its DarkChip rule in BN6's battles (issue #82, the Souls
 * track's S7; docs/MULTIROM.md, One net: the design's finer calls, and
 * Transplanted Souls; docs/ROM_DATA.md, BN5's mood in BN6's battles): a
 * mood of BN5's per side in the Souls' block (BN6_SOUL_MOOD), kept by
 * BN5's rules beside BN6's own: a hit taken lowers it by the hit's own
 * (BN5 0x08012818, never under 1), a hit landed that is no counter raises
 * the attacker's by the hit's gain (0x080127FA, 0xFE at most), a counter
 * calms him to Full Synchro unless he is in a form (0x0801700C), BN6's
 * twins of BN5's other writers mirrored (the chip's Full Synchro and Anger,
 * Anger's start and end, a Custom screen's script), a Soul's unison and its
 * end calm him (0x08011A8C), and the battle opens as worried as his HP says
 * (the run's rule since issue #64: its BN5 battles opened so on the guest
 * core). A BN5 DarkChip used makes him dark for the
 * battle (0x0801007C): his mood 0, BN6's own too (no Cross, no Beast Out, a
 * weak buster), a latch of ours (BN6_SOUL_DARK, never BN6's byte 2) that
 * keeps both there. BN5's worried rule deals one of BN5's DarkChips into
 * the hand's end while he worries or is dark (0x08025118), and the deck's
 * compaction shelves the others where no draw reaches them (the run's
 * shelf since issue #64, four at most: the run's DarkChips are in its
 * folder, where BN5 keeps its own apart); BN6's own DarkChips keep BN6's rule
 * (darkbn6.c). UNITE reads this mood (soulunite.c), and BN5's dark gate
 * this dark: a chip of the registry's own whose BN5 record names the gate
 * (BlakWing, HolyDrem, a Navi's V1, SP and DS chips) is BN5's dud where it
 * is not to be used now (0x08010110). */
#ifndef CW_SOULMOOD_H
#define CW_SOULMOOD_H

#include <stdbool.h>
#include <stdint.h>

/* Who the mood's bytes are (claims.h) */
#define SOULMOOD_CLAIMS "BN5's mood (soulmood.c)"

/* (soulbreak's mood, soulunite.h's SOULBREAK_MOOD: the differential's own test, tests/xdiff/mutations.json) */
enum {
	SOULMOOD_BREAK_GAIN = 1,     /* a hit landed raises no mood */
	SOULMOOD_BREAK_RULE = 2,     /* no DarkChip dealt into a worried hand */
	SOULMOOD_BREAK_DARK = 4,     /* a BN5 DarkChip used leaves him as he was */
	SOULMOOD_BREAK_WORRY = 8,    /* UNITE stands while he worries or is dark */
	SOULMOOD_BREAK_GATE = 16,    /* a chip BN5's dark gate refuses used as it is */
	SOULMOOD_BREAK_AFTER = 32,   /* a BN5 DarkChip used leaves his NaviStats as they were */
};

/* (from a hook) side worried or dark by this mood (BN5's emotion getter's 1 or 5, a form and anger aside: 0x08012738) */
bool soulmood_worried(int side);
/* (from a hook) a Soul's unison begun or ended on side: BN5's calm (0x80,
 * where he is not dark: 0x08011A8C) */
void soulmood_unison(int side);
/* ... the Navi `me`'s (a side's partner's his own mood: soul_field_of) */
void soulmood_calm_of(uint32_t me);
/* (from a hook, the chip's use, r5 `me`) BN5's DarkChip use (0x08010028):
 * a DarkChip of BN5's (the registry's) makes him dark for the battle */
void soulmood_chip_used(uint32_t me, int chip);
/* (from a hook, the chip's use's end: r5 `me`, r7 `attack`, its variables) BN5's dark gate (0x08010110): where a chip
 * of the registry's own is not to be used now (BlakWing while he is not dark, HolyDrem while he is, a Navi's chips by
 * theirs), its attack made BN5's dud and true, *record the record its family is read from now */
bool soulmood_chip_refused(uint32_t me, uint32_t attack, int chip, uint32_t *record);

/* (the differential's item, dark=1: src/dev/xdiff.c through soulunite_dev) each battle begun dark as by BN5's dark meter under 470 (BN5's battle
 * opens at mood 0, 0x08010F14), his palette BN5's 2 for it */
void soulmood_dev_dark(bool on);

/* The hooks, once a core, after the Souls' code; false (nothing written)
 * where a byte it stands on is not Gregar's as expected. */
bool soulmood_install(int breaks);

#endif
