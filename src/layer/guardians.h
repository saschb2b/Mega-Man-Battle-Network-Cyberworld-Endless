/* The guardian Navis as the boss sequence presents them: name, epithet,
 * mugshot, colour, and what they say, chosen by what they remember of
 * MegaMan (docs/BOSSES.md). */
#ifndef CW_GUARDIANS_H
#define CW_GUARDIANS_H

#include <stdbool.h>

#define GUARDIAN_NO_MUGSHOT -1
#define GUARDIAN_MEGAMAN_MUGSHOT 0x37

typedef struct {
	const char *name;       /* "ElecMan" */
	const char *epithet;    /* "Master of Current" */
	int mugshot;            /* the game's mugshot, GUARDIAN_NO_MUGSHOT for Falzar's Navis */
	int pose;               /* overworld animation shown on the title card, -1 for none */
	unsigned char r, g, b;  /* the title card's accent */
} Guardian;

/* navi index as in the battle's enemy table (1 HeatMan .. 16 ElementMan, 18 Colonel) */
const Guardian *guardian(int navi);

/* A line is one or more chat boxes split by '|', each up to three lines
 * split by '\n'. `who` tells which mugshot speaks each box: 'N' the
 * guardian, 'M' MegaMan. */
typedef struct {
	const char *boxes;
	const char *who;
} GuardianLine;

/* Before the battle: the story beat that fits first (first meeting, a
 * rematch, revenge for a loss, a stronger version), then variety by how
 * often they met. `version` 0-2 as make_boss sets it. */
GuardianLine guardian_intro(int navi, int version);
/* After MegaMan wins. */
GuardianLine guardian_defeat(int navi);

/* The area a guardian keeps, for the title card ("Central Area"). */
const char *guardian_area_name(int biome);
/* A line under an area's name on its title card. */
const char *guardian_area_motto(int biome);

#endif
