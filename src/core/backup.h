/* The saves in one file (backup.c): savedata/ and the player's settings as
 * a .cwsave, which a phone keeps beside the ROMs in the folder the player
 * chose (the launcher, src/launcher/), so that a reinstall finds them
 * there, and which the browser's page exports and imports
 * (web/play/app.js writes the same format). Nothing of the ROM is in it. */
#ifndef CW_BACKUP_H
#define CW_BACKUP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Its name in a ROM folder, and a download's */
#define BACKUP_NAME "cyberworld-endless.cwsave"
/* The largest one read (a run's state is under half a megabyte) */
#define BACKUP_MAX (16u << 20)

typedef struct {
	uint64_t stamp;      /* when it was made, seconds since 1970 (0: unknown) */
	int files;
	int runs, best;      /* the profile's runs and deepest layer (0, 0: none in it) */
	int run_depth;       /* the layer of a run saved in it, 0 for none */
	uint32_t hash;       /* its files' (names and bytes): two with the same hold the same saves */
} BackupInfo;

/* The saves in `data_dir` as one .cwsave's bytes (malloc'd, `n` long),
 * stamped `stamp`; NULL where there are none (no profile yet) */
uint8_t *backup_pack(const char *data_dir, uint64_t stamp, size_t *n);
/* What a .cwsave holds; false for bytes that are none, or damaged */
bool backup_info(const uint8_t *bytes, size_t n, BackupInfo *info);
/* Its saves into `data_dir`, the savedata/ there kept as savedata.old/
 * first (the one kept before gone); false, and nothing changed, for bytes
 * that are no .cwsave */
bool backup_unpack(const char *data_dir, const uint8_t *bytes, size_t n);
/* The saves in `data_dir` now, as backup_info says them; false for none */
bool backup_local_info(const char *data_dir, BackupInfo *info);
/* A file's bytes (malloc'd), at most BACKUP_MAX; NULL where it cannot be read */
uint8_t *backup_read_file(const char *path, size_t *n);
/* ... written whole or not at all (a .tmp beside it, then renamed) */
bool backup_write_file(const char *path, const uint8_t *bytes, size_t n);

#endif
