/* The run's chips in the game (director_folder.c), for the director's other parts. */
#ifndef CW_DIRECTOR_FOLDER_H
#define CW_DIRECTOR_FOLDER_H

#include <stdbool.h>
#include <stdint.h>

#include "flags.h"

void folder_now(uint16_t folder[BN6_FOLDER_ENTRIES]);
void folder_made_save(void);
void folder_made_load(void);
void note_folder_codes(void);
void own_folder_chips(void);
void own_library_chips(void);
void dev_folder(void);

/* The pack's ids: BN6's pack holds 0-319 (eToolkit +0x4C, 12 bytes an id;
 * past it lie the key items' counts: a BblStar2 bought once read as no
 * change), its extension 320-511 (BN6_PACK_EXT, which the engine points
 * BN6's pack routines at: docs/EMULATION.md, src/emu/xchips_install.c). */
#define PACK_CHIPS 512
/* Chip `id`'s pack entry, a bus address (12 bytes: a count per code of its
 * record's four, then four u16 order stamps), 0 where the pack cannot be
 * read */
uint32_t pack_entry(int id);
/* Whether the pack lists chip `id`: BN6's 1-314, as its list does (0x13A,
 * BN6_PACK_LIST_END), and the registry's of BN5's */
bool pack_listed(int id);

/* Whether the run's folder holds chips of BN5's that sit out for want of
 * its ROM (their placeholders: src/core/chips.h) */
bool folder_old_chips_out(void);
void set_start_folder(void);
void star_records(bool on);
void star_folder_pack(void);
void library_to_game(void);
void library_from_game(void);
void programs_from_game(void);
void bugfrag_trade(void);

#endif
