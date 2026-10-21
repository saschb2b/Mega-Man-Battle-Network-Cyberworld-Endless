/* --dev arena2v2: a partner's own Custom screen, BN6's own (epic #167,
 * docs/ONLINE_ARENA.md, A partner's own Custom screen). A partner has no
 * GBA in the pair, so his own client runs his screen outside it: never
 * hashed, never part of the battle's determinism, never on a spectator's
 * client. A lone core taken from his side's GBA at a Custom screen ends
 * BN6's link battle within frames ("Communication error", BN6's own words
 * on its screen: it has no other GBA on its cable), so the screen is a
 * plain copy of the pair, both GBAs on their own cable with no hook on
 * them (arena_pair_open_plain): each turn the pair is copied into it as
 * his side's GBA's Custom screen opens (arena_pair_save, arena_pair_load:
 * the battle behind the screen is the battle's own), his deck (his own
 * folder in the order the pair dealt it, arena_team_custom.h) is written
 * into BN6's battle deck on his side's GBA there and BN6 sets its Custom
 * screen up anew from it (the battle phase's step put back to 0), so that
 * BN6 deals his hand, loads its chips' icons and runs its own rules on his
 * keys; the other GBA there holds still at its own. Once his OK is
 * pressed his picks (the slots of that hand, in order) are read from it;
 * only they cross, in his stream, and the pair writes them into his hand
 * on both GBAs. */
#ifndef CW_ARENA_TEAM_SCREEN_H
#define CW_ARENA_TEAM_SCREEN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct ArenaPair;
typedef struct TeamScreen TeamScreen;

/* side's partner's screen: a plain pair of the ROM's bytes (BN5's
 * installed in them, as the pair's) made from `pair`'s cores; NULL where
 * it could not be made */
TeamScreen *team_screen_new(const uint8_t *rom, size_t size, struct ArenaPair *pair, unsigned side);
void team_screen_free(TeamScreen *s);
/* His turn: `pair` copied in, `deck` (n entries, chip | code << 9, in the
 * order drawn) written into BN6's deck on his side's GBA, its Custom
 * screen set up anew */
bool team_screen_open(TeamScreen *s, struct ArenaPair *pair, const uint16_t *deck, unsigned n);
/* A frame of it with his `keys`: true at the frame his OK is pressed,
 * his picks then in picks[0..*n) (the slots of the hand BN6 dealt him) */
bool team_screen_frame(TeamScreen *s, uint16_t keys, uint8_t *picks, unsigned *n);
/* his picks so far (the slots of the hand, in order), how many; and the
 * turn ended without his OK (his side's GBA's screen closed first) */
unsigned team_screen_picks(const TeamScreen *s, uint8_t *picks);
void team_screen_close(TeamScreen *s);
/* his side's GBA's picture there (ARENA_W x ARENA_H), and whether a turn is under way */
const uint32_t *team_screen_video(const TeamScreen *s);
bool team_screen_open_now(const TeamScreen *s);
/* what a frame of it costs here, mean and worst, ms; and a turn's copy of the pair */
double team_screen_ms(const TeamScreen *s, bool worst);
double team_screen_copy_ms(const TeamScreen *s);

#endif
