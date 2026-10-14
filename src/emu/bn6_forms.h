/* MegaMan's forms in BN6's battle engine (bn6.h includes it): the tables
 * the engine reads by his form (BN6_BATTLE_FORM), the places Gregar's code
 * reads the form itself, and a Cross's pending record and used bits; at
 * Gregar's addresses (docs/ROM_DATA.md, MegaMan's forms; issue #82). A
 * table holds BN6_FORMS entries: 0 none, 1-10 the Crosses (Gregar's five,
 * then Falzar's SpoutCross to DustCross, 6-10, which Gregar keeps and never
 * offers), 0xB-0x16 Beast Out (BN6_FORM_BEAST, a Cross's from
 * BN6_FORM_CROSS_BEAST), 0x17-0x18 BeastOver. The Souls' proxy form
 * (src/emu/soulproxy.c) is form 8, Falzar's TenguCross. */
#ifndef CW_BN6_FORMS_H
#define CW_BN6_FORMS_H

#define BN6_FORMS             25          /* the entries of a table by form (0-0x18) */
#define BN6_FORM_HEAT_CROSS   1           /* BN6_BATTLE_FORM: HeatCross (BN6_FLAG_HEAT_CROSS's) */
#define BN6_FORM_PROXY        8           /* BN6_BATTLE_FORM: Falzar's TenguCross, which no Gregar CROSSSELECT offers: the Souls' proxy form */
#define BN6_FORM_NAME_BASE    0x1AB       /* MegaMan's NameID in a form: this + the form (0x08015B22 writes it; 0x1A0 in none), the object id table's entry */
/* The tables by form (each a pointer word's in the code names it, docs/ROM_DATA.md) */
#define BN6_FORM_SPRITES      0x0800FCBCu /* u8 a form: MegaMan's sprite, its number in his category (0x0800FC9E reads it; Crosses 0, his own) */
#define BN6_FORM_OVERLAYS     0x0801127Cu /* u32 a form: the routine that spawns a Cross's overlay as he crosses (0x08011268 calls it) */
#define BN6_FORM_OVERLAYS_END 0x08011398u /* u32 a form: the routine that takes it away as he leaves the form (0x08011384) */
#define BN6_FORM_OVERLAY_NONE 0x08010F68u /* mov pc,lr: BN6_FORM_OVERLAYS' entry 0 (no form), no overlay */
#define BN6_FORM_ELEMENTS     0x080108B8u /* u8 a form: his element (0x0801086C, into his object +0x0E and collision data +2) */
#define BN6_FORM_WEAKNESSES   0x080108D1u /* u8 a form: the second wheel's elements he is weak to (0x0801086C, into collision data +0x18) */
#define BN6_FORM_PASSIVES     0x08014550u /* u32 a form: the routine giving his AI data the form's passive (0x08014536: an Airshoe, a SuperArmor) */
#define BN6_FORM_PASSIVES_COLL 0x080146B8u /* u32 a form: ... his collision data's (0x0801469C) */
#define BN6_FORM_LOADOUTS     0x08020354u /* 6 bytes a form: his AI data +4-+8 and +0x11 (his charged shot's power attack at +3; 0x0800FEEC) */
#define BN6_FORM_LOADOUT_SIZE 6           /* BN6_FORM_LOADOUTS: bytes a form's entry */
#define BN6_FORM_PALETTES     0x080203EAu /* u8 a form: his body's palette (0x0801002C, and four readers more) */
#define BN6_FORM_BUSTER_BONUS 0x080126B0u /* u8 a form: Attack added to his buster's (0x0801265A: HeatCross 1) */
#define BN6_FORM_MUGSHOTS     0x0801E700u /* u8 a form: his face in the emotion window (0x0801E660, 0x0801E6A8) */
#define BN6_OBJECT_IDS        0x080182C4u /* (version, actor type, AI index) per object id, 3 bytes (RomLayout.enemy_ids'): a form's NameID's
                                             (BN6_FORM_NAME_BASE + the form) gives MegaMan's arm, his records and struct1 as he crosses */
