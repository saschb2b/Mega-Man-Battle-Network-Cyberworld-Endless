/* What MegaMan reads of the layer behind a way at a fork (fork_words.c;
 * docs/world/level-design/fork-read.md). */
#ifndef CW_FORK_WORDS_H
#define CW_FORK_WORDS_H

#include <stdbool.h>

/* Where he reads it: at a fork's exit pad (a dark warp on the layer too),
 * or at a portal of Lan's HP, whose own words have said the guardian */
enum { FORK_PAD, FORK_PORTAL };

/* His read of a layer that holds `holds` (HOLDS_* bits, net.h), `dark` a
 * dark layer's board: its two strongest calls, boxes marked as ta_talk's;
 * NULL at a portal with nothing to tell (the Net Dealer waits on every
 * act's first layer) */
const char *fork_words(unsigned holds, bool dark, int where);

#endif
