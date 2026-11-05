/* A shop's screen in BN6 Cybeast Gregar (USA), part of bn6.h's addresses
 * (it includes this): the state the screen keeps (bn6f ShopControl's), the
 * list it shows and what it shows it from, which the Order Service's sort
 * takes up (issue #124; BN6's sorts in bn6_chips.h). Names follow the bn6f
 * disassembly; see docs/rom/. */
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
#define BN6_SHOP_SOURCE       0x0201AE00u /* an order shop's entries the Library holds, by ID, as the screen opens (bn6f sub_8048DF8): BN6_SHOP_LIST copied from them at each
                                             BN6_SHOP_REDRAW, the first empty one their end; an order lowers the stock of its chip's here (sub_8047B88) and of its kind,
                                             chip and code's in the shop data (sub_8048DAE), never by its place */
#define BN6_SHOP_KIND_ITEM    1      /* BN6_SHOP_ENTRY's kind: a key item (HPMemory 0x70) */
#define BN6_SHOP_KIND_CHIP    2      /* ... a chip in its code */
#define BN6_SHOP_KIND_PROGRAM 3      /* ... a NaviCust program, its id program * 4 */

/* The map left for the shop, which a moved screen's entry follows (xbracket.c), and the engine's routines that screen calls by name */
#define BN6_MAP_TO_SHOP       0x08005642u /* the map's sub-state that leaves it for a shop: its objects freed, the fade waited out (a call
                                           * to 0x080062F8, then beq to its end), the chat box closed, the map left (a call with r0 0x21
                                           * first), BN6_MODE_SHOP written (movs r1,#0x2C; strb r1,[r0]): read, its calls a moved screen's
                                           * entry makes in turn (xbracket.c) */
#define BN6_COPY_WORDS        0x0800093Cu /* r2 bytes copied from r0 to r1 a word at a time (BN4's 0x08000994, the same body) */
#define BN6_HBLANK_DMA_SET    0x080068E8u /* an HBlank DMA set (r0 its source, r1 its register, r2 its count, r3 its control; r4 and r5 a
                                           * line buffer and its bytes, which BN6's own caller 0x080068EC gives as 0x02008E80 and 0x500)
                                           * at 0x020081B0, BN4's 0x080060A4 at 0x02008920 of the first four */
#define BN6_FADE_TEST         0x080062F8u /* Z clear while the screen fades (BN6_FADE_ACTIVE; bn6f IsScreenFadeActive) */
#define BN6_MAP_TO_SHOP_END   0x080056B6u /* ... its end, past its pop {pc} */

/* The map entered again after such a screen (BN6_SUB_ENTER): where EnterMap puts MegaMan, and the random battle its
 * first frame may roll, which the game's flag (BN6_FLAG_NO_ENCOUNTERS, cleared by EnterMap) no longer holds */
#define BN6_ENTER_X           (BN6_GAMESTATE + 0x24) /* PlayerX, PlayerY, PlayerZ (16.16): where EnterMap puts MegaMan (bn6f
                                                       * spawnOWPlayerObjectForEnterMap_809e064), the map entered again after a
                                                       * screen of its own (the bracket moved, xbracket.h) too */
#define BN6_ENTER_Y           (BN6_GAMESTATE + 0x28) /* PlayerY, 16.16 */
#define BN6_ENTER_Z           (BN6_GAMESTATE + 0x2C) /* PlayerZ, 16.16 */
#define BN6_ENTER_FACING      (BN6_GAMESTATE + 0x30) /* FacingDirectionAfterWarp: the eighth he faces there (a u32, its byte read) */
#define BN6_ENCOUNTER_SKIP    0x08005AF3u /* the check's pop {r5,pc} (Thumb): no battle this frame, the roll's counter reset */

#endif
