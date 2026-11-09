/* Battle Network 5: Team Colonel (USA): its NaviCust, for the seven
 * programs BN6 has none of (docs/rom/navicust.md, BN5's NaviCust programs;
 * docs/multirom/gaps.md, NaviCust programs): the records, the effects'
 * table, what each program sets and what reads it. Split from bn5.h, which
 * includes it. */
#ifndef CW_BN5_NAVICUST_H
#define CW_BN5_NAVICUST_H

#include "bn5.h"   /* (BN5_NAVI_STATS) */

/* the buster's levels, which its Attack, Speed and Charge programs raise */
#define BN5_NAVI_ATTACK   (BN5_NAVI_STATS + 0x01) /* u8 Attack, Speed and Charge: the buster's levels less one (0-4), as BN6's (BN6_NAVI_ATTACK): */
#define BN5_NAVI_SPEED    (BN5_NAVI_STATS + 0x02) /* a shot does Attack + 1 (0x0800F538), a charged one ten times that (0x0800F5B4); Charge picks */
#define BN5_NAVI_CHARGE   (BN5_NAVI_STATS + 0x03) /* the charge's frames, 100 down to 60 (0x08010682, its table 0x0801CA68); its boot state 0, 0, 0 */
#define BN5_NCP_RECORDS   0x0813D628u /* its programs' records, 16 bytes a colour variant (program x 4 + variant), BN6's layout: +0 its category, +1 a plus */
                                      /* ... part, +3 its colour, +4 its bug's kind, +8 and +0xC its shape and compressed shape, 5 x 5 cells (BN6's 7 x 7) */
#define BN5_NCP_RECORD    0x0813EFC4u /* ... a record's address by its number (r0 x 16) */
#define BN5_NCP_NAMES     0x0875A6BCu /* its programs' names, a text archive by program (49 entries) */
#define BN5_NCP_EFFECTS   0x0813FC2Cu /* the placed programs' effects, a routine a program (48): each sets bytes of MegaMan's NaviStats */
#define BN5_NAVI_STAT_SET 0x08010E18u /* a NaviStats byte set: r0 the Navi, r1 its offset, r2 the value */
#define BN5_NAVI_STAT_GET 0x08010E30u /* ... read */
#define BN5_NCP_CHIVALRY  22          /* Chivalry: NaviStats +0x34 = 1, which picks the L button's talk (0x080A1E0E: the archive 0x08756BC4, as Humor's +0x25 picks 0x08755630) */
#define BN5_NCP_AUTORUN   23          /* AutoRun: +0x35 = 1; on the net (the map's group 0x80 or more) MegaMan's update sets flag 0x10D8 each frame (0x080A35FC): he runs unheld */
#define BN5_NCP_HUBBATC   27          /* HubBatc: flag 0x10B1 set, then the routines of SprArmr, Custom1, MegFldr1, FstBarr, Shield, FlotShoe, AirShoes and UnderSht (0x0813FF24) */
#define BN5_NCP_SOULCLEN  29          /* SoulClen: +0x20 = 1; a battle's end then raises the dark meter by 1 or 3 in place of its plain rule (0x08012894, its read 0x08012912) */
#define BN5_NCP_MEGAVIRS  33          /* MegaVirs: +0x37 = 1, GigaVirs (34): +0x38 = 1: the virus tier 1 or 2 (BN5_VIRUS_TIER) */
#define BN5_NCP_GIGAVIRS  34
#define BN5_NCP_SOULT     38          /* SoulT+1, a plus part: +0x32 one more, 6 at most (0x08140038): a Soul's turns are 3 + it, 1 to 9 (0x08024FFA, the battle's copy read signed) */
#define BN5_VIRUS_TIER    0x0801184Cu /* 2 with GigaVirs, 1 with MegaVirs, else 0 */
#define BN5_ENEMY_RAISED  0x0800CF00u /* an enemy id raised as a battle's enemies are set (its one call, 0x0800684A): by the tier, or the game's level where higher */
                                      /* ... (0x08000D50: flags 0x011F, 0x0120); a virus two ids a tier (a version), to its family's last; a Navi one, to its third */

#endif
