/* The shared dive's link (issue #589; docs/online/shared-dive.md): the
 * Square's connection, kept as the week's portal warps MegaMan into the
 * dive, for the whole dive. Each layer MegaMan stands on is a room of the
 * server's (DIVE enter: the week, its seed, the layer's depth, side and
 * area); his place goes as the Square's does (STATE), the room's divers
 * come back (SNAPSHOT) as puppets on the layer (dive_people.h), and the
 * server's feed shows in a band. A link lost leaves the dive solo. */
#ifndef CW_DIVE_LINK_H
#define CW_DIVE_LINK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* The dive the week's portal chose, begun or gone on with (square_week.h),
 * and the visit's connection taken over for it; false where it did not
 * begin. */
bool dive_link_begin(void);
/* Whether a dive is under way (weekly.h). */
bool dive_link_diving(void);
/* The visit's connection taken over for the dive under way (a visit from
 * the dive back to it), or let go where it is not connected. */
void dive_link_adopt(void);
/* Whether a dive's link is open. */
bool dive_link_on(void);
/* Each frame of the game while on: the room, the place, the divers, the
 * dive's end told and the link closed. */
void dive_link_frame(void);
void dive_link_draw(void);
/* The keys on their way to the game at home or on a dive: the party's at a
 * guardian's door, a crew's A on a diver, a phrase left on its layer. */
uint32_t dive_link_keys(uint32_t keys);
/* A line into the feed's band (a find's first finder, a split's place). */
void dive_link_line(const char *line);
/* The dive's messages, from online_mail_frame. */
void dive_link_mail(const uint8_t *m, size_t n);
/* The link closed at once (the game shut down). */
void dive_link_close(void);
/* The game's scene left (to the title, a reset): the link closed, but for
 * the scene's own restart as the dive begins (dive_link_begin). */
void dive_link_scene_left(void);

#endif
