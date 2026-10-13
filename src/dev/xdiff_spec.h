/* The differential's spec (xdiff.h lists its items): the battle, MegaMan's
 * setup in each game, the keys, the RNG words and the comparator's own
 * test's breaks, parsed from SPEC's NAME=VALUE items. */
#ifndef CW_XDIFF_SPEC_H
#define CW_XDIFF_SPEC_H

#include <stdbool.h>
#include <stdint.h>

#include "bn6.h"
#include "foes.h"
#include "xdiff_events.h"

#define MAX_KEYS  128
#define MAX_POKES 8
#define MAX_TWINS 16
#define MAX_FLAGS 16
#define MAX_SCRIPTS 8
#define MAX_SCRIPT_STEPS 32
#define MAX_SHOTS 16

typedef struct { uint32_t t, n, keys; } Key;
typedef struct { uint32_t at, v; int size; bool both, bn5; uint32_t at5; } Poke;   /* size 0: a call's veneer, from BN6's routine `at` to `v`;
                                                                                   both: BN5's copy too; bn5: `at` BN5's, its copy's (a chip's
                                                                                   moved code's too); at5: BN5's at its own place (each=) */

/* A Custom screen's keys (cust5=, cust6=): game 5 or 6's screen n (1 the
 * first), each step its keys for its frames from the frame its window first
 * reads open */
typedef struct { uint32_t keys; int n; } ScriptStep;
typedef struct { int game, screen, nsteps; ScriptStep steps[MAX_SCRIPT_STEPS]; } CustomScript;
/* A picture of a Custom screen (shot=): game 5 or 6's screen n, its frame f from the frame its window first reads open */
typedef struct { int game, screen, frame; char path[160]; } Shot;

typedef struct {
	int nfoes;
	struct { int id, panel; } foes[MAX_FOES];
	int mm, hp, frames;
	int field, field6;   /* the battlefield, BN5's number, and BN6's (-1: as xfields.h installs it), 0 plain */
	int mm_element;      /* MegaMan's element as the fight begins, in both (-1 his own) */
	int flags[2][MAX_FLAGS], nflags[2];        /* event flags set before the battle: [0] BN5's, [1] BN6's */
	uint16_t folder[2][BN6_FOLDER_ENTRIES];    /* its first folder, each game's (0 none given) */
	int nfolder[2];
	uint16_t deck[2][BN6_FOLDER_ENTRIES];      /* each game's battle deck in this order, written as its battle runs (0 none given) */
	int ndeck[2];
	unsigned souls6;                           /* the Souls BN6's MegaMan holds, the Souls installed in its engine (soulunite.h); 0 none */
	int unitebreak;                            /* (the comparator's own test) UNITE's rule broken (SOULUNITE_BREAK_*) */
	int soulbreak;                             /* ... a Soul's look, abilities or passives broken (soulunite.h's SOULBREAK_*) */
	int buster[3];                             /* Attack, Speed and Charge in both, levels less one; -1 each game's own */
	bool safe;                                 /* MegaMan kept from harm: his collision data's BN6_OBJ_INVULNERABLE every frame */
	bool dark;                                 /* MegaMan dark as the battle begins: BN5's dark meter under 470, BN6's engine the same
	                                              (soulmood.h's soulmood_dev_dark; the Souls installed, souls6) */
	bool dslist;                               /* BN6's chip AI's lists written from BN5's as its fight begins (xnavi_dark.h) */
	uint16_t dschips[BN6_FOLDER_ENTRIES];      /* MegaMan's side's chip AI list in BN5, BN5's chip ids, written as its fight begins (dslist too) */
	int ndschips;
	int hold;                                  /* (a chip's spec) the foes kept paralysed this many frames from each fight frame on, the chip's
	                                              target holding still in both games; 0 none */
	CustomScript scripts[MAX_SCRIPTS];
	int nscripts;
	Shot shots[MAX_SHOTS];
	int nshots;
	Key keys[MAX_KEYS];
	int nkeys;
	uint32_t seed[2];
	Poke pokes[MAX_POKES];
	int npokes;
	XdTwin twins[MAX_TWINS];
	int ntwins;
	char out[256];
	bool bn5, bn6, follow, replay;
} Spec;

/* SPEC's items into s (its defaults first): false, said on stderr, where
 * one is not understood or no foe is given */
bool xd_spec_parse(const char *text, Spec *s);

#endif
