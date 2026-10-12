/* Another game's battlefields in BN6's numbers (xfields.c; docs/ROM_DATA.md,
 * Battlefields; issue #82): a BN5 record names its field by BN5's number,
 * and BN5's fields are not BN6's. BN5 lays them out as BN6 does (12 bytes,
 * a nibble a panel, the panel's type), and numbers its panel types as BN6
 * does but for three; the fields BN5's areas' records name are each
 * either one of BN6's own of the same panels, or written, its panels'
 * types as BN6's, over a field of BN6's no battle names, in the core's ROM
 * copy, from the player's BN5 ROM as the core starts. Numbers alone here. */
#ifndef CW_XFIELDS_H
#define CW_XFIELDS_H

/* BN5's fields into the core's copy, once a core (xvirus_install calls it
 * where BN5's ROM is read): each checked first (BN6's own the same panels,
 * or the field written over still BN6's), the log names any that fails. */
void xfields_install(int xrom);
/* BN6's number for field `field` of game `xrom`, as installed: -1 where
 * it is none (not installed, or a panel BN6 has no type for) */
int xfield_bn6(int xrom, int field);
/* BN6's panel type for BN5's panel type `type` (BN6_PANEL_TYPE), -1 for
 * one BN6 has none of (its lava, its sea and its 5: bn5.h) */
int xfield_type(int type);

#endif
