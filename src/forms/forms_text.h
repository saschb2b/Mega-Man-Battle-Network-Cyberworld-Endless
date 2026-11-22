/* The forms screen's words (forms.h; docs/game/meta/forms.md): its name,
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

/* The Souls by guardian: BN5's Team Colonel's six (Colonel, ShadowMan,
 * NumberMan, TomahawkMan, KnightMan, ToadMan) and Team ProtoMan's
 * (ProtoMan, GyroMan, SearchMan, NapalmMan, MagnetMan, Meddy), each the
 * Soul's name and its kind of chip (docs/game/meta/souls.md) */
#define FORMS_BN5_SOULS 12
extern const char *const forms_bn5_soul_names[FORMS_BN5_SOULS];
extern const char *const forms_bn5_soul_kinds[FORMS_BN5_SOULS];

#endif
