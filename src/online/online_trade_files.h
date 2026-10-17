/* Checked native storage operations for the trade journal, kept separate
 * from ordinary queued saves and the player's import/Undo directories. */
#ifndef CW_ONLINE_TRADE_FILES_H
#define CW_ONLINE_TRADE_FILES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TRADE_PATH_MAX 1100
#define TRADE_FILE_LIMIT (4u << 20)

bool trade_files_supported(void);
bool trade_files_path(char out[TRADE_PATH_MAX], const char *dir, const char *name);
/* One writer per data directory, across processes. Caller releases fd. */
int trade_files_lock(const char *root);
void trade_files_unlock(int fd);
bool trade_files_sync(const char *dir);
bool trade_files_write(const char *path, const uint8_t *data, size_t n);
uint8_t *trade_files_read(const char *path, size_t *n);
/* 0 missing, 1 regular file, 2 directory, -1 error or other kind. */
int trade_files_kind(const char *path);
bool trade_files_mkdir(const char *path);
bool trade_files_move(const char *from, const char *to);
bool trade_files_remove(const char *path);
bool trade_files_remove_dir(const char *path);
/* A sorted digest over every regular file's name, length and bytes.
 * content=true excludes run.trade and transfer.manifest, whose metadata
 * is not gameplay. Symlinks, nesting and temporary files are refused. */
bool trade_files_hash(const char *dir, bool content, uint8_t out[32]);
bool trade_files_copy(const char *from, const char *to);
/* Re-certify visible prepared bytes after interruption: every file and
 * their directory must be acknowledged durable before sending PREPARED. */
bool trade_files_certify(const char *dir);

#ifdef CW_TRADE_FILES_TEST
/* Called before/after each mutating or durability operation. Returning
 * false injects an I/O error; a test may instead exit to model process death. */
void trade_files_test_hook(bool (*hook)(const char *point));
#endif

#endif
