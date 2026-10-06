/* The guardian Navis as the boss sequence presents them: name, epithet,
 * mugshot, colour, and what they say, chosen by what they remember of
 * MegaMan (docs/BOSSES.md). */
#ifndef CW_GUARDIANS_H
#define CW_GUARDIANS_H

#include <stdbool.h>
#include <stddef.h>

#include "bn5.h"
#include "stage_npc.h"
#include "super_boss.h"

#define GUARDIAN_NO_MUGSHOT -1

typedef struct {
	const char *name;       /* "ElecMan" */
	const char *epithet;    /* "Master of Current" */
	int mugshot;            /* the game's mugshot, GUARDIAN_NO_MUGSHOT for Falzar's Navis */
	int pose;               /* overworld animation shown on the title card, -1 for none */
	unsigned char r, g, b;  /* the title card's accent */
} Guardian;

/* navi index as in the battle's enemy table (1 HeatMan .. 16 ElementMan, 18 Colonel;
 * the super bosses 19 Bass and 20 the Cybeast Gregar, super_boss.h) */
const Guardian *guardian(int navi);
/* Another game's Navis as a territory's guardians (docs/BOSSES.md, BN5's
 * Navis): BN5's Team Colonel by their AI index there (bn5.h BN5_NAVI_*,
 * Colonel 7 .. ToadMan 12) as navis 24-29 here, each with his own rival
 * record (BN6's Colonel and TomahawkMan are other Navis' copies) */
#define GUARDIAN_OLDER_FIRST 24
#define GUARDIAN_OLDER_LAST  29
static inline bool guardian_older(int navi) { return navi >= GUARDIAN_OLDER_FIRST && navi <= GUARDIAN_OLDER_LAST; }
static inline int guardian_older_ai(int navi) { return navi - GUARDIAN_OLDER_FIRST + BN5_NAVI_COLONEL; }
static inline int guardian_of_older(int ai) { return ai - BN5_NAVI_COLONEL + GUARDIAN_OLDER_FIRST; }
/* His overworld sprite and mugshot in his game's lists 6 and 8 (the same
 * number), 0 for none */
int guardian_older_sprite(int navi);
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
/* The face a guardian speaks with: his mugshot, or the one set for him
 * (Falzar's Navis, whose faces Gregar lacks: a portrait, portrait.c), else
 * none. */
int guardian_face(int navi);
void guardian_set_face(int navi, int face);

/* The area a guardian keeps, for the title card ("Central Area"). */
const char *guardian_area_name(int biome);
/* The same in a sentence, with its article ("the Graveyard"), for the
 * layer's kind `side` (LAYER_UNDERNET and LAYER_SECRET name their own). */
const char *guardian_area_in_text(int biome, int side);
/* Whether another game's area draws `biome` in this run (its battles the
 * older net's). */
bool guardian_area_older(int biome);
/* Its name in nine letters at most, for the PET's PLACE beside the layer
 * ("JudgeTree 12"). */
const char *guardian_area_short(int biome);
/* A line under an area's name on its title card. */
const char *guardian_area_motto(int biome);

/* (guardian_lines.c) A first battle's Guardian Data with his battle data,
 * after `power` (or NULL); and MegaMan on an older net guardian's Soul */
const char *guardian_data_words(const char *power);
const char *guardian_soul_words(int navi, int kind, bool held, const char *chip, const char *dark);

#endif
