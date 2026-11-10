/* The Library's labels (library.h; docs/game/meta/library.md): the
 * screen's name, the entries' on its tabs, BN6's own names for the
 * classes, and what a slot never filled shows. */
#ifndef CW_LIBRARY_TEXT_H
#define CW_LIBRARY_TEXT_H

#include <stddef.h>

#include "library_pages.h"

extern const char library_title[];
extern const char *const library_entry_names[LIB_ENTRIES];
extern const char *const library_class_names[LIB_CLASSES];
extern const char library_unseen[];
/* "12/200", a page's or an entry's count */
void library_count_words(char *out, size_t n, int held, int of);
/* "BN5 37/241", the entry's whole count on the band */
void library_entry_words(char *out, size_t n, int entry, int held, int of);

#endif
