/* What the shared dive says (docs/online/shared-dive.md; the voice rules,
 * docs/voice/rules.md): the feed's lines as the server tells them. */
#ifndef CW_DIVE_WORDS_H
#define CW_DIVE_WORDS_H

#include <stdbool.h>
#include <stdint.h>

/* A line of the feed: `kind` ONLINE_FEED_*, the diver's name, the layer,
 * a guardian's party's size; "" where the kind is unknown. */
const char *dive_feed_words(unsigned kind, const char *name, unsigned depth, unsigned party);
/* The band at a guardian's door while a party forms: `count` in it, whether
 * MegaMan leads it, its leader's name. */
const char *dive_door_words(unsigned count, bool leads, const char *leader);

/* The band's join-in line (dive_join.h): his join into a crewmate's battle
 * sent, while it starts. */
const char *dive_join_joined_words(void);
/* The band's line as MegaMan opens a Mystery Data another opened first
 * this week (`mine` he is the first: `name` his own). */
const char *dive_find_words(const char *name, bool mine);
/* The band's line for a bust's place among the week's (DIVE_PLACE): its
 * time in tenths, its place of `count`. */
const char *dive_place_words(unsigned tenths, unsigned place, unsigned count);
/* A deletion mark's boxes (talk_start's, MegaMan's): who fell, to what
 * (`cause` its name, "" unknown; `navi` a Navi's); `won` a mark of the net cleared. */
const char *dive_mark_words(const char *name, const char *cause, bool navi, bool won);
/* A phrase left on a panel: its boxes (who left it, the phrase, how many
 * waved), MegaMan's wave after them where `wave`. */
const char *dive_phrase_words(const char *name, const char *phrase, unsigned waves, bool wave);

#endif
