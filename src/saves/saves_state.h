/* What the SAVES screen's parts share (saves.c its course, saves_actions.c
 * what its buttons do, saves_draw.c its picture, saves_layout.c where its
 * buttons are, saves_text.c its words): the screen's state, SV. */
#ifndef CW_SAVES_STATE_H
#define CW_SAVES_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "backup.h"
#include "game.h"

/* Its buttons, those shown in each view */
enum {
	SAVES_EXPORT, SAVES_IMPORT, SAVES_UNDO, SAVES_AUTO, SAVES_FOLDER,
	SAVES_KEEP, SAVES_TAKE, SAVES_DONE, SAVES_CONTINUE, SAVES_ACTIONS
};
typedef enum { SAVES_HOME, SAVES_COMPARE, SAVES_RESULT } SavesView;

typedef struct {
	const Scene *back;     /* where it goes back to: the title or the ROMs screen */
	SavesView view;
	int focus;             /* the button the cursor is on */
	BackupInfo here, file; /* this device's saves and the file's */
	bool have;             /* this device has saves */
	bool undo;             /* ... and the ones an import replaced */
	bool busy;             /* a picker open */
	bool from_place;       /* the file compared is the copy's place's own (mirror.h) */
	bool offer_undo;       /* the result offers UNDO IMPORT, or UNDO after one */
	bool undone;           /* ... the result of an UNDO IMPORT */
	bool resume;           /* the result offers CONTINUE */
	bool guard_input;      /* the frame it opened: no press taken yet */
	uint8_t *bytes;        /* the file's */
	size_t size;
	BackupStatus status;   /* the file's, checked */
	char note[700];        /* the comparison's or the result's words */
} Saves;

extern Saves SV;

/* This device's saves read again (mirror_local, read only) */
void saves_refresh(void);
/* The file's bytes let go */
void saves_clear_file(void);
/* The comparison of `bytes` (taken: SV's to free) with this device's */
void saves_compare(uint8_t *bytes, size_t n, bool from_place);
/* ... of the copy's place's own file (mirror_held): false for none */
bool saves_compare_held(void);
/* The result view, saying `text` */
void saves_result(const char *text);
/* Leaving the comparison without USE THE FILE'S: this device's kept */
void saves_leave_compare(bool keep_these);

#endif
