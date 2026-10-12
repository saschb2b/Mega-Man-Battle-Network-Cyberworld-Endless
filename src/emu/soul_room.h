/* The Souls' room in the core's ROM copy (free_space.h's FREE_SOULS to
 * FREE_PANELS, docs/EMULATION.md), laid out by what each part of the
 * Souls' install writes there once a core (issue #82, the Souls track):
 * fixed places, the same at every boot with the same ROMs, so a state's
 * pointers into them stay good; and the numbers the Souls take in BN6's
 * grown tables. tests/test_core.c holds the parts in order and apart. */
#ifndef CW_SOUL_ROOM_H
#define CW_SOUL_ROOM_H

#include "free_space.h"

/* ---- S2: UNITE (soulunite.c), 0x000-0x1000 ---- */
#define SOULS_UNITE        (FREE_SOULS + 0x000000u)   /* soulunite.c's stubs, A on UNITE, its art and the Souls' pictures, to: */
#define SOULS_UNITE_END    (FREE_SOULS + 0x001000u)
/* ---- S3-S5 ---- */
#define SOULS_STUBS        (FREE_SOULS + 0x001000u)   /* the stubs the moved code calls and the engine's C answers (bx lr each, hooked): */
#define SOUL_STUB(k)       (SOULS_STUBS + 4u * (k))
#define SOUL_STUB_PENDING  SOUL_STUB(0)   /* BN5's pending Soul (0x080123FA): r0 the Soul, r1 its chaos, r2 ColonelSoul's chip, r3 its turns */
#define SOUL_STUB_NONE     SOUL_STUB(1)   /* a spawn the engine leaves out: r0 0, no object */
#define SOUL_STUB_FORM     SOUL_STUB(2)   /* BN5's battle NaviStats byte by object (0x08010DEE), r1 0x2C: the Soul's number in the proxy form */
#define SOUL_STUB_FORM_SIDE SOUL_STUB(3)  /* ... by side (0x08010D64) */
#define SOUL_STUBS_COUNT   4              /* (those four soulcode.c's; the next soulpassive.c's:) */
#define SOUL_STUB_READY    SOUL_STUB(4)   /* the proxy's passives as he unites (BN6_FORM_PASSIVES' 8): the Soul's */
#define SOUL_STUB_REAPPLY  SOUL_STUB(5)   /* ... and as his form's state is set again (BN6_FORM_PASSIVES_COLL's 8) */
#define SOUL_STUB_STRIKE   SOUL_STUB(6)   /* (xstrike.c's:) Colonel's side's flag set (BN5's 0x080CAD06), r5 the object: BN6_COLONEL_SIDES */
#define SOUL_STUB_STRIKE_CHECK SOUL_STUB(7) /* ... BN5's test of an obstacle at r0, r1 (0x080CABEA): r0 1 or 2 its strike, r1 the side, r2 as BN5 leaves it */
#define SOULS_PHASES       (FREE_SOULS + 0x001080u)   /* a Soul's unison, 4 words: BN6's crossing's phases with BN5's phase 1 (the Soul's image) */
#define SOULS_STUBS_END    (FREE_SOULS + 0x001100u)
#define SOULS_VENEERS      (FREE_SOULS + 0x001100u)   /* the moved code's veneers on to BN6's routines, 12 bytes each, to: */
#define SOULS_VENEERS_END  (FREE_SOULS + 0x001A00u)
#define SOULS_THUMB        (FREE_SOULS + 0x001A00u)   /* the project's own Thumb the Souls' hooks jump to: soulchips.c's 0x40 bytes, */
#define SOUL_STUB_COUNTER  (SOULS_THUMB + 0x40u)      /* ... BN5's counter's start (0x0800E338) as BN6's twin and the family's first frame (soulcode.c), */
#define SOULS_COLONEL      (SOULS_THUMB + 0x80u)      /* ... ColonelSoul's (soulcolonel.c): his button's mode's two phases, BN5's moved (8 bytes), */
#define SOUL_COLONEL_DRAW  (SOULS_THUMB + 0x90u)      /* ... his loaded chip drawn on the chips' screen (BN5's 0x080254F8, moved) before BN6's last draw, */
#define SOUL_COLONEL_DRAW2 (SOULS_THUMB + 0xC0u)      /* ... the same at the end of UNITE's screen's mode, */
#define SOUL_COLONEL_STRIKE (SOULS_THUMB + 0x100u)    /* ... BN5's obstacle states' block (0x08018002-0x0801802E) where BN6's lack it, */
#define SOUL_COLONEL_UNDO  (SOULS_THUMB + 0x180u)     /* ... and B giving the button's chip back (BN5's 0x08024CEA-0x08024D28), */
#define SOULS_COLONEL_END  (SOULS_THUMB + 0x1C0u)
#define SOULS_THUMB_END    (FREE_SOULS + 0x001C80u)   /* ... to */
#define SOULS_ARM_OF       (FREE_SOULS + 0x001C80u)   /* list 0's arm row by sprite number grown: BN6's bytes, then the Souls', to: */
#define SOULS_ARM_OF_END   (FREE_SOULS + 0x001D80u)
#define SOULS_ARM_ROWS     (FREE_SOULS + 0x001D80u)   /* list 0's arm rows grown: BN6's three, then the Souls' (BN5's points), to: */
#define SOULS_ARM_ROWS_END (FREE_SOULS + 0x002100u)
#define SOULS_POWERS       (FREE_SOULS + 0x002100u)   /* BN6's power attacks grown (bn6_attacks.h): 148 entries, then the Souls', to: */
#define SOULS_POWERS_END   (FREE_SOULS + 0x002500u)
#define SOULS_CHARGES      (FREE_SOULS + 0x002500u)   /* BN6's charge times grown with them, 10 bytes a power attack, to: */
#define SOULS_CHARGES_END  (FREE_SOULS + 0x002C00u)
#define SOULS_LOADOUTS     (FREE_SOULS + 0x002C00u)   /* the Souls' loadouts, 6 bytes each by BN5's number 7-12, in BN6's ids, to: */
#define SOULS_LOADOUTS_END (FREE_SOULS + 0x002C40u)
#define SOULS_FACES        (FREE_SOULS + 0x002C80u)   /* the emotion window's faces grown: BN6's table of tiles' words, then the Souls', to: */
#define SOULS_FACES_END    (FREE_SOULS + 0x002D00u)
#define SOULS_FACE_PALS    (FREE_SOULS + 0x002D00u)   /* ... its palettes, BN6's 0x20 each by face, then the Souls', to: */
#define SOULS_FACE_PALS_END (FREE_SOULS + 0x003100u)
#define SOULS_SONGS        (FREE_SOULS + 0x003100u)   /* kept free (the Souls' sounds go to the slot each sound has, xsong_install_at), to: */
#define SOULS_SONGS_END    (FREE_SOULS + 0x004000u)
#define SOULS_CODE         (FREE_SOULS + 0x004000u)   /* BN5's code the Souls run, moved (soulcode_bn5.c), to: */
#define SOULS_CODE_END     (FREE_SOULS + 0x007000u)
#define SOULS_ART          (FREE_SOULS + 0x007000u)   /* what they draw: the attacks' sprites BN6 has none of (the rest in FREE_SOULS_ART), their faces, */
#define SOULS_ART_END      (FREE_SOULS + 0x016000u)
#define SOULS_SPRITES      (FREE_SOULS + 0x016000u)   /* ... MegaMan in each Soul, BN5's sprite list 0, to: */
#define SOULS_SPRITES_END  FREE_PANELS

