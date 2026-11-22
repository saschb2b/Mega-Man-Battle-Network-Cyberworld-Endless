/* The forms screen's picture (forms.h), in BN6's PET screens' look
 * (pet_look.h; docs/engine/ui.md): the band with the screen's name, its
 * pages as tags where the run holds Crosses and Souls, and on the Crosses'
 * page the count of CROSSSELECT's rows filled in a pill, as FOLDER EDIT's
 * 30/30; two windows side by side as the folder editor's folder and pack,
 * CROSSSELECT's five and the spares, each row CROSSSELECT's own art from
 * the ROM (in colour among the five, grey among the spares, its "chosen"
 * palette once taken up), the orange arrow on the cursor's; the Souls'
 * page one window of their names and kinds; KeyItem's INFORMATION box
 * under them for the cursor's row. And where each part a tap takes lies. */
#ifndef CW_FORMS_DRAW_H
#define CW_FORMS_DRAW_H

#include <SDL.h>
#include <stdbool.h>

#include "forms.h"

enum { FORMS_PAGE_CROSSES, FORMS_PAGE_SOULS, FORMS_PAGES };
enum { FORMS_FIVE, FORMS_SPARE, FORMS_SIDES };   /* the Crosses' two windows */
#define FORMS_SOUL_ROWS 5                        /* Souls shown at once */

/* what the picture shows, made as the screen opens and as a swap changes the five */
typedef struct {
	int t;
	FormsRun run;
	int page;                                   /* FORMS_PAGE_* */
	bool pages;                                 /* the run holds both: the tags on the band, L and R between them */
	int col[FORMS_SIDES][FORMS_CROSSES];        /* each window's rows: indices into run.crosses, in CROSSSELECT's order */
	int n[FORMS_SIDES];
	int side, row;                              /* the cursor on the Crosses' page */
	int taken_side, taken_row;                  /* a Cross taken up, its side -1 for none */
	int soul_at, soul_top;                      /* the cursor on the Souls' page, the first row shown */
} FormsView;

void forms_draw_view(int x, int y, const FormsView *v);

/* where a tap takes each part, from the picture's top left */
SDL_Rect forms_cross_rect(int side, int row);   /* a Cross's row in its window */
SDL_Rect forms_soul_rect(int i);                /* the Souls' row i shown */
SDL_Rect forms_tag_rect(int page);              /* a page's tag on the band, where the run holds both */

#endif
