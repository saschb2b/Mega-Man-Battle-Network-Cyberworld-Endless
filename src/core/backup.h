/* The saves in one file (backup.c): savedata/ progress and the current run as
 * a .cwsave, which a phone keeps beside the ROMs in the folder the player
 * chose (the launcher, src/launcher/), so that a reinstall finds them
 * there, and which the browser's page exports and imports
 * (the browser's page downloads the core's bytes). Nothing of the ROM is in it. */
#ifndef CW_BACKUP_H
#define CW_BACKUP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Its name in a ROM folder, and a download's */
#define BACKUP_NAME "cyberworld-endless.cwsave"
/* The largest one read (a run's state is under half a megabyte) */
#define BACKUP_MAX (16u << 20)
#define BACKUP_MANIFEST "savedata/transfer.manifest"
#define BACKUP_FORMAT 2

typedef enum {
	BACKUP_OK, BACKUP_DAMAGED, BACKUP_NEWER_FORMAT, BACKUP_NEWER_BUILD,
	BACKUP_INCOMPATIBLE_RUN, BACKUP_NEEDS_BN5, BACKUP_IO
} BackupStatus;

typedef struct {
	uint64_t stamp;      /* last progress write, seconds since 1970 (0: unknown) */
	int files;
	int runs, best;      /* the profile's runs and deepest layer (0, 0: none in it) */
	int run_depth;       /* the layer of a run saved in it, 0 for none */
	uint32_t hash;       /* its files' (names and bytes): two with the same hold the same saves */
	int format, run_area, run_act;
	bool needs_bn5, has_profile, has_run, has_state;
	uint32_t run_magic, layer_make, revision, parent_revision;
	char version[64], system[24], device[64], save_id[33], parent_id[33];
	BackupStatus status;
} BackupInfo;

/* The saves in `data_dir` as one .cwsave's bytes (malloc'd, `n` long),
 * stamped `stamp` (0: the latest progress file's write time); NULL where
 * there are none (no profile yet). Unchanged progress keeps its manifest. */
uint8_t *backup_pack(const char *data_dir, uint64_t stamp, size_t *n);
/* Mobile adapters supply the system's device name/model before exporting. */
void backup_set_device(const char *name);
/* What a .cwsave holds; false for bytes that are none, or damaged */
bool backup_info(const uint8_t *bytes, size_t n, BackupInfo *info);
/* Preflight after backup_info: a refused file never reaches the local saves. */
BackupStatus backup_check(const BackupInfo *info, bool bn5);
BackupStatus backup_restore(const char *data_dir, const uint8_t *bytes, size_t n, bool bn5);
/* Its saves into `data_dir`, the savedata/ there kept as savedata.old/
 * first (the one kept before gone); false, and nothing changed, for bytes
 * that are no .cwsave */
bool backup_unpack(const char *data_dir, const uint8_t *bytes, size_t n);
/* Undo the last successful import; swaps the two complete savedata folders. */
bool backup_can_undo(const char *data_dir);
/* Read the prior folder without changing it or its manifest. Empty prior
 * progress is a valid zero summary; false reports damaged/unreadable data. */
bool backup_undo_info(const char *data_dir, BackupInfo *info);
bool backup_undo(const char *data_dir);
/* Recover an import interrupted between folder renames before reading saves. */
bool backup_recover(const char *data_dir);
/* The saves in `data_dir` now, as backup_info says them; false for none */
bool backup_local_info(const char *data_dir, BackupInfo *info);
/* A file's bytes (malloc'd), at most BACKUP_MAX; NULL where it cannot be read */
uint8_t *backup_read_file(const char *path, size_t *n);
/* ... written whole or not at all (a .tmp beside it, then renamed) */
bool backup_write_file(const char *path, const uint8_t *bytes, size_t n);

#endif
