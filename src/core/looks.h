/* MegaMan's look (issue #854, docs/game/looks.md): colour channels on his
 * plain body, each coloured from one of the palettes the profile owns. A
 * look is six bytes (a palette and a colour per channel), what a profile
 * keeps three of and what the Square's wire carries. Looks only: nothing
 * here changes how a run plays. The palettes' colours and names are ours
 * (looks.c, looks_text.c); the profile's looks and palettes owned are kept
 * by looks_profile.c; a look is written onto BN6's palettes by
 * src/emu/look_apply.h. */
#ifndef CW_LOOKS_H
#define CW_LOOKS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define LOOK_CHANNELS 3     /* 0 armour (BN6's light blue ramp), 1 suit (the dark blue), 2 accent (the gold) */
#define LOOK_PALETTES 12
#define LOOK_COLOURS  16    /* a palette's colours */
#define LOOK_SLOTS    3     /* the looks a profile keeps */
#define LOOK_NONE     0xFF  /* a channel's palette and colour: the channel as BN6 draws it */

enum { LOOK_ARMOUR, LOOK_SUIT, LOOK_ACCENT };

typedef struct {
	uint8_t palette[LOOK_CHANNELS];   /* a LookPalette, or LOOK_NONE */
	uint8_t colour[LOOK_CHANNELS];    /* 0 .. LOOK_COLOURS - 1, or LOOK_NONE with its palette */
} Look;

enum LookPalette {
	LOOK_CLASSIC,      /* BN6's own blue and a starter spread: owned from the start */
	LOOK_UNDERNET,
	LOOK_GREGAR,
	LOOK_PROTOMAN,
	LOOK_NEBULA,
	LOOK_TOURNAMENT,
	LOOK_HAZARD,
	LOOK_CENTRAL,      /* Central Town */
	LOOK_SEASIDE,
	LOOK_SKY,
	LOOK_MYSTERY,
	LOOK_GLITCH
};

/* ---- the palettes (looks.c, looks_text.c): no profile, no ROM ---- */

/* Palette `p`'s name as the screen shows it ("" past the last). */
const char *looks_palette_name(int p);
/* Palette `p`'s colour `i`, BGR555 as the GBA's (0 past either's end). */
uint16_t looks_colour(int p, int i);
/* Whether every channel is a palette and colour in range, or both
 * LOOK_NONE: a look read from a file or a peer is checked with it. It does
 * not ask whether the palettes are owned. */
bool look_valid(const Look *look);
/* The look of BN6's own colours, every channel LOOK_NONE. */
Look look_plain(void);

/* ---- the profile's (looks_profile.c) ---- */

/* Whether the profile owns palette `p` (Classic always). */
bool looks_owned(int p);
/* Palette `p` given to the profile: true where it is new, the profile then
 * saved; false where it was owned already or `p` is out of range. */
bool looks_grant(int p);
/* The look slot worn, 0 .. LOOK_SLOTS - 1. */
int looks_slot(void);
/* Slot `slot` worn from now (the profile saved); out of range is ignored. */
void looks_select(int slot);
/* Slot `slot`'s look (look_plain past the slots or where it is invalid). */
Look looks_get(int slot);
/* Slot `slot` set to `look` and the profile saved: false, and nothing set,
 * where the slot is out of range or the look not look_valid. Ownership is
 * the screen's to keep (it offers the owned palettes alone). */
bool looks_set(int slot, const Look *look);
/* The look worn: the selected slot's, or the dev switch's where one is
 * set for the session (looks_set_override). */
Look looks_worn(void);
/* A look worn for the session in place of the slot's, not saved
 * (`--dev look=`); NULL drops it. */
void looks_set_override(const Look *look);
/* `fn` called whenever the worn look changes (look_apply.c registers its
 * apply: a layer below cannot call it) */
void looks_on_change(void (*fn)(void));
/* The state in words: `look P.C,P.C,P.C slot N` (`-` a channel as BN6
 * draws it), then what `fn` adds (look_apply.c's `lookseen`) */
void looks_describe(FILE *f);
void looks_describe_with(void (*fn)(FILE *f));
/* `--dev look=P.C,P.C,P.C` read: the channels given (P.C or -), the rest
 * BN6's; false where the words are no look */
bool looks_parse(const char *words, Look *out);
/* The profile just read (profile_load): one saved before looks were kept
 * takes Classic owned, every slot BN6's own colours and slot 0. */
void looks_profile_loaded(void);

#endif
