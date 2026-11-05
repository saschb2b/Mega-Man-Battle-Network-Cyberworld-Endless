/* The extras' words (extras_text.c): each one's name, as the setup's
 * EXTRAS, the summary and the statistics name it, and what it is, a line
 * for the information box, in the plain style of the game's menus
 * (docs/voice/rules.md). */
#ifndef CW_EXTRAS_TEXT_H
#define CW_EXTRAS_TEXT_H

#include "meta.h"

/* Extra `e`'s (run.h's EXTRA_*): its name, what it is and what else to
 * know (NULL: nothing) */
const HelperInfo *extra_words(int e);

#endif
