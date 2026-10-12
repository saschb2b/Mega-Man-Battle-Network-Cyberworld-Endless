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
#define BN6_SOUL_OPENED       0x07        /* u8 1 once the battle's opening set his mood (BN6_BATTLE_MOOD) as worried as his HP says */
#define BN6_SOUL_NEXT         0x08        /* u8 the Soul the Custom screen committed (OK), taken as form BN6_FORM_PROXY is entered; 0 none */
#define BN6_SOUL_NEXT_CHAOS   0x09        /* u8 ... 1 for its Chaos Unison */
#define BN6_SOUL_NEXT_TURNS   0x0A        /* u8 ... its turns (3, 1 for Chaos Unison: BN5's OK, 0x08024FFA) */
#define BN6_SOUL_SLOT         0x0B        /* u8 what element 0xB is on this Custom screen: BN6_SLOT_EMBLEM or BN6_SLOT_UNITE */
#define BN6_SOUL_STANDS       0x0C        /* u8 what stands under OK on this Custom screen: BN6_STANDS_UNITE, BN6_STANDS_BEAST */
#define BN6_SOUL_SCREEN       0x0D        /* u8 1 once a Custom screen has opened with the Soul united: its end waits for the turn's start after it
                                             (BN6 checks a form's end before the screen too, 0x0800840C; BN5 alone after it, 0x08007D0C) */
#define BN6_SOUL_USED         0x10        /* u32 the Souls united this battle: bit n Soul n, bit 16 + n its Chaos Unison (BN5_SOULS_USED's layout) */
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
#define BN6_BATTLE_PLAYER_STEP 0x10       /* ... bytes from one side's to the next */
#define BN6_BATTLE_SETTINGS   (BN6_BATTLE_STATE + 0x3C) /* u32 the battle's BattleSettings; its +3 the mode (0x0802D234), BN6_BATTLE_LINK a link */
#define BN6_SETTINGS_MODE     0x03        /* a BattleSettings' +3: the battle's mode (0x0802D242), BN6_BATTLE_LINK a link */
#define BN6_BATTLE_LINK       1           /* BN6_SETTINGS_MODE: a link battle (where neither game offers UNITE) */
#define BN6_NAVI_MOOD         0x0E        /* a battle NaviStats' +0x0E: its side's mood (BN6_BATTLE_MOOD's), read by BN5's thresholds (BN5_MOOD_WORRIED) */
#define BN6_NAVI_FORM         0x2C        /* a battle NaviStats' +0x2C: its side's form (BN6_BATTLE_FORM's) */
#define BN6_NAVI_SIDE_NAVI    0x29        /* a battle NaviStats' +0x29: the Navi it is, 0 MegaMan (the turns count down for him alone, 0x08015A16) */
#define BN6_AI_ANGER          0x34        /* u16 a player's AI data +0x34: his anger's frames (both games' emotion getters: 3, which UNITE stands in) */
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

#endif
