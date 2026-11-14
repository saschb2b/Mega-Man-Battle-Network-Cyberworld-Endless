/* The SAVES screen's words (saves_text.c): its labels, the panels' rows,
 * the copy's line and what the comparison and the result say, in the
 * plain style of the game's menus (docs/voice/rules.md), apart from the logic
 * that says when. */
#ifndef CW_SAVES_TEXT_H
#define CW_SAVES_TEXT_H

#include <stdbool.h>
#include <stddef.h>

#include <stdint.h>

#include "backup.h"
#include "cloud_link.h"
#include "mirror.h"
#include "transfer_link.h"

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
/* home's line while the picker for `action` (SAVES_EXPORT, SAVES_IMPORT,
 * SAVES_FOLDER) is open, and after it was cancelled */
const char *saves_waiting(int action);
const char *saves_cancelled(int action);
/* A checkpoint that would not load after an import */
const char *saves_no_continue(void);

/* Saves sent by a code (saves_code.h): a code as shown, two groups of
 * five; RECEIVE's digits typed so far, a dash for each still to come;
 * how long a sent code holds (`ms` left); RECEIVE's ask; and SEND's or
 * RECEIVE's line for the link's state (`expired`: a sent code's time up) */
void saves_code_shown(const char *code, char *out, size_t n);
void saves_code_typed(const char *code, char *out, size_t n);
void saves_code_left(uint32_t ms, char *out, size_t n);
const char *saves_code_prompt(void);
void saves_code_words(const TransferState *s, bool receiving, bool expired, char *out, size_t n);

/* Saves kept in step (saves_cloud.h): the comparison's words where the file
 * is the cloud's; why a cloud's saves were not taken in; where taken
 * saves came from ("Cloud: saves from Android, 11-14 08:12"); a trouble's
 * line; the syncing and the CLOUD view's answers; and the cloud's line for
 * the link's state (what CLOUD does, where this system has none) */
void saves_cloud_compare_note(const BackupInfo *here, bool have, bool unreadable, char *out, size_t n);
void saves_cloud_refused(BackupStatus status, const BackupInfo *cloud, char *out, size_t n);
void saves_cloud_from(const BackupInfo *cloud, char *out, size_t n);
const char *saves_cloud_trouble(CloudTrouble trouble);
const char *saves_cloud_syncing(void);
const char *saves_cloud_deleted(void);
const char *saves_cloud_unlinked(void);
void saves_cloud_status(const CloudState *s, char *out, size_t n);

#endif
