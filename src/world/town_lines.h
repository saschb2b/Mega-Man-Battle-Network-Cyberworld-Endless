/* The towns' people and what is said there (town_lines.c), for town.c. */
#ifndef CW_TOWN_LINES_H
#define CW_TOWN_LINES_H

#define MAX_FOLK 14   /* of the game's 16 NPCs */

/* Central Town's facings (the game's: 1 +x, 3 +y, 5 -x, 7 -y) */
enum { FACE_NE = 1, FACE_SE = 3, FACE_SW = 5, FACE_NW = 7 };

/* Someone in the town: where (source world units, moved with the piece
 * there), which way, what they say, and how far they pace that way and
 * back (0: they stand). */
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

/* What Lan and MegaMan say as a run begins: on the first dive ever, Dad's
 * call about the Endless Net; after that, a word about the last one; the
 * port by `landmark_at`, after the town's `arrival` */
const char *town_intro(const char *landmark_at, const char *arrival);

#endif
