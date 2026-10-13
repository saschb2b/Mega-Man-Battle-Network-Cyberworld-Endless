/* SAVES on the title (docs/SAVES.md, issue #114): the saves exported to
 * one file and imported from one, compared first; Undo; the automatic
 * copy's place. Three views: home, the comparison and the result. */
#ifndef CW_SAVES_H
#define CW_SAVES_H

#include <stdbool.h>

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
/* Its name in the title's line of keys (saves_text.c) */
const char *saves_word_hint(void);

#ifdef __EMSCRIPTEN__
/* The browser page's calls (web/play/app.js, Makefile's exports): SAVES
 * opened, 0 in a run; the file its chooser gave, "" for none */
int cw_saves_open(void);
void cw_saves_import(const char *path);
#endif

#endif
