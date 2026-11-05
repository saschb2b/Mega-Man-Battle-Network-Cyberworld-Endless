/* BN4 Red Sun's Navis' recipes as tools/xnavirecipe_bn4.py makes them
 * from BN4's and BN6's ROMs (docs/multirom/bn4-navis.md, GutsMan built):
 * xnavi_bn4.c, written by the generator, never by hand. Each a Navi of
 * BN6's Navi object at its seat's numbers (xnavi_recipe.h XN_BN4_*), its
 * code, records and art BN4's and its sounds in the pool BN4's Navis share
 * (xnavi_bn4_sounds.h), installed where BN4's ROM is read, after BN4's
 * families (xinstall.c). */
#ifndef CW_XNAVI_BN4_H
#define CW_XNAVI_BN4_H

#include "xnavi_recipe.h"

extern const XNavi *const xnavi_bn4[];
extern const int xnavi_bn4_count;
/* ... and the whole ones held from the install while their spec cannot run (the generator's HELD): compiled, not installed */
extern const XNavi *const xnavi_bn4_held[];
extern const int xnavi_bn4_held_count;

#endif