/* Gregar's code that reads the form itself, not through a table */
#define BN6_CHIP_BONUS_HEAT   0x0800EF62u /* cmp r0,#1 in the chips' bonuses by form (0x0800EF34), MegaMan's form in r0: HeatCross's +50 to Fire
                                             chips; Falzar's TenguCross's +10 to Wind chips stands further on (cmp r0,#8 at 0x0800EFE6) */
#define BN6_ARM_FLASH_FORM    0x080EC8DEu /* cmp r0,#0 after the routine that spawns MegaMan's arm flash (0x080EC8B2) reads his form into r0: in a
                                             Cross the flash (the first kind's type 5) takes the form as its animation, the Cross's arm */
#define BN6_CROSS_SUMMON      0x08014BB6u /* movs r3,#0x14 before the summon of the Cross's Navi behind MegaMan (0x080BE0B4) as he crosses, the form
                                             in r4: the summon draws the Navi of that number (phase 1 of the crossing, 0x08014B98) */
#define BN6_CROSS_COMMIT      0x08029394u /* movs r1,#0 before CROSSSELECT's commit writes the form chosen (r0: its row + 1) into
                                             BN6_CROSS_RECORD (0x08015952; the routine 0x0802937A, then the row's used bit) */
/* A Cross chosen, in RAM */
#define BN6_CROSS_RECORD      0x0203CED0u /* the Custom screen's record of the form chosen: +0 the form (0xFF none), +1, +3; copied to BN6_CROSS_PENDING */
#define BN6_RECORD_CHAOS      0x01        /* BN6_CROSS_RECORD +1: 0 for a Cross (0x0802937A); BN5's Chaos Unison, which no code of Gregar's reads */
#define BN6_RECORD_TURNS      0x03        /* BN6_CROSS_RECORD +3: a Cross's turns (0x0802A118), which no code of Gregar's reads; BN5's Soul's */
#define BN6_CROSS_PENDING     0x0203A980u /* the form each side takes as its next turn starts, 16 bytes a side, laid out as BN6_CROSS_RECORD */
#define BN6_CROSS_PENDING_SIZE 0x10       /* BN6_CROSS_PENDING: bytes a side's */
#define BN6_CROSSES_USED      0x020349A0u /* u32: bit i as CROSSSELECT's Cross i is chosen (HeatCross bit 0), cleared as a battle begins */

/* BN5's Souls on the proxy form (src/emu/soulunite.c, soullife.c; the Souls
 * track's S2, docs/MULTIROM.md, Transplanted Souls): their state in RAM, the
 * Custom screen's element 0xB they share with Beast Out (BN6's emblem is
 * BN5's UNITE button), and the places Gregar's code is hooked or pointed
 * elsewhere for them */
#define BN6_SOULS             0x0203FF70u /* the Souls' state, BN6_SOULS_SIDE bytes a side (the RAM plan's 0x0203FF70-0x0203FFEF, docs/EMULATION.md):
                                             zeroed with the battle's RAM as each battle is set up (0x080071F2), read and written by our hooks alone */
#define BN6_SOULS_SIZE        0x80        /* BN6_SOULS: its bytes, both sides' and room for what the track's later milestones keep */
#define BN6_SOULS_SIDE        0x20        /* BN6_SOULS: bytes a side's, laid out: */
#define BN6_SOUL_ID           0x00        /* u8 the Soul united, BN5's number (its Navi's AI index: Team Colonel's 7-12), 0 none (BN5's form byte) */
#define BN6_SOUL_CHAOS        0x01        /* u8 1 for a Chaos Unison (BN5's AIData +0x12) */
#define BN6_SOUL_TURNS        0x02        /* u8 its turns left (BN5's AIData +0x0F): one less as each Custom screen opens, 0 ends it as the next turn starts */
#define BN6_SOUL_PHASE        0x03        /* u8 the Chaos Unison's charge phase (BN5's AIData +0x1F; the track's S6, 0 until then) */
#define BN6_SOUL_CHIP         0x04        /* u16 ColonelSoul's chip (BN5's AIData +0x32; S5, 0 until then) */
#define BN6_SOUL_LEVEL        0x06        /* u8 the chaos level (BN5's AIData +0x6C; S6, 0 until then) */
#define BN6_SOUL_OPENED       0x07        /* u8 1 once the battle's opening set his mood (BN6_SOUL_MOOD) as worried as his HP says */
#define BN6_SOUL_NEXT         0x08        /* u8 the Soul the Custom screen committed (OK), taken as form BN6_FORM_PROXY is entered; 0 none */
#define BN6_SOUL_NEXT_CHAOS   0x09        /* u8 ... 1 for its Chaos Unison */
#define BN6_SOUL_NEXT_TURNS   0x0A        /* u8 ... its turns (3, 1 for Chaos Unison: BN5's OK, 0x08024FFA) */
#define BN6_SOUL_SLOT         0x0B        /* u8 what element 0xB is on this Custom screen: BN6_SLOT_EMBLEM or BN6_SLOT_UNITE */
#define BN6_SOUL_STANDS       0x0C        /* u8 what stands under OK on this Custom screen: BN6_STANDS_UNITE, BN6_STANDS_BEAST */
#define BN6_SOUL_SCREEN       0x0D        /* u8 1 once a Custom screen has opened with the Soul united: its end waits for the turn's start after it
                                             (BN6 checks a form's end before the screen too, 0x0800840C; BN5 alone after it, 0x08007D0C) */
#define BN6_SOUL_MOOD         0x0E        /* u8 the side's mood as BN5 keeps it (BN5_BATTLE_MOOD's: 0 dark, 1-0x40 worried, 0xFF Full Synchro;
                                             src/emu/soulmood.c), beside BN6's own (BN6_NAVI_MOOD) */
#define BN6_SOUL_DARK         0x0F        /* u8 the side dark for the battle (BN5's use byte, never BN6's byte 2): BN6_DARK_CHIP once a BN5 DarkChip
                                             made it so, BN6_DARK_METER where it began so (BN5's dark meter under 470: the differential's alone) */
#define BN6_DARK_CHIP         1           /* BN6_SOUL_DARK: a BN5 DarkChip used, its meter as neutral (BN5's palette 3, 0x0800DDEC) */
#define BN6_DARK_METER        2           /* BN6_SOUL_DARK: dark by BN5's meter (its palette 2, 0x0800DDDC) */
#define BN6_SOUL_USED         0x10        /* u32 the Souls united this battle: bit n Soul n, bit 16 + n its Chaos Unison (BN5_SOULS_USED's layout) */
#define BN6_SOUL_NEXT_CHIP    0x14        /* u16 the chip ColonelSoul's button took, committed at OK, 0 none (BN5's Custom record +6, 0xFFFF none) */
#define BN6_SOUL_CYCLE        0x16        /* u8 the Chaos Unison's charge's frame in its phases' cycle (BN5's AIData +0x1C; S6) */
#define BN6_SOUL_REQUEST      0x17        /* u8 1 while its release's request (AIData +0x44's 0x8000 or 0x10000) is the Souls': the attack's end
                                             clears it as BN5's does (0x0800F2D0's 0x1803F) */
#define BN6_SLOT_EMBLEM       0           /* BN6_SOUL_SLOT: BN6's Beast Out emblem, its own rule */
#define BN6_SLOT_UNITE        1           /* BN6_SOUL_SLOT: BN5's UNITE, its rule (the last chip picked of a held Soul's kind) */
#define BN6_STANDS_UNITE      1           /* BN6_SOUL_STANDS: UNITE could stand (Souls held, he neither worried nor dark nor beasted out) */
#define BN6_STANDS_BEAST      2           /* BN6_SOUL_STANDS: Beast Out stands (0x08029FB0) */
/* The Custom screen's elements (BN5's the same, BN5_CUSTOM_UNITE) */
#define BN6_CUSTOM_ELEMENTS   (BN6_CUSTOM_SCREEN + 0x100) /* its 12 elements (0-9 the hand, 0xA OK, 0xB the emblem/UNITE), 12 bytes each (0x08027E1C): */
#define BN6_ELEMENT_SIZE      0x0C        /* ... bytes an element's */
#define BN6_ELEMENT_TYPE      0x00        /* u8 what it is: 0 a chip, BN6_ELEMENT_EMBLEM element 0xB standing; else the template's (none) */
#define BN6_ELEMENT_SLOT      0x04        /* u8 a chip's: bit 0 set on the hand's first where the battle's state +0x17 is (0x08027E4E); UNITE's: the
                                             last pick's element (BN5 0x08024B4C). BN5's check wants a chip's bit 0 and its +5 clear */
#define BN6_ELEMENT_SOUL      0x05        /* u8 UNITE's: the Soul the last pick lights (1-12) */
#define BN6_ELEMENT_CHAOS     0x06        /* u8 UNITE's: 1 for Chaos Unison (the art's palette and the picture's 0xD) */
#define BN6_ELEMENT_STATE     0x07        /* u8 element 0xB's: 0 lit, 1 grey, 2 chosen */
#define BN6_ELEMENT_DECK      0x08        /* u32 a chip's: its entry of BN6_BATTLE_DECK */
#define BN6_ELEMENT_EMBLEM    2           /* BN6_ELEMENT_TYPE: element 0xB standing (BN6's emblem, BN5's UNITE) */
#define BN6_CUSTOM_UNITE      (BN6_CUSTOM_ELEMENTS + 0xB * BN6_ELEMENT_SIZE) /* element 0xB (BN5_CUSTOM_UNITE's twin) */
#define BN6_CUSTOM_NAVI       (BN6_CUSTOM_SCREEN + 0x10) /* u8 the Navi the side fights as, 0 MegaMan (both games' stand rules want 0) */
#define BN6_CUSTOM_EMBLEM_TAKEN (BN6_CUSTOM_SCREEN + 0x17) /* u8 1 while the emblem is chosen (mode 0x48 sets it): CROSSSELECT does not open */
#define BN6_CUSTOM_CROSS_TAKEN (BN6_CUSTOM_SCREEN + 0x19) /* u8 1 once a Cross is chosen on CROSSSELECT: the emblem stays grey (0x08028F6C) */
#define BN6_MODE_UNITE        0x24        /* BN6_CUSTOM_MODE: BN5's UNITE chosen (0x08027044 from the mode table 0x08026AA4), which no Gregar code sets */
#define BN6_BATTLE_PLAYERS    (BN6_BATTLE_STATE + 0xD0) /* each side's player object, a word every BN6_BATTLE_PLAYER_STEP bytes (0x080103BC) */
#define BN6_BATTLE_SIDE_NAVIS (BN6_BATTLE_STATE + 0x80) /* each side's four Navis' objects (0 none), 0x10 bytes a side (BN5's the same: 0x0810436C) */
#define BN6_T1_ANIM           0x10        /* an object's animation (CurAnim, u8: 0x0800F2B6 sets it) */
#define BN6_T1_PARENT         0x4C        /* an object's parent (a charge glow's owner, BN5's the same: 0x080E0E46) */
#define BN6_BATTLE_PLAYER_STEP 0x10       /* ... bytes from one side's to the next */
#define BN6_BATTLE_SETTINGS   (BN6_BATTLE_STATE + 0x3C) /* u32 the battle's BattleSettings; its +3 the mode (0x0802D234), BN6_BATTLE_LINK a link */
#define BN6_SETTINGS_MODE     0x03        /* a BattleSettings' +3: the battle's mode (0x0802D242), BN6_BATTLE_LINK a link */
#define BN6_BATTLE_LINK       1           /* BN6_SETTINGS_MODE: a link battle (where neither game offers UNITE) */
#define BN6_NAVI_MOOD         0x0E        /* a battle NaviStats' +0x0E: its side's mood (BN6_BATTLE_MOOD's), read by BN5's thresholds (BN5_MOOD_WORRIED) */
#define BN6_NAVI_FORM         0x2C        /* a battle NaviStats' +0x2C: its side's form (BN6_BATTLE_FORM's) */
#define BN6_NAVI_SIDE_NAVI    0x29        /* a battle NaviStats' +0x29: the Navi it is, 0 MegaMan (the turns count down for him alone, 0x08015A16) */
#define BN6_AI_ANGER          0x34        /* u16 a player's AI data +0x34: his anger's frames (both games' emotion getters: 3, which UNITE stands in) */
#define BN6_AI_FLAGS          0x44        /* u32 a player's AI data +0x44: his requests (0x40 to leave his form: 0x08015994; 0x4000 to enter one) */
#define BN6_AI_MODES          0x48        /* u32 a player's AI data +0x48: flags (0x08010312 sets them; BN5's 0x200, BN5_AI_UNDER, no code of BN6's) */
#define BN6_SUPPORT_MARK      0x080ED12Au /* movs r0,#1 in the support family (0x080ED0DC, r7 the attack's variables) past the chip's routine, before
                                             its +1 marked: the attack ended at the next frame (BN5's twin 0x080EC400 ends it now, 0x080EC448), */
#define BN6_SUPPORT_END       0x080ED130u /* ... its end (bl 0x08011714, then pop) */
#define BN6_ATTACK_SUB        0x03        /* the attack's variables' +3: its subfamily (the support routine by it: BN6_CHIP_ROUTINES) */
#define BN6_SUPPORT_DARKINVS  0x65        /* BN5's DarkInvs's support routine (its support 0x3B as BN6's 0x2A + 0x3B: xchips_code.h) */
#define BN6_REACTION_START    0x080EBA9Cu /* movs r0,#0x49 in the attack's requests (0x080EBA74, r5 him) past its 0x80000 test: BN6's family 0x49 started
                                             (bl 0x08011680 next), where BN5's twin (0x080EB0F0) starts BN5's of the number */
#define BN6_REACTION_CALL     0x080EBA9Eu /* ... the bl that starts it */
#define BN6_REACTION_BN5      (0x50 + 0x49) /* ... BN5's family 0x49 as BN6's grown table holds it (xattack_grow.h: BN5's family F at 0x50 + F) */
#define BN6_AI_DRAIN          0x36        /* u16 a player's AI data +0x36: while not 0, 1 HP a frame lost (BN6_AI_DRAIN_TEST) and his mood held (0x08014490's
                                             readers); BN5 keeps its chip AI's time there (BN5_AI_UNDER_TIME), which drains nothing */
#define BN6_AI_DRAIN_TEST     0x080144A2u /* ldr r3,[r5,#0x58] in the drain (0x08014498, push {lr}; from 0x0801B13A), past its time stop's test, */
#define BN6_AI_DRAIN_DONE     0x080144B2u /* ... its pop {pc} */
#define BN6_PLAYER_INIT_AI    0x080172F6u /* ldrb r0,[r5,#0x16] in the player's set-up (0x080172F0, his state 0, r5 him), past 0x0800F35C, which lays his
                                             AI data out: before any of his set-up names it */
#define BN6_PLAYER_FRAME_END  0x0801AD10u /* pop {r4,r7,pc}: the player's frame's end (0x0801AC6C, r5 him), where BN5's twin (0x080178E4) calls
                                             0x0800DBD8 last, the time under its chip AI (BN5_AI_UNDER_TIME) counted down */
#define BN6_TOOLKIT_PAUSE     0x3C        /* eToolkit +0x3C: a block whose +0xA (BN6_PAUSED) is not 0 while the battle is paused (bn6f battle_isPaused,
                                             0x0800A03C; BN5's +9) */
#define BN6_PAUSED            0x0A
#define BN6_PLAYER_IDLE_FREE  0x080F1870u /* the player's idle state past the chip request's test (0x080F186E: r0 0xFFFF, none asked), its routine's
                                             frame push {r4,r6,r7,lr}: where BN5's twin (0x080F04CC) runs its chip AI while BN5_AI_UNDER stands */
#define BN6_AI_POWER_CHAOS    0x11        /* u8 a player's AI data +0x11: his loadout's sixth (BN5's Chaos Unison's power attack, 0x0801CA18) */
#define BN6_AI_CHARGE         0x1D        /* u8 his charge: 0 none, 1 charging, 2 full (both games' 0x08012F30 / 0x080105B0) */
#define BN6_AI_CHARGE_BUTTON  0x1E        /* u8 ... its button: 1 A, 2 B (0x08012EEC, 0x08012F02) */
#define BN6_NAVI_TURN_EXTRA   0x32        /* a battle NaviStats' +0x32, signed: turns added to a Cross's three, 1-9 in all (0x0802A118; BN5's OK too) */
/* The hooks (soulunite.c, soullife.c; docs/EMULATION.md, Hooks) */
#define BN6_UNITE_STAND       0x08027EC6u /* tst r0,r0 after Beast Out's test (0x08029FB0, r0) in the Custom screen's set-up: element 0xB stands */
#define BN6_EMBLEM_LIT        0x0802A57Au /* the emblem's lit test (push {r4,lr}; r0 1 lit): the set-up's (0x08027ECC) and the picks' (0x08028F64) */
#define BN6_UNITE_LIT         0x08028F48u /* element 0xB lit or grey after each pick (push {r4,r6,r7,lr}; from 0x08028E3C) */
#define BN6_EMBLEM_A          0x08028D6Cu /* A on element 0xB (push {lr}; r4 the element, r5 the screen): the emblem added as a pick, mode 0x48 */
#define BN6_EMBLEM_B          0x08029072u /* cmp r0,#2 in B's handler: the last pick element 0xB (r4), Beast Out's taken back (0x08029076) */
#define BN6_EMBLEM_B_TAIL     0x080290F2u /* ... its common end: element 0xB's +7 0 (r4), the picks' update (0x08028E32), the window's art, sound 0x83 */
#define BN6_CUSTOM_PICK_ICONS 0x0600A4A0u /* (VRAM) each pick's icon in the column under the chip's window, 0x80 bytes a pick (0x080281D4) */
#define BN6_EMBLEM_GIVEN      0x08029418u /* cmp r0,#1 (a link battle) as the turn's picks leave the deck (0x080293F8): a pick of type 2's +4 names
                                             the chip it took (its deck entry 0xFFFF), in a link battle alone; BN5's in every battle (0x080250A2) */
#define BN6_EMBLEM_COMMIT     0x08029150u /* bl 0x08029344 at OK, Beast Out's commit, for a pick of type 2 (r0 its element), */
#define BN6_EMBLEM_COMMIT_NEXT 0x08029154u /* ... and on to the next pick */
#define BN6_TURN_COUNT        0x08008472u /* bl 0x08015A16 for side 0's player (r0) in the step that opens each Custom screen (0x08008452), right
                                             after its opening call (0x0802D6A0) and before its window slides in: BN5's place (0x08007774) */
#define BN6_TURN_COUNT_1      0x0800847Cu /* ... side 1's */
#define BN6_TURN_END          0x0801488Au /* bl 0x080159C6 (Beast Out's end) for side 0 (r0) in the form check (0x0801486C) with no form pending: run
                                             as a turn's change begins (0x0800840C) and as the turn starts after its Custom screen (0x080084A2) */
#define BN6_TURN_END_1        0x080148B2u /* ... side 1's */
#define BN6_TURN_END_GO       0x08014890u /* ... the end requested (0x08015994: AIData +0x44 bit 0x40, then 0x0801562C, form 0); side 1's 0x080148B8 */
#define BN6_TURN_END_GO_1     0x080148B8u /* ... side 1's */
#define BN6_FORM_BREAK        0x0801B034u /* ldrh r0,[r5,#0x28] in the weakness break: a Cross's NameID (0x1AC-0x1C1) breaks it (0x08015766) */
#define BN6_FORM_BREAK_NOT    0x0801B052u /* ... where none breaks */
/* The art and pictures (literal words, each the only load of its word) */
#define BN6_EMBLEM_ART_TILES  0x0802874Cu /* the word naming the emblem's art for the chip's window (0x08720F70, 0x540 bytes; 0x0802871C) */
#define BN6_EMBLEM_ART_PALS   0x08028750u /* the word naming its palettes (0x08723750, 0x20 bytes each by element 0xB's +6) */
#define BN6_UNITE_PICTURES    0x080270CCu /* mode 0x24's word naming the Souls' pictures, 0x80 bytes by the Soul (0xD Chaos): Gregar's 0x087237D0, its
                                             chips' icons, where BN5's are 0x0874B2BC (BN5_SOUL_PICTURES) */
#define BN6_UNITE_PICTURES_2  0x08028200u /* ... the same word in its routine 0x080281E4, whose only caller is mode 0x24 (0x080271A2) */
#define BN6_UNITE_PICTURE_PAL 0x080270C4u /* mode 0x24's word naming the picture's palette (0x0872AED0, 0x20 bytes) */

/* A Soul's look (soullook.c; the Souls track's S3): where BN6's engine
 * draws MegaMan by his form, hooked for the proxy form with a Soul */
#define BN6_T1_PARAM          0x04        /* its spawn's parameters, r4's bytes (a Soul's image: the Soul; an arm's flash: its kind) */
#define BN6_SPRITES_MEGAMAN   0           /* sprite list 0: MegaMan's (+0 him, 1-12 his Beast Outs), by RomLayout.sprite_lists' first word, */
#define BN6_SPRITES_MEGAMAN_COUNT 14      /* ... its numbers (0x08031CEC on, list 1's after them; 13 the placeholder): code reads list 0 past them, */
#define BN6_SPRITES_THROUGH   0x45        /* ... into lists 1 and 2's tables after it, the battle sprites (a Falzar Navi on the map: list 0's
                                             0x34-0x38, list 2's 6-10), to this: list 3's table at 0x08031E00 */
#define BN6_SPRITES_LIST3     3           /* ... list 3, whose word names list 0's table + 4 x BN6_SPRITES_THROUGH */
#define BN6_ARM_LISTS         0x080188A0u /* a word a list (0-2): its rows of an arm's points, 0x44 bytes a row, two signed bytes a point by */
#define BN6_ARM_ROW_SIZE      0x44        /* ... the animation (0x08018886, through its literal 0x0801889C; 0x08018842 reads a point) */
#define BN6_ARM_ROWS0         0x080188C0u /* list 0's three rows (MegaMan's, two of his Beast Outs'), */
#define BN6_ARM_ROWS0_COUNT   3           /* ... three of them */
#define BN6_ARM_ROW_OF        0x080188B0u /* ... the row of each list-0 number, a byte each (14), named by the literal */
#define BN6_ARM_ROW_OF_REF    0x080188ACu /* ... here, 0x08018886's other read */
#define BN6_FORM_SPRITE_READ  0x0800FCA4u /* ldrb r2,[r0,r1]: the sprite selector's (0x0800FC9E) read of BN6_FORM_SPRITES by the form (r1), MegaMan's, */
#define BN6_FORM_SPRITE_BACK  0x0800FCA6u /* ... on to its movs r1,#0 (list 0) and back */
#define BN6_FORM_PALETTE_READ 0x08010066u /* ldrb r4,[r1,r0]: his palette (0x0801002C) from BN6_FORM_PALETTES by the form (r0), */
#define BN6_FORM_PALETTE_BACK 0x08010068u /* ... on to its b to the palette's set */
#define BN6_FORM_PALETTE_READ2 0x080BF518u /* ldrb r0,[r1,r0]: an object drawn as MegaMan in his form (0x080BF4FC), its palette by the form, */
#define BN6_FORM_PALETTE2_BACK 0x080BF51Au /* ... on to its sprite_setPalette */
#define BN6_CROSSING_PHASES   0x08014A82u /* ldr r1,[pc]: the crossing's phases for forms 1-10 (r0 the pending form; 0x08014A38), */
#define BN6_CROSSING_PHASES_BACK 0x08014A84u /* ... on to their call by the phase (r7's +0) */
#define BN6_CROSSING_PHASES_TABLE 0x08014AB4u /* ... the table: */
#define BN6_CROSSING_PHASE0   0x08014B18u /* phase 0 (MegaMan back on his panel), */
#define BN6_CROSSING_PHASE2   0x08014BEEu /* phase 2 (his sprite loaded by the form, the form written, the overlay spawned, sounds 0x8D, 0x77), */
#define BN6_CROSSING_PHASE3   0x08014CC0u /* phase 3 (the crossing's end); phase 1 (0x08014B98) the Cross's summon (BN6_CROSS_SUMMON) */
#define BN6_CROSSING_SOUND    0x08014C8Eu /* movs r0,#0x8D in phase 2: the sound BN5's unison plays in its phase 1 (0x08012054), */
#define BN6_CROSSING_SOUND_NEXT 0x08014C94u /* ... on to its movs r0,#0x77 */
#define BN6_TOOLKIT_CAMERA    0x0C        /* eToolkit +0x0C: the camera's state, +0x0C its shake's frames and +0x0E its strength, as BN6's */
#define BN6_CAMERA_RENDER_X   0x3C        /* camera's displayed 16.16 world coordinates, after shake (bn6f sub_300631C) */
#define BN6_CAMERA_RENDER_Y   0x40        /* camera's displayed Y coordinate, after shake (bn6f sub_300631C) */
#define BN6_CAMERA_RENDER_Z   0x44        /* camera's displayed Z coordinate, after shake (bn6f sub_300631C) */
#define BN6_CAMERA_SHAKE      0x0C        /* ... camera_initShakeEffect sets them (0x080302A8; BN5's 0x08030E1C the same), the shake each */
#define BN6_CAMERA_SHAKE_STRENGTH 0x0E    /* ... frame drawing two random numbers (BN6 0x080301E8 from the second word, BN5 0x08030D7C from the first) */
#define BN6_BATTLE_FLAG_HOLD  0x20        /* BN6_BATTLE_FLAGS (bn6_panels.h): set through a unison's or crossing's last phase, which clears it (BN6
                                           * 0x08014CEA, BN5 0x080121AE); the camera shakes on through it (0x080301F8) */
#define BN6_SOUL_IMAGE_SPRITE 0x080C1D40u /* movs r2,#0xB in the set-up of BN6's first kind's type 0x2A, BN5's Soul's image (0x080C047C), which
                                             Gregar keeps and never spawns (its spawner 0x080C1EAC no code calls): the number of list 2, */
#define BN6_SOUL_IMAGE_SPRITE_NEXT 0x080C1D42u /* ... on past it */
#define BN6_SOUL_IMAGE_DECO   0x080C1D88u /* bl 0x08010DF6 there: a decoration by BN6's Navi of its +4, */
#define BN6_SOUL_IMAGE_DECO_NEXT 0x080C1D8Cu /* ... past it, */
#define BN6_SOUL_IMAGE_UNDECO 0x080C1E66u /* ... and bl 0x08011044, its taking away, */
#define BN6_SOUL_IMAGE_UNDECO_NEXT 0x080C1E6Au /* ... past it */
#define BN6_FACE_OF_FORM      0x0801E6C2u /* ldrb r6,[r2,r4]: the emotion window's face by the form (r4; 0x0801E6A8) from BN6_FORM_MUGSHOTS, */
#define BN6_FACE_OF_FORM_OUT  0x0801E6EAu /* ... on to its end (r0 the face, r1 the count under it: r6, r3) */
#define BN6_FACE_OF_FORM2     0x0801E666u /* ... the Custom screen's (0x0801E660), */
#define BN6_FACE_OF_FORM2_OUT 0x0801E696u /* ... its end */
#define BN6_FACE_OWNER        0x48        /* the emotion window's +0x48: the object whose face it shows (its +0x16 the side) */
#define BN6_FACES             0x0801CD08u /* the faces' tiles, a word a face (0x100 bytes each to 0x06017680), 23 faces: */
#define BN6_FACES_REF         0x0801CD04u /* ... the one word naming it (0x0801CBC4) */
#define BN6_FACE_PALS         0x0872D050u /* the faces' palettes, 0x20 bytes each by the face, */
#define BN6_FACE_PALS_REF     0x0801CD74u /* ... the one word naming it (0x0801CC26) */
#define BN6_FACE_COUNTER      0x0801D814u /* whether the window counts under the face (Beast Out's turns; HeatCross's flag 0xE0 and no link) */
#define BN6_FACE_DIGITS       0x0801CBFEu /* ldr r1,[pc]: the count's digits (0x0872C8D0, 0x80 bytes by 10 - the count), r4 the face, */
#define BN6_FACE_DIGITS_NEXT  0x0801CC00u /* ... on past it */
#define BN6_FLASH_HEIGHT      0x080BA5A2u /* movs r0,#0xf8 in BN6's arm flash's set-up (the first kind's type 5, 0x080BA549): a Cross's flash */
#define BN6_FLASH_HEIGHT_NEXT 0x080BA5A8u /* ... raised 8 pixels (its owner's NameID 0x1AC-0x1B5, his form not 0), on to its strb r0,[r5,#0xc]; */
                                          /* ... else its kind's own height (r4 + 3) */
#define BN6_ARM_FLASH_SPAWN   0x080EC954u /* mov r7,r8 in MegaMan's arm flash routine (0x080EC8B2), r4 the flash's word made (its kind the low */
#define BN6_ARM_FLASH_CALL    0x080EC956u /* ... byte), before its bl to BN6's spawner of the flash (0x080BA6A0, the first kind's type 5), */
#define BN6_ARM_FLASH_CALL2   0x080EC958u /* ... the bl's second halfword, */
#define BN6_ARM_FLASH_SPAWN_BACK 0x080EC95Au /* ... and past it, the routine's end */
/* BN5's mood in BN6's battles (soulmood.c; S7) */
#define BN6_BATTLE_SCREENS    (BN6_BATTLE_STATE + 0x07) /* u8: the Custom screens opened this battle, one more as each opens (0x08026920) */
#define BN6_COLL_MOOD_LOSS    0x8E        /* u16 the mood a hit takes from the struck (BN5's +0x86: 0x0801A24C, 0x0801701C) */
#define BN6_COLL_MOOD_GAIN    0x90        /* u16 the mood it gives the attacker (BN5's +0x88), BN6_HIT_COUNTER for a counter (BN6 reads it for that alone) */
#define BN6_HIT_COUNTER       0x8000      /* BN6_COLL_MOOD_GAIN: a counter hit (both games' lit 0x8000 at 0x0801A524, 0x08017308) */
#define BN6_MOOD_HIT          0x0801A24Cu /* movs r1,#0x8e in a hit's mood step (0x0801A200: r4 the struck's collision data, r5 the struck), past
                                             BN6's own Full Synchro for a counter: BN5's twin 0x08016FE6 raises the attacker's mood there too */
#define BN6_MOOD_SET          0x08015BECu /* a side's mood set (r0 the side, r1 the mood), BN5's 0x080127CE's twin of another body (no write
                                             in Beast Out and its tiredness, where BN5's none at 0); its write */
#define BN6_MOOD_SET_WRITE    0x08015C0Eu /* ... strb r4,[r0,#0xe] (r6 the side), */
#define BN6_MOOD_SET_PAST     0x08015C10u /* ... past it */
#define BN6_MOOD_CALM         0x080143A6u /* a mood calmed to 0x80 (r5 him: BN5's 0x08011A8C's twin, its anger's end with it); its write */
#define BN6_MOOD_CALM_WRITE   0x080143B0u /* ... strb r1,[r0,#0xe], */
#define BN6_MOOD_CALM_PAST    0x080143B2u /* ... past it */
#define BN6_MOOD_FROM_CHIP    0x0801286Cu /* the set's callers twins of BN5's: the chip's Full Synchro (BN5 0x0800FFAE), */
#define BN6_MOOD_FROM_ANGER   0x08014374u /* ... Anger's start (0x08011A5A), */
#define BN6_MOOD_FROM_SCRIPT  0x08027310u /* ... a Custom screen's script (0x0802359C); */
#define BN6_CALM_FROM_CHIP    0x08012876u /* the calm's: the chip's Anger (0x0800FFB8), */
#define BN6_CALM_FROM_ANGER   0x0801438Eu /* ... Anger's end (0x08011A74) */
#define BN6_COMPACT           0x0802945Au /* the deck's compaction (BN5's 0x080250EA, the same body), its call as a Custom screen opens */
#define BN6_COMPACT_CALL      0x08026954u /* ... (BN5 calls its worried rule just before, 0x08022CFC) */
/* A Soul's chips (soulchips.c; S5) */
#define BN6_COLL_INVULNERABLE 0x28        /* u16 frames left unharmed (0x0800EAFA writes it with BN6_OBJ_INVULNERABLE; BN5's +0x2A, 0x0800CD84) */
#define BN6_CHARGE_TEST       0x08013236u /* the chip charge's test by the form (r0 the chip, r5 MegaMan; from 0x0801336C): push {r4,r6,r7,lr} */
#define BN6_CHARGED_RELEASE   0x0800FBA4u /* cmp r1,#0xd in the release of a charged A (0x0800FB74 on): r1 the A power attack (AI data +5), */
#define BN6_CHARGED_CHIP_USE  0x0800FBECu /* ... the ids that use the chip charged (0x0D, 0x1F, 0x20, 0x29, 0x2D, 5) going on here (movs r0,#1, the chip's use); the rest run the power attack */
#define BN6_CHARGE_BONUS      0x08012C7Cu /* a charged chip's bonus by the form (r0 whether charged): ElecCross's 0x4000, SlashCross's 40, else 0 */
#define BN6_CHIP_MODIFIED     0x0801285Eu /* strh r0,[r7,#8] in the chip's use (0x080127C0): its damage as its modifier (0x08012A38) left it, */
#define BN6_CHIP_MODIFIED_SOUND 0x0801287Au /* ... movs r0,#0x87: the modifier's sound where it did something, then on (0x08012880) */
#define BN6_CHIP_USED_END     0x080128E8u /* ldrh r0,[r7,#0x14] before its last call (0x0800B79A, the DarkChip step) */
#define BN6_CHIP_FAMILY_READ  0x080128EEu /* ldrb r0,[r4,#0xb]: the family it returns, the record's (r4) */
#define BN6_CHIP_FAMILY_NEXT  0x080128F0u /* ... on past it */
#define BN6_SHADOW_TYPE_SIDE  0x08010340u /* movs r1,#0x29 in the shadow type of an actor (0x08010332, BN5's 0x0800E0B6): past its battle mode's */
#define BN6_SHADOW_TYPE_DONE  0x0801034Eu /* ... test, its NaviStats' Navi read next; its return, r0 the type (4 a battle's every actor) */
/* A Soul's abilities (soulpower.c; S4) */
#define BN6_FORM_LOADOUT_READ 0x0800FF3Eu /* ldrb r1,[r0]: the loadout's (0x0800FEEC) row by the form, r0 its address in BN6_FORM_LOADOUTS */
#define BN6_FORM_ELEMENT_READ 0x08010884u /* ldrb r0,[r4,r0]: his element (0x0801086C) from BN6_FORM_ELEMENTS by the form, */
#define BN6_FORM_ELEMENT_NEXT 0x08010892u /* ... on to its bl 0x08019F8C (the element set) */
#define BN6_FORM_WEAKNESS_READ 0x080108A8u /* ldrb r0,[r4,r0]: his weakness from BN6_FORM_WEAKNESSES by the form, */
#define BN6_FORM_WEAKNESS_NEXT 0x080108AAu /* ... on to its bl 0x08019F9E */
#define BN6_FORM_BONUS_READ   0x0801268Cu /* ldrb r1,[r1,r0]: his buster's Attack bonus (0x0801265A) from BN6_FORM_BUSTER_BONUS by the form, */
#define BN6_FORM_BONUS_NEXT   0x0801268Eu /* ... on to its add */
#define BN6_POWER_EMPTY       0x08011A24u /* mov pc,lr: the power attacks' empty routine (ids 5, 0x0D, 0x0E ... name it) */
#define BN6_CHIP_BONUS_DONE   0x0800F090u /* the chips' bonuses by form (0x0800EF34): its end, r6 the bonus, r7 its mark */
/* A Soul's passives (soulpassive.c; S5) */
#define BN6_HAND_SIZE_SET     0x0802A494u /* strb r0,[r5,#6]: the Custom screen's hand size set (0x0802A408) */
/* ColonelSoul's button and chip shot (soulcolonel.c; S5), and the obstacle strike either Colonel's side arms (xstrike.c) */
#define BN6_DUST_ELEMENTS     0x08027F10u /* DustCross's elements 8 and 9 as a Custom screen opens (r4 the elements, r5 the screen, r7 the form;
                                             BN5's 0x08023CF4 for its Souls): push {r4,r6,lr} */
#define BN6_DUST_LIT          0x08028F84u /* DustCross's element 8 lit after a pick (BN5's ColonelSoul's 0x08024C04): push {r4,r6,r7,lr} */
#define BN6_DUST_PHASES       0x08027410u /* ldr r1,[pc,#0x10] in mode 0x38 (0x08027406, r4 element 8): its phases' table, then */
#define BN6_DUST_PHASES_NEXT  0x08027412u /* ... ldrb r0,[r5,#2]: the phase */
#define BN6_OK_END            0x08029222u /* pop {r4,r6,r7,pc}: OK's (0x08029110) end, where BN5's goes on to element 8's commit (0x08024EC4) */
#define BN6_TURN_FORMS        0x0801486Cu /* push {r4,lr}: the turn's form check (r6 and r7 the sides' pending records, +8 the player), BN5's
                                             0x08011DD4, which first gives ColonelSoul his chip (0x080124A6) */
#define BN6_DUST_ART          0x08028342u /* ldr r0,[pc,#0x1c] in element 8's draw on the chips' screen (its type 8's, 0x08028340): DustCross's
                                             button's tiles, 0x180 bytes by its state (BN5's 0x0802415E: ColonelSoul's, lit or grey), then */
#define BN6_DUST_ART_NEXT     0x08028344u /* ... ldr r2: their size */
#define BN6_DUST_INFO_ART     0x08028782u /* ldr r0 in the chip's window's art for elements 8 and 9 (0x0802877C, BN5's 0x080245A4): the
                                             picture, 0x540 bytes, */
#define BN6_DUST_INFO_ART_NEXT 0x08028784u /* ... ldr r1: where it goes */
#define BN6_DUST_INFO_PAL     0x0802878Cu /* ... its palette, */
#define BN6_DUST_INFO_PAL_NEXT 0x0802878Eu /* ... ldr r1: where it goes */
#define BN6_DUST_NAME_PAL     0x080287DEu /* ... and its name line's (0x080287D2, BN5's 0x0802460A) */
#define BN6_DUST_NAME_PAL_NEXT 0x080287E0u /* ... ldr r1: where it goes */
#define BN6_CUSTOM_B          0x08029032u /* push {r4,lr}: B on the chips (mode 4), the last pick given back (BN5's 0x08024CEA gives the
                                             button's chip back first), its end BN6_EMBLEM_B_TAIL */
#define BN6_PICK_ICON         0x080281D4u /* a pick's icon into the picks' column (r0 its chip, r1 its place; BN5's 0x08024004) */
#define BN6_PICK_SLOT         0x08029CD4u /* a pick's slot drawn on the chips' BG (r0 its place; BN5's 0x080258E8) */
#define BN6_OK_SPENT          0x08029458u /* pop {r4,r6,r7,pc}: OK's picks' deck entries spent (0x080293F8), where BN5's spends the button's
                                             too (0x080250CC) */
#define BN6_MODE4_LAST_DRAW   0x08026CFAu /* bl 0x08029D34 in mode 4 (the chips', 0x08026CCC): BN5 draws the loaded chip just before */
#define BN6_MODE4_LAST        0x08029D34u /* ... the call, */
#define BN6_MODE4_BACK        0x08026CFEu /* ... and past it */
#define BN6_MODE24_END        0x08027058u /* pop {pc}: mode 0x24's (UNITE's chosen, 0x08027044) end, BN5's draws the loaded chip before */
#define BN6_OBSTACLE_FLAGS_A  0x0801B3CAu /* bl 0x0801A180 (an object's collision flags) in an obstacle's states (0x0801B394, BN5's 0x08017FCC), */
#define BN6_OBSTACLE_FLAGS_B  0x0801B506u /* ... (BN5's 0x08018138), */
#define BN6_OBSTACLE_FLAGS_C  0x0801B646u /* ... (BN5's 0x080182A0), */
#define BN6_OBSTACLE_FLAGS_D  0x0801B786u /* ... (BN5's 0x080183FC): BN5's block 0x08018002-0x0801802E stands before each */
#define BN6_OBJ_FLAGS         0x0801A180u /* the collision flags of r5's object (BN5's 0x08016C70) */
#define BN6_BATTLE_PAUSED     0x0800A098u /* the battle's pause, its flags' bit 4 (BN5's 0x08008A78), the Z flag clear while paused */
#define BN6_PANEL_FIELD       0x10        /* BN6_PANEL_FLAGS: the field's panels' (every one of its 18, the border's none: the strike's test, BN5's
                                             0x080CABF8, the same bit at BN5_PANEL_FLAGS' place) */
#define BN6_PANEL_ENEMIES     0x04000000u /* BN6_PANEL_FLAGS: an object of side 1 on it (bn6f's "enemy alliance"; BN5's the same, its table 0x080CAC90 by side) */
#define BN6_PANEL_ALLIES      0x08000000u /* ... of side 0 (bn6f's "ally alliance") */

/* Chaos Unison (the Souls track's S6, soulchaos.c): what it stands on of BN6's charge, release, request and glow, by BN5's */
#define BN6_CHAOS_CHARGE      0x08012F62u /* push {r4,r7,lr}: a charge's time by r0 (1 A, 2 B), r5 MegaMan (BN5's 0x08010654, which a chaos B
                                             answers from his AIData +0x11's power by the chaos level) */
#define BN6_CHAOS_DUE         0x08012E8Au /* bl 0x08012EBC: the charge's state each frame (BN5's 0x0801053C), r4 his AI data; BN5's 0x080105F0 */
#define BN6_CHAOS_PHASE       0x08012E8Eu /* ... after it, pop {r4,r6,pc}: the chaos phase's step (BN6 has none) */
#define BN6_CHAOS_RELEASE     0x080131D4u /* bl 0x0800FFE4: the release's request, r0 1 or 2 (a charged B), r4 his AI data (BN5's 0x08010870,
                                             0x8000 or 0x10000 there for a chaos B) */
