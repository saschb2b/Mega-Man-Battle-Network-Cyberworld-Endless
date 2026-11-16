/* Layers made and entered (director_layer.c), for the director's other parts. */
#ifndef CW_DIRECTOR_LAYER_H
#define CW_DIRECTOR_LAYER_H

#include <stdbool.h>
#include <stdio.h>

#include "glitch.h"       /* (the run's Glitch layer, made with the layer: docs/game/glitch-layer.md) */
#include "liberation.h"   /* (a dark layer's board, made with the layer: docs/game/liberation/) */
#include "runlog.h"

void begin_area(bool new_act);
void map_label(void);
bool guardian_heard(void);
void set_encounter(const Encounter *e, bool force);
void lock_run(void);
void roll_encounter(void);
/* The kind (loot.h's ENC_*) of the layer's random battle `index`: the run's
 * first, the act's opening ones, else a normal one */
int encounter_kind(int index);
bool new_layer(bool leaving);
void forget_heard(void);
/* What stands on the layer, for a state (director_describe; tests/scenarios
 * reads them): `exitpad` shut or open (a guardian's, a dark layer's), `heard`
 * yes once a Navi has named the act's guardian, and the Mystery Data
 * taken by colour, `greentaken`, `bluetaken`, `purpletaken`, `locks` the
 * locks still standing (a cube, a door, a Link Navi's obstacle) and
 * `pcode` told once a Navi has said a Security Cube's, `gate` a Navi gate's
 * Navi and sealed or earned (his code), `on` what MegaMan stands on (a
 * teleport's pad, teleport1 or teleport2, a set piece's panels, gap, lane
 * or path, floor else); nothing changed */
void layer_describe(FILE *f);
/* What the run's layer at `depth` of kind `side` will hold, from its seed
 * (HOLDS_* bits, net.h; FORK_DARK where it is a dark layer's board): a
 * fork's read (docs/world/level-design/fork-read.md); `biome` its area,
 * -1 for the run's own */
#define FORK_DARK (1u << 31)
unsigned fork_holds_in(int depth, int side, int biome);
static inline unsigned fork_holds(int depth, int side) { return fork_holds_in(depth, side, -1); }

#endif
