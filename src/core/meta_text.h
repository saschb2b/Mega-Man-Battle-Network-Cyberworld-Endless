/* The meta layer's menu words (meta_text.c): the starting folders' names,
 * what each plays like and costs, and how each opens, which the setup's
 * Folder row, the summary and its closest goal say. */
#ifndef META_TEXT_H
#define META_TEXT_H

#include <stddef.h>

#include "meta.h"

/* Folder `folder`'s words (0 .. FOLDER_COUNT - 1; the Standard folder's
 * for any other; all NULL for an id no folder holds). */
const FolderInfo *meta_folder_words(int folder);

/* The summary's closest goal (meta_next_goal): what to do and what it
 * opens, a whole sentence for KeyItem's information box */
typedef enum { GOAL_ENDLESS, GOAL_FOLDER, GOAL_FOLDER_HINT, GOAL_CROSS, GOAL_THREAT } MetaGoal;
/* Goal `goal`'s sentence, made into `buf` (n bytes): `arg` the folder
 * (GOAL_FOLDER, its milestone; GOAL_FOLDER_HINT, its hint while unheard) or the threat to win on (GOAL_THREAT), else unread. */
const char *meta_goal_words(char *buf, size_t n, MetaGoal goal, int arg);

#endif
