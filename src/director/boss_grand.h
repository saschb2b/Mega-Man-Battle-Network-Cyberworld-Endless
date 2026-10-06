/* The super bosses' staging (boss_grand.c; docs/BOSSES.md, Super bosses):
 * what boss.c's sequence does in their place, frame by frame. */
#ifndef CW_BOSS_GRAND_H
#define CW_BOSS_GRAND_H

#include <stdbool.h>
#include <stdint.h>

#include "guardian_objs.h"

/* MegaMan walks up this long, and stops this far (world units) from a
 * super boss, in a bigger arena than a guardian's */
#define GRAND_WALK_FRAMES 75
#define GRAND_WALK_NEAR   40

/* A new layer whose guardian is a super boss (`g`, its scripts in
 * `archive`). */
void grand_begin(uint32_t archive, const GuardianStage *g);
/* While he waits, MegaMan `near` his arena (in the room before it or on
 * its bridge): the theme fades and comes back, the floor shakes. */
void grand_approach(bool near);
/* The entrance, frame `t` from MegaMan's step in: false while it runs;
 * true once his title card shows. */
bool grand_enter(int t);
/* The fall after his last word, frame `t`: false while it runs, true once
 * his data may show. */
bool grand_fall(int t);
/* Whether the area's theme comes back once he has fallen (Bass's area's;
 * after the Cybeast the Net stays quiet). */
bool grand_theme_after(void);

#endif
