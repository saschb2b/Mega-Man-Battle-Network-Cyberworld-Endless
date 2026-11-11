/* The run's reward picks on a screen of ours (docs/game/reward-pick.md): a
 * guardian's NaviCust draft, a vault's, a Server's and a Glitch copy's
 * three chips, an official gate's, Mr.Prog's first gift. BN6 has no
 * screen for a pick of rewards (its chat box's options held a name a
 * line), so this one is drawn in BN6's PET look (docs/engine/ui.md): each
 * choice on the Library's card, its picture and details read from the
 * player's ROM. A pick's script (reward_words.c) says its lines, sets
 * REWARD_PICK_FLAG with its kind in the flags after it, and ends; after
 * the frame the screen opens over the held game, and the choice runs the
 * script that gives it (`talk_script`). Its course here, its picture in
 * reward_pick_draw.c, its words in reward_pick_text.c. */
#ifndef CW_REWARD_PICK_H
#define CW_REWARD_PICK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* the picks: the screen each opens takes its name (reward_pick_text.c) */
enum { PICK_DRAFT, PICK_VAULT, PICK_ORDER, PICK_OFFICIAL, PICK_SERVER, PICK_COPY, PICK_GIFT, PICK_KINDS };
/* what a choice is */
enum { PICK_CHIP, PICK_PROGRAM, PICK_HP };
#define PICK_MOST 3
/* B on a pick: its skip's script (the draft's BugFrags, a row of its
 * own), or none: B leaves (a vault's, which keeps), or does nothing (the
 * gift, which waits for its pick) */
#define PICK_B_LEAVES  -1
#define PICK_B_NOTHING -2

/* Event flags a pick's script sets to open its screen: the kind's bits
 * first (REWARD_KIND_FLAG + bit, three), then REWARD_PICK_FLAG; the
 * director clears them as it opens it */
#define REWARD_PICK_FLAG 0x147C
#define REWARD_KIND_FLAG 0x1467   /* 0x1467-0x1469 */
#define REWARD_KIND_BITS 3

typedef struct {
	uint8_t what;       /* PICK_CHIP, PICK_PROGRAM, PICK_HP */
	uint16_t id;        /* the chip's id, the program's number, the HPMemory given */
	uint8_t code;       /* a chip's code (0-25, 26 '*'), a program's colour (1-6) */
	int16_t script;     /* what gives it, a script of the layer's text archive */
} PickChoice;

typedef struct {
	uint8_t n;
	PickChoice c[PICK_MOST];
	int16_t skip;       /* B's script, or PICK_B_LEAVES, PICK_B_NOTHING */
	int16_t frags;      /* the skip's BugFrags, its row under the choices (0: no row) */
} PickOffer;

/* Kind `kind`'s offer, made as its scripts are built (reward_words.c):
 * a layer's are made again with it, so they follow run.layer_seed; its
 * screen's course, picture and keys go through director.h
 * (director_pick_*), which the game's scene calls */
void reward_pick_offer(int kind, const PickOffer *o);
/* What the information box says of a chip (its description, BN6's or its
 * own game's) or a program (what it does, MegaMan's words of the NaviCust)
 * of id `id` (PICK_CHIP, PICK_PROGRAM), into `out`; the lines it takes in
 * the box; the lines the box holds whole (reward_pick_draw.c; the dev
 * tool --pick-fit holds every text to them, docs/game/reward-pick.md) */
void reward_pick_info_text(int what, int id, char *out, size_t n);
int reward_pick_info_lines(const char *s);
int reward_pick_info_room(void);

#endif
