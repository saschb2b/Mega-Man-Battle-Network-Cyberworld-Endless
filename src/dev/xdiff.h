/* --xdiff SPEC (tools.c; docs/DEVTOOLS.md, The differential): a family
 * moved into BN6's battle engine (src/emu/xvirus.c, issue #82) against
 * its own game's, frame by frame. One battle is fought twice: in BN5's own
 * engine on the guest core (the reference, guest_dev_battle: BN5's battle
 * as BN5 has it) and in BN6's with the families installed (the subject,
 * forced as the foes step forces one, the ids BN6's), a frame of each in
 * turn once each fights, the same keys given by each battle's timer, the
 * same RNG words written in both once the fight begins, and each fight
 * frame of each written to a trace for tools/xdiff.py, which compares them
 * by role.
 *
 * SPEC, items apart by ';':
 *   foes=4F@62,01@51   BN5's enemy ids (hex) at their panels, column then row
 *   mm=22              MegaMan's panel (column, row; 22 where none)
 *   hp=100             MegaMan's HP and max HP in both
 *   frames=1200        the fight frames traced
 *   keys=150:UP:5,...  buttons held while the timer reads T to T+N-1 (A, B,
 *                      L, R, UP, DOWN, LEFT, RIGHT, START, SELECT joined by +)
 *   seed=P:S           the RNG words (hex) both cores take after the first
 *                      frame of the fight
 *   rng=follow         BN6's RNG words written from BN5's after each fight
 *                      frame (BN5's buster draws once more a shot than
 *                      BN6's: docs/DEVTOOLS.md, The differential)
 *   out=PATH           the traces: PATH.bn5 and PATH.bn6
 *   game=bn5|bn6       one of them alone
 *   poke=A:V[:N]       (the comparator's own test) N bytes (2 or 4) of V
 *                      written at A in BN6's ROM copy after the install, A
 *                      one of BN5's in a block it moved (its copy written)
 *                      or BN6's own
 *   call=F:T           (the same) the family's veneer on to BN6's routine F
 *                      aimed at T instead */
#ifndef CW_XDIFF_H
#define CW_XDIFF_H

/* The tool, in the game's place: 0 when both traces were written */
int xdiff_run(const char *spec);

#endif
