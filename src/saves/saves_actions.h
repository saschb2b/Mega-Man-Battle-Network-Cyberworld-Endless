/* What the SAVES screen's buttons do (saves_actions.c): export, import,
 * Undo, the copy's row and folder, the comparison's answers, the result's
 * CONTINUE; and the pickers' ends. */
#ifndef CW_SAVES_ACTIONS_H
#define CW_SAVES_ACTIONS_H

#include <stdbool.h>

/* The buttons the view shows now, in reading order: their count */
int saves_actions(int *out);
/* A button chosen */
void saves_activate(int action);
/* A picker's end taken up: true where one ended this frame */
bool saves_picked(void);
/* The saves read again after they changed under the game (an import, an
 * Undo, the cloud's): the profile and the rival's record over the title,
 * the browser's copy kept, SAVES' panel */
void saves_reload(void);

#endif
