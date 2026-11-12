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
#include "xdiff_ncp.h"

#define MAX_KEYS  128
#define MAX_POKES 8
#define MAX_TWINS 16
#define MAX_FLAGS 16
#define MAX_SCRIPTS 8
#define MAX_SCRIPT_STEPS 32
#define MAX_SHOTS 16

/* (game=bn4) BN4's battle alone, its end told; traced alone (bn4:trace, OUT.bn4); traced against BN6's (bn4:bn6) */
enum { XD_BN4_ALONE = 1, XD_BN4_TRACE, XD_BN4_BN6 };

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
/* Pictures of the fight (film=): each game's at the battle's timer values from `from` to `to`, `step` apart, into
 * PREFIX_bn5_tNNNN.bmp and PREFIX_bn6_tNNNN.bmp, side by side by the same clock the comparison keeps */
#define MAX_PANELS 8   /* (panels=) the panels a spec may give a type */

typedef struct { int from, to, step; char prefix[160]; } Film;

typedef struct {
	int nfoes;
	struct { int id, panel; } foes[MAX_FOES];
	int mm, hp, frames;
	int field, field6;   /* the battlefield, BN5's number, and BN6's (-1: as xfields.h installs it), 0 plain */
	int sides;           /* whose each column is, both records' +6 (bn6_panels.h's BN6_SIDES_*): 0 the plain three and three */
	int mm_element;      /* MegaMan's element as the fight begins, in both (-1 his own) */
	int flags[2][MAX_FLAGS], nflags[2];        /* event flags set before the battle: [0] BN5's, [1] BN6's */
	uint16_t folder[2][BN6_FOLDER_ENTRIES];    /* its first folder, each game's (0 none given): BN5's nine-bit, BN6's in this build's
	                                              form (chip_entry.h), as its deck */
	int nfolder[2];
	uint16_t deck[2][BN6_FOLDER_ENTRIES];      /* each game's battle deck in this order, written as its battle runs (0 none given) */
	int ndeck[2];
	unsigned souls6;                           /* the Souls BN6's MegaMan holds, the Souls installed in its engine (soulunite.h); 0 none */
	int unitebreak;                            /* (the comparator's own test) UNITE's rule broken (SOULUNITE_BREAK_*) */
	int soulbreak;                             /* ... a Soul's look, abilities or passives broken (soulunite.h's SOULBREAK_*) */
	int buster[3];                             /* Attack, Speed and Charge in both, levels less one; -1 each game's own */
	struct { int variant, col, row; } ncp[2];  /* a NaviCust program placed on each game's own board ([0] BN5's, [1] BN6's: its variant, program x 4
	                                              + colour, and where), and each game's own compile run before the battle; variant 0 none */
	bool safe;                                 /* MegaMan kept from harm: his collision data's BN6_OBJ_INVULNERABLE every frame */
	bool dark;                                 /* MegaMan dark as the battle begins: BN5's dark meter under 470, BN6's engine the same
	                                              (soulmood.h's soulmood_dev_dark; the Souls installed, souls6) */
	unsigned souls4;                           /* (a BN4 spec) the Souls BN4's MegaMan holds, bit n - 1 its Soul n (Roll 1 .. Wood 12): each
	                                              one's flag, Double Soul's and its pseudo-chip past BN4's gate (xd4_souls_put); 0 none */
	int karma4;                                /* (a BN4 spec) MegaMan's karma as BN4's battle begins (BN4_NAVI_KARMA: under BN4_KARMA_DARK a dark
	                                              chip plays, bn4_chips.h); -1 the guest's own */
	bool calm4;                                /* (an attack DarkChip's BN4 spec) BN4's MegaMan's confusion at the fight's start ended by BN4's
	                                              own step (xd4_start_confusion): the run gives none (the owner's call of 12 November 2026) */
	bool confuse6;                             /* (a mutation of such a spec) BN6's MegaMan given that confusion instead, which must part */
	bool mood4;                                /* (a DarkChip's BN4 spec) BN4's MegaMan's mood kept at the DarkChip's use, which BN4 sets 0, its
	                                              dark emotion, for the battle (xd4_dark_mood): the run takes none of BN4's dark rules */
	bool dslist;                               /* BN6's chip AI's lists written from BN5's as its fight begins (xnavi_dark.h) */
	uint16_t dschips[BN6_FOLDER_ENTRIES];      /* MegaMan's side's chip AI list in BN5, BN5's chip ids, written as its fight begins (dslist too) */
	int ndschips;
	int hold;                                  /* (a chip's spec) the foes kept paralysed this many frames from each fight frame on, the chip's
	                                              target holding still in both games; 0 none */
	struct { uint8_t col, row, type; } panels[MAX_PANELS];   /* (panels=) panels given a type in each game as its fight begins: a
	                                              panel chip's spec (AquaUp's cracked, GreenWd's grass), the type's number the same in BN4,
	                                              BN5 and BN6 for the ones a spec names (bn4.h's panel types) */
	int npanels;
	CustomScript scripts[MAX_SCRIPTS];
	int nscripts;
	Shot shots[MAX_SHOTS];
	int nshots;
	Film film;           /* (step 0: none) */
	Key keys[MAX_KEYS];
	int nkeys;
	uint32_t seed[2];
	Poke pokes[MAX_POKES];
	int npokes;
	XdTwin twins[MAX_TWINS];
	int ntwins;
	char out[256];
	bool bn5, bn6, follow, replay;
	int bn4;            /* (game=bn4...) BN4 Red Sun's own battle on the guest core, XD_BN4_* (xdiff_bn4.h, xdiff_bn4_trace.h); 0 none */
	int lag;            /* (lag=N) BN6's battle N fight frames behind BN4's: BN6's engine runs no state while the opening is paused
	                     * (docs/devtools/differential.md, The BN4 trace), so BN6's is traced N frames more, its keys by the timer N
	                     * later, and with rng=replay the family's own draws given the reference's of N frames before; 0 none */
	uint32_t skip_key[XD_SETUP_SKIPS];   /* (setupskip=KEY:N,...) with rng=replay against BN4's: the reference's first N setup */
	int skip_n[XD_SETUP_SKIPS], nskips;  /* draws of KEY left without a twin, BN6's paired from the opening's end
	                                      * (tools/xdiff_bn4.py opening_skips; legit.json's bn4 opening_draws) */
} Spec;

