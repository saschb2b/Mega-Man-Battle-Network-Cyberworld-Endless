/* The Navis' recipes as tools/xnavirecipe.py makes them from the two ROMs
 * (issue #82; docs/multirom/navis.md): xnavi_bn5.c, written
 * by the generator, never by hand. The control (BN6's own HeatMan, derived
 * by the same walk) and BN5's Navis whose code is proven against BN5's own
 * (tools/xdiff.py), each at its fixed numbers (xnavi_recipe.h). */
#ifndef CW_XNAVI_BN5_H
#define CW_XNAVI_BN5_H

#include "xnavi_recipe.h"

/* The control (--dev xnavicontrol): BN6's own HeatMan, AI 1, moved through
 * the same steps as another game's Navi, as Navi AI XN_FIRST_AI with ids
 * XN_FIRST_ID on; it must play frame for frame as HeatMan does
 * (tools/xcontrol.py --navi). */
extern const XNavi xnavi_control;
/* BN5's engine objects moved once for every track (xnavi_objects.h,
 * tools/bn5_objects.py): its image object's module, as a family's recipe
 * has code and art (no AI of its own) */
extern const XVirusFamily xnavi_objects_bn5;
/* BN5's Navis, installed at once where its ROM is read, in their order */
extern const XNavi *const xnavi_bn5[];
extern const int xnavi_bn5_count;

#endif
