/* library_text.h. The classes are named as BN6's Library names its tabs,
 * a slot never filled as its StdChip list shows one. */
#include "library_text.h"

#include <stdio.h>

const char library_title[] = "LIBRARY";
const char *const library_entry_names[LIB_ENTRIES] = { "BN6", "BN5", "BN4" };
const char *const library_class_names[LIB_CLASSES] = { "StdChip", "MegaChip", "GigaChip" };
const char library_unseen[] = "??";

void library_count_words(char *out, size_t n, int held, int of) { snprintf(out, n, "%d/%d", held, of); }

void library_entry_words(char *out, size_t n, int entry, int held, int of) {
	snprintf(out, n, "%s %d/%d", library_entry_names[entry], held, of);
}
