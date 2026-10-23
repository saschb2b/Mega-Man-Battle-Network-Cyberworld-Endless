/* The title's menu (docs/GUIDE.md, The title): NEW GAME, CONTINUE where
 * a run is saved, and OPTIONS, each a row of the title's own lettering
 * (BN6's two its own tiles, OPTIONS set from their letters and PRESS
 * START's); the cursor's arrow, CONTINUE's word on where the run goes
 * on, and the word on the keys under the picture. */
#ifndef CW_TITLE_MENU_H
#define CW_TITLE_MENU_H

#include <stdbool.h>

typedef struct {
	bool has_save;     /* CONTINUE's row */
	int saved_depth;   /* the saved run's layer, 0 unknown */
	bool saved_home;   /* ... saved at home, between acts */
	int cursor;        /* the row the arrow is on */
	int frame;         /* frames since the menu came: the arrow's turn */
	bool asking;       /* NEW GAME's question over it: no arrow */
} TitleMenu;

/* Its rows: NEW GAME, CONTINUE where a run is saved, OPTIONS the last. */
int title_menu_rows(bool has_save);
void title_menu_draw(int x0, int y0, const TitleMenu *m);
/* The word under the picture: ENTER before START where a keyboard is the
 * only controller (a PC player had no word of which key it is), then
 * with the menu the keys for SAVES and the ROMs' screen; none while
 * something is `over` the menu (NEW GAME's question, Options, whose panel
 * stops short of the picture's foot). */
void title_hint_draw(int x0, int y0, bool menu, bool pressed, bool over);

#endif
