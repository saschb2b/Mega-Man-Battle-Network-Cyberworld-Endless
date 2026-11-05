/* BN4 Red Sun's virus families' recipes as tools/xrecipe_bn4.py makes them
 * from BN4's and BN6's ROMs (docs/multirom/families.md; docs/rom/bn4.md,
 * Virus families): xvirus_bn4.c, written by the generator, never by hand.
 * Each is whole (no decision open; the open ones stay in the generator's
 * report) and numbered past BN5's families (its slot from 32: AI 64, ids
 * 0x2C0 on), installed wherever BN4's ROM is read, as the nets' visitors
 * (docs/multirom/bn4.md). BN6's types copied for them (the thunder) are in the same
 * file, their install before them (xvirus_bn4_copy.h). */
#ifndef CW_XVIRUS_BN4_H
#define CW_XVIRUS_BN4_H

#include "bn4.h"
#include "xvirus.h"
#include "xvirus_bn4_copy.h"

extern const XVirusFamily *const xvirus_bn4[];
extern const int xvirus_bn4_count;

#endif
