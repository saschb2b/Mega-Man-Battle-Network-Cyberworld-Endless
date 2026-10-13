/* The pickers (one file a platform: pick_desktop.c, pick_android.c,
 * pick_ios.c, pick_none.c; what they share in pick_saves.c): the system's
 * own chooser for a ROM file or the folder the ROMs are in, open beside
 * the game's window while it goes on drawing; on a phone the folder kept,
 * looked in again; and SAVES' choosers and transfer place. */
#ifndef CW_PICK_H
#define CW_PICK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* What a picker chooses: a file (a desktop's, one ROM), the folder the
 * ROMs are in, or files, several at once (a phone's) */
enum { PICK_FILE = 1, PICK_FOLDER = 2, PICK_FILES = 4 };

typedef struct {
	int status;        /* 1 chosen, -1 cancelled or failed */
	bool path;         /* `text` is the file chosen (a desktop's, an import's), else what a phone's look found */
	char text[1024];
} PickResult;

/* What this platform's pickers choose (none: the ROM is put where the
 * game looks, by hand) */
unsigned pick_kinds(void);
/* Opens one of `kind`, for cartridge `slot` (a desktop's dialog names
 * that ROM); false where none opened */
bool pick_open(int kind, int slot);
/* Whether one is open */
bool pick_busy(void);
/* A picker closed since the last call: true, and what it gave in `r` */
bool pick_done(PickResult *r);
/* (phones) The folder kept looked in again for the ROMs the ROM folder
 * lacks, those found copied there: their bits (1 BN6, 2 BN5), 0 for
 * none new, -1 where no folder is kept or it cannot be opened (`msg` says
 * why then) */
int pick_look(char *msg, size_t n);
/* (phones) The folder kept: its name, false for none */
bool pick_folder(char *name, size_t n);
/* The SAVES screen's own pickers (docs/SAVES.md). An import gives a file
 * to read: the one chosen (a desktop's), or a copy in the data folder
 * named saves-import.cwsave (a phone's, the browser's page); an export
 * copies `from` where the player chooses. These return at once, and
 * pick_saves_done reports their end. */
enum { SAVES_CAN_AUTO = 1, SAVES_CAN_FOLDER = 2 };
unsigned pick_saves_capabilities(void);
bool pick_saves_import(void);
bool pick_saves_export(const char *from);
bool pick_saves_choose_folder(void);
bool pick_saves_busy(void);
/* The browser's wait for its page's file let go (B, or its time out) */
void pick_saves_cancel(void);
#ifdef __EMSCRIPTEN__
/* The page's file (web/play/app.js, through cw_saves_import), "" for none */
void pick_saves_page(const char *path);
#endif
/* status: 1 done, -1 cancelled, -2 failed. path is true for an import;
 * text is its file, the export's destination, or why it failed. */
bool pick_saves_done(PickResult *r);
/* The transfer place: the folder kept, else this system's own (a phone's
 * ROM folder, a desktop's Downloads, the data folder); false for none */
bool pick_saves_place(char *name, size_t n);
/* Its identity for the copy's receipts, never exported: a full path, an
 * Android tree's URI, an iOS folder's; false where there is none */
bool pick_saves_scope(char *out, size_t n);
/* The place's BACKUP_NAME read into memory (malloc'd): 1 read, 0 none
 * there (or no place, or one that cannot be listed: a copy there would
 * fail too), -1 there but unreadable (an iCloud file not yet down too) */
int pick_saves_read(uint8_t **bytes, size_t *n);
/* ... its size and change time, without reading it: false where the
 * system cannot say (a phone's provider) or it is not there */
bool pick_saves_stat(uint64_t *size, int64_t *mtime);
/* `n` bytes written into the place as BACKUP_NAME (backup.h), whole */
bool pick_saves_put(const uint8_t *bytes, size_t n);
/* The automatic copy on or off: device-local, never in a .cwsave. Phones
 * start with it on, the other systems off. */
bool pick_saves_auto_enabled(void);
void pick_saves_auto(bool enabled);

#endif
