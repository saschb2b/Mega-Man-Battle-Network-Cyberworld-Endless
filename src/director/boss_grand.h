/* The super bosses' staging (boss_grand.c; docs/BOSSES.md, Super bosses):
 * what boss.c's sequence does in their place, frame by frame. */
#ifndef CW_BOSS_GRAND_H
#define CW_BOSS_GRAND_H

#include <stdbool.h>
#include <stdint.h>

#include "guardian_objs.h"

/* MegaMan's steps up to a super boss take this long at most, across a
 * bigger arena than a guardian's; where the layer gives him no place
 * beside him (guardian_stand), he stops this far (world units) from him */
#define GRAND_WALK_FRAMES 150
#define GRAND_WALK_NEAR   40

/* A new layer whose guardian is a super boss (`g`, its scripts in
 * `archive`). */
void grand_begin(uint32_t archive, const GuardianStage *g);
/* While he waits, MegaMan `near` his arena (in the room before it or on
 * its bridge): the theme fades and comes back, the floor shakes. */
void grand_approach(bool near);
/* The entrance, frame `t` from MegaMan's step in, `there` once he has
 * stepped up to him (its clock stops short of the white till then): false
 * while it runs; true once his title card shows. */
bool grand_enter(int t, bool there);
/* The fall after his last word, frame `t`: false while it runs, true once
 * his data may show. */
bool grand_fall(int t);
/* Whether the area's theme comes back once he has fallen (Bass's area's;
 * after the Cybeast the Net stays quiet). */
bool grand_theme_after(void);

#endif
