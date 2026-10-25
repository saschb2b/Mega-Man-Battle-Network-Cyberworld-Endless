/* The saves in one file (backup.c, docs/engine/saves.md): savedata/'s progress and
 * the current run as a .cwsave, which SAVES exports and imports on every
 * system (src/saves/) and the automatic copy keeps in a folder
 * (src/launcher/mirror.c), so that a phone's reinstall finds it there.
 * Nothing of the ROM is in it. */
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
/* Every progress file of a save slot in savedata/, by its name: the
 * normal slot's, and the week's net's each after "weekly-" (weekly.h) */
#define SAVE_PROGRESS_NAMES 15
extern const char *const save_progress_names[SAVE_PROGRESS_NAMES];

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
 * there are none (no profile yet). Unchanged progress keeps its manifest;
 * changed progress writes a new one (savedata/transfer.manifest). */
uint8_t *backup_pack(const char *data_dir, uint64_t stamp, size_t *n);
/* What a .cwsave holds; false for bytes that are none, or damaged */
bool backup_info(const uint8_t *bytes, size_t n, BackupInfo *info);
/* Preflight after backup_info: a refused file never reaches the local saves. */
BackupStatus backup_check(const BackupInfo *info, bool bn5);
/* Its saves into `data_dir`: every file staged and flushed first, the
 * savedata/ there kept as savedata.old/ (Undo's), the one kept before that
 * as saves-replaced-1.cwsave (the last three kept); anything but BACKUP_OK
 * and the saves are as they were */
BackupStatus backup_restore(const char *data_dir, const uint8_t *bytes, size_t n, bool bn5);
bool backup_unpack(const char *data_dir, const uint8_t *bytes, size_t n);
/* Undo the last successful import; swaps the two complete savedata folders. */
bool backup_can_undo(const char *data_dir);
/* Read the prior folder without changing it or its manifest. Empty prior
 * progress is a valid zero summary; false reports damaged/unreadable data. */
bool backup_undo_info(const char *data_dir, BackupInfo *info);
bool backup_undo(const char *data_dir);
/* An import cut short put right, before anything reads the saves: a
 * savedata/ missing or not the one installed is the one before again.
 * False where it cannot be; then backup_blocked() until it is. */
bool backup_recover(const char *data_dir);
/* savedata/ is missing and the saves are in a folder beside it that could
 * not be put back: nothing is to write a fresh savedata/ over them */
bool backup_blocked(void);
/* The saves in `data_dir` now, as backup_info says them; false for none
 * (status BACKUP_OK) or none readable (another status). Read only:
 * nothing is written. */
bool backup_local_info(const char *data_dir, BackupInfo *info);
/* A file's bytes (malloc'd), at most BACKUP_MAX; NULL where it cannot be read */
uint8_t *backup_read_file(const char *path, size_t *n);
/* ... written whole or not at all (a .tmp beside it, then renamed) */
bool backup_write_file(const char *path, const uint8_t *bytes, size_t n);

#endif
