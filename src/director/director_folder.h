/* The run's chips in the game (director_folder.c), for the director's other parts. */
#ifndef CW_DIRECTOR_FOLDER_H
#define CW_DIRECTOR_FOLDER_H

#include <stdbool.h>
#include <stdint.h>

#include "flags.h"

void folder_now(uint16_t folder[BN6_FOLDER_ENTRIES]);
extern uint16_t folder_made[BN6_FOLDER_ENTRIES];

#define FOLDER_MADE_MAGIC 0x43464C44u   /* "CFLD" */

void note_folder_codes(void);
void own_folder_chips(void);
void dev_folder(void);

/* The chips a folder holds, the standard, mega and giga ones, the ones a
 * shop or a trader gives: past them in the pack lies other memory, a count
 * of which a purchase moved the other way (a BblStar2 bought read as no
 * change). */
#define PACK_CHIPS 314   /* (chip_pool.c's LAST_CHIP + 1) */

void set_start_folder(void);
void star_records(bool on);
void star_folder_pack(void);
void library_to_game(void);
void library_from_game(void);
void programs_from_game(void);
void pack_watch(void);
void bugfrag_trade(void);

#endif
