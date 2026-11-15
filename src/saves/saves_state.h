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
#include "transfer_link.h"

/* Its buttons, those shown in each view (SEND and RECEIVE's view's
 * digits: SAVES_DIGIT + 0..9) */
enum {
	SAVES_EXPORT, SAVES_IMPORT, SAVES_SEND, SAVES_RECEIVE, SAVES_UNDO, SAVES_AUTO, SAVES_FOLDER,
	SAVES_KEEP, SAVES_TAKE, SAVES_DONE, SAVES_CONTINUE, SAVES_ERASE, SAVES_GET,
	SAVES_TURN_ON, SAVES_LINK, SAVES_ADD, SAVES_UNLINK, SAVES_DELETE,
	SAVES_DIGIT, SAVES_ACTIONS = SAVES_DIGIT + 10
};
/* (SEND: the code the server holds the saves under; RECEIVE: a code typed,
 * saves_code.c) */
typedef enum { SAVES_HOME, SAVES_COMPARE, SAVES_RESULT, SAVES_SEND_CODE, SAVES_RECEIVE_CODE } SavesView;
/* Home's pages, as Options' tabs (docs/engine/saves-screens.md): the cloud
 * (saves_cloud.c), a code (saves_code.c), a file and its copy */
typedef enum { SAVES_PAGE_CLOUD, SAVES_PAGE_CODE, SAVES_PAGE_FILE, SAVES_PAGES } SavesPage;

typedef struct {
	const Scene *back;     /* where it goes back to: the title or the ROMs screen */
	SavesView view;
	SavesPage page;        /* home's page shown (kept while the game runs) */
	bool paged;            /* ... chosen yet: the first opening's is the first there is */
	int page_focus[SAVES_PAGES];   /* each page's line last chosen (saves_home.c), its cursor's when shown again */
	int focus;             /* the button (home: the line) the cursor is on */
	BackupInfo here, file; /* this device's saves and the file's */
	bool have;             /* this device has saves */
	bool unreadable;       /* ... saves it cannot read (here.status says why) */
	bool undo;             /* ... and the ones an import replaced */
	bool busy;             /* a picker open */
	bool from_place;       /* the file compared is the copy's place's own (mirror.h) */
	bool offer_undo;       /* the result offers UNDO IMPORT, or UNDO after one */
	bool undone;           /* ... the result of an UNDO IMPORT */
	bool resume;           /* the result offers CONTINUE */
	int guard_input;       /* frames left with no press taken (a comparison that opened by itself waits half a second) */
	uint8_t *bytes;        /* the file's */
	size_t size;
	BackupStatus status;   /* the file's, checked */
	char note[700];        /* the comparison's or the result's words */
	char said[96];         /* home's line in the copy's place: a picker's wait, or its cancel (a playtester's EXPORT said nothing, session 74) */
	char code[11];         /* RECEIVE's digits typed so far */
	uint32_t sent_hash;    /* the saves SEND last sent (BackupInfo.hash): shown again while their code holds */
	bool typing;           /* RECEIVE's native keyboard on (text_input.h) */
	TransferState transfer;   /* SEND's or RECEIVE's link as this frame's update found it, for the picture */
	bool from_cloud;       /* the file compared is the cloud slot's (saves_cloud.c) */
	bool from_code;        /* ... a code's (saves_code.c) */
	bool code_link;        /* SEND's view shows a cloud link's code (Add a device), not saves' */
	bool linking;          /* RECEIVE's view opened from the cloud's Link with a code */
} Saves;

extern Saves SV;

/* This device's saves read again (mirror_local, read only) */
void saves_refresh(void);
/* Home's pages this system has, in order: their count */
int saves_pages(SavesPage *out);
/* Home shown on `page` (where the system has it), the cursor on its first
 * line */
void saves_home_page(SavesPage page);
/* The file's bytes let go */
void saves_clear_file(void);
/* The comparison of `bytes` (taken: SV's to free) with this device's */
void saves_compare(uint8_t *bytes, size_t n, bool from_place);
/* ... of the copy's place's own file (mirror_held): false for none */
bool saves_compare_held(void);
/* ... of the cloud slot's saves (taken), where both it and this system
 * moved on since they were last in step */
void saves_compare_cloud(uint8_t *bytes, size_t n);
/* The result view, saying `text` */
void saves_result(const char *text);
/* Leaving the comparison without USE THE FILE'S: this device's kept */
void saves_leave_compare(bool keep_these);

#endif
