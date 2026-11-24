/* STUB (looks-screen's branch, until looks-core's lands): a look written
 * into MegaMan's plain palettes in the core's ROM copy. */
#ifndef CW_LOOK_APPLY_H
#define CW_LOOK_APPLY_H

#include <stdint.h>

#include "looks.h"

enum { LOOK_ART_BATTLE, LOOK_ART_FIELD, LOOK_ART_MUGSHOT, LOOK_ART_FACE, LOOK_ARTS };

void look_apply(const Look *look);
void look_palette_of(const Look *look, int which, uint16_t out16[16]);

#endif
