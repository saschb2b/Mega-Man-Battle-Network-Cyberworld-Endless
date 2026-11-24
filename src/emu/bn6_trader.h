/* The Chip Traders in BN6 Cybeast Gregar (USA), part of bn6.h's addresses
 * (it includes this): the machines' tables and trade (src/layer/trader.c),
 * and their screen (bn6f ChipTraderControl) as a Bring request's choice
 * drives it (src/world/bring_choice.c). Names follow the bn6f disassembly;
 * see docs/rom/. */
#ifndef CW_BN6_TRADER_H
#define CW_BN6_TRADER_H

/* Chip Traders (src/layer/trader.c) */
#define BN6_TRADER_TEXT       0x086C3758u /* TextScriptChipTrader: 0 Chip Trader, 6 Special, 12 BugFrag Trader */
#define BN6_TRADER_POOLS      0x0804CDE0u /* 5 x (map key, chip list, rarity weights): prizes by the trader's map */
#define BN6_TRADER_KINDS      0x0809B7E4u /* 6 x (map key, u8 trader script): which lines the trade screen shows */
#define BN6_TRADER_MODES      0x0804BDCCu /* 2 weights: the prize one the Library has (192) or a new one (64) */
/* the BugFrag Trader's trade, which BN6's trader machine makes on the
 * Undernet's map (bn6f sub_809A078; director_folder.c bugfrag_trade) */
/* The Chip Trader's screen (bn6f ChipTraderControl, Gregar 0x0804B0C8), as a Bring request's choice drives it (src/world/bring_choice.c) */
#define BN6_TRADER_LIST       0x0202A3E0u /* the Pack's list it shows (bn6f unk_202A3E0): 0x20 an entry, the item at +0x1C (id << 6 | code, xchips_wide.h) */
#define BN6_TRADER_PUT        0x0202A020u /* the chips put in (bn6f word_202A020), entries as the list's */
#define BN6_TRADER_ENTRY      0x20        /* ... an entry's bytes */
#define BN6_TRADER_ITEM       0x1C        /* ... its item, u16 */
#define BN6_TRADER_LISTED     0x0804B150u /* strh r0,[r5,#0x28]: the list made (bn6f sub_811FE7C), r0 its entries, before it is sorted (bn6f sub_804A360) */
#define BN6_TRADER_TRADE      0x0804B26Eu /* the trade's Yes, faded out (bn6f sub_804A460): bl BN6_TRADER_TAKE's routine, then the prize drawn, given and the game saved */
#define BN6_TRADER_LEFT       0x0804B254u /* ... and the way out with no trade, as B leaves (movs r0,#0, r7 BN6_TRADER_STATE): the amount and prize cleared, the map's
                                             look put back; a trade's own way sets the map's fade for the prize's cutscene, which a screen left with none waits for in black */
#define BN6_TRADER_SAY        0x0804BCD8u /* its line (bn6f sub_804AF10): ldr r0,=BN6_TRADER_TEXT, r1 the script, before bl chatbox_runScript */
#define BN6_TRADER_SAY_RUN    0x0804BCDAu /* ... that bl */
#define BN6_TRADER_ASK_OTHER  8           /* the script it asks with once the chips are put in, for an amount but 3 ("Add these 10?") */
#define BN6_TRADER_RESET      0x0804B0ADu /* bn6f sub_804A2E8: clears the trade's state and the submenu's */
#define BN6_TRADER_PRIZE      0x0804CAC5u /* bn6f sub_804BD00: a prize from the map's pool, r0 its chip, r1 its code */
#define BN6_GIVE_CHIPS        0x08021AEFu /* GiveChips (chip, code, count) */
#define BN6_TAKE_BUGFRAGS     0x0803D09Du /* TakeBugfrags (count) */
#define BN6_SAVE_GAME         0x0803F76Du /* bn6f sub_803F798: the game saved to the cartridge, as after every trade */
#define BN6_FLAG_TRADER_HOWL  0xF6        /* EVENT_F6: the machine's howl, set by the trade's script; the machine clears it */

#endif
