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

/* MegaMan on a diver he faces (`mate`: one of the crew), and the
 * question to split up with the crew. */
const char *dive_crew_meet_words(const char *name, bool mate);
const char *dive_crew_split_question(void);

#endif
