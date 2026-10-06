/* What the super bosses say, and MegaMan, Lan and Dad at their meetings
 * (super_lines.c; docs/VOICE.md, docs/BOSSES.md, Super bosses). */
#ifndef CW_SUPER_LINES_H
#define CW_SUPER_LINES_H

#include <stdbool.h>

#include "rivals.h"

/* Their meeting, by their record `r` and the form `version` they come in
 * (super_boss.h): Bass speaks, the Cybeast roars while MegaMan, Lan and
 * Dad speak; ta_talk's boxes, the battle's call last. */
const char *super_intro(int navi, int version, const Rival *r);
/* Their last word, beaten. */
const char *super_defeat(int navi, int version);
/* MegaMan's battle data on them, once fought in any run (guardian_tip's,
 * its last box when to strike). */
const char *super_tip(int navi);
/* What their data says as it is taken: its head ("MegaMan downloaded
 * ...") and MegaMan's words on it, the first time a profile takes it
 * (`first`) or after. */
const char *super_reward_head(int navi);
const char *super_reward_words(int navi, int version, bool first);
/* Their title card's lines (the words beside the name, as a guardian's
 * epithet): the top line and the line under the name, at `depth` (the Net
 * the Cybeast ends) */
void super_card(int navi, int version, int depth, const char **top, const char **sub);

#endif
