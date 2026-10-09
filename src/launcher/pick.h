/* The launcher's pickers (one file a platform: pick_desktop.c, pick_android.c,
 * pick_ios.c, pick_none.c): the system's own chooser for a ROM file or the
 * folder the ROMs are in, open beside the game's window while it goes on
 * drawing, and on a phone the folder kept: looked in again, and the
 * saves' copy written there. */
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
	bool path;         /* `text` is the file chosen (a desktop's), else what a phone's look found */
	bool saves;        /* (a phone's folder) a .cwsave was in it: copied to the data folder's found.cwsave */
	char text[1024];
} PickResult;

/* What this platform's pickers choose (none: the ROM is put where the
 * game looks, by hand) */
unsigned pick_kinds(void);
enum { SAVES_CAN_AUTO = 1, SAVES_CAN_FOLDER = 2 };
unsigned pick_saves_capabilities(void);
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
/* (phones) The saves file at `from` written into the folder kept as
 * BACKUP_NAME (backup.h); false where none is kept or it cannot be */
bool pick_saves_put(const char *from);

/* The SAVES screen's own pickers. An import stages a file in the data
 * folder; an export copies `from` to a place the player can reach. These
 * return at once, and pick_saves_done reports their end. */
bool pick_saves_import(void);
bool pick_saves_export(const char *from);
bool pick_saves_choose_folder(void);
bool pick_saves_busy(void);
/* status: 1 complete, -1 cancelled, -2 failed. path is true for an
 * import; text is its local path, the export's destination, or an error. */
bool pick_saves_done(PickResult *r);
/* The kept transfer folder, else this system's known transfer place. */
bool pick_saves_place(char *name, size_t n);
/* Stable destination identity for device-local copy receipts; never exported.
 * Provider folders with the same display name must have different identities. */
bool pick_saves_scope(char *out, size_t n);
/* Read the kept place's existing file into `to`, without changing it. */
bool pick_saves_get(const char *to);
/* Stage a different file from the known places (kept folder, Downloads,
 * data folder). The SAVES screen compares it before any automatic write. */
bool pick_saves_find(const char *to);
/* Automatic discovery skips exact files already dismissed or successfully
 * exported by this device, so they cannot hide another known place. Zero
 * means no fingerprint to skip. Manual Import still uses pick_saves_find. */
bool pick_saves_discover(const char *to, uint32_t ignored_hash, uint32_t exported_hash);
/* Device-local: never travels in a .cwsave. Phones initially enable this
 * for their kept ROM folder; other systems initially leave it off. */
bool pick_saves_auto_enabled(void);
void pick_saves_auto(bool enabled);

#endif
