/* What BN6 Cybeast Gregar (USA) has to show a character's mood, part of
 * bn6.h's addresses (it includes this): MegaMan's overworld animations past
 * walking, the NPC script's hop, the chat box's face and pose commands, the
 * battle's "!!" burst, the map objects' table and slots, and the sounds its
 * scripts play at such moments. BN6 has no emotion-bubble set of its own
 * (no "?", heart, note, sweat or sleep icon over a head; docs/ROM_DATA.md,
 * Expression). Names follow the bn6f disassembly; see docs/ROM_DATA.md. */
#ifndef CW_BN6_EXPRESSION_H
#define CW_BN6_EXPRESSION_H

/* MegaMan's overworld sprite and its animations (facing 0-7 added where a
 * number is a first of eight) */
#define BN6_OW_MEGAMAN          0x37   /* sprite list 6's MegaMan, plain (no LZ77): the player's on the net, an NPC's in a cutscene, the Square's peers */
/* The player object's body: the sprite its spawn loads (a liberation's
 * Navi in play, player_body.c) */
#define BN6_PLAYER_SPRITE_LOAD 0x0809E7EAu /* movs r0,#0x80 before the player object's spawn (bn6f owPlayer's sub_809D270) loads its sprite: r6 its
                                             list x 4 (BN6_OW_NPC_LIST on the net), r4 its number (PETNaviToNPCSpriteTable's, 0x37 MegaMan's);
                                             it runs again where BN6_PLAYER_MAIN is set to 0 */
#define BN6_OW_NPC_LIST       0x18        /* sprite_load's list x 4: list 6, the Navis and Mr. Progs */
/* An object's sprite (bn6f's ObjectSprite), at the object less the low
 * four bits of the header's byte BN6_OBJ_SPRITE_AT, as sprite_setColorShader
 * finds it */
#define BN6_OBJ_SPRITE_AT     0x02        /* ObjectHeader TypeAndSpriteOffset: its high nibble x 16, the sprite's offset in the object */
#define BN6_SPRITE_SHADER     0x06        /* ObjectSprite +6, u16, the colour shader on its palette as it is drawn: bit 15 subtracts the
                                             BGR555 colour below it from every colour, else adds it (bn6f sub_3005EF0); an NPC script's
                                             command 0x2B sets it */
#define BN6_SHADE_DARK        0xA108      /* ... less 8 of each channel: BN6's own darkening (owPlayer's at a map's dark spot, coordinate
                                             trigger 0x3C), the shade BN5's board gives a Navi who has acted */
#define BN6_OW_MEGAMAN_ANIMS    0x49   /* ... its animations: 0-7 standing, 8-0xF walking, 0x10-0x17 running (+ facing) */
#define BN6_OW_ANIM_GOT         0x18   /* BN6_OW_MEGAMAN: the item-get pose, a hand raised (3 frames, 35 long), text F8 01 0x18 with sound 0x73 */
#define BN6_OW_ANIM_LOG_IN      0x19   /* BN6_OW_MEGAMAN: + facing, logging in (9 frames blank, 4 lighter, 4 himself) */
#define BN6_OW_ANIM_LOG_OUT     0x21   /* BN6_OW_MEGAMAN: + facing, logging out */
#define BN6_OW_ANIM_ARRIVE      0x2D   /* BN6_OW_MEGAMAN: a jack-in's arrival, blank then turning round once (10 frames, 50 long), with sound 0x76 */
#define BN6_OW_ANIM_STARTLED    0x2E   /* BN6_OW_MEGAMAN: + 0-3, a step back, guarding, four ways (1 frame) */
#define BN6_OW_ANIM_AIM         0x33   /* BN6_OW_MEGAMAN: + 0-3, the buster held out four ways (3 frames looping) */
#define BN6_OW_ANIM_SHOCKED     0x38   /* BN6_OW_MEGAMAN: sparks beside him, shaken (2 frames looping) */
#define BN6_OW_ANIM_CROUCH      0x39   /* BN6_OW_MEGAMAN: hunched, hurt (2 frames looping) */
#define BN6_OW_ANIM_COLLAPSE    0x3F   /* BN6_OW_MEGAMAN: standing, kneeling, down (3 frames) */
#define BN6_OW_ANIM_ARMS_UP     0x40   /* BN6_OW_MEGAMAN: both arms raised over his head (4 frames looping) */
#define BN6_OW_ANIM_KNEEL       0x41   /* BN6_OW_MEGAMAN: on one knee, still (1 frame) */
#define BN6_OW_ANIM_PANTING     0x42   /* BN6_OW_MEGAMAN: on one knee, heaving (4 frames looping) */
#define BN6_OW_ANIM_LEAP        0x43   /* BN6_OW_MEGAMAN: in mid-leap (1 frame) */
#define BN6_OW_ANIM_PONDER      0x44   /* BN6_OW_MEGAMAN: crouched, looking about (3 frames, 9 long) */
/* (four-way animations, + 0-3: the player's facing / 2, his facings 1, 3,
 * 5 and 7 the grid's +x, +y, -x and -y (BN6_TALK_PROBES); facing 5's
 * 0x35 seen pointing -x in the Square's captures, issue #176, the others
 * assumed in that order) */

