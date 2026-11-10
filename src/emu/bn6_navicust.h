/* BN6's NaviCust programs' tables, as a program of another game is added to
 * them (docs/rom/navicust.md, Program 48; src/emu/xncp.c): every table a
 * program's number indexes, the words of BN6's code that name each, and
 * the NaviCust's list's loop. Split from bn6.h; the
 * records' and the codes' own places are RomLayout's (navicust_programs,
 * navicust_codes, navicust_names). bn6_pet.h, which bn6.h includes,
 * includes it. */
#ifndef CW_BN6_NAVICUST_H
#define CW_BN6_NAVICUST_H

#define BN6_NCP_RUN            47          /* the number of the list's RUN item: a record of one colour and no shape, key item BN6_NCMENU_RUN */
#define BN6_NCP_RECORD         16          /* a colour variant's record: +0 category, +1 part, +3 colour, +4 bug's kind, +8 and +0xC its shape and compressed shape */
#define BN6_NCP_RECORD_SHAPE   8           /* ... its shape's pointer, */
#define BN6_NCP_RECORD_SMALL   12          /* ... its compressed shape's */
#define BN6_NCP_RECORDS_OWN    189         /* BN6's records: programs 0-46 four each, then RUN's one; its shapes stand right after */
#define BN6_NCP_SHAPE          49          /* a shape: seven rows of seven cells */
#define BN6_NCP_RECORD_REFS    { 0x0813D56Cu, 0x0813DE44u } /* the words that name the records: the reader by number's (0x0813D560), the give-all's (0x0813DE10) */
#define BN6_NCP_EFFECTS        0x0813E52Cu /* the placed programs' effects, a routine a program: 47 of them, programs 0-46, code right after */
#define BN6_NCP_EFFECTS_OWN    47          /* ... its entries */
#define BN6_NCP_EFFECTS_REF    0x0813E528u /* ... the word that names the table, loaded three times as the board's programs apply (0x0813E464) */
#define BN6_NCP_CODE_ROWS      48          /* the compression codes' rows, ten bytes a program, 0-47 (navicust_codes): another table right after */
#define BN6_NCP_CODE_ROW       10          /* ... a row's bytes */
#define BN6_NCP_CODES_REF      0x0813E178u /* ... the word that names them, in the code input (0x0813E114) */
#define BN6_NCP_DESCS          0x0873CC04u /* the programs' descriptions: LZ77, a word (its length << 8), then an archive of 48 entries, 0-47 */
#define BN6_NCP_DESC_ENTRIES   48          /* ... its entries */
#define BN6_NCP_DESC_FRAMED    46          /* ... an entry in the box's frame: five bytes before its lines, five after (HP+500's) */
#define BN6_NCP_DESCS_SHOP_REF 0x080471F4u /* the words that name them: a shop's list of archives (the address plain), */
#define BN6_NCP_DESCS_MENU_REF 0x08137B18u /* the NaviCust screen's load list (bit 31 set: compressed, to 0x0201E670), */
#define BN6_NCP_DESCS_LINK_REF 0x08130878u /* and the link's program trade's (to 0x02025070), whose list stops before RUN: left BN6's */
#define BN6_NCP_NAME_ENTRIES   75          /* the names' archive's entries (navicust_names): 0-46 the programs, 47 RUN's, 48-63 fillers, one text with entry 64's */
#define BN6_NCP_NAMES_SIZE     0x278u      /* ... its bytes, to the descriptions' block right after it */
#define BN6_NCP_NAME_REFS      { 0x08042048u, 0x0804204Cu, 0x08047718u, 0x081194C4u, 0x0811A00Cu, 0x0811AD64u, 0x0811CAACu, 0x08122088u, 0x0812952Cu, 0x081295ACu, 0x08133314u } /* the words that name it */
                                           /* ... the words that name it: the chat box's table's two, a shop's, the PET's screens' and the NaviCust's own */
#define BN6_NCP_NAME_LABELS    64          /* ... from here the Navi status screen's labels (its list's numbers from 0x40, 0x0812950E) */
#define BN6_NCMENU_LIST_LOOP   0x08137BFCu /* the NaviCust's list's builder (0x08137BF4): its loop's top, r4 the key item, 0x91 on */
#define BN6_NCMENU_LIST_NEXT   0x08137C3Eu /* ... `cmp r4, r0` after `adds r4, #1` and the bound's load: on to BN6_NCMENU_LIST_LOOP while r4 <= BN6_NCMENU_RUN */
#define BN6_NCMENU_LIST_DONE   0x08137C42u /* ... the loop's end */
#define BN6_NCMENU_LIST_BOUND  0x08137CACu /* ... the word that holds its bound, BN6_NCMENU_RUN: this loop's alone */
#define BN6_TOOLKIT_NC_GRID    0x58        /* eToolkit +0x58: the NaviCust's board (BN6_NAVICUST_GRID) */
#define BN6_NCP_PLACED_COLUMN  3           /* a placed program's entry (BN6_NAVICUST_PLACED, 8 bytes): +0 u16 its variant, its column, */
#define BN6_NCP_PLACED_ROW     4           /* ... its row: its shape's seven by seven lands three left of and above them */
#define BN6_NAVI_STAT_SET      0x0801379Eu /* a NaviStats byte set: r0 the Navi (0 MegaMan), r1 its offset, r2 the value (bn6f SetCurPETNaviStatsByte) */
#define BN6_NAVI_STAT_GET      0x080137B6u /* ... read, into r0 */

#endif
