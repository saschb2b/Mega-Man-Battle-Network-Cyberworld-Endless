/* BN5's virus families' recipes as tools/xrecipe.py makes them from the two
 * ROMs (issue #82; docs/MULTIROM.md, Transplanted families): xvirus_bn5.c,
 * written by the generator, never by hand. Each is whole (no decision
 * open) and numbered by its place in the generator's install order; these
 * are the families installed wherever BN5's ROM is read (xvirus.c), each at
 * numbers of its own (the static model: its AI index from 32, its six ids
 * from 0x200, its home, code, art, sprite numbers, effects, attacks' types
 * and song slots no other's; tests/test_core.c checks they are, and holds
 * the generated CanGard and Handi to the recipes made by hand,
 * xvirus_families.h, field for field). */
#ifndef CW_XVIRUS_BN5_H
#define CW_XVIRUS_BN5_H

#include "xvirus.h"

extern const XVirusFamily *const xvirus_bn5[];
extern const int xvirus_bn5_count;

#endif
