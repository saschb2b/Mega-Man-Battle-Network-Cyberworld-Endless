/* The emote strip's and the palette's PET text (emote_strip_text.c;
 * docs/online/square/emotes.md): the pages' names, the keys' hints, whom an
 * emote is aimed at. The emotes' and phrases' own names are the
 * protocol's (online_text.h). */
#ifndef CW_EMOTE_STRIP_TEXT_H
#define CW_EMOTE_STRIP_TEXT_H

/* the two pages, L and R between them, and the palette's tab beside them */
extern const char *const strip_text_pages[2];
extern const char strip_text_netpass[];
/* the keys' hints: the favourites' box's, the palette's */
extern const char strip_text_favourites[];
extern const char strip_text_keep[];
extern const char strip_text_tap[];
extern const char strip_text_on_l[];
/* whom it goes to: "AT name" aimed, "TO EVERYONE" not */
const char *strip_text_aim(const char *name);
/* a favourite just kept: "ON L+UP" for way 0-3 */
const char *strip_text_kept(int way);

#endif
