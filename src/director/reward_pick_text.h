/* The reward pick's words (reward_pick_text.c): each screen's name, and
 * what its information box says of the row under the cursor. */
#ifndef CW_REWARD_PICK_TEXT_H
#define CW_REWARD_PICK_TEXT_H

#include <stdbool.h>
#include <stddef.h>

/* The screen's name on its band, by pick (PICK_*) */
const char *reward_pick_title(int kind);
/* The information box's label */
const char *reward_pick_label(void);
/* A program's part, on the information box's tag as KeyItem's: a
 * program part, a plus part or either (NAVI_*) */
const char *reward_pick_part(int part);
/* The box of what MegaMan holds of a choice, BN6's shops' label for it */
const char *reward_pick_items(void);
/* A chip new to the Library, marked on its row as E-Mail marks a new mail */
const char *reward_pick_new(void);
/* What the information box says of HPMemory (`count` against his max HP
 * now) and of the draft's skip (`frags` BugFrags), into `out`; a chip's is
 * its description, BN6's, a program's what it does, MegaMan's words of
 * the NaviCust */
void reward_pick_hp_info(char *out, size_t n, int count, int max_hp);
void reward_pick_skip_info(char *out, size_t n, int frags);
/* The skip's row and HPMemory's row */
const char *reward_pick_skip_name(void);
const char *reward_pick_hp_name(void);

#endif