/* SPEC's items into s (its defaults first): false, said on stderr, where
 * one is not understood or no foe is given */
bool xd_spec_parse(const char *text, Spec *s);

/* A folder written as its game's first, each chip owned (its mark its key XOR the game's), with no Regular chip or
 * TagChips: the folders' pointer and marks' at their eToolkit places (game's own read and write) */
typedef struct {
	uint32_t toolkit, folders, marks, keys, key_xor, regular, tags;   /* (the NaviStats' Folder1Reg, its two TagChips; 0 none) */
	uint32_t id_mask;   /* (an entry's id: chip_entry.h's in BN6, nine bits in BN5) */
	uint8_t (*rd8)(uint32_t);
	uint32_t (*rd32)(uint32_t);
	void (*wr8)(uint32_t, uint8_t);
	void (*wr16)(uint32_t, uint16_t);
} FolderPlaces;

/* A spec's folder put into its game's state (folder_put's FolderPlaces: BN6's or BN5's) */
void xd_folder_put(const FolderPlaces *p, const uint16_t *folder);
/* The folder's chips owned in BN6's pack, one of each chip and code */
void xd_pack_own(const uint16_t *folder);

/* The opening's draws of the spec's keys paired from its end (xd_replay_setup_skip): with rng=replay against BN4's alone;
 * false where the log has no room for them */
bool xd_setup_skips(const Spec *s, XdLog *a, XdLog *b);

#endif
