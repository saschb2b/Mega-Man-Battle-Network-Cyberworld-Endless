/* The PET in BN6 Cybeast Gregar (USA), part of bn6.h's addresses (it
 * includes this): its menu, the words KeyItem and E-Mail read, and the
 * state its screens share, each screen's fields in it. Names follow the
 * bn6f disassembly; see docs/ROM_DATA.md. */
#ifndef CW_BN6_PET_H
#define CW_BN6_PET_H

/* The PET menu (docs/ROM_DATA.md, the PET): ePETMenuData, +0 its state, +4
 * the cursor (6 Comm, 7 Save), +5 bit 0 open, +0xF the entry the engine's
 * hook on its input took (7 Save); the state table's pointer to its input
 * handler, and that handler; the grey's store to Save's colour */
#define BN6_PET_MENU          0x0200DF20u
#define BN6_PET_CURSOR        (BN6_PET_MENU + 0x04) /* the cursor: 6 Comm, 7 Save */
#define BN6_PET_MENU_OPEN     (BN6_PET_MENU + 0x05) /* bit 0 open */
#define BN6_PET_HOLD          (BN6_PET_MENU + 0x09) /* a count that holds its input off */
#define BN6_PET_MENU_TAKEN    (BN6_PET_MENU + 0x0F) /* the entry the engine's hook on its input took (7 Save) */
#define BN6_KEYS_PRESSED      0x0200A272u           /* eJoypad +2: the keys just pressed (u16, the GBA's bits) */
#define BN6_PET_INPUT_PTR     0x08120B1Cu /* the state table's pointer to its input handler */
#define BN6_PET_INPUT         0x08120B91u /* that handler (Thumb) */
#define BN6_PET_GREY_SAVE     0x08120F26u /* the grey's store to Save's colour */
/* The KeyItem screen's words (docs/ROM_DATA.md, the PET): the names'
 * archive (uncompressed, an entry per id) and the literals that point at
 * it (the KeyItem screen's, the text scripts', the shop's, SubChip's); the
 * descriptions' (LZ77) and the literal the screen runs them through */
#define BN6_KEY_NAMES         0x0873B938u /* the key items' names: an archive, uncompressed, an entry per id */
#define BN6_KEY_NAMES_PTRS    { 0x08042034u, 0x0804761Cu, 0x081265FCu, 0x0812A870u } /* the literals that point at it: the KeyItem screen's, the text scripts', the shop's, SubChip's */
#define BN6_KEY_DESCS         0x0873BD88u /* their descriptions' archive (LZ77) */
#define BN6_KEY_DESC_PTR      0x0812A9C4u /* the literal the screen runs them through */
/* The E-Mail screen's (docs/ROM_DATA.md, the PET): senders (script 2n) and
 * subjects (2n + 1), and bodies (n), both LZ77, each read through a literal
 * holding its unpacked buffer; a row of 4 bytes a mail (its icon, bit 7 Lan's
 * HP, its places in the sorts); the list of mail ids, newest first, and its
 * count; the flags per mail: received, unread, read */
#define BN6_MAIL_TEXT         0x086D10D8u /* the senders (script 2n) and subjects (2n + 1), LZ77 */
#define BN6_MAIL_TEXT_PTR     0x08129E90u /* the literal they are read through */
#define BN6_MAIL_TEXT_BUF     0x02025700u /* their unpacked buffer */
#define BN6_MAIL_BODIES       0x086CE598u /* the bodies (script n), LZ77 */
#define BN6_MAIL_BODY_PTR     0x0812A12Cu /* the literal they are read through */
#define BN6_MAIL_BODY_BUF     0x0201C700u /* their unpacked buffer */
#define BN6_MAIL_TABLE        0x0812A2F8u /* a row of 4 bytes a mail: its icon (bit 7 Lan's HP), its places in the sorts */
#define BN6_MAIL_LIST         0x02006530u /* the mail ids, newest first */
#define BN6_MAIL_COUNT        0x02001140u /* their count */
#define BN6_FLAG_MAIL_GOT     0x1CA0      /* + mail: received */
#define BN6_FLAG_MAIL_NEW     0x1D20      /* + mail: unread */
#define BN6_FLAG_MAIL_READ    0x1DA0      /* + mail: read */

