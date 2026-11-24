/* MegaMan's colours in BN6 Gregar (bn6.h includes it): the palettes his
 * plain body is drawn with on the net's field, in battle, in a chat box and
 * in the emotion window, which of their indexes are one shade ramp, the code
 * that picks a palette, and BN6's sprite renderer's OBJ palette slots, shared
 * by content each frame (issue #854's step 1; docs/rom/megaman-colours.md).
 * Addresses only: no colour of BN6's is kept here. */
#ifndef CW_BN6_LOOK_H
#define CW_BN6_LOOK_H

/* A sprite's palettes (gfx.c's sprite layout): a frame's +4 is the offset,
 * from the sprite's data past its 4-byte header, of a size word, then
 * palettes of BN6_LOOK_PAL_SIZE bytes; the renderer draws palette (sprite
 * +4) + (sprite +5) of them. His three sprites are stored raw (no LZ77), so
 * the renderer reads their palettes from the ROM copy every frame. */
#define BN6_LOOK_PAL_SIZE     0x20        /* a palette: 16 BGR555 colours, index 0 clear */
/* In battle: sprite list 0's entry 0, his body in no form and in every
 * Cross (BN6_FORM_SPRITES gives a Cross 0; the Cross's colours are its own
 * palettes in this one block) */
#define BN6_LOOK_BATTLE_WORD  0x08031CECu /* list 0's word for entry 0 */
#define BN6_LOOK_BATTLE_SPRITE 0x081D8000u /* ... the sprite it names (raw) */
#define BN6_LOOK_BATTLE_PALS  0x081DE160u /* ... its palette 0, then 42 more (43 in its block, BN6_LOOK_BATTLE_PAL_COUNT) */
#define BN6_LOOK_BATTLE_PAL_COUNT 43
#define BN6_LOOK_PAL_PLAIN    0           /* his own colours (0x0801002C in no form, no mood, no tint) */
#define BN6_LOOK_PAL_PALE     1           /* palette 0 paled: AI data +0x48 bit 0x200 (its setter not traced) */
#define BN6_LOOK_PAL_SYNCHRO  4           /* Full Synchro (NaviStats +0x0E 0xFF), every index but 0 and 1 its own */
#define BN6_LOOK_PAL_TINT_BASE 0x12       /* NaviStats +0x10 = n (1-4): palette 0x12 + 5n (+1 pale, +4 Full Synchro); */
#define BN6_LOOK_PAL_TINT_STEP 5          /* ... four tints of his whole body; no writer of that byte found */
#define BN6_LOOK_TINT         0x10        /* NaviStats +0x10, the tint's number (0 none) */
#define BN6_LOOK_PALETTE_PICK 0x0801002Cu /* r5 his object: the palette his body takes this frame, by form (BN6_FORM_PALETTES), */
                                          /* ... Full Synchro, the tint and the pale bit; 14 callers draw MegaMan with it */
/* The battle sprite's ramps (palette 0; every Cross's palette changes 2-9
 * alone, keeps 10-15) */
#define BN6_LOOK_OUTLINE      1           /* the outline, every art's */
#define BN6_LOOK_ARMOUR_FIRST 2           /* light blue: helmet, arms, legs, a four-shade ramp 2-5 */
#define BN6_LOOK_ARMOUR_LAST  5           /* ... the armour ramp's darkest */
#define BN6_LOOK_SUIT_FIRST   6           /* dark blue: the bodysuit, 6-9 (6 the same colour as 2, its highlight) */
#define BN6_LOOK_SUIT_LAST    9           /* ... the suit ramp's darkest */
#define BN6_LOOK_SHINE        10          /* white glints */
#define BN6_LOOK_GOLD         11          /* the helmet's fins and the emblem's ring */
#define BN6_LOOK_EMBLEM       12          /* the emblem's red; 13-15 skin */
/* On the net's field and in the real world's maps: sprite list 6's 0x37
 * (PUPPET_MEGAMAN, the Square's peers too) */