/* ---- the numbers the Souls take ---- */
#define SOUL_LIST0_FIRST   0x45          /* BN6's list-0 number of the first Soul's sprite (BN5's 7), past what BN6 reads through list 0, */
#define SOUL_LIST0_ART_FIRST 0x4B        /* ... of the first sprite their code draws that BN6 has none of (tools/xsoul.py's ART_FIRST), */
#define SOULS_LIST0_ENTRIES 0x60         /* ... the list's words (list 0 is the Souls' alone: the tracks' allocations, issue #82) */
#define SOUL_EFFECT_FIRST  0xC0          /* BN6's grown effect ids the Souls' code takes (xvirus_grow.h's 256), to: */
#define SOUL_EFFECT_LAST   0xCF
#define SOUL_POWER_FIRST   0x94          /* BN6's power attack id of the first Soul's (BN6's own 148 to 0x93) */
#define SOUL_FACE_FIRST    0x17          /* BN6's emotion window face of the first Soul's, past BN6's 23 (0x0801CD08) */
#define SOULS_FACE_TILES   (SOULS_ART + 0xE000u)   /* the Souls' faces, 0x100 bytes each (BN5's 0x087417FC on), past the attacks' sprites */
#define SOULS_DIGITS       (SOULS_ART + 0xE600u)   /* ... and BN5's emotion window's digits, 0x80 bytes each (0x08741DFC on), to: */
#define SOULS_DIGITS_END   (SOULS_ART + 0xEB80u)
#define SOULS_LIST0        (SOULS_ART + 0xEC00u)   /* BN6's sprite list 0 grown, SOULS_LIST0_ENTRIES words: BN6's as its code reads */
#define SOULS_LIST0_END    SOULS_ART_END           /* ... them through it to BN6_SPRITES_THROUGH, the Souls' full sprites, what they draw */
/* FREE_SOULS_ART's last 0x1000 bytes: ColonelSoul's button's art, BN5's copied (soulcolonel.c; tools/xsoul.py's sprites stay before it) */
#define SOULS_COLONEL_ART  (FREE_SOULS_ART_END - 0x1000u) /* its tiles on the chips' screen by state: lit, grey, chosen (BN5's lit), 0x180 each */
#define SOULS_COLONEL_INFO (SOULS_COLONEL_ART + 0x480u)   /* ... its picture in the chip's window, 0x540 bytes, */
#define SOULS_COLONEL_INFO_PAL (SOULS_COLONEL_ART + 0x9C0u) /* ... its palette, */
#define SOULS_COLONEL_NAME_PAL (SOULS_COLONEL_ART + 0x9E0u) /* ... and the name line's, 0x20 bytes each, to */
#define SOULS_COLONEL_ART_END (SOULS_COLONEL_ART + 0xA00u)

#endif
