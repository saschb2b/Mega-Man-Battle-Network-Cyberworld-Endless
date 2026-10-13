/* The SAVES screen's words (saves_text.c): its labels, the panels' rows,
 * the copy's line and what the comparison and the result say, in the
 * plain style of the game's menus (docs/VOICE.md), apart from the logic
 * that says when. */
#ifndef CW_SAVES_TEXT_H
#define CW_SAVES_TEXT_H

#include <stdbool.h>
#include <stddef.h>

#include "backup.h"
#include "mirror.h"

/* The screen's title and its back button */
const char *saves_word_title(void);
const char *saves_word_back(void);
/* A button's label, as SV has it now (USE THE FILE'S or BRING BACK, the
 * copy on or off) */
const char *saves_label(int action);
/* A panel's title: this device's, or the file's */
const char *saves_panel_title(bool file);
/* A panel's rows (up to 4, of `width` characters at most): the run, its
 * area, the runs and best, and with `source` the system and the date;
 * a damaged file's only says so. The number of rows. */
int saves_rows(const BackupInfo *info, bool damaged, bool source, char rows[4][64]);
/* The line under home's panel: where the copy goes and how it went;
 * `blocked` where the saves could not be put back (backup_blocked) */
void saves_status_line(const MirrorStatus *m, bool blocked, bool can_choose, char *out, size_t n);
/* The comparison's words: what USE THE FILE'S replaces (`unreadable`:
 * this device's saves that cannot be read), and with
 * `place` (the copy's place's own file, the copy on) what KEEP THESE does
 * to it */
void saves_compare_note(const BackupInfo *here, bool have, bool unreadable, const char *place, char *out, size_t n);
/* Why a file is refused; this device's saves are kept */
void saves_refusal(BackupStatus status, const BackupInfo *file, char *out, size_t n);
/* The results: an import (`before`: the saves it replaced, `had` any),
 * an UNDO IMPORT, an export (to `where`), the same saves */
void saves_imported(const BackupInfo *now, const BackupInfo *before, bool had, char *out, size_t n);
void saves_undone(const BackupInfo *now, bool have, char *out, size_t n);
void saves_exported(const char *where, char *out, size_t n);
const char *saves_same(void);
/* An export that could not be written */
const char *saves_not_written(void);
/* A checkpoint that would not load after an import */
const char *saves_no_continue(void);

#endif
