/* Labels, summaries, and outcome messages for SAVES. */
#ifndef CW_SAVES_TEXT_H
#define CW_SAVES_TEXT_H

#include <stddef.h>
#include <stdbool.h>
#include "backup.h"

const char *saves_label(int action);
const char *saves_hint(int action);
void saves_summary(const BackupInfo *info, int row, char *out, size_t n);
void saves_refusal(BackupStatus status, const BackupInfo *info, char *out, size_t n);
void saves_loss(const BackupInfo *here, const BackupInfo *file, char *out, size_t n);
void saves_transfer_status(char *out, size_t n);
void saves_action_hint(int action, char *out, size_t n);
void saves_details_text(char *out, size_t n);
const char *saves_panel_title(bool file);
const char *saves_view_title(void);
void saves_page_label(int page, int pages, char *out, size_t n);
void saves_success(bool undo, int previous_layer, char *out, size_t n);
void saves_undo_loss(char *out, size_t n);

#endif
