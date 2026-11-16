/* The older nets in the state's words (older_describe.c; docs/devtools/
 * agents-tools.md, tests/scenarios' xrom_*): BN5's and BN4's areas, Navis,
 * viruses, chips and Souls as a run with their ROMs holds them, a fact a
 * line, each line's first word its own. It reads the run, the layer and
 * the battle's RAM, and writes nothing.
 *
 * In a run: `acts` (each act's area by its game, bn6, bn5 or bn4, in the
 * run's order), `olderacts N`, `olderguards N` (acts ending at an older
 * net's Navi). On a layer: `areafrom GAME`, `guardianfrom GAME`,
 * `guardianstands ANIM front|back|unseen` (his NPC's animation, and
 * whether his sprite draws it and faces the camera). In a battle:
 * `enemies N`, `enemyhp N` (theirs together), `enemynames A+B`,
 * `olderfoes N` (those moved in from an older net), `olderrecord ADDR|none`
 * (the older net's own record it was rolled from); on the Custom screen
 * `drawn ID+CODE ...` and `picked N`, after it `hand N`, `nextchip ID`,
 * `nextchipfrom GAME`, `nextpower N`; and where the Souls stand in the engine `soulsheld MASK`,
 * `unite lit|grey|none` (what stands under OK), `united N` (BN5's Soul's
 * number, 0 none), `unitedturns N`, `soulsused MASK`. */
#ifndef CW_OLDER_DESCRIBE_H
#define CW_OLDER_DESCRIBE_H

#include <stdio.h>

void older_describe(FILE *f);

#endif
