/* UNITE on BN6's Custom screen (issue #82, the Souls track's S2; docs/
 * MULTIROM.md, Transplanted Souls): BN5's Souls unite in BN6's battles by
 * BN5's rule, each behind the proxy form (soulproxy.h), its look and its
 * abilities HeatCross's until the track's next milestones bring BN5's.
 * Element 0xB under OK is BN6's Beast Out emblem and BN5's UNITE button
 * in one place (docs/ROM_DATA.md, UNITE on BN6's Custom screen): it is
 * UNITE while the last chip picked lights a held Soul (BN5's check: the
 * chip of the Soul's kind as BN5 has it, soulkinds.h; a DarkChip of it
 * for its Chaos Unison; each once a battle), the emblem otherwise where
 * Beast Out stands, and a grey UNITE where only UNITE stands. UNITE stands
 * while the run holds a Soul and MegaMan is neither worried nor dark by
 * BN5's mood (his battle mood, opened as worried as his HP says) nor
 * beasted out; A on it runs BN5's own code for it, moved, into BN6's own
 * mode for a UNITE chosen (0x24, which Gregar keeps and never enters); B
 * gives the chip back; OK commits the proxy form with the Soul's turns (3,
 * Chaos Unison 1) and its used bit. Beast Out stays grey while a Soul is
 * united. A Soul's life from there is soullife.h's. */
#ifndef CW_SOULUNITE_H
#define CW_SOULUNITE_H

#include <stdbool.h>

/* (--dev unitebreak=N, the differential's own test: each must part BN6's
 * unison from BN5's, tests/xdiff/mutations.json) */
enum {
	SOULUNITE_BREAK_TURNS = 1,   /* a Soul committed with 4 turns where BN5 gives 3 */
	SOULUNITE_BREAK_USED = 2,    /* its used bit not set: the same Soul again in the battle */
	SOULUNITE_BREAK_KIND = 4,    /* a Soul lit by the kind after its own */
	SOULUNITE_BREAK_GREY = 8,    /* Beast Out not greyed while a Soul is united */
	SOULUNITE_BREAK_LEAVE = 16,  /* the Soul kept as MegaMan leaves the proxy form (a Cross chosen over it) */
};

/* The Souls in BN6's engine as the core starts, over the proxy form
 * (soulproxy_install first, then this): the Custom screen's hooks, a
 * Soul's life (soullife_install), BN5's A on UNITE moved and its art and
 * the Souls' pictures copied from BN5's ROM into the Souls' free space,
 * mode 0x24's pictures pointed at them. Once a core; false (nothing
 * written) where BN5's ROM is not read or a byte it stands on is not as
 * expected. `breaks` SOULUNITE_BREAK_*, 0 none. */
bool soulunite_install(int breaks);
/* The run's Souls, bit k BN5's Soul 7 + k (souls.h), for the battles from
 * the next Custom screen on: the director's as each battle begins, the
 * differential's from its spec. None (0) leaves BN6's Custom screen as it
 * is, byte for byte. */
void soulunite_held(unsigned souls);

#endif