#define BN6_LOOK_FIELD_WORD   0x08032270u /* list 6's word for entry 0x37 */
#define BN6_LOOK_FIELD_SPRITE 0x0842FD60u /* ... the sprite it names (raw) */
#define BN6_LOOK_FIELD_PALS   0x08437C34u /* ... its palette 0, then 4 more (BN6_LOOK_FIELD_PAL_COUNT), a cutscene's */
#define BN6_LOOK_FIELD_PAL_COUNT 5
#define BN6_LOOK_FIELD_BLUE_FIRST 2       /* one seven-shade blue ramp, 2-8: 2-5 the armour's, 6-8 the suit's, */
#define BN6_LOOK_FIELD_BLUE_LAST 8        /* ... 9 white, 10 gold, 11 the emblem's red, 12-15 skin */
#define BN6_LOOK_FIELD_SUIT_FIRST 6       /* ... 2-5 the armour's shades, 6-8 the suit's */
#define BN6_LOOK_FIELD_GOLD   10          /* the field's gold, the helmet's stripe and fins: the look's accent */
#define BN6_LOOK_FIELD_INDEX  0x02000AA5u /* u8 the palette he takes on the field (bn6f eStruct2000aa0 +5, 0 in play) */
#define BN6_LOOK_FIELD_INDEX_SET 0x0809E770u /* ldrb r0,[r7,#5] then sprite_setPalette (BN6_SPRITE_PALETTE_SET), each frame */
/* In a chat box: the mugshot, sprite list 8's 0x37 */
#define BN6_LOOK_MUGSHOT_WORD 0x08032674u /* list 8's word for entry 0x37 */
#define BN6_LOOK_MUGSHOT_SPRITE 0x084ED738u /* ... the sprite it names (raw) */
#define BN6_LOOK_MUGSHOT_PAL  0x084EDD1Cu /* ... its one palette: 1-3 the helmet's blue (1 its outline), 4-7 the green behind */
#define BN6_LOOK_MUGSHOT_BLUE_FIRST 1     /* ... him, 8 red, 11 gold, 9 and 13-15 skin, 12 black: its own layout, */
#define BN6_LOOK_MUGSHOT_BLUE_LAST 3      /* ... not the field's or the battle's */
#define BN6_LOOK_MUGSHOT_GOLD 11          /* the mugshot's gold, the helmet's side pieces: the look's accent */
/* The emotion window (the battle's and the Custom screen's face): face 0's
 * palette in BN6_FACE_PALS, which soullook.c copies and repoints
 * (BN6_FACE_PALS_REF), so it is read through that word */
#define BN6_LOOK_FACE_WINDOW_FIRST 1      /* the window's border and fill, which share the face's palette */
#define BN6_LOOK_FACE_WINDOW_LAST 2       /* ... the window's fill */
#define BN6_LOOK_FACE_BLUE_FIRST 11       /* the face's helmet blue, 11-13 */
#define BN6_LOOK_FACE_BLUE_LAST 13        /* ... the helmet ramp's last */

/* The sprite renderer's OBJ palettes. Each frame every sprite's palette is
 * copied into the work palette's first free slot, or the slot already
 * holding the same 32 bytes (BN6_OBJ_PAL_ALLOC): slots go by content, not by
 * object. Past the mode's limit a sprite is marked (+3 bit 0x10) and keeps
 * the slot it had. The slots from the limit up are fixed: a sprite's own
 * (+0x15's high nibble, 0x08002818 copies its palette there) */
#define BN6_OBJ_PAL_WORK      0x03001550u /* the OBJ work palette, 16 slots of 0x20 (to palette RAM through 0x03001750) */
#define BN6_OBJ_PAL_FADED     0x03001750u /* ... as the screen's fade leaves it, DMA'd to 0x05000200 */
#define BN6_OBJ_PAL_ALLOC     0x03006108u /* IWRAM Thumb: a sprite's palette into a slot (its ROM copy 0x081D6608, */
#define BN6_IWRAM_CODE_ROM    0x081D0500u /* ... as the IWRAM code's: IWRAM 0x03000000 + x is this + x) */
#define BN6_OBJ_PAL_COUNT     0x0200F340u /* u32 the slots given this frame */
#define BN6_OBJ_PAL_LIMIT     0x020098A8u /* u8 the slots the renderer may give, set by the mode: */
#define BN6_OBJ_PAL_MODE_FIELD 0x080027C4u /* ... 12 (the maps, the PET's screens), */
#define BN6_OBJ_PAL_MODE_BATTLE 0x080027D4u /* ... 10 (a battle), */
#define BN6_OBJ_PAL_MODE_EIGHT 0x080027E4u /* ... 8, */
#define BN6_OBJ_PAL_MODE_ALL  0x080027F4u /* ... 16 */
#define BN6_OBJ_PAL_FIXED     0x08002818u /* a sprite's palette into its fixed slot (+0x15 >> 4) */
#define BN6_OBJ_SLOT_EMOTION  12          /* the emotion window's face in battle */
#define BN6_OBJ_SLOT_MUGSHOT  13          /* a chat box's mugshot (bank 0); 14 the chat box, 12 the HP box on the field */

/* A NetBattle's two sides (src/online/arena_looks.c; docs/rom/megaman-colours.md,
 * NetBattles): his battle block's palettes BN6 never picks, each side's look
 * written there in the Arena's cores, and the places BN6 picks his palette */
#define BN6_LOOK_PAL_SPARE    { 0x19, 0x1A, 0x1E, 0x1F, 0x23, 0x24, 0x28, 0x29 } /* zero in the ROM, never drawn (measured) */
#define BN6_LOOK_PAL_SPARES   8           /* ... how many */
#define BN6_LOOK_PICK_END     0x0801009Eu /* adds r0,r4,#0 at the pick's end: r4 the palette (0, 1 or 4 in no form), r5 the object */
#define BN6_LOOK_FORM_READ2   0x080BF516u /* ldr r1,=BN6_FORM_PALETTES: an object drawn as MegaMan (0x080BF4FC), r0 its side's form, */
#define BN6_LOOK_FORM_READ2_SET 0x080BF51Au /* ... its palette set (bl sprite_setPalette, r0 the palette) */
#define BN6_LOOK_FACE_COPY    0x0801CC2Au /* ldr r1,=the work palette's slot: the emotion window's face palette copied (r0 its */
                                          /* ... address, the faces' palettes + 0x20 x r4 the face, r5 the window) */

#endif
