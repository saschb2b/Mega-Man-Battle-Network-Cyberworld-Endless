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
/* A program's part on its card: a program part or a plus part (NAVI_*) */
const char *reward_pick_part(int part);
/* What the information box says: a program does `does` (navicust's
 * words); a chip the Library has or has not; HPMemory's `count`; the
 * draft's skip for `frags` BugFrags; into `out` */
void reward_pick_program_info(char *out, size_t n, const char *does, int held);
void reward_pick_chip_info(char *out, size_t n, bool in_library);
void reward_pick_hp_info(char *out, size_t n, int count);
void reward_pick_skip_info(char *out, size_t n, int frags);
/* The skip's row and HPMemory's row */
const char *reward_pick_skip_name(void);
const char *reward_pick_hp_name(void);

#endif
