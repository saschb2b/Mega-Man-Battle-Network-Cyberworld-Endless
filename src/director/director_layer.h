/* Layers made and entered (director_layer.c), for the director's other parts. */
#ifndef CW_DIRECTOR_LAYER_H
#define CW_DIRECTOR_LAYER_H

#include <stdbool.h>

#include "runlog.h"

void begin_area(bool new_act);
void map_label(void);
bool guardian_heard(void);
void set_encounter(const Encounter *e, bool force);
void lock_run(void);
void roll_encounter(void);
bool new_layer(bool leaving);
void forget_heard(void);

#endif
