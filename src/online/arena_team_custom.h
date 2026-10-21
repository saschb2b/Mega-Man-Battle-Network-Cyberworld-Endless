/* --dev arena2v2: a partner's own folder, deck and picks in the pair
 * (epic #167, docs/ONLINE_ARENA.md, A partner's own Custom screen). His
 * folder is his own (a carried MegaMan's, or his side's lead's as his
 * side's GBA holds it), dealt into a deck of his own as BN6 deals a
 * side's (the Regular chip first, the TagChips after it, the rest
 * shuffled; a chip that sits out of BN6's battles left out), the shuffle
 * seeded by his side's GBA's random number, so that every client and
 * spectator deals the same. His Custom screen is BN6's own, on a core of
 * his own client outside the pair (arena_team_screen.h); what it decides
 * crosses in his stream as pick messages (TEAM_PICK_*: each pick a value
 * of the stream), which the pair takes as it samples his side's GBA's
 * frames. As that GBA's screen closes with his picks in (or as they come
 * in after it), they are set for the iteration it is at plus BN6's link
 * delay, in both GBAs' RAM, and each GBA writes them into his own hand
 * (arena_team_navi.h) at that iteration, plus chips merged into the chip
 * before them as BN6's hand merges them. The deck, the turn's state and
 * the picks live in both cores' RAM (RAM_TEAM_CUSTOM), so a rollback puts
 * them back and the pair's hash covers them; the folder, the same on
 * every client, is folded into the hash. Research, never a player's
 * battle. */
#ifndef CW_ARENA_TEAM_CUSTOM_H
#define CW_ARENA_TEAM_CUSTOM_H

#include <stdbool.h>
#include <stdint.h>

struct mCore;

#define TEAM_CUSTOM_PICKS 5   /* the chips picked at most */
/* The leads wait for their partners (the owner's call, 22 October 2026):
 * a side's GBA holds its Custom screen's packet back while its partner
 * picks (arena_team_wait.h), so that BN6's own link waits on both GBAs;
 * past this many of that GBA's frames held, his picks are taken as they
 * stand (none, where none came) and the battle goes on: 60 seconds */
#define TEAM_WAIT_LIMIT   3600u
#define TEAM_WAIT_SAY     120u  /* ... his own client sends the picks he has this many frames before it, to be in time */
/* A pick message in a partner's stream (past the GBA's buttons): pick
 * `index` of `count` (count 0: none picked), the slot of the hand his
 * screen dealt; a frame of his stream holding one carries no keys */
#define TEAM_PICK_MARK    0x8000u
#define TEAM_PICK(count, index, slot) \
	((uint16_t)(TEAM_PICK_MARK | ((unsigned)(count) & 7u) << 11 | ((unsigned)(index) & 7u) << 8 | ((unsigned)(slot) & 0xFFu)))

/* side's partner's folder before the battle: 30 entries (chip | code <<
 * 9, 0xFFFF none), its Regular chip and TagChips (entries, 0xFF none);
 * folder NULL: his side's lead's, as his side's GBA holds it */
void arena_team_custom_folder(unsigned side, const uint16_t *folder, uint8_t regular, const uint8_t tags[2]);
/* from the spawn's hook: the core's decks and turns emptied, to be dealt anew */
void arena_team_custom_begin(void);
/* From the pair, as side's GBA (core[side]) begins a frame of its own
 * clock: his deck dealt where it is not, his turn begun with BN6's Custom
 * screen there, a pick message of his stream taken, his picks set for
 * that GBA's iteration plus `delay` once its screen has closed and they
 * are in, in both cores. The keys his battle takes for the frame: none
 * while that screen is open, nor in a pick message. */
uint16_t arena_team_custom_frame(struct mCore *core[2], unsigned side, uint16_t value, uint32_t delay);
/* from the key copy's hook at iteration `tick`: each partner's picks set
 * for it written into his own hand at `hand[side]` (`short_one`: a chip
 * short, the mutation) */
void arena_team_custom_take(uint32_t tick, const uint32_t hand[2], bool short_one);
/* side's partner's deck as `core` holds it: its entries in the order drawn,
 * how many (his own client's screen deals from it); 0 before it is dealt */
unsigned arena_team_custom_deck(struct mCore *core, unsigned side, uint16_t out[30]);

/* From a hook, on the core of side's GBA: whether it holds its packet
 * for its partner now (dealt, his turn open and his picks not in, he on
 * the field); and whether it owes the packet, held back */
bool arena_team_custom_holds(unsigned side);
void arena_team_custom_owe(unsigned side, bool owed);
bool arena_team_custom_owed(unsigned side);
/* As `core` (side's GBA's) holds it: the frames it has held for its
 * partner this turn, whether it holds now (his lead has pressed OK and
 * waits), whether he is still picking */
unsigned arena_team_custom_waited(struct mCore *core, unsigned side);
bool arena_team_custom_lead_waits(struct mCore *core, unsigned side);
bool arena_team_custom_picking(struct mCore *core, unsigned side);

/* the partners' folders folded into the pair's hash */
uint64_t arena_team_custom_hash(uint64_t h);
/* how the turns went (decks dealt, picks come, set and taken), said in the output */
void arena_team_custom_report(void);

#endif
