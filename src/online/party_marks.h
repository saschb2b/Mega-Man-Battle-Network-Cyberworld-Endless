/* Who is who on a party's field (issue #713; docs/online/shared-dive.md,
 * Who is who): each MegaMan of the party marked by his feet with his
 * diver's name, cut to what a panel holds, the player's own behind the
 * PET's arrow in its orange, the others in the crew's colour; and a partner's
 * own HP box over BN6's, which shows the lead's. Our own drawing over the
 * field's picture, as the Arena's two a side marks his seats
 * (arena_team_view.h): read from the field's core as it stands for the
 * frame shown, so a rollback leaves nothing behind; it changes nothing,
 * and nothing of it is in the field's hash. */
#ifndef CW_PARTY_MARKS_H
#define CW_PARTY_MARKS_H

#include <stdint.h>
#include <stdio.h>

#include "party_field.h"

struct mCore;

/* the party's seats by their sessions (START's), `mine` this game's: each
 * one's name as the server has it now (this game's own the visit's), or
 * "P1" to "P4" where it has none (the dev's stood-in partners: NULL) */
void party_marks_seats(const uint32_t seats[PARTY_MOST], unsigned mine);
/* Over the picture of the field's core (at P.core_x, P.core_y), as seat
 * `mine` of `n` sees it */
void party_marks_draw(struct mCore *field, unsigned mine, unsigned n);
/* ... in the state's words (online_describe.h): "hpbox own" or "bn6",
 * each seat's mark ("markK own" or "crew", its left and top on the
 * picture, its width, its tag) and whether any two meet ("marks N apart") */
void party_marks_describe(FILE *f, struct mCore *field, unsigned mine, unsigned n);

#endif
