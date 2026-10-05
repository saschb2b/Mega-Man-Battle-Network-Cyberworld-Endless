/* The NaviCust's board watched (director_board.c), for the director's other parts. */
#ifndef CW_DIRECTOR_BOARD_H
#define CW_DIRECTOR_BOARD_H

#include <stdbool.h>
#include <stdint.h>

extern int no_room_told;
extern uint8_t off_explained[47 * 4 / 8 + 1];
bool bit_of(const uint8_t *set, int v);
void bit_set(uint8_t *set, int v);
void off_board_forget(void);
void off_board_save(void);
void off_board_load(void);
const char *program_off_board(int *variant);
int board_programs(uint8_t *out, int max);
bool fits_beside_as(int v, bool compressed);
bool fits_beside_placed(int v);
void grant_spins(void);
void spins_sync(void);
void item_given(int item);
void spin_watch(void);
void reg_watch(void);
void code_watch(void);
const char *bug_cause(void);
bool fits_free_as(int v, bool compressed);
bool fits_as_it_stands(int v);
void draft_fit_watch(void);
bool in_draft(int v);
void bug_watch(void);

#endif
