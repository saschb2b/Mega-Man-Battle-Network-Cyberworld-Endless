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

/* The spec's BN4 families moved into BN6's engine (xvirus_bn4.h: installed with --dev bn4families) and its Navis' code
 * (xnavi_bn4.h), those its foes (BN4's enemy ids) are versions of, into fams (MAX_FOES at most): how many */
int xd4_families(const Spec *s, const XVirusFamily **fams);
/* A BN4 enemy id as BN6's engine numbers it: a moved family's or Navi's version its first_id + the version (the recipe's
 * numbers, Billy's 13-18 BN6's 0x2C0-0x2C5, GutsMan's 132-135 0x384-0x387); any other id as it is */
int xd4_bn6_id(int id);
/* BN4's families moved into BN6's engine as installed (--dev bn4families; 0 where they are not), and the k-th: a
 * mutation's call= aims their veneers as BN5's */
int xd4_family_count(void);
const XVirusFamily *xd4_family(int k);

#endif
