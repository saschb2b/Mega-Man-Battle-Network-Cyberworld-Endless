/* BN4 Red Sun's virus families' recipes as tools/xrecipe_bn4.py makes them
 * from BN4's and BN6's ROMs (docs/multirom/families.md; docs/rom/bn4.md,
 * Virus families): xvirus_bn4.c, written by the generator, never by hand.
 * Each is whole (no decision open; the open ones stay in the generator's
 * report) and numbered past BN5's families (its slot from 32: AI 64, ids
 * 0x2C0 on), installed only where BN4's ROM is read for research
 * (xrom_research). None is whole yet: xvirus_bn4_count is 0. */
#ifndef CW_XVIRUS_BN4_H
#define CW_XVIRUS_BN4_H

#include "bn4.h"
#include "xvirus.h"

extern const XVirusFamily *const xvirus_bn4[];
extern const int xvirus_bn4_count;

#endif