/* The NPC script's hop in place, as BN6's excited bystanders do it (bn6f
 * npc_initHopMovement_809e8fc): the shadow left on the floor, the hop, the
 * shadow taken back, BN6's own bytes 22 11 60 DC 21 */
#define BN6_NPCSCRIPT_HOP       0x11   /* NPC command: a hop, two bytes after it: its rise and its fall */
#define BN6_NPCSCRIPT_HOP_RISE  0x60   /* ... the rise every one of BN6's hops in place takes (a few take 0x9E) */
#define BN6_NPCSCRIPT_HOP_FALL  0xDC   /* ... and its fall */
#define BN6_NPCSCRIPT_SHADOW_ON 0x21   /* NPC command: the shadow under the feet again (bn6f ns_give_attached_shadow) */
#define BN6_NPCSCRIPT_SHADOW_OFF 0x22  /* NPC command: the shadow left on the floor through a hop (bn6f ns_give_detatched_shadow) */
#define BN6_NPCSCRIPT_HOP_CODE  0x0809FE34u /* the command's routine (bn6f npc_initHopMovement_809e8fc) */

/* The chat box's face and the player's pose, from a text script */
#define BN6_TEXT_MUGSHOT        0xF5   /* text command: F5 00 face shows a mugshot, F5 01 hides it, F5 02 bank its palette bank, F5 03 n its animation */
#define BN6_TEXT_MUGSHOT_CODE   0x08041CC4u /* the command's routine (bn6f chatbox_F5_mugshot) */
#define BN6_MUGSHOT_BANKS       0x08044214u /* (face, palette bank) pairs, 0xFF ending them, F5 00 reads (bn6f byte_8044244) */
#define BN6_MUGSHOT_ANIMS       0x08044230u /* (face, animation) pairs F5 00 reads, empty in BN6: 0xFF first (bn6f byte_8044260) */
#define BN6_MUGSHOT_STILL       0      /* BN6_TEXT_MUGSHOT: F5 03's animation, a face's sub-animation: the face still */
#define BN6_MUGSHOT_BLINK       1      /* BN6_TEXT_MUGSHOT: ... blinking, the mouth shut (a face's pause between words) */
#define BN6_MUGSHOT_TALK        2      /* BN6_TEXT_MUGSHOT: ... the mouth moving while letters print, blinking between */
#define BN6_MUGSHOT_EYES_SHUT   3      /* BN6_TEXT_MUGSHOT: ... eyes shut, Lan's face (0x00) alone has it */
#define BN6_TEXT_PLAYER         0xF8   /* text command: F8 00 holds him, F8 01 n his animation n, F8 02 lets go, F8 03 waits for its end, F8 04 the one before the last F8 01 (each F8 01 keeps the one it replaces); run with no text printed, no chat box opens (issue #176) */
#define BN6_TEXT_SOUND          0xFD   /* text command: FD 00 then a u16 sound, played without opening the box */

/* The battle's burst over a fighter (sprite list 5's 0x07, plain) */
#define BN6_FX_BURST            0x07   /* sprite list 5's burst, 4 animations of 9 frames (18 long): 0 and 2 "!!", 1 a bat, 3 a skull */
#define BN6_FX_BURST_OBJECT     0x6B   /* the battle's type-4 object that shows it at a fighter's head, its Param1 the animation */
#define BN6_FX_BURST_CODE       0x080E93BCu /* ... its routine (bn6f t4_0x6b_80E807C); its sprite loaded at 0x080E93DC */
#define BN6_FX_BURST_SPAWN      0x080E9464u /* ... its spawner (bn6f sub_80E8124), a counter hit's at 0x0801A42E (CollisionData +0x74) */
#define BN6_FX_SPARKLE          0x05   /* sprite list 5's white and blue bursts, 14 animations: 0, 8 and 9 small star bursts, 7 "TRAP!" */
#define BN6_FX_GREEN_SPARKLE    0x15   /* sprite list 5's green sparkles, 4 animations: 0 and 1 a ring looping, 3 a ring then a flash */
#define BN6_BATTLE_ROLL         0x50   /* sprite list 3's Roll (LZ77, 7864 bytes): her animation 4 is a heart, 1 frame */

