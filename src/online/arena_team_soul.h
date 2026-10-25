/* Two MegaMen a side: a partner's UNITE (issue #181; docs/online/arena/two-a-side/unite.md,
 * A partner's UNITE). His own Custom screen offers UNITE and ColonelSoul's
 * button as his lead's does (BN5's installs' hooks on his side's GBA of
 * his copy, his own Souls held: arena_team_screen.h); what his OK
 * committed crosses in his stream after his Cross (the proxy form,
 * TEAM_CROSS), as Soul messages (TEAM_SOUL, TEAM_SOUL_CHIP). Each GBA
 * takes them from his side's packet (arena_team_link.h) into his own
 * Souls block (BN6_SOULS_PARTNERS), the Soul pending as the Custom
 * screen's OK leaves a lead's in his side's: the crossing into the proxy
 * form takes it up for his object (soullife.h, by object), his turns
 * counted and his Soul ended as BN6 does for a lead (arena_team_cross.h
 * calls them for him). */
#ifndef CW_ARENA_TEAM_SOUL_H
#define CW_ARENA_TEAM_SOUL_H

#include <stdbool.h>
#include <stdint.h>

/* From a hook: side's partner's Soul message (TEAM_CROSS_COUNT's index
 * 1, 2 or 3 and its slot) into his own Souls block (`second`: the
 * cable's second GBA, where teamunite=-1 leaves it out, the mutation) */
void team_soul_said(unsigned side, unsigned index, unsigned slot, bool second);
/* From a hook: his turn begun: a Soul said for the last that never
 * crossed dropped */
void team_soul_turn_open(unsigned side);
/* From a hook: his pending Soul's chaos and turns, for his crossing's
 * pending record (0 and 0 where none) */
/* ... the Soul his UNITE committed this turn (0 none), and the Souls he
 * has united this battle (BN6_SOUL_USED's bits) */
uint8_t team_soul_next(unsigned side);
uint32_t team_soul_used(unsigned side);
uint8_t team_soul_chaos(unsigned side);
uint8_t team_soul_turns(unsigned side);
/* how the Souls went, every core's, said in the output */
void team_soul_report(void);

#endif
