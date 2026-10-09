/* State shared by the saves screen's course, picture, and words. */
#ifndef CW_SAVES_STATE_H
#define CW_SAVES_STATE_H

#include <stddef.h>
#include <stdint.h>

#include "backup.h"
#include "game.h"
#include "platform.h"

enum { SAVES_EXPORT, SAVES_IMPORT, SAVES_AUTO, SAVES_UNDO, SAVES_FOLDER, SAVES_ACTIONS };
enum { SAVES_KEEP, SAVES_TAKE };

typedef struct {
	const Scene *back;
	BackupInfo here, file;
	bool have, comparing, busy, undo;
	int focus, operation;
	uint8_t *bytes;
	size_t size;
	BackupStatus status;
	char note[1100], place[700];
} Saves;

extern Saves SV;
void saves_draw(void);
bool saves_pointer(void);
void saves_activate(int action);
void saves_answer(bool take);
void saves_refresh(void);
bool saves_back(void);

/* Browser callbacks, also named in Makefile's exports. */
void cw_saves_open(void);
void cw_saves_import(const char *path);

#endif
