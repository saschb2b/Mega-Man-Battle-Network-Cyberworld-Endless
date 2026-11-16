/* Layers made and entered (director_layer.c), for the director's other parts. */
#ifndef CW_DIRECTOR_LAYER_H
#define CW_DIRECTOR_LAYER_H

#include <stdbool.h>

#include "glitch.h"       /* (the run's Glitch layer, made with the layer: docs/game/glitch-layer.md) */
#include "liberation.h"   /* (a dark layer's board, made with the layer: docs/game/liberation/) */
#include "runlog.h"

void begin_area(bool new_act);
void map_label(void);
/* (extras_describe.c) What the layer holds of the run's extras, in a
 * state's words, for director_describe: the side layer and the ways off
 * the run's own, the Glitch layer, the tournament's bracket, the Virus
 * Battler, DarkChips (docs/devtools/scenarios.md, The extras) */
void extras_describe(FILE *f);
bool guardian_heard(void);
void set_encounter(const Encounter *e, bool force);
void lock_run(void);
void roll_encounter(void);
/* The kind (loot.h's ENC_*) of the layer's random battle `index`: the run's
 * first, the act's opening ones, else a normal one */
int encounter_kind(int index);
bool new_layer(bool leaving);
void forget_heard(void);
/* What the run's layer at `depth` of kind `side` will hold, from its seed
 * (HOLDS_* bits, net.h; FORK_DARK where it is a dark layer's board): a
 * fork's read (docs/world/level-design/fork-read.md); `biome` its area,
 * -1 for the run's own */
#define FORK_DARK (1u << 31)
unsigned fork_holds_in(int depth, int side, int biome);
static inline unsigned fork_holds(int depth, int side) { return fork_holds_in(depth, side, -1); }

#endif
