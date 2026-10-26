/* The meta layer's menu words (meta_text.c): the starting folders' names,
 * what each plays like and costs, and how each opens, which the setup's
 * Folder row, the summary and its closest goal say. */
#ifndef META_TEXT_H
#define META_TEXT_H

#include "meta.h"

/* Folder `folder`'s words (0 .. FOLDER_COUNT - 1; the Standard folder's
 * for any other). */
const FolderInfo *meta_folder_words(int folder);

#endif
