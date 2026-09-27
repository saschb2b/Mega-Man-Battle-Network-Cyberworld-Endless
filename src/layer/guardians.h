/* The guardian Navis as the boss sequence presents them: name, epithet,
 * mugshot, colour, and what they say, chosen by what they remember of
 * MegaMan (docs/BOSSES.md). */
#ifndef CW_GUARDIANS_H
#define CW_GUARDIANS_H

#include <stdbool.h>

#define GUARDIAN_NO_MUGSHOT -1

typedef struct {
	const char *name;       /* "ElecMan" */
	const char *epithet;    /* "Master of Current" */
	int mugshot;            /* the game's mugshot, GUARDIAN_NO_MUGSHOT for Falzar's Navis */
	int pose;               /* overworld animation shown on the title card, -1 for none */
	unsigned char r, g, b;  /* the title card's accent */
} Guardian;

/* navi index as in the battle's enemy table (1 HeatMan .. 16 ElementMan, 18 Colonel) */
const Guardian *guardian(int navi);

/* Before the battle: the story beat that fits first (first meeting, a
 * rematch, revenge for a loss, a stronger version), then variety by how
 * often they met; MegaMan and Lan's part in it, and the battle's call.
 * `version` 0-2 as make_boss sets it. Chat boxes split by '|', speaker
 * marks as ta_talk reads them; unmarked boxes are the guardian's. */
const char *guardian_intro(int navi, int version, int biome);
/* After MegaMan wins. */
const char *guardian_defeat(int navi);
/* What MegaMan knows of a guardian's way of fighting, said on its layer
 * before the arena (a playtester met EraseMan's ghosts and his erasing
 * blow unwarned); NULL where there is nothing to add. */
const char *guardian_tip(int navi);
/* A guardian's overworld sprite (list 6): its own where Gregar has one,
 * else a HeelNavi's (Falzar's Navis, BlastMan, ElementMan). */
#define GUARDIAN_HEEL_SPRITE 0x43
int guardian_sprite(int navi);
/* The face a guardian speaks with (a HeelNavi's for the copies of the
 * Navis Gregar has no face of). */
int guardian_face(int navi);

/* The area a guardian keeps, for the title card ("Central Area"). */
const char *guardian_area_name(int biome);
/* The same in a sentence, with its article ("the Graveyard"), for the
 * layer's kind `side` (LAYER_UNDERNET and LAYER_SECRET name their own). */
const char *guardian_area_in_text(int biome, int side);
/* A line under an area's name on its title card. */
const char *guardian_area_motto(int biome);

#endif
