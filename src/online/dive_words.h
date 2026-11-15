/* What the shared dive says (docs/online/shared-dive.md; the voice rules,
 * docs/voice/rules.md): the feed's lines as the server tells them. */
#ifndef CW_DIVE_WORDS_H
#define CW_DIVE_WORDS_H

#include <stdint.h>

/* A line of the feed: `kind` ONLINE_FEED_*, the diver's name, the layer,
 * a guardian's party's size; "" where the kind is unknown. */
const char *dive_feed_words(unsigned kind, const char *name, unsigned depth, unsigned party);

#endif
