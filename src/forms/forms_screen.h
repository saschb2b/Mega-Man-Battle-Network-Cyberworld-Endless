/* The forms screen (issue #774; docs/game/meta/forms-screen.md): MegaMan's forms
 * in this run, opened over the PET outside battle. Its Crosses in BN6's
 * folder editor's way: CROSSSELECT's five on the left, the others held on
 * the right as spares, each row CROSSSELECT's own; A takes one up (its row
 * as CROSSSELECT colours a row taken) and A on the other side swaps the
 * two, at no cost. Its Souls listed with their kind, the chip each takes,
 * nothing to equip. L and R turn between the two where the run holds
 * both, B or START leaves (B first puts back a Cross taken up); a tap
 * takes a row, a second tap on it is A. What the run holds comes from the
 * director (pet_forms.h), and the five chosen go back to it. */
#ifndef CW_FORMS_SCREEN_H
#define CW_FORMS_SCREEN_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define FORMS_CROSSES 10   /* CROSSSELECT's Crosses: Gregar's five, then Falzar's (crossselect.h) */
#define FORMS_ACTIVE 5     /* ... the rows it lists, the five marked */
#define FORMS_SOULS 24     /* BN5's twelve and BN4's twelve */

/* A Cross the run holds, as the director gives it */
typedef struct {
	int cross;                         /* its number in CROSSSELECT's order, 0-9 */
	const char *name, *weak, *strong;  /* "HeatCross"; the attacks that hit it twice as hard; what it does more, or NULL */
	const uint8_t *art, *grey;         /* its CROSSSELECT row, 9 x 2 tiles: in colour, and grey (NULL: none) */
	const uint8_t *pal, *chosen;       /* ... its palette, and the one of a row taken */
} FormsCross;

/* A Soul the run holds */
typedef struct {
	const char *name;   /* "ToadSoul" */
	const char *kind;   /* the kind of chip it takes to unite: "Aqua" */
} FormsSoul;

/* What the run holds of MegaMan's forms, made by the director as it opens */
typedef struct {
	FormsCross crosses[FORMS_CROSSES];   /* in CROSSSELECT's order */
	int n_crosses;
	uint16_t active;                     /* bit i: Cross i among CROSSSELECT's five (crossselect_active's) */
	FormsSoul souls[FORMS_SOULS];
	int n_souls;
	void (*choose)(uint16_t active);     /* the five chosen, back to the director as they change */
} FormsRun;

/* The five as a run has them, bit i Cross i: of `marked`, those `held`,
 * then the first held in CROSSSELECT's order until five are; every one
 * held where five or fewer are */
uint16_t forms_five(uint16_t held, uint16_t marked);

void forms_screen_open(const FormsRun *run);
bool forms_screen_shown(void);
/* The frame's course while it is up, or its key still held after it
 * closed: true while it holds the game */
bool forms_screen_update(void);
/* ... drawn over the game's picture (P.core_x, P.core_y) while it is up */
void forms_screen_draw(void);
/* Put away at once: the game's scene left */
void forms_screen_shut(void);
/* The state in words, while it is up (docs/devtools/scenarios.md): its
 * page, CROSSSELECT's five and the spares, the cursor's row, a Cross taken
 * up; reads only */
void forms_screen_describe(FILE *f);
/* What the cursor is on, for the second screen: a Cross's index into the
 * run's (`*soul` -1), or a Soul's (`*cross` -1), both -1 on nothing; false
 * while it is not up */
bool forms_screen_cursor(int *cross, int *soul);
/* The run as the screen holds it, its five as chosen so far (NULL while it is not up) */
const FormsRun *forms_screen_run(void);

#endif
