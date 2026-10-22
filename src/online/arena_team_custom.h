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
 * of the stream), which come to both GBAs in his side's GBA's packets
 * (arena_team_link.h). Each GBA works his turn out from them on its own:
 * his deck dealt as its Custom screen first opens, his picks in as their
 * last message comes, and as the screen closes with them in (or as they
 * come in after it) they are set for the battle loop's next iteration and
 * written into his own hand there (arena_team_navi.h), plus chips merged
 * into the chip before them as BN6's hand merges them. Both GBAs read the
 * same packets at the same frame of their battles, so each keeps the same
 * turn without a byte written across from the other. The deck, the turn's
 * state and the picks live in each core's RAM (RAM_TEAM_CUSTOM), so a
 * rollback puts them back and the pair's hash covers them; the folder,
 * the same on every client, is folded into the hash. */
#ifndef CW_ARENA_TEAM_CUSTOM_H
#define CW_ARENA_TEAM_CUSTOM_H

#include <stdbool.h>
#include <stdint.h>

#include "bn6.h"
#include "team_check.h"

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
/* ... and his Cross (a count no pick has): the form his own CROSSSELECT
 * committed (BN6_CROSS_RECORD's +0, 1-0x18), said before his picks */
#define TEAM_CROSS_COUNT  7u
#define TEAM_CROSS(form)  TEAM_PICK(TEAM_CROSS_COUNT, 0, form)
/* ... and the Soul his UNITE committed with the proxy form (said after
 * his Cross message): BN5's number (1-12) and 1 for its Chaos Unison;
 * ColonelSoul's chip, where his button took one, in two (arena_team_soul.h) */
#define TEAM_SOUL(soul, chaos) TEAM_PICK(TEAM_CROSS_COUNT, 1, ((unsigned)(soul) & 0xFu) | ((unsigned)(chaos) & 1u) << 4)
#define TEAM_SOUL_CHIP(chip) TEAM_PICK(TEAM_CROSS_COUNT, 2u + ((unsigned)(chip) >> 8 & 1u), (unsigned)(chip) & 0xFFu)
/* ... and a Program Advance his screen made at OK: its first pick (0-4),
 * how many picks it took (3 or 4) and its chip (BN6_PA_FIRST on), said
 * before his picks; the pair joins those picks into it as BN6's OK does */
#define TEAM_PA_COUNT     6u
#define TEAM_PA(at, took, id) \
	TEAM_PICK(TEAM_PA_COUNT, at, (((unsigned)(id) - BN6_PA_FIRST) & 0x1Fu) | ((unsigned)(took) - 3u) << 5)

/* A hand as BN6's OK builds it from picks (bn6f sub_8029110): the chips'
 * ids, powers, plus chips' bonuses and folder entries */
typedef struct {
	uint16_t id[BN6_HAND_MAX], power[BN6_HAND_MAX], bonus[BN6_HAND_MAX], entry[BN6_HAND_MAX];
} TeamHand;
/* `n` picks' folder entries (chip | code << 9) into `out` as BN6's OK
 * builds a hand: where pa_at (its first pick + 1, 0 none) is given, the
 * pa_took picks from there joined into chip pa_id first (sub_802B6F2),
 * then each Atk+10, Atk+30 or Navi+20 merged into the chip before it
 * (sub_8029224); the chips in the hand */
unsigned team_hand_build(const uint16_t *entries, unsigned n, unsigned pa_at, unsigned pa_took, unsigned pa_id, TeamHand *out);

/* side's partner's folder before the battle: 30 entries (chip | code <<
 * 9, 0xFFFF none), its Regular chip and TagChips (entries, 0xFF none);
 * folder NULL: his side's lead's, as his side's GBA holds it */
void arena_team_custom_folder(unsigned side, const uint16_t *folder, uint8_t regular, const uint8_t tags[2]);
/* side's partner's MegaMan's Crosses, Beast Out and Souls, as every game
 * was given them (NULL none): what his messages may say (team_check.h) */
void arena_team_custom_allow(unsigned side, const TeamAllowed *allowed);
/* from the spawn's hook: the core's decks and turns emptied, to be dealt anew */
void arena_team_custom_begin(void);
/* From the pair, before side's GBA (core) runs a frame: his lead's
 * folder read from it where his own was not given */
void arena_team_custom_sampled(struct mCore *core, unsigned side);
/* From a hook, each frame side's packet comes with `word` (his keys or a
 * message, arena_team_link.h): his deck dealt where it is not, his turn
 * begun with BN6's Custom screen, a message taken, the wait's frames
 * counted on his side's GBA, his picks set for the battle loop's next
 * iteration once its screen has closed and they are in (`second`: the
 * cable's second GBA, for the output). The keys his battle takes: none
 * while that screen is open, nor in a message's frame. */
uint16_t arena_team_custom_link(unsigned side, uint16_t word, bool second);
/* from the key copy's hook at iteration `tick`: each partner's picks set
 * for it written into his own hand at `hand[side]` (`second`: the second
 * GBA's core, where teampick=-1 leaves a chip short and teampa=-1 his
 * Program Advance out, the mutations) */
void arena_team_custom_take(uint32_t tick, const uint32_t hand[2], bool second);
/* side's partner's deck as `core` holds it: its entries in the order drawn,
 * how many (his own client's screen deals from it); 0 before it is dealt */
unsigned arena_team_custom_deck(struct mCore *core, unsigned side, uint16_t out[30]);

/* From a hook, on the core of side's GBA: whether it holds its packet
 * for its partner now (dealt, his turn open and his picks not in, he on
 * the field); and whether it owes the packet, held back */
bool arena_team_custom_holds(unsigned side);
/* From a hook: the form side's partner's own CROSSSELECT committed this
 * turn (0 none), his pending form until his next turn opens */
uint8_t arena_team_custom_cross(unsigned side);
/* ... taken from this core's RAM as the crossing is done (BN6 takes its records' so) */
void arena_team_custom_cross_taken(unsigned side);
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
