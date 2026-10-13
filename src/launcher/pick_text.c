/* pick_text.h. Menu words: short and plain, the file named as the player
 * finds it, as the SAVES screen's (src/saves/saves_text.c). */
#include "pick_text.h"

#include <stdio.h>

#include "backup.h"

const char *pick_text_title(int kind) {
	static const char *const titles[] = { "Import saves", "Export saves", "Folder for the saves' copy" };
	return titles[kind >= 0 && kind < 3 ? kind : 0];
}

void pick_text_none_found(const char *where, char *out, size_t n) { snprintf(out, n, "No %s in %s.", BACKUP_NAME, where); }

const char *pick_text_not_written(void) { return "The file could not be written."; }

const char *pick_text_folder_not_kept(void) { return "The folder could not be kept."; }

#ifdef __EMSCRIPTEN__
const char *pick_text_downloads(void) { return "your downloads"; }
#endif
