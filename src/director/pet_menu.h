/* Our own PET start menu (issue #793; docs/game/pet-menu.md): BN6's menu
 * runs under it and draws every piece of it (its buttons, icons, panel,
 * PLACE and EXIT), which the picture lays out again in two columns of six
 * (pet_menu_frame.c); BN6's own list folded down the left and then the
 * right, Forms after MegaMan. Its keys are ours while it takes input: BN6's
 * cursor is set to the entry's own row and BN6's A, B or START is passed
 * on, so each entry opens as BN6 opens it (Library, Options and Save
 * through pet.c's hook), Forms our forms screen, Colours our colours
 * screen (src/looks/). */
#ifndef CW_PET_MENU_H
#define CW_PET_MENU_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* The entries in the folded list's order: the left column down, then the
 * right; new ones go after Forms, before E-Mail, so BN6's counts from
 * ChipFolder hold at both ends */
typedef enum { PM_FOLDER, PM_SUBCHIP, PM_LIBRARY, PM_MEGAMAN, PM_FORMS, PM_LOOKS, PM_MAIL, PM_KEYITEM, PM_OPTIONS, PM_SAVE, PM_ENTRIES } PetEntry;
#define PM_EXIT PM_ENTRIES      /* the cursor on EXIT: the grid's last place, under the right column */
#define PM_ROWS ((PM_ENTRIES + 2) / 2)   /* a column's rows: the entries and EXIT in two full columns */
#define PM_LEFT PM_ROWS         /* the left column's entries, ChipFolder to Forms */

/* Once, on the core's ROM copy: BN6's icon lists grown by a ninth icon,
 * the Status icon (row 8's animation), resting under the screen, so BN6
 * loads its tiles for Forms' icon; false, nothing written, where the ROM
 * is not Gregar's as read (Forms then shows MegaMan's) */
bool pet_menu_install(void);
/* Whether it stands over BN6's menu (open and taking input, outside the Net Square) */
bool pet_menu_up(void);
/* The cursor: an entry, or PM_EXIT */
int pet_menu_cursor(void);
/* The entry's BN6 row (its button, its icon, its BN6_PET_CURSOR), -1 for Forms and Colours */
int pet_menu_bn6_row(int entry);
/* The entry's place, or EXIT's: column 0 or 1, row 0 to PM_ROWS - 1 */
void pet_menu_place(int entry, int *col, int *row);
/* The player's keys while it takes input: moved on ours, the rest BN6's (pet_keys) */
uint32_t pet_menu_keys(uint32_t keys);
/* Whether START was among the keys pet_menu_keys last passed on to the game */
bool pet_menu_start(void);
/* After each frame of the game outside the Net Square, never as one is
 * drawn: what the picture needs of the game read (pet_menu_frame.c) */
void pet_menu_see(void);
/* Whether the menu's picture covers the frame now, every row from
 * PET_MENU_BAND on: the frame need not be copied into the picture whole
 * then, only its band's rows, and what the menu reads of it given by
 * pet_menu_under: the core's frame `v` (240 x 160), each pixel through
 * `shade` with its row (NULL: as it is; the town's hour) */
#define PET_MENU_BAND 16
bool pet_menu_covers(void);
void pet_menu_under(const uint32_t *v, uint32_t (*shade)(uint32_t c, int y));
/* The picture: BN6's own frame of its menu `px` (240 x 160, the core's
 * pixels) laid out again, while the menu takes input (pet_menu_frame.c);
 * drawn from what pet_menu_see read, it reads nothing of the game */
void pet_menu_frame(uint32_t *px);
/* Whether the map's picture was kept for the menu to stand on (BN6's teal where not) */
bool pet_menu_field_kept(void);
/* The state in words: `petmenu at ENTRY` while it is up, and `petfield kept`
 * or `none`, the field behind it; reads only */
void pet_menu_describe(FILE *f);

#endif
