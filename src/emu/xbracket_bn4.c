/* xbracket_bn4.c: BN4 Red Sun's bracket screen (main mode 0x24) moved into BN6, written by tools/xrecipe_bracket.py from
 * BN4's and BN6's ROMs (python3 tools/xrecipe_bracket.py --write; --check says whether this is what it writes). Do not edit:
 * change the generator and write it again. Numbers alone: addresses in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA) and
 * CRC32s; the code and art are the player's ROM's, copied as the core starts (src/emu/xbracket.c; docs/multirom/
 * bn4-tournament.md, Moved into BN6). */
#include "xbracket_bn4.h"

#include "rom.h"
#include "xbracket.h"

static const XvBlock blocks[] = {
	{ 0x08043814, 0x08043F34, XBR_CODE + 0x14, 0x77065BA5u },   /* the screen and its set-up */
	{ 0x08103F90, 0x08104710, XBR_CODE + 0x810, 0x75CFE297u },   /* the plates' menu-sprite type */
	{ 0x08053E24, 0x08054180, XBR_CODE + 0x1024, 0x42D8B2F8u },   /* the menu sprites' runner */
	{ 0x080455C0, 0x080455E0, XBR_CODE + 0x1800, 0x8DD46D5Au },   /* the Free Tournament's plates and lines */
	{ 0x08002908, 0x08002934, XBR_CODE + 0x2008, 0x6018FDD0u },   /* the menu sprite's sprite set (BN6's twin reads its own table) */
	{ 0x08001BE8, 0x08001CA4, XBR_CODE + 0x2828, 0x99664B7Cu },   /* the BG controls' preset set (BN6's twin reads its own presets) */
	{ 0x080281D0, 0x080281E8, XBR_CODE + 0x3010, 0x3E81CC19u },   /* BN4_MENU_SPRITE_PTRS' first six */
	{ 0x08044844, 0x08044888, XBR_CODE + 0x3804, 0xD834E641u },   /* the Free Tournament's spawn lists */
	{ 0x086C4E40, 0x086C792C, XBR_ART + 0x0, 0x878BF492u },   /* BN4_BRACKET_TILES to the last of BN4_BRACKET_MAPS' maps */
	{ 0x084B7E50, 0x084BEADC, XBR_ART + 0x2B10, 0xD632DD15u },   /* BN4_MENU_SPRITE_PTRS 2-5, one after another: the lit lines' sprites (2, 3) and the plates' (4, 5), BN4_PLATE_SPRITES' values */
};
static const XvData data[] = {
	{ 0x08001BFE, 0x08001C00, XV_RAW },
	{ 0x08001C00, 0x08001C04, XV_POOL },
	{ 0x08001C04, 0x08001CA4, XV_RAW },
	{ 0x0800290C, 0x0800290E, XV_RAW },
	{ 0x0800292E, 0x08002930, XV_RAW },
	{ 0x08002930, 0x08002934, XV_POOL },
	{ 0x080281D0, 0x080281D4, XV_POOL },
	{ 0x080281D4, 0x080281D8, XV_POOL },
	{ 0x080281D8, 0x080281DC, XV_POOL },
	{ 0x080281DC, 0x080281E0, XV_POOL },
	{ 0x080281E0, 0x080281E4, XV_POOL },
	{ 0x080281E4, 0x080281E8, XV_POOL },
	{ 0x08043814, 0x08043818, XV_POOL },
	{ 0x08043818, 0x0804381C, XV_POOL },
	{ 0x0804381C, 0x08043820, XV_POOL },
	{ 0x08043820, 0x08043824, XV_POOL },
	{ 0x08043824, 0x08043828, XV_POOL },
	{ 0x08043828, 0x0804382C, XV_POOL },
	{ 0x0804382C, 0x08043830, XV_POOL },
	{ 0x08043830, 0x08043834, XV_POOL },
	{ 0x08043834, 0x08043838, XV_POOL },
	{ 0x08043838, 0x0804383C, XV_POOL },
	{ 0x0804383C, 0x08043840, XV_POOL },
	{ 0x08043840, 0x08043844, XV_POOL },
	{ 0x08043844, 0x08043848, XV_POOL },
	{ 0x08043848, 0x0804384C, XV_POOL },
	{ 0x0804384C, 0x08043850, XV_POOL },
	{ 0x08043850, 0x08043854, XV_POOL },
	{ 0x08043854, 0x08043858, XV_POOL },
	{ 0x08043858, 0x0804385C, XV_POOL },
	{ 0x0804385C, 0x08043860, XV_POOL },
	{ 0x08043860, 0x08043864, XV_POOL },
	{ 0x08043864, 0x08043868, XV_POOL },
	{ 0x08043868, 0x0804386C, XV_POOL },
	{ 0x0804386C, 0x08043870, XV_POOL },
	{ 0x08043870, 0x08043874, XV_POOL },
	{ 0x08043874, 0x08043878, XV_POOL },
	{ 0x08043878, 0x0804387C, XV_POOL },
	{ 0x0804387C, 0x08043880, XV_POOL },
	{ 0x08043880, 0x08043884, XV_POOL },
	{ 0x08043884, 0x080438F8, XV_RAW },
	{ 0x0804392C, 0x0804395C, XV_RAW },
	{ 0x0804396C, 0x08043970, XV_POOL },
	{ 0x08043970, 0x08043974, XV_POOL },
	{ 0x08043974, 0x08043978, XV_POOL },
	{ 0x08043978, 0x0804397C, XV_POOL },
	{ 0x0804397C, 0x08043980, XV_POOL },
	{ 0x08043A74, 0x08043A78, XV_POOL },
	{ 0x08043A78, 0x08043A7C, XV_POOL },
	{ 0x08043A7C, 0x08043A80, XV_POOL },
	{ 0x08043A80, 0x08043A84, XV_POOL },
	{ 0x08043A84, 0x08043A88, XV_POOL },
	{ 0x08043AD2, 0x08043AD4, XV_RAW },
	{ 0x08043AD4, 0x08043AD8, XV_POOL },
	{ 0x08043AD8, 0x08043ADC, XV_POOL },
	{ 0x08043ADC, 0x08043AE0, XV_POOL },
	{ 0x08043AE0, 0x08043AE4, XV_POOL },
	{ 0x08043AE4, 0x08043AE8, XV_POOL },
	{ 0x08043AE8, 0x08043AEC, XV_POOL },
	{ 0x08043AEC, 0x08043AF0, XV_POOL },
	{ 0x08043AF0, 0x08043AF4, XV_POOL },
	{ 0x08043AF4, 0x08043AF8, XV_POOL },
	{ 0x08043AF8, 0x08043AFC, XV_POOL },
	{ 0x08043AFC, 0x08043B00, XV_POOL },
	{ 0x08043B00, 0x08043B04, XV_POOL },
	{ 0x08043B04, 0x08043B08, XV_POOL },
	{ 0x08043B08, 0x08043B0C, XV_POOL },
	{ 0x08043B0C, 0x08043B10, XV_POOL },
	{ 0x08043B10, 0x08043B14, XV_POOL },
	{ 0x08043B14, 0x08043B18, XV_POOL },
	{ 0x08043B18, 0x08043B1C, XV_POOL },
	{ 0x08043B1C, 0x08043B24, XV_RAW },
	{ 0x08043B24, 0x08043B28, XV_POOL },
	{ 0x08043B28, 0x08043B2C, XV_POOL },
	{ 0x08043B2C, 0x08043B30, XV_POOL },
	{ 0x08043B30, 0x08043B34, XV_POOL },
	{ 0x08043B8E, 0x08043B90, XV_RAW },
	{ 0x08043BA6, 0x08043BA8, XV_RAW },
	{ 0x08043BF4, 0x08043BF8, XV_POOL },
	{ 0x08043BF8, 0x08043BFC, XV_RAW },
	{ 0x08043C60, 0x08043C64, XV_POOL },
	{ 0x08043C64, 0x08043C68, XV_POOL },
	{ 0x08043C68, 0x08043C6C, XV_POOL },
	{ 0x08043CA4, 0x08043CA8, XV_POOL },
	{ 0x08043CA8, 0x08043CAC, XV_RAW },
	{ 0x08043CAC, 0x08043CB0, XV_POOL },
	{ 0x08043CB0, 0x08043CB4, XV_POOL },
	{ 0x08043CB4, 0x08043CD4, XV_RAW },
	{ 0x08043D16, 0x08043D18, XV_RAW },
	{ 0x08043D18, 0x08043D1C, XV_POOL },
	{ 0x08043D1C, 0x08043D20, XV_POOL },
	{ 0x08043D20, 0x08043D24, XV_POOL },
	{ 0x08043D24, 0x08043D28, XV_POOL },
	{ 0x08043D28, 0x08043D2C, XV_POOL },
	{ 0x08043D2C, 0x08043D30, XV_POOL },
	{ 0x08043D48, 0x08043D70, XV_RAW },
	{ 0x08043DFE, 0x08043E90, XV_RAW },
	{ 0x08043E90, 0x08043E94, XV_POOL },
	{ 0x08043E94, 0x08043E98, XV_POOL },
	{ 0x08043E98, 0x08043EA0, XV_RAW },
	{ 0x08043EEA, 0x08043F18, XV_RAW },
	{ 0x08043F18, 0x08043F1C, XV_POOL },
	{ 0x08043F1C, 0x08043F20, XV_POOL },
	{ 0x08043F20, 0x08043F30, XV_RAW },
	{ 0x08043F30, 0x08043F34, XV_POOL },
	{ 0x08044844, 0x08044888, XV_RAW },
	{ 0x080455CA, 0x080455CC, XV_RAW },
	{ 0x080455CC, 0x080455D0, XV_POOL },
	{ 0x080455DA, 0x080455DC, XV_RAW },
	{ 0x080455DC, 0x080455E0, XV_POOL },
	{ 0x08053E24, 0x08053E28, XV_POOL },
	{ 0x08053E28, 0x08053E2C, XV_POOL },
	{ 0x08053E2C, 0x08053E30, XV_POOL },
	{ 0x08053E30, 0x08053E34, XV_POOL },
	{ 0x08053E34, 0x08053E38, XV_POOL },
	{ 0x08053E38, 0x08053E3C, XV_POOL },
	{ 0x08053E3C, 0x08053E40, XV_POOL },
	{ 0x08053E40, 0x08053E44, XV_POOL },
	{ 0x08053E44, 0x08053E48, XV_POOL },
	{ 0x08053E48, 0x08053E4C, XV_POOL },
	{ 0x08053E4C, 0x08053E50, XV_POOL },
	{ 0x08053E50, 0x08053E54, XV_POOL },
	{ 0x08053E54, 0x08053E58, XV_POOL },
	{ 0x08053E58, 0x08053E5C, XV_POOL },
	{ 0x08053E5C, 0x08053E60, XV_POOL },
	{ 0x08053E60, 0x08053E64, XV_POOL },
	{ 0x08053E64, 0x08053E68, XV_POOL },
	{ 0x08053E68, 0x08053E6C, XV_POOL },
	{ 0x08053E6C, 0x08053E70, XV_POOL },
	{ 0x08053E70, 0x08053E74, XV_POOL },
	{ 0x08053E74, 0x08053E78, XV_POOL },
	{ 0x08053E78, 0x08053E7C, XV_POOL },
	{ 0x08053E7C, 0x08053E80, XV_POOL },
	{ 0x08053E80, 0x08053E84, XV_POOL },
	{ 0x08053E84, 0x08053E88, XV_POOL },
	{ 0x08053E88, 0x08053E8C, XV_POOL },
	{ 0x08053E8C, 0x08053E90, XV_POOL },
	{ 0x08053E90, 0x08053E94, XV_POOL },
	{ 0x08053E94, 0x08053E98, XV_POOL },
	{ 0x08053E98, 0x08053E9C, XV_POOL },
	{ 0x08053E9C, 0x08053EA0, XV_POOL },
	{ 0x08053EA0, 0x08053EA4, XV_POOL },
	{ 0x08053EA4, 0x08053EA8, XV_POOL },
	{ 0x08053EA8, 0x08053EAC, XV_POOL },
	{ 0x08053EAC, 0x08053EB0, XV_POOL },
	{ 0x08053EB0, 0x08053EB4, XV_POOL },
	{ 0x08053EB4, 0x08053EB8, XV_POOL },
	{ 0x08053EB8, 0x08053EBC, XV_POOL },
	{ 0x08053EBC, 0x08053EC0, XV_POOL },
	{ 0x08053EC0, 0x08053EC4, XV_POOL },
	{ 0x08053EC4, 0x08053EC8, XV_POOL },
	{ 0x08053EC8, 0x08053ECC, XV_POOL },
	{ 0x08053ECC, 0x08053ED0, XV_POOL },
	{ 0x08053ED0, 0x08053ED4, XV_POOL },
	{ 0x08053ED4, 0x08053ED8, XV_POOL },
	{ 0x08053ED8, 0x08053EDC, XV_POOL },
	{ 0x08053EDC, 0x08053EE0, XV_POOL },
	{ 0x08053EE0, 0x08053EE4, XV_POOL },
	{ 0x08053EE4, 0x08053EE8, XV_POOL },
	{ 0x08053EE8, 0x08053EEC, XV_POOL },
	{ 0x08053EEC, 0x08053EF0, XV_POOL },
	{ 0x08053EF0, 0x08053EF4, XV_POOL },
	{ 0x08053EF4, 0x08053EF8, XV_POOL },
	{ 0x08053EF8, 0x08053EFC, XV_POOL },
	{ 0x08053EFC, 0x08053F00, XV_POOL },
	{ 0x08053F00, 0x08053F04, XV_POOL },
	{ 0x08053F04, 0x08053F08, XV_POOL },
	{ 0x08053F08, 0x08053F0C, XV_POOL },
	{ 0x08053F0C, 0x08053F10, XV_POOL },
	{ 0x08053F10, 0x08053F14, XV_POOL },
	{ 0x08053F14, 0x08053F18, XV_POOL },
	{ 0x08053F18, 0x08053F1C, XV_POOL },
	{ 0x08053F1C, 0x08053F68, XV_RAW },
	{ 0x08053F86, 0x08053F88, XV_RAW },
	{ 0x08053F88, 0x08053F8C, XV_POOL },
	{ 0x08053FE0, 0x08053FE4, XV_POOL },
	{ 0x0805407C, 0x08054090, XV_RAW },
	{ 0x080540AA, 0x080540B8, XV_RAW },
	{ 0x080540B8, 0x080540BC, XV_POOL },
	{ 0x0805417E, 0x08054180, XV_RAW },
	{ 0x08103FA0, 0x08103FFC, XV_RAW },
	{ 0x0810400E, 0x08104010, XV_RAW },
	{ 0x08104010, 0x08104014, XV_POOL },
	{ 0x08104014, 0x08104018, XV_POOL },
	{ 0x08104018, 0x0810401C, XV_POOL },
	{ 0x0810401C, 0x08104020, XV_POOL },
	{ 0x08104030, 0x08104034, XV_POOL },
	{ 0x08104034, 0x08104038, XV_POOL },
	{ 0x08104038, 0x0810403C, XV_POOL },
	{ 0x0810403C, 0x08104040, XV_POOL },
	{ 0x081040F0, 0x081040F4, XV_POOL },
	{ 0x081040F4, 0x081040F8, XV_POOL },
	{ 0x081040F8, 0x081040FC, XV_POOL },
	{ 0x081040FC, 0x08104100, XV_POOL },
	{ 0x08104100, 0x08104104, XV_POOL },
	{ 0x08104104, 0x08104108, XV_POOL },
	{ 0x08104108, 0x0810416C, XV_RAW },
	{ 0x081041B2, 0x081041B4, XV_RAW },
	{ 0x081041B4, 0x081041B8, XV_POOL },
	{ 0x0810425A, 0x0810425C, XV_RAW },
	{ 0x0810425C, 0x08104260, XV_POOL },
	{ 0x08104260, 0x08104264, XV_POOL },
	{ 0x08104264, 0x08104268, XV_POOL },
	{ 0x08104268, 0x0810426C, XV_POOL },
	{ 0x0810426C, 0x08104290, XV_RAW },
	{ 0x081042E6, 0x081042E8, XV_RAW },
	{ 0x081042E8, 0x081042EC, XV_POOL },
	{ 0x081042EC, 0x081042F0, XV_POOL },
	{ 0x081042F0, 0x081042F4, XV_POOL },
	{ 0x081042F4, 0x081042F8, XV_POOL },
	{ 0x081042F8, 0x081042FC, XV_POOL },
	{ 0x081042FC, 0x08104300, XV_POOL },
	{ 0x08104300, 0x08104304, XV_POOL },
	{ 0x08104304, 0x08104308, XV_POOL },
	{ 0x08104308, 0x0810430C, XV_POOL },
	{ 0x0810430C, 0x081043A8, XV_RAW },
	{ 0x081043BA, 0x081043BC, XV_RAW },
	{ 0x081043BC, 0x081043C0, XV_POOL },
	{ 0x081043C0, 0x081043C4, XV_POOL },
	{ 0x081043C4, 0x081043C8, XV_POOL },
	{ 0x081043C8, 0x081043CC, XV_POOL },
	{ 0x081043E4, 0x081043E8, XV_POOL },
	{ 0x081043E8, 0x081043EC, XV_POOL },
	{ 0x081043EC, 0x081043F0, XV_POOL },
	{ 0x0810441E, 0x08104420, XV_RAW },
	{ 0x08104420, 0x08104424, XV_POOL },
	{ 0x08104424, 0x08104428, XV_POOL },
	{ 0x08104428, 0x0810442C, XV_POOL },
	{ 0x0810442C, 0x08104430, XV_POOL },
	{ 0x081044AA, 0x081044AC, XV_RAW },
	{ 0x081044E2, 0x081044E4, XV_RAW },
	{ 0x0810450A, 0x0810450C, XV_RAW },
	{ 0x0810450C, 0x08104510, XV_POOL },
	{ 0x0810458A, 0x0810458C, XV_RAW },
	{ 0x0810458C, 0x08104590, XV_POOL },
	{ 0x08104590, 0x08104610, XV_RAW },
	{ 0x0810461E, 0x08104620, XV_RAW },
	{ 0x08104620, 0x08104624, XV_POOL },
	{ 0x08104624, 0x08104628, XV_POOL },
	{ 0x08104708, 0x0810470C, XV_POOL },
	{ 0x0810470C, 0x08104710, XV_RAW },
	{ 0x084B7E50, 0x084BEADC, XV_RAW },
	{ 0x086C4E40, 0x086C792C, XV_RAW },
};
static const XvPair calls[] = {
	{ 0x08000568, 0x080005D4 },   /* read: the music's start: the song r0 kept at the game state's +0x19 (BN6's +0xF), its "none" 0xFF (BN6's 0x63); the screen plays BN4's song 0x09, which a run copies into a BN6 slot (src/audio/xsong.c), DIFF; audited: BN6's plays song r0 as BN4's does, its own song numbers: the screen's song is BN4's 0x09 copied into a BN6 slot (src/audio/xsong.c), its immediate set to that slot as the screen installs */
	{ 0x08000938, 0x080008E0 },   /* search, SAME */
	{ 0x08000994, 0x0800093C },   /* search, SAME */
	{ 0x080009A8, 0x08000950 },   /* search, SAME */
	{ 0x080009E4, 0x0800098C },   /* search, SAME */
	{ 0x08001D24, 0x080017AA },   /* search, SAME */
	{ 0x08001D58, 0x080017E0 },   /* search, SAME */
	{ 0x08001DA8, 0x0800183C },   /* search, SAME */
	{ 0x08001DFC, 0x080018C2 },   /* near body (1.00), SAME */
	{ 0x08001E18, 0x080018D0 },   /* read: a veneer to IWRAM's fill of a BG map's rect with r3 (BN4's 0x03005A5D, BN6's 0x03005EBB: the same instructions, both games' IWRAM dumped in romlab); BN6's 0x080018C2, of the same veneer body, copies a rect instead, SAME */
	{ 0x08002048, 0x08001B1C },   /* near body (1.00), SAME */
	{ 0x08002814, XBR_STUB_NONE },   /* the host's: the leave's OBJ palette put back for BN4's map (its body copies BN4's own 0x20 bytes; the host's return reloads BN6's map) */
	{ 0x0800282C, 0x08002694 },   /* near body (1.00), SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x080028C4, 0x0800272C },   /* search, SAME */
	{ 0x0800295C, 0x08002818 },   /* search, SAME */
	{ 0x080029B8, 0x08002874 },   /* search, SAME */
	{ 0x08002C90, 0x08002C24 },   /* search, SAME */
	{ 0x08002D80, 0x08002D14 },   /* search, SAME */
	{ 0x08002DBE, 0x08002D52 },   /* search, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002E80, 0x08002E14 },   /* search, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x08005572, XBR_STUB_NONE },   /* the host's: the leave's warp back to the map (the host returns to BN6's map) */
	{ 0x08005664, XBR_STUB_NONE },   /* the host's: the leave's script resumed where one waits (BN4_BRACKET_FROM_SCRIPT; the host's own return) */
	{ 0x08005AFC, 0x08006270 },   /* search, SAME */
	{ 0x08005B7C, 0x080062F8 },   /* search, SAME */
	{ 0x080060A4, XBR_STUB_HBLANK },   /* the move's shim: the HBlank DMA set (r0 its source, r1 its register, r2 its count, r3 its control into the state at BN4's 0x02008920, BN6's 0x020081B0): BN6's 0x080068E8 sets two fields more from r4 and r5 (BN6's own caller, 0x080068EC, its line buffer 0x02008E80 and 0x500), which the shim gives as the source and the screen's 0x140 bytes */
	{ 0x080060BC, 0x08006910 },   /* search, SAME */
	{ 0x0802DF2C, XBR_STUB_RETURN },   /* the host's: the leave's map entered again (the host's own return) */
	{ 0x08044744, XBR_STUB_NONE },   /* the host's: BN4_NEXT_OPPONENT: the set-up's next opponent for BN4's cutscene (a run picks its own) */
	{ 0x0804E558, XBR_STUB_NONE },   /* the host's: the set-up's chat box closed (BN4's text engine: the host's map has its own) */
	{ 0x08053908, XBR_STUB_NONE },   /* the host's: the set-up's chat box flags cleared (0x40) */
	{ 0x080A6D62, XBR_STUB_NONE },   /* the host's: MegaMan's overworld player unlocked as the screen leaves (the host's map) */
	{ 0x08111894, 0x0814F414 },   /* near body (1.00), SAME */
};
static const XvPair words[] = {
	{ 0x02001460, RAM_BRACKET + 0x40 },   /* BN4_TOURNAMENTS: the plan's */
	{ 0x02008BB0, RAM_BRACKET + 0x300 },   /* BN4_MENU_SPRITES: the plan's */
	{ 0x02009440, RAM_BRACKET + 0x100 },   /* BN4_BRACKET_BLEND: the plan's */
	{ 0x0200ACC0, RAM_BRACKET + 0x0 },   /* BN4_BRACKET: the plan's */
	{ 0x0202E000, RAM_BRACKET + 0x1000 },   /* BN4_BRACKET_TILES_RAM: the plan's */
	{ 0x03002A50, 0x03001960 },   /* its twin's (a pair's bodies) */
	{ 0x03002A90, 0x030019A0 },   /* 03002A50's twin's, 0x40 on */
	{ 0x04000052, 0x04000052 },   /* I/O or video, as it is */
	{ 0x06008020, 0x06008020 },   /* I/O or video, as it is */
	{ 0x08043884, 0x00000000 },   /* the replaced script return's argument */
	{ 0x08102FC9, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x081049ED, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08104D85, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08104F85, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x081051DD, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08105421, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x081057FD, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08105985, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08105AED, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08106521, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08106615, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08106D19, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08106F61, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08107015, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x081070E1, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810715D, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x081072C9, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08107375, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08107495, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08107545, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810778D, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08107B79, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08107FC5, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08108329, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08108735, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08108881, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08108A61, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08108B05, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08108D09, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08108DA5, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08108F0D, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08109071, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x081094C9, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08109B1D, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08109EF1, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x08109FA5, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810A075, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810A159, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810A411, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810A561, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810A62D, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810A895, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810AA19, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810ACAD, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810AD71, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810AF0D, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810B141, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810B369, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810B449, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810B54D, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810B5FD, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810B7CD, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810BA21, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810BB19, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810BBE1, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810BCB1, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810BD89, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810BF15, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810BFE5, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x0810C465, XBR_STUB_NONE | 1u },   /* a menu sprite type the screen never spawns: the stub */
	{ 0x083E99EC, 0x00000000 },   /* a menu sprite the screen never draws */
	{ 0x084B5B70, 0x00000000 },   /* a menu sprite the screen never draws */
};
/* its reads of eToolkit +0x3C (BN4's tournaments, BN6's game state): each a call to the shim that loads the tournaments' place */
const XbrSite xbracket_sites[] = {
	{ 0x08043BB2, 1 },
	{ 0x08043DBE, 1 },
	{ 0x08043EA2, 1 },
	{ 0x08104056, 2 },
	{ 0x0810407C, 2 },
	{ 0x08104200, 1 },
	{ 0x08104436, 2 },
	{ 0x08104456, 1 },
	{ 0x08104516, 2 },
	{ 0x08104634, 2 },
	{ 0x08104692, 1 },
};
const int xbracket_site_count = (int)(sizeof xbracket_sites / sizeof *xbracket_sites);

