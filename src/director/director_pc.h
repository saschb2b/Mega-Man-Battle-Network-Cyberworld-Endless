/* The between-act PC's choice, before BN6 receives its jack-in key. */
#ifndef CW_DIRECTOR_PC_H
#define CW_DIRECTOR_PC_H

#include <stdbool.h>
#include <stdint.h>

uint32_t director_pc_keys(uint32_t keys);
/* After the completed core frame, before another frame can start. */
bool director_pc_update(void);
void director_pc_draw(void);
void director_pc_reset(void);

#endif
