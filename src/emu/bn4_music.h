/* BN4's music: what starts its songs and where (docs/rom/bn4.md, Where BN4
 * plays its songs; docs/research/sources.md, The Jukebox's songs), read for
 * the Jukebox's names (tools/bn4_songs.py), none of it run. PlayMusic, the
 * map-music lists and the battles' songs are bn4.h's (BN4_PLAY_MUSIC,
 * BN4_MAP_MUSIC, BN4_RECORD_MUSIC, BN4_MODE_SONGS, BN4_SONG_*). */
#ifndef CW_BN4_MUSIC_H
#define CW_BN4_MUSIC_H

/* What starts a song (r0 the song, its number in the song table 0x08155FF8) */
#define BN4_PLAY_MUSIC_AGAIN  0x08000586u /* PlayMusic begun even where it is the one playing (the title's) */
#define BN4_PLAY_MUSIC_FADE   0x080005F2u /* ... faded in: r1 the fade's speed, r2 the player; the BGM byte set for songs up to 0x21 */

/* The scripts' commands that start one (a map's, a cutscene's, an object's, a text's): a song a hword after the command */
#define BN4_MAP_SCRIPT_CMDS   0x0802A7D0u /* the map scripts' 0x40 commands: 0x2C a song as PlaySoundEffect, 0x2D one (below 0 the map's), */
                                          /* 0x2E one faded in, 0x23 a cutscene; the cutscenes' (BN4_CUTSCENE_COMMANDS) 0x49, 0x4A, 0x4B */
#define BN4_NPC_SCRIPT_CMDS   0x080A85D0u /* an object's script's 0x4C (the object's +0x5C; 0x01 and 0x2C none): 0x28 a song as */
                                          /* PlaySoundEffect, 0x47 one */
#define BN4_TEXT_SOUND_CMDS   0x08050040u /* a text script's 0xFD, by its next byte: 0, 6 and 7 a song as PlaySoundEffect, 1 one, 0x0B one faded in */
#define BN4_MAP_SCRIPTS       0x08029C4Cu /* a map group's two tables (its maps' scripts as each is entered, and each frame's): the real */
#define BN4_NET_MAP_SCRIPTS   0x08029CC4u /* ... world's 6, and the net's 21 from 0x80 (0x83-0x87, 0x89-0x8B and 0x8F none) */

/* The songs its code starts, where */
#define BN4_BATTLE_WIN_SONG   0x080071F8u /* a battle's win (the battle's end's step 2, 0x0800719C): PlayMusic of r2, */
#define BN4_SONG_VICTORY      0x18        /* ... this for the battle modes 0x42 and 0x43 or where the options' bit 1 is set, */
#define BN4_SONG_FANFARE      0x1E        /* ... else this: BN6's pick (BN6's 0x19 and 0x1F, the Jukebox's Victory and Fanfare) */
#define BN4_BATTLE_LOST       0x08007252u /* ... its step 3, a battle lost (its count of losses kept): for the modes from 0x46 */
#define BN4_SONG_DEFEAT       0x19        /* ... this */
#define BN4_SONG_TUTORIAL     0x21        /* the story battles 0-2 and 7's (BN4_STORY_BATTLES; modes 0x40-0x43, Mettaurs) */
#define BN4_MODE_JACK_IN      0x0C        /* the main mode of a jack-in's tunnel (its step in 0x0200A140): its first step (0x080298E0) */
#define BN4_SONG_JACK_IN      0x0E        /* ... plays this */
#define BN4_MODE_TITLE        0x00        /* the title's main mode (0x14 goes back to it); its start (0x08025424) begins song 0x00 by */
                                          /* BN4_PLAY_MUSIC_AGAIN */
#define BN4_MODE_CREDITS      0x38        /* the staff roll's (0x0811012C), which plays BN4_SONG_CREDITS (0x08110210) */
#define BN4_SONG_CREDITS      0x1C
#define BN4_MODE_OPERATION    0x3C        /* the link's Operation Battle menu, which plays BN4_SONG_TOURNAMENT_DAY (0x08040852), as */
#define BN4_SONG_TOURNAMENT_DAY 0x09      /* ... the bracket's start does (BN4_BRACKET_SONG_SITE) and the tournaments' scenes */
#define BN4_SONG_GAME_OVER    0x1A        /* BN4_MODE_GAME_OVER's (0x08048B6A) */
#define BN4_NAVICUST_OPEN     0x08047828u /* the NaviCust opened from the map (a map object's code, 0x0809E2D2): plays BN4_SONG_NAVICUST */
#define BN4_SONG_NAVICUST     0x1D
#define BN4_FREE_BRACKET_WON  0x08045320u /* the Free Tournament's bracket, MegaMan its winner (its message 0x10): plays BN4_SONG_CHAMPION, */
#define BN4_SONG_CHAMPION     0x1F        /* ... which the tournaments' stages (maps 0x02:3, 0x03:3, 0x04:4) start in their scenes too */

/* A warp, as a script's 0x48 asks it (the research's way to each map group in romlab: docs/rom/bn4.md) */
#define BN4_WARP_SET          0x08005664u /* r0 a list of warps, 16 bytes each (group, map, 0, its fade; x, y, z as words), r1 the one */
#define BN4_WARP              0x08005572u /* ... then this, r0 the script's byte: the map's fade and the warp begun */

#endif
