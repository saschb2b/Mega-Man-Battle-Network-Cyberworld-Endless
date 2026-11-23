/* Who is who on a party's field (issue #713; docs/online/shared-dive.md,
 * Who is who): each MegaMan of the party marked by his feet with his
 * diver's name, cut to what a panel holds, the player's own behind the
 * PET's arrow in its orange, the others in the crew's colour, a partner's
 * HP under his name in BN6's own digits (hp_plate.h); and a partner's
 * own HP box over BN6's, which shows the lead's. Our own drawing over the
 * field's picture, as the Arena's two a side marks his seats
 * (arena_team_view.h): read from the field's core as it stands for the
 * frame shown, so a rollback leaves nothing behind; it changes nothing,
 * and nothing of it is in the field's hash. */
#ifndef CW_PARTY_MARKS_H
#define CW_PARTY_MARKS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "party_field.h"

/* a name's width at most: a panel's 40 pixels less its plate's edges and
 * a gap, so that two MegaMen on neighbouring panels of a row keep their
 * marks apart */
#define PARTY_MARKS_BUDGET 34

struct mCore;

/* the party's seats by their sessions (START's), `mine` this game's: each
 * one's name as the server has it now (this game's own the visit's), or
 * "P1" to "P4" where it has none (the dev's stood-in partners: NULL) */
/* whole letters of `name` that fit `budget` pixels of the pixel font with
 * `suffix` after them (the crew's plates cut a name so too, dive_crew_view.h) */
void party_marks_cut(const char *name, const char *suffix, int budget, char *out, size_t size);
void party_marks_seats(const uint32_t seats[PARTY_MOST], unsigned mine);
/* Over the picture of the field's core (at P.core_x, P.core_y), as seat
 * `mine` of `n` sees it */
void party_marks_draw(struct mCore *field, unsigned mine, unsigned n);
/* MegaMan down in a crew's battle, the field fighting on (issue #780): one
 * line in the band, what a win does for him (`spent`: his pull-back used,
 * a win does not bring him back) */
void party_marks_down(bool spent);
/* ... in the state's words (online_describe.h): "hpbox own" or "bn6",
 * each seat's mark ("markK own" or "crew", its left and top on the
 * picture, its width, its tag) and whether any two meet ("marks N apart") */
void party_marks_describe(FILE *f, struct mCore *field, unsigned mine, unsigned n);

#endif
