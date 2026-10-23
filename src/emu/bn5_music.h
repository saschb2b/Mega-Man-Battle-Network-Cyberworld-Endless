/* BN5's music: what starts its songs, and where it starts the ones runs
 * play (docs/ROM_DATA.md, Where BN5 plays its songs; docs/SOURCES.md, The
 * Jukebox's songs). Its code is BN6's twin; read for the Jukebox's names and
 * the cues runs take from BN5 (docs/MULTIROM.md, BN5's music in a run),
 * none of it run. */
#ifndef CW_BN5_MUSIC_H
#define CW_BN5_MUSIC_H

/* What starts a song (r0 the song, its number in the song table 0x08155BF4) */
#define BN5_PLAY_MUSIC        0x080005A0u /* PlayMusic: the game state's BGM byte (+0x0F) set and the song begun, unless it is the one playing; 0x63 stops them all */
#define BN5_PLAY_MUSIC_AGAIN  0x080005BEu /* ... begun even where it is the one playing */
#define BN5_PLAY_MUSIC_FADE   0x0800062Au /* ... faded in: r1 the fade's speed, r2 the player; the BGM byte set for songs up to 0x23 */
#define BN5_PLAY_SOUND        0x08000598u /* PlaySoundEffect: a song begun on the players its entry names, the BGM byte left as it is */
#define BN5_PLAY_MAP_MUSIC    0x0803730Cu /* the map's song played: the byte at +4 of the map's music state (0x020029AC, eToolkit +0x40) */
#define BN5_MAP_MUSIC_PICK    0x080372D8u /* ... that byte taken from the map-music lists (0x080361C8, by game state +7), 0x63 for a group none names */

/* The scripts' commands that start one (a map's, a cutscene's, an object's, a text's) */
#define BN5_MAP_SCRIPT_CMDS   0x08035934u /* the map scripts' 0x46 commands: 0x30 a song (s16; below 0 the map's), 0x31 one faded in, 0x26 a cutscene */
#define BN5_CUTSCENE_CMDS     0x08037844u /* the cutscenes' 0x75: 0x4C a song, 0x4D one faded in (song, speed, player); 0x14-0x37 the map scripts' 0x01-0x29 */
#define BN5_NPC_SCRIPT_CMDS   0x080A66D4u /* an object's script's 0x51 (the object's +0x5C): 0x47 a song */
#define BN5_TEXT_SOUND_CMDS   0x08040D60u /* a text script's 0xFD, by its next byte: 0 a song as PlaySoundEffect, 1 one as PlayMusic, 0x0B one faded in */
#define BN5_START_CUTSCENE    0x0803731Cu /* StartCutscene: r0 the script, r1 its word */
#define BN5_MAP_SCRIPTS       0x08034E44u /* a map group's two tables (its maps' scripts as each is entered, and each frame's): the real world's 12, */
#define BN5_NET_MAP_SCRIPTS   0x08034EBCu /* ... and the net's from 0x80 (0x95 the last; 0x85, 0x87, 0x89, 0x8B and 0x8F none) */

/* The songs a run takes from BN5, where BN5 starts them */
#define BN5_WIN_SONG          0x08007A82u /* a liberation's battle won (its win, 0x08007A50): the song in r2, 0x19 where the options' bit 2 is set, */
#define BN5_SONG_ONE_TURN_WIN 0x24        /* ... else 0x1F, and this where its Custom screens counted one (BattleState +7: won in the first turn) */
#define BN5_LIB_WON_STEP      0x0804E9F4u /* the board's routine 6 (BN5_LIB_STATE +0 = 0x18), its first step: the mission won, the Colonel's tally of */
#define BN5_SONG_LIB_WON      0x18        /* ... the phases (target, actual) over this, which plays once and ends; its fifth step the team's retreat */
#define BN5_SONG_BOSS_PRELUDE 0x1C        /* before a fight: as ProtoMan or Colonel shows himself, at the scenario comps' last maps, at a mission's opening */

#endif
