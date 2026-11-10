/* BN5's NaviCust program SoulT+1 as BN6's program 48 (docs/rom/navicust.md,
 * Program 48; docs/game/navicust.md, The older Net's program): BN6's 48
 * numbers are all taken (47 is the NaviCust list's RUN item), so every
 * table a program's number indexes is grown by one in the core's copy
 * where BN5's ROM is read. The records (BN6's 189, then program 48's four
 * from BN5's own, its five by five shape set in BN6's seven by seven), the
 * effects (a routine of the engine's that adds one to the NaviStats'
 * extra turns, BN6_NAVI_TURN_EXTRA, six at most, as BN5's does), the
 * compression codes (a row of none), the descriptions (BN5's lines in
 * BN6's codes and frame) and the names (its own in a filler's entry),
 * each word of BN6's code that names a moved table repointed, and the
 * NaviCust's list made to go on past RUN's key item and list RUN last.
 * The engine's Souls read the byte (soulunite.c). BN6's own read of it
 * (0x0802A118) only fills a Cross's and a Beast Out's record's turns,
 * which no code of Gregar's reads: a Beast Out is timed by its own counter
 * (battle NaviStats +0x21) and lasts its three turns with the program
 * placed, to the frame (tests/xdiff/beast_soult.json; the owner's call:
 * Souls only).
 * All or nothing: every word changed is read first and must be BN6's own.
 * Not in a state: every boot writes the same. */
#ifndef CW_XNCP_H
#define CW_XNCP_H

#include <stdbool.h>

/* Once a core as it starts, where BN5's ROM is read (nothing without it);
 * false where the core's copy holds neither BN6's bytes nor ours. */
bool xncp_install(void);
/* Whether this core has program 48 (installed here): what hands it out
 * asks first. */
bool xncp_installed(void);

#endif
