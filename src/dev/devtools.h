/* Dev tools for testing a run quickly (devtools.c, docs/DEVTOOLS.md): a
 * menu over the game (hold SELECT, press R) and the same switches from the
 * command line (--dev). */
#ifndef CW_DEVTOOLS_H
#define CW_DEVTOOLS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	bool god;       /* MegaMan's HP stays full, in battle and out */
	bool onehit;    /* enemies keep 1 HP: one hit deletes them */
	bool quiet;     /* no random battles */
	bool fragile;   /* MegaMan keeps 1 HP in battle: the run ends at the first hit */
	int speed;      /* game frames per frame shown: 1, 2, 4, 8 */
	bool powers;    /* the five Crosses and BeastOut open (a capture's: tools/trailer.py) */
	bool gem;       /* every random battle with a Mystery Data on the field */
	bool veteran;   /* a profile that has met seven guardians, found two Spins and beaten the rival (captures: the PET's mail) */
	int duels;      /* duels=N: the rival's wins made N, for a capture of a rung or an official gate (docs/RIVAL.md); -1 left alone */
	int hp;         /* hp=N: MegaMan's max HP N, his HP with it each time the max moves (a later act's fight swept at a playtester's HP); 0 left alone */
	int hp_now;     /* hp=N/H: ... and his HP H through the run's first 600 frames (a guest battle begun hurt); 0 the max */
	bool mapall;    /* the layer's map whole, as if every panel were seen (a capture of the map: tools/before_after.py) */
	int folder;     /* folder=ID: the run's folder all chip ID in * (a capture of one chip's battles, the first hand all of it); 0 none */
	int folder_n;   /* folder=ID/N: ... its first N entries alone (one beside the run's own chips) */
	int pack;       /* pack=N: a copy of each of the first N chips in the pack, in its first code (the folder editor's pack, issue #75) */
	int programs;   /* programs=N: a copy of each of the first N NaviCust programs, in a colour the ROM draws (the NaviCustomizer, issue #76) */
	int clock;      /* clock=N: the Net's clock at N notches from the run's start (docs/HOME.md: its guardians' HP) */
} DevFlags;

extern DevFlags dev;

/* "god,onehit,quiet,speed=4,powers" */
void devtools_parse(const char *spec);
/* With `veteran`, once the profile is loaded: its guardians' records and
 * Spins, where it has none yet; with `duels=N`, the rival's wins. */
void devtools_veteran(void);
/* The player's GBA keys: the menu takes them while it is open. */
uint32_t devtools_keys(uint32_t keys);
/* The menu is open: the game holds still. */
bool devtools_open(void);
/* 10000 zenny, the game's way (the menu's, and --talk zenny). */
void devtools_zenny(void);
/* After each game frame: the switches' effects. */
void devtools_update(void);
/* ... and after each frame of a battle on the guest core (guest.h): god,
 * onehit and fragile on its battle's objects. */
void devtools_guest_update(void);
void devtools_draw(void);

/* A frame to save after the next draw (the tour sets it); "" for none. */
extern char devtools_shot[512];

#endif
