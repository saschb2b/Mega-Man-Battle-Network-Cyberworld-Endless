/* The guardian Navis as the boss sequence presents them: name, epithet,
 * mugshot, colour, and what they say, chosen by what they remember of
 * MegaMan (docs/BOSSES.md). */
#ifndef CW_GUARDIANS_H
#define CW_GUARDIANS_H

#include <stdbool.h>
#include <stddef.h>

#include "stage_npc.h"

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
/* ElementMan, whose element changes as he fights: none of his answers his
 * every form (bn6f: no element in his stats or traits) */
#define GUARDIAN_ELEMENTMAN 16

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
/* ProtoMan's terms for the netbattle past an act's guardian, as ta_duel
 * shows them: the stake said (BN6 deletes MegaMan in a netbattle as
 * anywhere, its GAME OVER from inside the battle), and how he fights,
 * which MegaMan knows of his rival (a playtester asked for his tells after
 * a hand against him). Into `out`; the length they take, which
 * GUARDIAN_TERMS_MAX holds. */
#define GUARDIAN_TERMS_MAX 512
int guardian_netbattle_terms(char *out, size_t n);
/* What the net says of guardian `navi` (the clause after "Word is, "), for
 * a bystander to pass on where MegaMan has never battled him: true, vague,
 * no moves (docs/META.md, what MegaMan knows). NULL for none. */
const char *guardian_rumor(int navi);
/* Whether MegaMan knows guardian `navi`: they have battled him, in any
 * run (rivals.sav). Before that he names him nowhere, and briefs none of
 * his moves. */
bool guardian_known(int navi);
/* A guardian's overworld sprite (list 6), where Gregar has one; else a
 * HeelNavi's (Falzar's Navis, who stand in their battle sprites instead:
 * guardian_body). */
#define GUARDIAN_HEEL_SPRITE 0x43
int guardian_sprite(int navi);
/* How guardian `navi` stands on the net facing `face` (the overworld's
 * eighths: 1 up-right, 3 down-right, 5 down-left, 7 up-left). */
NpcBody guardian_body(int navi, int face);
/* The face a guardian speaks with (none for Falzar's Navis, whose faces
 * Gregar lacks). */
int guardian_face(int navi);

/* The area a guardian keeps, for the title card ("Central Area"). */
const char *guardian_area_name(int biome);
/* The same in a sentence, with its article ("the Graveyard"), for the
 * layer's kind `side` (LAYER_UNDERNET and LAYER_SECRET name their own). */
const char *guardian_area_in_text(int biome, int side);
/* Its name in nine letters at most, for the PET's PLACE beside the layer
 * ("JudgeTree 12"). */
const char *guardian_area_short(int biome);
/* A line under an area's name on its title card. */
const char *guardian_area_motto(int biome);

#endif
