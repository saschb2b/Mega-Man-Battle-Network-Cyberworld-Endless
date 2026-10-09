/* Labels, summaries, and outcome messages for SAVES. */
#ifndef CW_SAVES_TEXT_H
#define CW_SAVES_TEXT_H

#include <stddef.h>
#include "backup.h"

const char *saves_label(int action);
const char *saves_hint(int action);
void saves_summary(const BackupInfo *info, int row, char *out, size_t n);
void saves_refusal(BackupStatus status, const BackupInfo *info, char *out, size_t n);
void saves_loss(const BackupInfo *here, const BackupInfo *file, char *out, size_t n);

#endif
