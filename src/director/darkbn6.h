/* BN6's own DarkChips in its battles (docs/META.md, BN6's own DarkChips;
 * docs/ROM_DATA.md; issue #70): the records of the four its battles play
 * made whole in the core's ROM copy, their power honest to the BugFrags
 * held, and the hooks that see a DarkChip's dark power run, which the run's
 * price (darkchips.h) is counted by. */
#ifndef CW_DARKBN6_H
#define CW_DARKBN6_H

#include <stddef.h>

#include "hook.h"

/* The four records in the core's copy (it lasts the session, a state does
 * not hold it: again as each layer is made): the pack lists them (library
 * flag 0x20 cleared, so the folder's EDIT takes them in), they are
 * DarkChips to the folder (effect flag 0x20: three at most, MegaMan's
 * NAVIGATOR line, the purple card), and their icon, picture and palette are
 * their base chip's. */
void darkbn6_records(void);
/* The hooks on the game's chip use (once a core). */
void darkbn6_install(void);
/* As a BN6 battle begins (EV_BATTLE_START): its dark power forgotten, the
 * records' power as the BugFrags held say. */
void darkbn6_battle_begins(void);
/* The four records' power as the BugFrags held say: their own with one or
 * more, their base chip's with none, which is what lands (an honest
 * Custom screen); on the map, where the BugFrags change. */
void darkbn6_sync(void);
/* A hook's event (EV_DARK_RAN, EV_DARK_BASE): noted for the battle; the
 * battle's last BugFrag spent, the rest of it shows the base chips' power. */
void darkbn6_event(const HookEvent *e);
/* ... the battle over: the DarkChips whose dark power ran in it, and those
 * that ran as their base chip for want of a BugFrag, a bit per DarkChipID
 * (0 DrkSword .. 4 DarkPlus); each forgotten as it is taken. */
unsigned darkbn6_ran_take(void);
unsigned darkbn6_base_take(void);
/* The base chip BN6 plays DarkChip `id` (286-290) as without a BugFrag,
 * read from its own routines; 0 where they are not as expected. */
int darkbn6_base(int id);
/* What DarkChip `id`'s dark power does, in MegaMan's words, its numbers
 * the ROM's ("a 400 cut over the six panels in front of us"). */
void darkbn6_does(int id, char *out, size_t n);

#endif
