/* BN6's save to the cartridge (BN6_SAVE_GAME, bn6.h) and the mask it
 * holds the RAM under while it writes (docs/rom/game-state.md, The save's
 * mask; issue #734). */
#ifndef CW_BN6_SAVE_H
#define CW_BN6_SAVE_H

/* The save's key: its low byte XORs each of the BN6_SAVE_MASKED bytes
 * from 0x02000000 in place, a new key each save, while BN6_SAVE_GAME
 * writes and checks the cartridge (frames long), then again to unmask
 * them: the event flags (BN6_EVENT_FLAGS) read then are the key's bits */
#define BN6_SAVE_KEY    0x02001064u
#define BN6_SAVE_MASKED 0x6710u     /* 0x02000000-0x0200670F */

#endif
