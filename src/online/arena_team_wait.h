/* --dev arena2v2: the leads wait for their partners (epic #167, the
 * owner's call of 22 October 2026; docs/ONLINE_ARENA.md, The leads wait).
 * BN6's link battle closes its Custom screen once both GBAs' packets have
 * come over the cable (each GBA sends its own as its lead's OK has slid
 * the window out, BN6_LINK_SEND, and waits in mode 0x14 until it holds
 * both, BN6_CUSTOM_WAIT_LOOP). A side's GBA holds its packet back while
 * its partner picks: the other GBA waits for it as BN6 waits for a lead
 * still picking, and its own for itself, both by BN6's own cable, so the
 * moment the screen closes is one every client and spectator agrees on.
 * The hold is read in that GBA's RAM (arena_team_custom.h), which the
 * pair writes from the partner's stream; once his picks are in (at the
 * turn's limit his picks so far, or none, team_hurry.h), the packet goes
 * out. */
#ifndef CW_ARENA_TEAM_WAIT_H
#define CW_ARENA_TEAM_WAIT_H

#include <stdbool.h>

struct HookCore;

/* the two hooks on one of the pair's cores' table */
bool arena_team_wait_hooks(struct HookCore *core);
/* how often a packet was held back and sent late, said in the output */
void arena_team_wait_report(void);

#endif
