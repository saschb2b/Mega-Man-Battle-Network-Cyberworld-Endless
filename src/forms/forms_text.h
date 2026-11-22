/* The forms screen's words (forms_screen.h; docs/game/meta/forms-screen.md): its name,
 * its pages' and windows' labels, the count of CROSSSELECT's rows, and
 * what its INFORMATION box says of the cursor's row, in the PET's own
 * plain voice as KeyItem's (docs/voice/rules.md); and the Souls' names
 * and kinds, as BN5's and BN4's call them. */
#ifndef CW_FORMS_TEXT_H
#define CW_FORMS_TEXT_H

#include <stddef.h>

typedef enum {
	FT_TITLE,         /* the band: FORMS */
	FT_CROSSES,       /* the pages' tags */
	FT_SOULS,
	FT_FIVE,          /* the windows' labels: CROSSSELECT's five, */
	FT_SPARE,         /* ... the spares */
	FT_INFORMATION,   /* the box's label */
	FT_NONE,          /* the box where the run holds no form yet */
	FT_KEY,           /* the PET menu's hint for the key that opens it: R:FORMS */
	FT_WEAK,          /* the second screen's rows: a Cross's weakness, */
	FT_KIND,          /* ... a Soul's kind */
	FT_COUNT
} FormsWord;
const char *forms_word(FormsWord w);

/* "3/5": CROSSSELECT's rows filled */
void forms_count_words(char *out, size_t n, int five);
/* The box on a Cross: its name and weakness, then what it does more ("Fire chips +50, Buster +1") */
void forms_cross_words(char *out, size_t n, const char *name, const char *weak, const char *strong);
/* ... on a Cross taken up, the cursor on the other side: what A does there */
void forms_taken_words(char *out, size_t n, const char *taken, const char *there);
/* ... on a Soul: its name and the chip it takes */
void forms_soul_words(char *out, size_t n, const char *name, const char *kind);

/* The Souls by their game's numbers, each the Soul's name and its kind of
 * chip (issue #774's table; docs/game/meta/souls.md): BN5's 1-12, Team
 * ProtoMan's six then Team Colonel's (souls_held_bn5's bits), and BN4's
 * 1-12 (souls_held_bn4's) */
#define FORMS_GAME_SOULS 12
extern const char *const forms_bn5_soul_names[FORMS_GAME_SOULS];
extern const char *const forms_bn5_soul_kinds[FORMS_GAME_SOULS];
extern const char *const forms_bn4_soul_names[FORMS_GAME_SOULS];
extern const char *const forms_bn4_soul_kinds[FORMS_GAME_SOULS];

#endif
