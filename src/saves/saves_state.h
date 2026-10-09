/* State shared by the saves screen's course, picture, and words. */
#ifndef CW_SAVES_STATE_H
#define CW_SAVES_STATE_H

#include <stddef.h>
#include <stdint.h>

#include "backup.h"
#include "game.h"
#include "platform.h"

enum {
	SAVES_EXPORT, SAVES_IMPORT, SAVES_UNDO, SAVES_OPTIONS, SAVES_DETAILS,
	SAVES_AUTO, SAVES_FOLDER, SAVES_RETRY, SAVES_DONE, SAVES_CONTINUE,
	SAVES_KEEP, SAVES_TAKE, SAVES_HERE_DETAILS, SAVES_FILE_DETAILS,
	SAVES_PREV, SAVES_NEXT, SAVES_ACTIONS
};
typedef enum { SAVES_HOME, SAVES_TRANSFER, SAVES_COMPARE, SAVES_DETAIL, SAVES_RESULT } SavesView;

typedef struct {
	const Scene *back;
	BackupInfo here, file;
	bool have, busy, undo, undo_preview, result_undo, resume, result_resume, details_file, guard_input;
	SavesView view, details_back;
	int focus, operation, details_focus, details_page;
	uint8_t *bytes;
	size_t size;
	BackupStatus status;
	char note[1100], place[700], path[1100], feedback[200];
	uint32_t feedback_until;
} Saves;

extern Saves SV;
void saves_draw(void);
bool saves_pointer(void);
void saves_activate(int action);
void saves_answer(bool take);
void saves_refresh(void);
bool saves_back(void);
bool saves_comparing(void);
bool saves_disabled(int action);
void saves_feedback(const char *text);
void saves_reload(void);
void saves_clear_file(void);
void saves_result(const char *text, bool undo);
bool saves_picked(void);
void saves_move_focus(int dx, int dy);
int saves_details_pages(void);

/* Browser callbacks, also named in Makefile's exports. */
void cw_saves_open(void);
void cw_saves_import(const char *path);

#endif
