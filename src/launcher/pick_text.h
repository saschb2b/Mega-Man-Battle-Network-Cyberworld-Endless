/* The pickers' words for SAVES (pick_text.c): what a chooser is titled,
 * and why a file was not found or written, in the menus' plain style
 * (docs/voice/rules.md), apart from the platforms' code. */
#ifndef CW_PICK_TEXT_H
#define CW_PICK_TEXT_H

#include <stddef.h>

/* A desktop chooser's title: import (0), export (1), the folder (2) */
const char *pick_text_title(int kind);
/* No saves file in `where` */
void pick_text_none_found(const char *where, char *out, size_t n);
/* An export or a copy that could not be written */
const char *pick_text_not_written(void);
/* A folder that could not be kept */
const char *pick_text_folder_not_kept(void);
#ifdef __EMSCRIPTEN__
/* Where the browser's export went */
const char *pick_text_downloads(void);
#endif

#endif
