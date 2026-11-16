/* SAVES on the title (docs/engine/saves.md, issue #114): the saves exported to
 * one file and imported from one, compared first; Undo; the automatic
 * copy's place. Three views: home, the comparison and the result. */
#ifndef CW_SAVES_H
#define CW_SAVES_H

#include <stdbool.h>
#include <stdio.h>

/* Opened over the title or the ROMs screen; false elsewhere (in a run:
 * its progress is the title's to keep first) */
bool saves_open(void);
/* A file chosen, dropped or handed over by the browser's page: compared
 * with this device's before anything changes */
void saves_import(const char *path);
/* Each frame of the title, the ROMs screen and SAVES: a dropped .cwsave
 * (the ROMs screen takes its own), and over the first two the copy's
 * place holding saves not answered yet, compared */
void saves_tick(void);
/* The title's key for it: L let go with no SELECT while it was held (L
 * with SELECT is the sprite gallery's), SAVES opened; true where it took
 * the press */
bool saves_key(void);
/* SAVES in a headless run's state where it is up, a line each: its view
 * (`savesview` home, compare, result, send, receive), home's page (`savespage`
 * cloud, code, file), the line or button under the cursor (`savesfocus`
 * export, import, auto, folder, keep, take, done, continue, undo, ...),
 * this device's saves (`here` their hash, none; `herelayer` their run's
 * layer, `hereruns`), the file's beside them (`theirs`, `theirlayer`,
 * `theirstatus` ok or refused), whether an import can be undone (`undo`
 * yes or no), the copy's switch (`autocopy` on or off), home's question
 * (`savesasks` yes or no, the answer under its cursor), a picker open
 * (`savesbusy`), and the comparison's or the result's words (`savesnote`).
 * Reads only. */
void saves_describe(FILE *f);
/* Its name in the title's line of keys (saves_text.c) */
const char *saves_word_hint(void);

#ifdef __EMSCRIPTEN__
/* The browser page's calls (web/play/app.js, Makefile's exports): SAVES
 * opened, 0 in a run; the file its chooser gave, "" for none */
int cw_saves_open(void);
void cw_saves_import(const char *path);
#endif

#endif
