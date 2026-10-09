/* The saves screen's visible controls, shared by drawing and navigation. */
#ifndef CW_SAVES_LAYOUT_H
#define CW_SAVES_LAYOUT_H

#include <SDL.h>
#include <stdbool.h>

#include "saves_state.h"

typedef struct {
	SDL_Rect rect;
	int action;
} SavesButton;

typedef struct {
	SDL_Rect hero, file, status, note, content, hint, back;
	SavesButton buttons[SAVES_ACTIONS];
	int count;
} SavesLayout;

SavesLayout saves_layout(int width, int height, SavesView view, bool resume, bool result_undo);
/* Move in the requested direction using the visible buttons' centers. */
int saves_layout_move(const SavesLayout *layout, int focus, int dx, int dy);

#endif