#define BN6_CHAOS_CONSUME     0x080F180Eu /* bl 0x0800FFFE in the player's requests (0x080F1694, push {r4,r6,r7,lr}) past the charged B's 2, r0 the
                                             requests (BN5's 0x080F0436: 0x8000 and 0x10000 here) */
#define BN6_COUNTER_TEST      0x080EBA84u /* tst r0,r1 in the player's update (0x080EA734), r1 0x8600: BN6's counter requests, its 0x8000 one BN5 has
                                             not (BN5's 0x080EB0D8 tests 0x600), a request BN5's chaos B makes */
#define BN6_COUNTER_TEST_1D   0x080ED7B0u /* ... the same in family 0x1D's (BN5's 0x080ED422), */
#define BN6_COUNTER_TEST_1E   0x080ED90Eu /* ... family 0x1E's (0x080EDDB2), */
#define BN6_COUNTER_TEST_37   0x080EEE32u /* ... family 0x37's (0x080EEAE4) */
#define BN6_CHAOS_END         0x08011744u /* bl 0x0800FFEE in the attack's end (0x08011714), r0 0x1000003F the requests it clears, r6 the
                                             attack's kind (BN5's 0x0800F2D0 clears 0x1803F, its kind 6 his chaos) */
#define BN6_GLOW_SHOW         0x080E21E4u /* beq in the charge glow's update (the fourth kind's 8, r4 its owner): a B charge seen whatever the
                                             side (BN5's 0x080E0ECE: in chaos by the side, 0x080091EA) */
#define BN6_GLOW_SHOW_SIDE    0x080E21E6u /* ... the side's test */
#define BN6_GLOW_PICK         0x080E2272u /* movs r2,#8: the glow's sprite past A's 0x15 (BN5's 0x080E0F94: 0x1C in chaos), */
#define BN6_GLOW_PICK_NEXT    0x080E2274u /* ... on past it */
#define BN6_GLOW_LOAD         0x080E2280u /* ... its bl sprite_load, r1 list 5's offset 0x14, r2 the number */
#define BN6_GLOW_ANIM         0x080E2208u /* cmp r0,#0 past strb r0,[r5,#0x10]: its animation the charge's state (BN5's 0x080E0F0A: the chaos phase
                                             + 2 for a full chaos B), r6 its owner's AI data */
#define BN6_GLOW_SOUND        0x080E22A8u /* beq in its sounds (0x080E229A, r1 its owner): a B charge's whatever the side (BN5's 0x080E0FD4) */
#define BN6_GLOW_SOUND_SIDE   0x080E22AAu /* ... the side's test */
#define BN6_GLOW_SPRITE_CHAOS 0x1C        /* the chaos glow's number its +0x6C keeps (BN5's list 5's) */
#define BN6_CHAOS_FAMILY      0xC7        /* the failure's attack family (BN5's 0x39, 0x080EE724; xattack_grow.h's Souls' range) */

#endif
