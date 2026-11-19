/* CROSSSELECT's ten Crosses in Gregar (bn6_forms.h includes it; issue
 * #706): Falzar's five's flags beside Gregar's (BN6_FLAG_HEAT_CROSS on,
 * bn6.h), and the list, its count, its rows' art and palettes and R's
 * help, at Gregar's addresses (docs/rom/souls.md, CROSSSELECT;
 * src/emu/crossselect.c). */
#ifndef CW_BN6_CROSS_H
#define CW_BN6_CROSS_H

#define BN6_FLAG_SPOUT_CROSS  0x00E7      /* Falzar's five, the flags its own CROSSSELECT reads (bn6f's table at Falzar 0x08029F4C), which
                                             no code of Gregar's reads (crossselect.c lists them): SpoutCross, then TomahawkCross,
                                             TenguCross, GroundCross, DustCross to 0x00EB (docs/rom/souls.md, CROSSSELECT) */
#define BN6_CROSSES           10          /* CROSSSELECT's Crosses, Gregar's five (BN6_FLAG_HEAT_CROSS on), then Falzar's (BN6_FLAG_SPOUT_CROSS on) */
#define BN6_CROSS_ROWS        5           /* ... the rows it lists at most (its list at the screen's +0x50, their states at +0x55) */

/* CROSSSELECT's list of Crosses (crossselect.c; docs/rom/souls.md, CROSSSELECT), the same code as Falzar's but for its
 * table's flags, the form's base (1, Falzar's 6) and the used bit's (Falzar's + 5) */
#define BN6_CROSS_LIST        0x08029EF6u /* push {r4,r5,r6,lr}: the list (r5 the screen): Cross i of BN6_CROSS_ROWS whose flag is set, its
                                             used bit clear and i + 1 not the form he began in, into the screen's +0x50 on; r0 how many */
#define BN6_CROSS_LIST_STORE  0x08029F28u /* ... ldr r2,=0x50: Cross r4 listed as row r6, */
#define BN6_CROSS_LIST_NEXT   0x08029F38u /* ... adds r4,#1: on to the next Cross */
#define BN6_CROSS_LIST_BOUND  0x08029F3Au /* ... cmp r4,#5: its loop's bound */
#define BN6_CROSS_COUNT       0x08029EC8u /* push {r4,r6,lr}: how many have their flag set and their used bit clear (r0) */
#define BN6_CROSS_COUNT_BOUND 0x08029EEEu /* ... cmp r4,#5 */
#define BN6_CROSS_COUNT_END   0x08029EF2u /* ... adds r0,r6,#0: the count */
#define BN6_CROSS_FLAGS_REF   0x08029F44u /* the one word naming the flags' table (both routines' literal), */
#define BN6_CROSS_FLAGS       0x08029F48u /* ... the table: a word a Cross, BN6_FLAG_HEAT_CROSS on */
#define BN6_CROSS_SCREEN_LIST 0x50        /* the Custom screen's +0x50: the Cross of each row listed, BN6_CROSS_ROWS of them */
#define BN6_CROSS_SCREEN_STATE 0x55       /* ... +0x55: each row's state (1 once chosen) */
#define BN6_CROSS_ART         0x086E5D50u /* the rows' art, 0x240 bytes (9 x 2 tiles) a Cross, its five, then the five grey */
#define BN6_CROSS_ART_SIZE    0x240       /* ... bytes a row's art (9 x 2 tiles of 4 bits a pixel) */
#define BN6_CROSS_ART_SEND    0x08029DB8u /* bl 0x08000AC8 in a row's art (0x08029D94: r6 the row, r7 the Cross, r0 its art, grey past the
                                             cursor's row) sent to VRAM */
#define BN6_CROSS_PALS        0x086E73D0u /* the cursor's row's palette, 0x20 bytes a Cross, its five, then the five with the row chosen */
#define BN6_CROSS_PAL_PICK    0x08027984u /* adds r0,r0,r2 (r0 the Cross, r2 5 where its row is chosen) before 0x08029EAC loads it */
#define BN6_CROSS_PAL_SEND    0x08029EBAu /* bl 0x08000AC8 in 0x08029EAC: the palette (r0) sent to 0x03001AA0 */
#define BN6_CROSS_HELP        0x08028B52u /* bl 0x08040358: R's help on the cursor's row, its Cross's text (r0 the archive 0x086ED458, r1 the
                                             Cross: its scripts 0-4 Gregar's five; Falzar's adds 5 to its own), */
#define BN6_CROSS_HELP_SKIP   0x08028B64u /* ... past it, its sound and mode 0x58 (pop {r4,pc}): R does nothing */

#endif
