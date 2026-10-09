/* The carrier, its manifest and its transactional installer share these. */
#ifndef CW_BACKUP_INTERNAL_H
#define CW_BACKUP_INTERNAL_H

#include "backup.h"

#define BACKUP_HEAD 20
#define BACKUP_FILES_MAX 64
#define BACKUP_FILE_MAX (4u << 20)
#define BACKUP_MANIFEST_MAX 1600

typedef void (*BackupEach)(const char *name, const uint8_t *data, uint32_t size, void *user);
uint32_t backup_fnv(const uint8_t *p, size_t n, uint32_t h);
uint32_t backup_get32(const uint8_t *p);
void backup_put32(uint8_t *p, uint32_t v);
bool backup_walk(const uint8_t *b, size_t n, BackupEach each, void *user);
bool backup_saved_name(const char *name);
bool backup_progress_name(const char *name);
void backup_normalize_profile(uint8_t *data, size_t n, const uint8_t *local, size_t local_n);
bool backup_manifest_read(const uint8_t *data, size_t n, BackupInfo *info);
bool backup_manifest_update(const char *data_dir, uint64_t stamp, BackupInfo *info, char *out, size_t n);
void backup_remove_dir(const char *dir);

#endif
