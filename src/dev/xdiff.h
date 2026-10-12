/* --xdiff SPEC (tools.c; docs/DEVTOOLS.md, The differential): a family
 * moved into BN6's battle engine (src/emu/xvirus.c, issue #82) against
 * its own game's, frame by frame. One battle is fought twice: in BN5's own
 * engine on the guest core (the reference, guest_dev_battle: BN5's battle
 * as BN5 has it) and in BN6's with the families installed (the subject,
 * forced as the foes step forces one, the ids BN6's), a frame of each in
 * turn once each fights, the same keys given by each battle's timer, the
 * same RNG words written in both once the fight begins, and each fight
 * frame of each written to a trace for tools/xdiff.py, which compares them
 * by role. MegaMan can be the subject in turn (the Souls track): his setup
 * the same in both (flags, folders, decks, buster, safe), each Custom
 * screen passed by its own keys in each game, his player block traced (with
 * the Souls' state in BN6's engine and the Custom screen's UNITE).
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
 *   flags5=0000,000C   event flags (hex) set in BN5 before its battle (eToolkit
 *                      +0x44: Double Soul's 0000, a Soul's 0008 + k, Chaos
 *                      Unison's 0236); flags6= BN6's
 *   folder5=342C,...   each game's first folder (hex chip | code << 9, the
 *   folder6=3401       list repeated to 30), each chip owned, no Regular
 *                      chip or TagChips
 *   buster=A:S:C       MegaMan's Attack, Speed and Charge in both (levels
 *                      less one, 0-4)
 *   safe=1             MegaMan kept from harm: his collision data's
 *                      BN6_OBJ_INVULNERABLE before every frame of the battle
 *   cust5=1:A.4/-.8    a Custom screen's keys in BN5 (cust6= BN6's): screen
 *                      n (1 the first) from the frame its window first reads
 *                      open, each step BUTTONS.FRAMES ("-" none); a screen
 *                      with none passes with START, then A
 *   deck5=12D8,...     each game's battle deck in this order (hex chip |
 *   deck6=128C,...     code << 9, the list repeated to 30), written once its
 *                      battle runs and before its first Custom screen: each
 *                      hand its spec's
 *   souls6=10          the Souls in BN6's engine (soulunite.h, over the
 *                      proxy form), MegaMan holding these (hex, bit k BN5's
 *                      Soul 7 + k)
 *   unitebreak=N       (the comparator's own test) UNITE's rule broken in
 *                      BN6's engine (SOULUNITE_BREAK_*)
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