/* The PET's screens: the state they share, and each one's fields in it */
#define BN6_TOOLKIT_SUBMENU   0x34   /* eToolkit SubmenuPtr: the PET screen's state (0x02009A30), its first byte the screen: */
#define BN6_SUBMENU_FOLDERS   0x00   /* ... the ChipFolder list (bn6f HandleChipFolderMenu8123434) */
#define BN6_SUBMENU_SUBCHIP   0x04   /* ... SubChip */
#define BN6_SUBMENU_LIBRARY   0x08   /* ... the Library */
#define BN6_SUBMENU_STATUS    0x0C   /* ... MegaMan's status */
#define BN6_SUBMENU_MAIL      0x10   /* ... E-Mail */
#define BN6_SUBMENU_KEYITEM   0x14   /* ... KeyItem */
#define BN6_SUBMENU_COMM      0x18   /* ... Comm */
#define BN6_SUBMENU_SAVE      0x1C   /* ... Save */
#define BN6_SUBMENU_EDIT      0x20   /* ... the folder editor (bn6f sub_8133200) */
#define BN6_SUBMENU_NAVICUST  0x24   /* ... the NaviCustomizer (bn6f sub_81356D4) */
#define BN6_SUBMENU_STATE     0x01   /* the PET screen's state: its course, 0 while it is set up, then 4 open and 8 closing (the Library's and E-Mail's jump tables) */
#define BN6_EDIT_SIDE         0x03   /* the folder editor's state: the side, BN6_EDIT_PACK the pack's, 0 the folder's */
#define BN6_EDIT_PACK         0x04
#define BN6_EDIT_ROW          0x20   /* ... the cursor's row on the screen (0-6) */
#define BN6_EDIT_SCROLL       0x24   /* ... and the list's scroll: the entry under the cursor is their sum */
#define BN6_EDIT_PACK_ROW     0x2A   /* ... the pack's cursor row and its scroll (its list by chip ID, as BN6 sorts it first) */
#define BN6_EDIT_PACK_SCROLL  0x2E   /* ... the pack list's scroll: its entry under the cursor is row + scroll */
#define BN6_NCMENU_MODE       0x02   /* the NaviCustomizer's state: +2 what its cursor does, BN6_NCMENU_* (bn6f sub_81357C4's jump table) */
#define BN6_NCMENU_LIST       0x04   /* BN6_NCMENU_MODE: on the list of programs */
#define BN6_NCMENU_BOARD      0x08   /* BN6_NCMENU_MODE: on the board, nothing held */
#define BN6_NCMENU_TAKE       0x10   /* BN6_NCMENU_MODE: a program taken from the list, then BN6_NCMENU_HELD */
#define BN6_NCMENU_HELD       0x14   /* BN6_NCMENU_MODE: a program from the list held over the board */
#define BN6_NCMENU_PLACED     0x1C   /* BN6_NCMENU_MODE: a placed program's move or remove */
#define BN6_NCMENU_MOVE       0x20   /* BN6_NCMENU_MODE: a placed program taken up, then BN6_NCMENU_MOVED */
#define BN6_NCMENU_MOVED      0x24   /* BN6_NCMENU_MODE: a placed program held over the board */
#define BN6_NCMENU_ROW        0x20   /* ... the list's cursor row on the screen, u16 */
#define BN6_NCMENU_SCROLL     0x24   /* ... the list's scroll, u16: the entry under the cursor is their sum (bn6f sub_8136218) */
#define BN6_NCMENU_X          0x2A   /* ... the board's cursor, or the program held, on the 7x7 grid (BN6_NAVICUST_GRID): its column, u16 */
#define BN6_NCMENU_Y          0x2E   /* ... its row, u16 */
#define BN6_NCMENU_ENTRIES    0x0201DA80u /* the NaviCustomizer's list, 4 bytes an entry (bn6f word_201DA80): +0 its key item, BN6_PROGRAM_ITEMS + variant, or BN6_NCMENU_RUN; +2 the copies left to place */
#define BN6_NCMENU_RUN        0x14C  /* BN6_NCMENU_ENTRIES' item: RUN, the list's last */
#define BN6_LIBRARY_TAB       0x0D   /* the Library's state (bn6f HandleLibraryMenu8124B3C): the tab shown, 0 StdChip, 1 MegaChip, 2 GigaChip, 3 the other version's Navi chips (Falzar's in Gregar), 4 the P.A. Memo */
#define BN6_LIBRARY_TABS      5      /* the Library's tabs, BN6_LIBRARY_TAB 0-4 */
#define BN6_LIBRARY_COUNT     0x12   /* the Library's state: tab 0's entries, u16; each tab's block of five u16 BN6_LIBRARY_TAB_SIZE after the one before (bn6f sub_8125820) */
#define BN6_LIBRARY_ROW       0x14   /* the Library's state: tab 0's cursor row on the screen (0-6), u16 */
#define BN6_LIBRARY_SCROLL    0x18   /* the Library's state: tab 0's scroll, u16: the entry under the cursor is row + scroll */
#define BN6_LIBRARY_TAB_SIZE  0x0A   /* a tab's block in the Library's state: its count, its row, the row drawn, its scroll, the scroll drawn */
#define BN6_LIBRARY_LISTS     0x0201DC20u /* the Library's lists, tab 0's first (bn6f unk_201DC20), each tab's BN6_LIBRARY_LIST_SIZE after the one before: 4 bytes an entry, its chip at +0 (u16); StdChip's by number, 0 where never seen ("??"), the others the seen alone */
#define BN6_LIBRARY_LIST_SIZE 0x800
#define BN6_LIBRARY_PA        0x140  /* the first id of the P.A. Memo's entries, the Program Advances' records; the chips' tabs list ids 1-0x13F (bn6f sub_81258F8) */
#define BN6_MAIL_SHOWN        0x1E   /* E-Mail's state (bn6f HandleEmailMenu81279F8, a PET screen's or BN6_MODE_MAIL's): the mails its list holds, u16 */
#define BN6_MAIL_ROW          0x20   /* E-Mail's state: the cursor's row on the screen (0-3), u16 */
#define BN6_MAIL_SCROLL       0x24   /* E-Mail's state: the list's scroll, u16: the entry under the cursor is row + scroll (bn6f sub_8128318) */
#define BN6_MAIL_ENTRIES      0x02027590u /* the mails as E-Mail's list shows them, sorted (bn6f dword_2027590, sub_81283A0): BN6_MAIL_ENTRY bytes an entry, BN6_MAIL_MAX of them */
#define BN6_MAIL_ENTRY        0x20   /* a mail's entry in BN6_MAIL_ENTRIES: +0x10 its sender's script (2n), +0x14 its subject's (2n + 1), +0x1C its number as shown */
#define BN6_MAIL_ID           0x18   /* a mail's entry: its mail, u16 */
#define BN6_MAIL_MAX          128    /* the mails a list holds: BN6_MAIL_LIST's bytes, BN6_MAIL_ENTRIES' entries */

#endif
