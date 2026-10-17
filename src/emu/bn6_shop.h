/* A shop's screen in BN6 Cybeast Gregar (USA), part of bn6.h's addresses
 * (it includes this): the state the screen keeps (bn6f ShopControl's), the
 * list it shows and what it shows it from, and BN6's own sorts, the folder
 * editor's and the Chip Trader's, which the Order Service's list takes up
 * (issue #124). Names follow the bn6f disassembly; see docs/ROM_DATA.md. */
#ifndef CW_BN6_SHOP_H
#define CW_BN6_SHOP_H

#define BN6_SHOP_STATE        0x02011C10u /* a shop's screen (bn6f ShopControl's state): +0 its course (BN6_SHOP_AT_*), */
#define BN6_SHOP_AT_LIST      0x08   /* ... the list taking input (bn6f sub_8047950, Gregar 0x08047920) */
#define BN6_SHOP_AT_AGAIN     0x1C   /* ... a line said: once it holds, the list again, its keeper's script 1 run (bn6f sub_8047028) */
#define BN6_SHOP_REDRAW       (BN6_SHOP_STATE + 0x0D) /* ... nonzero: the list made again from its entries and drawn, next frame (bn6f sub_8047134) */
#define BN6_SHOP_COUNT        (BN6_SHOP_STATE + 0x12) /* ... the entries the list shows, u16 */
#define BN6_SHOP_ROW          (BN6_SHOP_STATE + 0x14) /* ... the cursor's row on the screen, u16 */
#define BN6_SHOP_SCROLL       (BN6_SHOP_STATE + 0x18) /* ... the list's scroll, u16: the entry under the cursor is their sum */
#define BN6_SHOP_DESC         (BN6_SHOP_STATE + 0x1C) /* ... its shop's entry in BN6_SHOP_DESCS, u32: +0 the currency, +8 its stock's offset in the shop data, +12 its entries */
#define BN6_SHOP_ARCHIVE      (BN6_SHOP_STATE + 0x24) /* ... its keeper's text archive, u32, unpacked: the screen's own scripts run from it */
#define BN6_SHOP_ENTRY        8      /* a stock entry: +0 its kind (BN6_SHOP_KIND_*), +1 the stock, +2 its id u16, +4 a chip's code or a program's colour, +6 the price u16 */
#define BN6_SHOP_LIST         0x02019A00u /* the screen's list of entries as shown (bn6f sub_8047B88): sorted, the hidden left out */
#define BN6_SHOP_SOURCE       0x0201AE00u /* the Order Service's entries the Library holds, by ID, as the screen opens (bn6f sub_8048DF8): BN6_SHOP_LIST copied from them at each BN6_SHOP_REDRAW, the first empty one their end */
#define BN6_SHOP_KIND_ITEM    1      /* BN6_SHOP_ENTRY's kind: a key item (HPMemory 0x70) */
#define BN6_SHOP_KIND_CHIP    2      /* ... a chip in its code */
#define BN6_SHOP_KIND_PROGRAM 3      /* ... a NaviCust program, its id program * 4 */

/* BN6's sorts (bn6f sub_8120018, Gregar 0x08121DF4): seven keys, ID,
 * ABCDE, Code, Attack, Element, No., MB, each up or down (its ties by ID
 * up either way); a key newly picked goes the way its byte here says, XOR
 * BN6_SORT_DOWN (bn6f sub_804AE04, the Chip Trader's SORT window) */
#define BN6_SORT_MODES        0x030069A0u /* IWRAM: a byte a key, 0 or BN6_SORT_DOWN */
#define BN6_SORT_DOWN         0x04
#define BN6_SORT_ELEMENTS     0x08121D3Cu /* a chip's place in the Element sort, a byte by its ChipElement (bn6f byte_811FF60), BN6_SORT_ELEMENT_COUNT */
#define BN6_SORT_ELEMENT_COUNT 12
#define BN6_SORT_NO_POWER     5      /* the Attack sort's place for a chip whose power is not shown (bn6f sub_811FF80) */
#define BN6_SORT_POWER_ODD    1      /* ... and for one of effect flag BN6_SORT_ODD_FLAG */
#define BN6_SORT_ODD_FLAG     0x10
#define BN6_SORT_COMPUTED     999    /* ... a shown power past this one BN6 computes, a Navi chip's: */
/* a Navi chip's power (BN6_NAVI_POWER_FIRST and up), by MegaMan's best
 * time against the Navi (bn6f sub_8010C76, Gregar's as Falzar's) */
#define BN6_NAVI_POWER_FIRST  1001
#define BN6_NAVI_TIMES        0x020018C0u /* u16 frames a Navi, by power - BN6_NAVI_POWER_FIRST */
#define BN6_NAVI_TIME_STEPS   0x08010CB8u /* BN6_NAVI_TIME_STEP_COUNT u32 times in BCD (0x1000 10 seconds): the first the time is not past, its step */
#define BN6_NAVI_TIME_STEP_COUNT 10
#define BN6_NAVI_POWERS       0x08020E54u /* u16 by step, BN6_NAVI_POWER_STEPS a Navi */
#define BN6_NAVI_POWER_STEPS  11     /* ... a step a time and one past the last */

#endif
