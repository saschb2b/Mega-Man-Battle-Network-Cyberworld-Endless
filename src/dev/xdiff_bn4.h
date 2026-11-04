/* The differential's BN4 view, its first step (docs/multirom/guest-battles.md,
 * BN4 on the guest): xdiff's game=bn4 fights BN4 Red Sun's own battle alone
 * on the guest core (guest.h), from a record made of the spec's foes (BN4's
 * enemy ids, hex, at their panels), mm=, hp= and seed=, and says how it
 * ended: won or lost, and MegaMan's HP left. Its keys are its own: each
 * Custom screen passed by START, then A; the buster tapped while it fights;
 * A through the results. --dev god keeps MegaMan's HP at hp=, --dev onehit
 * every enemy at 1 HP, in the guest's battle as in BN6's. film= keeps its
 * frames, by the guest's frame from the battle's asking, as
 * PREFIX_bn4_fNNNNN.bmp. Its trace, game=bn4:trace and bn4:bn6, is
 * xdiff.c's with xdiff_bn4_trace.h's parts (docs/devtools/differential.md,
 * The BN4 trace); the BN4 families' subject in BN6's engine is to come
 * (docs/multirom/bn4.md). */
#ifndef CW_XDIFF_BN4_H
#define CW_XDIFF_BN4_H

#include "xdiff_bn4_trace.h"   /* (the module's other part: its trace in BN6's units) */
#include "xdiff_spec.h"

/* 0 when its battle reached its end, won or lost; 1 where it did not; 2
 * where BN4's ROM is not beside BN6's or its guest did not boot */
int xdiff_bn4_run(const Spec *s);

/* A random battle's record of the spec's foes (BN4's ids at their panels), MegaMan at mm=, as guest_dev_battle takes BN4's:
 * its length (at most 0x80) */
uint32_t xd4_record(const Spec *s, uint8_t *rec);

#endif
