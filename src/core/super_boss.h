/* The super bosses (docs/BOSSES.md, Super bosses): Bass and the Cybeast
 * Gregar, BN6's own battles at places of their own. Who they are, where
 * each waits, the form each comes in, what BN6's records give their
 * battles and what they pay. Numbers and rules only, as pacing.c: the
 * ROM's values reach them through the callers. */
#ifndef CW_SUPER_BOSS_H
#define CW_SUPER_BOSS_H

#include <stdbool.h>
#include <stdint.h>

/* Their navi numbers: their AI index in BN6's enemy table, as every
 * guardian's (guardians.h) */
#define SUPER_BASS    19
#define SUPER_CYBEAST 20

static inline bool super_boss(int navi) { return navi == SUPER_BASS || navi == SUPER_CYBEAST; }

/* Their arena: 7 x 7 panels, an octagon, where a guardian's is 5 x 5 */
#define SUPER_ARENA_SIZE 7

/* BN6's version index (its enemy table's, 0-5) of each form: Bass 1800
 * HP (record 0x88), Bass SP 2700 (0x89), Bass BX 3400 (0x8D); Gregar
 * 2500 (0x84), Gregar SP 4000 (0x85) */
enum { SUPER_BASS_V1 = 0, SUPER_BASS_BX = 1, SUPER_BASS_SP = 3, SUPER_CYBEAST_V1 = 0, SUPER_CYBEAST_SP = 2 };

/* Bass's form by his record in any run: `wins` MegaMan's over him, and
 * whether the Cybeast has fallen in any run (the title's Bass mark): Bass
 * until he is beaten, then Bass SP, and Bass BX once he has fallen twice
 * and the beast once (he took its data) */
int super_bass_form(int wins, bool beast_fallen);
/* The Cybeast's by the Net it ends: the first Net's Gregar, Gregar SP on
 * every Net after (loop, 0 the first) */
int super_cybeast_form(int loop);
/* Super boss `navi`'s form at `depth`: Bass's by his record and the
 * profile's marks, the Cybeast's by the Net */
int super_form(int navi, int depth);

/* What BN6's records give a form's battle: its music (Bass's boss theme
 * 0x16, the Cybeast's final battle theme 0x17), its name's suffix ("",
 * " SP", " BX") and the Giga chip it pays (BN6's chip id: BassAnly,
 * Bass, ColForce; BugRSwrd) */
int super_song(int navi);
const char *super_suffix(int navi, int version);
int super_chip(int navi, int version);

/* The master of the endless net's Nest (a short net's is picked by its
 * setup, run_setup) and of the Secret Area: Bass once the profile's marks
 * hold the Secret Area's S (`marks`, meta.h's MARK_*), else `own`, the
 * area's pool's */
int super_nest_master(void);
int super_secret_master(uint16_t marks, int own);

/* BN6's AI indices no Server's battle may hold (loot.c): the super bosses
 * and the other Cybeast, MegaMan's beast forms, and the places of a Navi
 * the US version cut, drawn as MegaMan (17, 22) */
bool super_reserved_ai(int ai);

#endif
