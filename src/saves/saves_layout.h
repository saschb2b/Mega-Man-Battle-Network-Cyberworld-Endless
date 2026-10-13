/* Where the SAVES screen's panels and buttons are (saves_layout.c), shared
 * by its picture and its pointer and cursor: only the buttons that can be
 * used are laid out. */
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
	SDL_Rect here, file;   /* the panels (file: the comparison's) */
	SDL_Rect line;         /* home's line under its panel; the note */
	SDL_Rect back;
	SavesButton buttons[SAVES_ACTIONS];
	int count;
} SavesLayout;

/* The view's layout on a `width` x `height` canvas, its buttons
 * `actions` (`n` of them, in reading order) */
SavesLayout saves_layout(int width, int height, SavesView view, const int *actions, int n);
/* The button the cursor goes to from `focus`, in the direction (dx, dy) */
int saves_layout_move(const SavesLayout *layout, int focus, int dx, int dy);

#endif
