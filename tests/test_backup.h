/* ROM-free portable save transfer checks, returning their failure count. */
#ifndef CW_TEST_BACKUP_H
#define CW_TEST_BACKUP_H

#include <stddef.h>
#include <stdint.h>

int test_backup(void);
/* Host test link's fault-injection adapter, never included in the game. */
uint8_t *__wrap_backup_read_file(const char *path, size_t *n);
uint8_t *__real_backup_read_file(const char *path, size_t *n);

#endif