/* The overworld's map objects (bn6f OverworldMapObject) */
#define BN6_OW_OBJECTS          0x02011EE0u /* eOverworldMapObjects, 0x78 bytes each */
#define BN6_OW_OBJECTS_ACTIVE   0x02011E50u /* ... a bit per object in use (bn6f eActiveOverworldMapObjectsBitfield) */
#define BN6_OW_OBJECT_SLOTS     0x38   /* ... how many (bn6f NUM_OVERWORLD_MAP_OBJECTS) */
#define BN6_OW_OBJECT_BYTES     0x78   /* ... one's size */
#define BN6_OW_OBJECT_SPAWN     0x080047E0u /* SpawnOverworldMapObject: r0 its handler, r1-r3 X, Y and Z (16.16), r4 its id; r5 the object */
#define BN6_OW_OBJECT_RECORDS   0x080A4F24u /* OverworldMapObjects: handler 0's records, 16 bytes an id: sprite list x 4, sprite, animation */
#define BN6_OW_OBJECT_IDS       0xF4   /* ... how many ids (0x00-0xF3) */
#define BN6_OW_OBJECT_TRANSMIT  0x14   /* ... an id: list 5's 0x03 animation 1, a circle drawn and a mark in it (0xB2: its animation 2, lines and rings) */
#define BN6_OW_OBJECT_UNUSED    0x1F   /* ... an id whose record is all 0xFF, as 0x20, 0xB4, 0xB5 and 0xBF are */

/* Sounds BN6's scripts play at a moment of feeling (bn6f SoundOffsets.inc's
 * numbers; the words beside them in its text) */
#define BN6_SOUND_GOT           0x73   /* the item-get jingle ("Lan got:"), with BN6_OW_ANIM_GOT */
#define BN6_SOUND_RANK_UP       0x74   /* "Your Request Rank has been increased" */
#define BN6_SOUND_TYPE          0x65   /* a letter typed in the chat box (bn6f SOUND_TYPE) */
#define BN6_SOUND_HIT           0x6B   /* a hit (bn6f SOUND_HIT_6B) */
#define BN6_SOUND_BUSTER_CHARGE 0x71   /* the buster charging (bn6f SOUND_BUSTER_CHARGE) */
#define BN6_SOUND_LOG_IN        0x77   /* a Navi's log-in (bn6f SOUND_LOG_IN_77) */
#define BN6_SOUND_CURSOR        0x7F   /* a menu's cursor moved (bn6f SOUND_MENU_CUR_MOVE) */
#define BN6_SOUND_FADE          0x8E   /* bn6f SOUND_FADE_8E, its use not looked up (assumed soft) */
#define BN6_SOUND_LOG_OUT       0x76   /* the beam (bn6f SOUND_LOG_OUT_76): a Navi's log-in and log-out alike */
#define BN6_SOUND_LINK_NAVI     0x8F   /* "... became Lan's LinkNavi!" */
#define BN6_SOUND_APPEAR        0x94   /* a sparkle as something appears */
#define BN6_SOUND_BEEP          0xCD   /* "*BEEP! BEEP!*", an alarm */
#define BN6_SOUND_FALL          0x14B  /* "Waaa!!", "*thud*": a fall */
#define BN6_SOUND_CHIME         0x14F  /* "*ding,dong, ding,doooong*" */
#define BN6_SOUND_BARK          0x174  /* "BOW! WOW!" */
#define BN6_SOUND_WOOF          0x175  /* "*woof, woof!!*" */
#define BN6_SOUND_DOORBELL      0x177  /* "*ding, dong*" */
#define BN6_SOUND_INSTALLED     0x182  /* "... have been installed" (the NaviCust) */
#define BN6_SOUND_PHONE         0x18F  /* "*bring,bring!!*" */
#define BN6_SOUND_POPS          0x19A  /* "*pop,pop,pop!!*" */

#endif
