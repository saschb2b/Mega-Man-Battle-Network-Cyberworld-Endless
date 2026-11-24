/* looks_got_words.h. */
#include "looks_got_words.h"

#include <stdio.h>
#include <string.h>

#include "looks.h"

const char *looks_got_box(int p) {
	static char box[64];
	const char *name = looks_palette_name(p);
	/* (BN6's box is about twenty characters a line: a long name puts
	 * "colors" on a line of its own) */
	if (strlen(name) + 9 <= 20) snprintf(box, sizeof box, "MegaMan got:\n\"%s colors\"!!", name);
	else snprintf(box, sizeof box, "MegaMan got:\n\"%s\"\ncolors!!", name);
	return box;
}
