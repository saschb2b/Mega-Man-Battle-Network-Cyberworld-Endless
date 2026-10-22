/* Two MegaMen a side: a partner's outbox (arena_team_link.h), the messages
 * of his Custom screen waiting for his side's GBA's packets, one a packet.
 * It lives in a core's RAM (a side's room of RAM_TEAM_LINK), read and
 * written through `TeamMem` so that the pair's host code, a hook and a
 * test share it. TEAM_OUTBOX holds a whole turn's messages (his Cross, his
 * Soul, ColonelSoul's chip, a Program Advance and five picks: nine) with
 * room to spare; a push past it is refused, never lost unsaid. */
#ifndef CW_TEAM_OUTBOX_H
#define CW_TEAM_OUTBOX_H

#include <stdbool.h>
#include <stdint.h>

#define TEAM_OUTBOX       12     /* messages held at most */
#define TEAM_OUTBOX_HEAD  0x18   /* a room's u8 first entry (the entries u16 from +0) */
#define TEAM_OUTBOX_COUNT 0x19   /* ... and how many */
#define TEAM_OUTBOX_SIZE  0x1A   /* the room's bytes it takes */

typedef struct {
	uint8_t (*read8)(void *context, uint32_t address);
	void (*write8)(void *context, uint32_t address, uint8_t value);
	void *context;
} TeamMem;

/* `value` queued after the others in the outbox at `room`; false where it is full (nothing written) */
bool team_outbox_push(const TeamMem *m, uint32_t room, uint16_t value);
/* the first message taken out into *value; false where there is none */
bool team_outbox_pop(const TeamMem *m, uint32_t room, uint16_t *value);
unsigned team_outbox_count(const TeamMem *m, uint32_t room);

#endif
