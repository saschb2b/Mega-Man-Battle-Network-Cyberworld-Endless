/* Addresses in BN6 Cybeast Gregar (USA), the supported ROM (checked by
 * SHA-1 in rom.c). Names follow the bn6f disassembly; see docs/ROM_DATA.md. */
#ifndef CW_BN6_H
#define CW_BN6_H

/* EWRAM */
#define BN6_CHATBOX           0x02009CD0u /* eChatbox: +0 Visible, +4 script state */
#define BN6_CHATBOX_FLAGS     0x02009F38u /* eFlags2009F38 */
#define BN6_PLAYER            0x02009F40u /* overworld player object: +0x1C X, +0x20 Y (16.16) */
#define BN6_WARP              0x02011BB0u /* Warp2011bb0: the next map's warp data */
#define BN6_CUTSCENE          0x02011C50u /* CutsceneState: +0x1C script pos, +0x40 original pos */

/* ROM code */
#define BN6_ENTER_MAP_ON_WARP 0x08005C05u /* map_triggerEnterMapOnWarp (Thumb) */
#define BN6_OW_HOOK           0x08005A8Cu /* a per-frame overworld routine the engine borrows for a frame */

#endif
