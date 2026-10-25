/* Two MegaMen a side (epic #167, issue #181; docs/online/arena/two-a-side/partners-words.md, The
 * partners' words in BN6's packet): each partner's stream crosses BN6's
 * own link inside his side's GBA's packet, as his lead's pad does. A link
 * battle's packet (BN6_LINK_OUT, eight u16 a frame) leaves two bytes that
 * nothing of BN6's writes or reads (BN6_LINK_SPARE_LO and _HI); as the
 * battle stores its lead's pad there (BN6_LINK_PAD_STORE), his partner's
 * word for the frame goes into them: his keys, or one message of his
 * Custom screen (a pick, his Cross, a Program Advance, TEAM_PICK_MARK
 * set). The cable delivers each exchange to both GBAs, and each GBA reads
 * one exchange a frame of its battle, in order, so a partner's word comes
 * out of the same exchange as his lead's pad on both GBAs, at the same
 * frame of their battles: the two can no more part over a partner than
 * over a lead, however far apart in time the two GBAs run. Everything
 * else of his turn (his deck dealt, his picks in, the wait's limit, his
 * hand set) is worked out on each GBA from those words and its own RAM
 * (arena_team_custom.h), never written across from the other.
 *
 * His stream comes to his side's GBA as a pad does: the pair hands it his
 * value for each of that GBA's frames before the frame runs
 * (team_link_put), keys kept as the last held for its next packet,
 * messages queued, one a packet (team_outbox.h: a turn's with room to
 * spare, a message past it refused and said in the output). The words
 * are taken where BN6 reads both sides' packets and put where it stores
 * its pad: in sub_801FF18 the two together, in sub_801FFD6 each where it
 * does them (its argument may skip either). All of it is core RAM (RAM_TEAM_LINK),
 * emptied as the battle spawns, so a rollback puts it back with the state
 * and the pair's hash covers it. */
#ifndef CW_ARENA_TEAM_LINK_H
#define CW_ARENA_TEAM_LINK_H

#include <stdbool.h>
#include <stdint.h>

struct mCore;
struct HookCore;

/* the pad store's two hooks on one of the pair's cores (`second`: the
 * cable's second GBA's, for the output) */
bool team_link_hooks(struct HookCore *core, bool second);
/* from the spawn's hook: this core's outboxes and keys emptied */
void team_link_begin(void);
/* From the pair, before side's GBA (core) runs a frame of its own clock:
 * its partner's value for that frame, keys or a message */
void team_link_put(struct mCore *core, unsigned side, uint16_t value);
/* From a hook on side's GBA: a message of its own to send as its
 * partner's (the wait's limit saying his picks none), queued after his;
 * false where his outbox is full (team_outbox.h), nothing queued */
bool team_link_say(unsigned side, uint16_t message);
/* From a hook: side's partner's keys as the last packet of his side
 * brought them (none in a message's frame or on a Custom screen), what the
 * battle loop's key copy gives him */
uint16_t team_link_keys(unsigned side);
/* how the words went, every core's: said in the output */
void team_link_report(void);

#endif
