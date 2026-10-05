/* The towns' people and what is said there (town_lines.c), for town.c. */
#ifndef CW_TOWN_LINES_H
#define CW_TOWN_LINES_H

#define MAX_FOLK 14   /* of the game's 16 NPCs */

/* Central Town's facings (the game's: 1 +x, 3 +y, 5 -x, 7 -y) */
enum { FACE_NE = 1, FACE_SE = 3, FACE_SW = 5, FACE_NW = 7 };

/* Someone in the town: where (source world units, moved with the piece
 * there), which way, what they say, and how far they pace that way and
 * back (0: they stand). A walker needs a sprite whose four walks
 * (animations 9-15) are drawn: of the game's people in list 5, 0x2B-0x32
 * and 0x34-0x36 (0x38's and 0x3A-0x3C's are empty, so they vanish as
 * they walk; 0x28's and 0x33's south ones a still frame or none). */
typedef struct {
	int x, y, face, cat, sprite;
	const char *words;
	int walk;
} Folk;

/* A town's people and words, by its style (town.c's styles, in their
 * order): what check 0xF0 + n says, and MegaMan on arriving (or NULL) */
typedef struct {
	const Folk *folk;
	int nfolk;
	const char *const *checks;
	const char *arrival;
} TownLines;
#define TOWN_LINES 4
extern const TownLines town_lines[TOWN_LINES];

/* What Lan and MegaMan say as a run begins, in Lan's room: on the first
 * dive ever, Dad's call about the Endless Net; after that, a word about
 * the last one; and the jack-in from Lan's PC, after the town's `arrival` */
const char *town_intro(const char *arrival);

#endif
