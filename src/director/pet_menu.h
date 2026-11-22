/* Our own PET start menu (issue #793; docs/game/pet-menu.md): BN6's menu
 * runs under it and draws every piece of it (its buttons, icons, panel,
 * PLACE and EXIT), which the picture lays out again in two columns of six
 * (pet_menu_frame.c); BN6's own list folded down the left and then the
 * right, Forms after MegaMan. Its keys are ours while it takes input: BN6's
 * cursor is set to the entry's own row and BN6's A, B or START is passed
 * on, so each entry opens as BN6 opens it (Library, Options and Save
 * through pet.c's hook), Forms our forms screen. */
#ifndef CW_PET_MENU_H
#define CW_PET_MENU_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* The entries in the folded list's order: the left column down, then the
 * right; new ones go after Forms, before E-Mail, so BN6's counts from
 * ChipFolder hold at both ends */
typedef enum { PM_FOLDER, PM_SUBCHIP, PM_LIBRARY, PM_MEGAMAN, PM_FORMS, PM_MAIL, PM_KEYITEM, PM_OPTIONS, PM_SAVE, PM_ENTRIES } PetEntry;
#define PM_EXIT PM_ENTRIES      /* the cursor on EXIT */
#define PM_ROWS 6               /* a column's rows: twelve places */
#define PM_LEFT 5               /* the left column's entries, ChipFolder to Forms */

/* Whether it stands over BN6's menu (open and taking input, outside the Net Square) */
bool pet_menu_up(void);
/* The cursor: an entry, or PM_EXIT */
int pet_menu_cursor(void);
/* The entry's BN6 row (its button, its icon, its BN6_PET_CURSOR), -1 for Forms */
int pet_menu_bn6_row(int entry);
/* The entry's place: column 0 or 1, row 0-5 */
void pet_menu_place(int entry, int *col, int *row);
/* The player's keys while it takes input: moved on ours, the rest BN6's (pet_keys) */
uint32_t pet_menu_keys(uint32_t keys);
/* The picture: BN6's own frame of its menu `px` (240 x 160, the core's
 * pixels) laid out again, while the menu takes input (pet_menu_frame.c) */
void pet_menu_frame(uint32_t *px);
/* The state in words: `petmenu at ENTRY` while it is up; reads only */
void pet_menu_describe(FILE *f);

#endif
