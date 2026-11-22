/* What a guardian pays in the run's form (issue #774; docs/game/meta/forms.md):
 * his own power where his game's form is the run's, else his kind's, else
 * one of the form's the run lacks, picked by the layer's seed so that a
 * CONTINUE pays the same. A form's power is a number of its own: in CROSS
 * a Cross's navi (powers.c's 1-10), in SOUL BN5 BN5's Soul (1-12), in
 * SOUL BN4 BN4's (1-12, souls.h). A form's list holds only what stands
 * in BN6's engine this session: a Soul not yet moved is simply absent. */
#ifndef CW_FORMS_H
#define CW_FORMS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* The kinds (#774's table): a BN6 Navi's is his Cross's element, a BN5
 * or BN4 Navi's his Soul's (BN6's ProtoMan's and Colonel's their Souls'
 * in the older games); none for the rest */
/* (in BN5's numbering of its chips' kinds plus one, soulkinds.h's
 * BN5_CHIP_KIND, past it BN4's panel-cracking) */
enum { KIND_NONE, KIND_FIRE, KIND_AQUA, KIND_ELEC, KIND_WOOD, KIND_RECOVERY, KIND_PLUS, KIND_SWORD, KIND_INVISIBLE, KIND_CURSOR,
	KIND_OBSTACLE, KIND_WIND, KIND_BREAK, KIND_CRACK, KINDS };
#define FORM_POWERS 12   /* a form's powers at most (SOUL BN5's and SOUL BN4's twelve; CROSS's ten) */

int forms_kind(int navi);
/* Whether guardian `navi` has a Soul of his own that a run of `form`
 * cannot take (a BN5 or BN4 Navi's, his game's form not the run's): what
 * MegaMan feels in his data, his game's form opened for the runs after */
bool forms_soul_felt(int form, int navi);
/* Power `p`'s name in `form` ("HeatCross", "NapalmSoul"), NULL none */
const char *forms_power_name(int form, int p);
/* ... and its kind (KIND_*) */
int forms_power_kind(int form, int p);
/* Whether a new run may take `form` as far as the ROMs go (the setup's,
 * before any core has its Souls in): CROSS always, a Soul form where its
 * game's ROM is read */
bool forms_offered(int form);
/* The power guardian `navi` pays a run of `form` holding `held` (bit p -
 * 1), at the layer seed `seed`; 0 none (every power held). Only the powers
 * that stand this session count: Gregar's five Crosses always, Falzar's
 * where CROSSSELECT lists them, a Soul where BN6's engine unites it */
int forms_pay(int form, int navi, unsigned held, uint32_t seed);
/* ... and how: his own, his kind's, or one the run lacked */
enum { PAY_NONE, PAY_OWN, PAY_KIND, PAY_LACKED };
int forms_pay_how(int form, int navi, unsigned held, uint32_t seed, int *p);
/* The run's held powers of its form (bit p - 1) as the layer about to be
 * made was made: what its guardian's data is told by (run.cross_layer,
 * souls_layer_*) */
unsigned forms_held_layer(void);

/* (the run's side, powers.c's in a form run, the mix's its own) A run's
 * start: CROSS its Crosses' flags and BeastOut where the run has it, a
 * Soul form SoulT+1's turn where it has it (the BeastOut helper's, past
 * the first Net's Nest) */
void forms_bring(void);
/* After guardian `navi`'s battle won: his pay given (the Cybeast's
 * BeastOut in CROSS, SoulT+1's turn in a Soul form); true where a Soul
 * new to the run came */
bool forms_after_boss(int navi);
/* The Cross guardian `navi`'s data gives, for BN6's "MegaMan got:" box,
 * NULL none (a Soul form's) */
const char *forms_reward_cross(int navi);

/* A guardian `navi` fallen (any run's, the mix's too): his game's Soul
 * form opened for the runs after (profile.forms_open); and every fall the
 * rivals' records keep, as a run begins */
void forms_note_fall(int navi);
void forms_note_falls(void);
/* The run's form in its state, a line each: `runform` its name (`form`
 * is MegaMan's in a battle, battle_state.c), `powers`
 * those it holds ("HeatCross SpoutCross", none), `runsoulturn` SoulT+1's */
void forms_describe(FILE *f);

/* ProtoMan's netbattle won (director_duel.c): in a form run his kind's
 * power (Sword: SlashCross, ProtoSoul, ProtSoul), else one the run lacks,
 * given, and MegaMan's words for it added to `words` (n bytes, '|' apart) */
void forms_rival_pay(char *words, size_t n);

/* (forms_words.c) What MegaMan says at guardian `navi`'s data in a form
 * run: a power taken in another shape, a Soul sensed in a CROSS run, a
 * Soul's first teaching; the Nest's call where `beast`; NULL none */
const char *forms_reward_words(int navi, bool beast);
/* ... Dad's after the Cybeast's fall in a Soul form: SoulT+1's turn */
const char *forms_beast_words(void);
/* ... and after the netbattle won, the power it gave (`p`) */
const char *forms_rival_words(int p);

#endif
