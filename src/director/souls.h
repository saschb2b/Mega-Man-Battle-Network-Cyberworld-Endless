/* The run's Souls (docs/game/meta/souls.md; issues #69, #82): which of BN5's
 * Team Colonel Navis the run has beaten as a guardian (docs/game/bosses/bn5-navis.md,
 * BN5's Navis) and holds the Soul of, for every battle of the run, the
 * Souls standing in BN6's engine (docs/multirom/souls.md);
 * kept beside the run's checkpoint. A new run holds none. */
#ifndef CW_SOULS_H
#define CW_SOULS_H

#include <stdbool.h>
#include <stdint.h>

#define SOULS 6   /* Colonel, ShadowMan, NumberMan, TomahawkMan, KnightMan, ToadMan: BN5's Souls 7-12 */
#define SOULS_TEAM 6   /* ... and Team ProtoMan's, the other version's: ProtoMan, GyroMan, SearchMan, NapalmMan, MagnetMan, Meddy,
                          BN5's Souls 1-6 (docs/game/team-protoman.md), none held until each is earned from its guardian */
#define SOULS_OF_BN5 12   /* BN5's Souls by its numbers, 1-6 Team ProtoMan's, 7-12 Team Colonel's */
#define SOULS_OF_BN4 12   /* BN4's Souls by its numbers (its forms): 1 RollSoul, 2 GutsSoul, 3 WindSoul, 4 SrchSoul, 5 FireSoul,
                          6 ThunSoul, 7 ProtSoul, 8 NumSoul, 9 MetlSoul, 10 JunkSoul, 11 AquaSoul, 12 WoodSoul (soulbn4.h) */

/* A new run's: none, or the dev flag's (--dev souls=MASK, a bit a Soul) */
void souls_new_run(uint32_t seed);
/* ... where what is held is another run's (a run begun in the net) */
void souls_begin(uint32_t seed);
/* Saved beside the checkpoint; taken back on CONTINUE, for the run whose
 * seed it names. */
void souls_save(void);
void souls_load(uint32_t seed);
/* The Souls held, bit k BN5's Soul 7 + k (soulunite.h's, handed to BN6's
 * engine as each battle begins: encounter.c) */
unsigned souls_held(void);
/* The Souls held as the layer about to be made is made (the director's,
 * with the folder's codes), kept beside the checkpoint: what its finds
 * and stock lean to (loot_souls.h), the same on a CONTINUE's rebuild, a
 * Soul won on the layer counted from the next */
void souls_note_layer(void);
unsigned souls_layer(void);
/* Whether the run holds guardian `navi`'s Soul (guardians.h, guardian_older,
 * or guardian_team: Team ProtoMan's, BN5's Souls 1-6, kept apart from Team
 * Colonel's, so what is handed to BN6's engine and what the finds lean to
 * stay the six's until their Souls are moved), and his given */
bool soul_held(int navi);
void soul_give(int navi);
/* Every Soul the run holds by its game's numbers (issue #774's forms):
 * BN5's, bit k its Soul k + 1 (Team ProtoMan's in bits 0-5, Team Colonel's
 * 6-11), and BN4's, bit k its Soul k + 1; and the same as the layer about
 * to be made was made (souls_note_layer) */
unsigned souls_held_bn5(void);
unsigned souls_held_bn4(void);
unsigned souls_layer_bn5(void);
unsigned souls_layer_bn4(void);
/* (souls.c: a new run holds the Soul its Soul form brought,
 * run.soul_start) */
/* The dev flag's mask, for a new run's first Souls (devtools) */
extern uint8_t souls_dev_mask;

#endif
