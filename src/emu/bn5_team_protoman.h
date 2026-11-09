/* BN5's Team ProtoMan moved into BN6's engine (docs/multirom/navis.md, NapalmMan; docs/rom/navis.md, Team ProtoMan's
 * NapalmMan; docs/game/team-protoman.md): what their moved code's proof rests on beyond the generator's own reads, named for
 * tools/bn5_navi_twins.py (COMPANIONS), tools/xnavirecipe.py (CHIP_OBJECT_USES, EFFECT_CALLS) and tools/bn5_twins.py's
 * TYPES. None of it run: the recipes (src/emu/xnavi_bn5.c) copy the code. */
#ifndef CW_BN5_TEAM_PROTOMAN_H
#define CW_BN5_TEAM_PROTOMAN_H

/* NapalmMan (AI 4, ids 0x113-0x116; BN6's Navi AI 29, ids 0x1FC-0x1FF) */
#define BN5_NAPALM_COMPANION     0x0800EE82u /* his companion's start: BN5's overlay spawner with kind 4 (movs r4,#4 at 0x0800EE84),
                                                a kind BN6 lacks: copied with him, the kind his own (XN_KIND_OF, 33) */
#define BN5_NAPALM_COMPANION_END 0x0800F00Eu /* ... and its end, the body of KnightMan's (0x0800F000), copied with him */
#define BN5_NAPALM_HALF_HP_CHIP  0x080F5F22u /* below half his HP (+0x24 under +0x26 / 2), the chip he holds next written for his
                                                attack: BN5's "the chip an object uses" (0x0800D010) its id, a version holding none
                                                skipping the call (0x080F5F2E) */
#define BN5_NAPALM_FLAME_SPAWN   0x080CB4FCu /* his third kind's 0x33 spawns the third kind's 0x00 (movs r4,#0x11 its kind, the
                                                spawner 0x080C4ABC at 0x080CB500): the type the Navis' copy now owns for every track */
#define BN5_NAPALM_PANEL_FLAMES  0x080CC2A2u /* his third kind's 0x38's flames over a pattern of panels: movs r2,#0x4B, handed to
                                                BN5's effects-over-panels routine 0x08018838 (BN6's twin 0x0801BD3C) at 0x080CC2AA */

#endif
