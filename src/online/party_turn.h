/* A partner's own turn on his own game, on the shared dive's one field
 * (issue #589; docs/online/shared-dive.md, As built): as the field's Custom
 * screen opens, BN6's own Custom screen on a core of his game alone (the
 * field copied in, his deck as the field dealt it, his record:
 * arena_team_screen.h's team_screen_one), run on his pad; the field's
 * picture shown again once his OK is pressed. Only what it decides crosses,
 * in his seat's stream as the two-a-side's partner's does (TEAM_PICK,
 * TEAM_PA: arena_team_custom.h), one message a frame; the turn's limit
 * near, the picks he has so far. Never part of the field's frames or its
 * hash. */
#ifndef CW_PARTY_TURN_H
#define CW_PARTY_TURN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "party_core.h"

/* the battle's end, or a new one: his screen gone */
void party_turn_reset(void);
/* The word of my stream for this frame, seat 1-3: a message of my turn,
 * else my pad (`bot`: the dev's partybot keys on my screen) while my
 * screen is open none, else my pad's keys. `rom` the field's ROM copy
 * (EMU_ROM_SIZE bytes), for my screen's core */
uint16_t party_turn_word(PartyCore *field, const uint8_t *rom, unsigned seat, uint16_t pad, bool bot);
/* the picture to show while seat's screen is open (NULL: the field's) */
const uint32_t *party_turn_video(unsigned seat);
/* how seat's turns went, said in the output (each seat's own: under the dev's partycore
 * every stood-in partner his own screen on this game) */
void party_turn_report(unsigned seat);
/* a seat's last word as the guess of his next: his keys, never a message of his
 * turn again (each comes once) */
uint16_t party_turn_guess(uint16_t last);

#endif
