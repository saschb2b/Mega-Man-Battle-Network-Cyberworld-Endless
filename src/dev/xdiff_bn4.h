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
#include "xvirus.h"   /* (XvMoved: a summon's draw keyed as a moved range) */

/* 0 when its battle reached its end, won or lost; 1 where it did not; 2
 * where BN4's ROM is not beside BN6's or its guest did not boot */
int xdiff_bn4_run(const Spec *s);

/* A random battle's record of the spec's foes (BN4's ids at their panels), MegaMan at mm=, as guest_dev_battle takes BN4's:
 * its length (at most 0x80) */
uint32_t xd4_record(const Spec *s, uint8_t *rec);

/* The spec's BN4 families moved into BN6's engine (xvirus_bn4.h: installed with --dev bn4families) and its Navis' code
 * (xnavi_bn4.h), those its foes (BN4's enemy ids) are versions of, and the families a Navi of them summons, into fams
 * (MAX_FOES at most): how many */
int xd4_families(const Spec *s, const XVirusFamily **fams);
/* The draws of the summons of the BN4 Navis among fams (their shims' in their code rooms, xvirus_shim_summon) keyed as
 * BN4_SPAWN_ENEMY's (BN4_SPAWN_ENEMY_DRAW): into out (max at most), each as a moved range of two bytes, how many */
int xd4_summon_draws(const XVirusFamily *const *fams, int n, XvMoved *out, int max);
/* A BN4 enemy id as BN6's engine numbers it: a moved family's or Navi's version its first_id + the version (the recipe's
 * numbers, Billy's 13-18 BN6's 0x2C0-0x2C5, GutsMan's 132-135 0x384-0x387); any other id as it is */
int xd4_bn6_id(int id);
/* BN4's Navis' sound pool pointed at the spec's foes' (xnavi_bn4_sounds_for), as their battle's first frame points it: before
 * BN6's run's pokes, so a poke of a pool slot's entry holds in the battle whichever Navi was installed last */
void xd4_pool_point(const Spec *s);
/* The spec's folder4 as the guest's first folder, before its battle is asked: the entries, and each chip's byte of the
 * save's copy that BN4's gate reads against its key (BN4_CHIP_GATE); nothing where the spec gives none. And its karma4,
 * MegaMan's karma in his NaviStats, which the battle copies as it begins */
void xd4_folder_put(const Spec *s);
/* The spec's souls4 given to BN4's MegaMan before its battle: each Soul's event flag (BN4_SOUL_FLAGS; Blue Moon's six have none in
 * Red Sun, said and skipped), Double Soul's (BN4_FLAG_DOUBLE_SOUL), and its pseudo-chip's byte (BN4_SOUL_CHIPS + n) written as
 * the game writes it giving the Soul, so BN4_CHIP_GATE lets UNITE's pick through at OK (docs/multirom/bn4-souls.md, How BN4
 * takes a form) */
void xd4_souls_put(const Spec *s);
/* ... and BN4's MegaMan's form (its battle NaviStats' BN4_NAVI_FORM) said as it changes, once a frame where souls4 gave a Soul:
 * a Soul's unison and its turns' end, as the trace's numbers have them */
void xd4_form_watch(void);
/* (calm4) BN4's MegaMan's confusion at the fight's start, which BN4 gives him once an attack DarkChip is chosen (its source not
 * yet traced: docs/game/meta/darkchips.md, BN4's confusion after an attack DarkChip), ended by BN4's own status step: the first
 * time its timer holds more than 1 in a battle, 1, so its countdown (BN4_CONFUSE_STEP) ends it and takes its mark away as it
 * would; (confuse6) BN6's MegaMan given the timer BN4 gives instead, once, the comparator's own test. Before each frame of a
 * battle, `game` 4 or 6 */
void xd4_start_confusion(const Spec *s, int game);
/* BN4's families moved into BN6's engine as installed (--dev bn4families; 0 where they are not), and the k-th: a
 * mutation's call= aims their veneers as BN5's */
int xd4_family_count(void);
const XVirusFamily *xd4_family(int k);

#endif
