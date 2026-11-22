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

/* Where the save masks and unmasks it, in bn6f sub_803F798 (BN6_SAVE_GAME):
 * the instruction before the mask's call (the RAM still whole), and the one
 * after each unmask's call returns (the RAM whole again: the copy checked,
 * or three copies failed). A sum that differs after the unmask goes back
 * for a new key and masks again (save_mask.h). */
#define BN6_SAVE_MASK_AT      0x0803F7A6u   /* ldr r0, the region's start for the mask */
#define BN6_SAVE_MASK_AT_OP   0x482Du
#define BN6_SAVE_UNMASKED     0x0803F7F0u   /* pop {r0-r2}: the copy checked */
#define BN6_SAVE_GAVE_UP      0x0803F7DAu   /* pop {r0-r2}: three copies failed */
#define BN6_SAVE_UNMASKED_OP  0xBC07u

#endif