/* BN4_MAP_LEAVE's calls the entry makes after BN6's own way out of the map: BN6's twins, and BN4's preset's set moved */
const XbrCall xbracket_leave[] = {
	/* (left out: 08003516: near body (0.26), None, no twin of the same body) */
	{ 0x0, 0x08004703, false },   /* 0800437A: search, SAME */
	{ 0x13, 0x08001BE8, true },   /* BN4's BG controls' preset, moved */
	/* (left out: 08001E94: near body (0.90), None, no twin of the same body) */
	{ 0x0, 0x08001AFD, false },   /* 08002028: search, SAME */
	{ 0x0, 0x080023A9, false },   /* 08002554: search, SAME */
	{ 0x0, 0x080024A3, false },   /* 0800264E: search, SAME */
	{ 0x0, 0x08003963, false },   /* 08003856: near body (1.00), SAME */
	{ 0x0, 0x08001821, false },   /* 08001D80: search, SAME */
	{ 0x40, XBR_STUB_DISPLAY | 1u, false },   /* the render info's DISPCNT made 0x40 */
};
const int xbracket_leave_count = (int)(sizeof xbracket_leave / sizeof *xbracket_leave);

const XVirusFamily xbracket_bn4 = {
	.name = "BN4's bracket screen",
	.game = XROM_BN4_RED_SUN_US,
	.blocks = blocks, .nblocks = (int)(sizeof blocks / sizeof *blocks),
	.data = data, .ndata = (int)(sizeof data / sizeof *data),
	.calls = calls, .ncalls = (int)(sizeof calls / sizeof *calls),
	.words = words, .nwords = (int)(sizeof words / sizeof *words),
};
