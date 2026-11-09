/* Ten-bit chip ids in the core's copy (xchips_wide.h). */
#include "xchips_wide.h"

#include <stdio.h>

#include "bn6.h"
#include "chip_entry.h"
#include "chips.h"
#include "claims.h"
#include "debug.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"

#define BUS 0x08000000u

/* BN6's code that takes a chip's id in nine bits, each halfword BN6's
 * (was) and ten bits' (now): a shift's count (23 to 22, 9 to 10, a code's
 * 26 to 27), the 0x01FF literal's low half (0x03FF), BN6_ENTRY_CHEAT's
 * entry. Numbers only, as tools/chip_ids.py --table writes them from the
 * census's verdicts (docs/rom/chips-code.md, Ten-bit chip ids). */
#define ARCHIVE_LOOP(r) \
	{ (r) + 0x08u, 0x29FF, 0x29FF }, { (r) + 0x0Au, 0xDD02, 0xDD03 }, { (r) + 0x0Cu, 0x3004, 0x3004 }, { (r) + 0x0Eu, 0x0609, 0x39FF }, \
	{ (r) + 0x10u, 0x0E09, 0x3901 }, { (r) + 0x12u, 0x6800, 0xE7F9 }, { (r) + 0x14u, 0x46F7, 0x6800 }, { (r) + 0x16u, 0x0000, 0x46F7 }
#define WORD(at, was, now) \
	{ (at), (uint16_t)((was) & 0xFFFFu), (uint16_t)((now) & 0xFFFFu) }, { (at) + 2u, (uint16_t)((was) >> 16), (uint16_t)((now) >> 16) }
#define ARCHIVE_WORD(r, was, now) WORD((r) + 0x18u, was, now)
/* a description's archive picked of two by id > 255 (ldr r0,=second; ldrh r1,[r5,#0xe]; cmp r1,#0xff; bgt; ldr r0,=first)
 * made one of four by the id's high byte: ldrh r1,[r5,#0xe]; ldr r0,=the four words (the first's literal, which
 * xchips_install.c points at them: `first` and `table` the two ldr, BN6's and ours); lsrs r2,r1,#8; lsls r2,r2,#2; ldr
 * r0,[r0,r2] */
#define DESC_READ(r, second, first, table) \
	{ (r), second, 0x89E9 }, { (r) + 2u, 0x89E9, table }, { (r) + 4u, 0x29FF, 0x0A0A }, { (r) + 6u, 0xDC00, 0x0092 }, { (r) + 8u, first, 0x5880 }

static const struct { uint32_t at; uint16_t was, now; } changes[] = {
	{ 0x0800A584u, 0x05C0, 0x0580 },
	{ 0x0800A586u, 0x0DC0, 0x0D80 },
	{ 0x0800A7F2u, 0x0A41, 0x0A81 },
	{ 0x0800A7F4u, 0x05C0, 0x0580 },
	{ 0x0800A7F6u, 0x0DC0, 0x0D80 },
	{ 0x0800A814u, 0x05C0, 0x0580 },
	{ 0x0800A816u, 0x0DC0, 0x0D80 },
	{ 0x0800AFD4u, 0x0A60, 0x0AA0 },
	{ 0x0800AFDEu, 0x05E0, 0x05A0 },
	{ 0x0800AFE0u, 0x0DC0, 0x0D80 },
	{ 0x0800B00Cu, 0x05E0, 0x05A0 },
	{ 0x0800B00Eu, 0x0DC0, 0x0D80 },
	{ 0x0800B03Cu, 0x0A60, 0x0AA0 },
	{ 0x0800B046u, 0x05E0, 0x05A0 },
	{ 0x0800B048u, 0x0DC0, 0x0D80 },
	{ 0x0800B074u, 0x0A61, 0x0AA1 },
	{ 0x0800B076u, 0x05E0, 0x05A0 },
	{ 0x0800B078u, 0x0DC0, 0x0D80 },
	{ 0x0801C050u, 0x05C0, 0x0580 },
	{ 0x0801C052u, 0x0DC0, 0x0D80 },
	{ 0x0801D348u, 0x05C0, 0x0580 },
	{ 0x0801D34Au, 0x0DC0, 0x0D80 },
	{ 0x0801D3E4u, 0x01FF, 0x03FF },
	{ 0x08021AE2u, 0x0A41, 0x0A81 },
	{ 0x08021AECu, 0x01FF, 0x03FF },
	{ 0x08021C2Eu, 0x0A7F, 0x0ABF },
	{ 0x08021C64u, 0x01FF, 0x03FF },
	{ 0x080269B4u, 0x01FF, 0x03FF },
	{ 0x08027178u, 0x05C0, 0x0580 },
	{ 0x0802717Au, 0x0DC0, 0x0D80 },
	{ 0x08027504u, 0x01FF, 0x03FF },
	{ 0x0802769Au, 0x05C0, 0x0580 },
	{ 0x0802769Cu, 0x0DC0, 0x0D80 },
	{ 0x080276BAu, 0x05C0, 0x0580 },
	{ 0x080276BCu, 0x0DC0, 0x0D80 },
	{ 0x080277CAu, 0x05C0, 0x0580 },
	{ 0x080277CCu, 0x0DC0, 0x0D80 },
	{ 0x08027D12u, 0x05C9, 0x0589 },
	{ 0x08027D14u, 0x0DC9, 0x0D89 },
	{ 0x08027D36u, 0x05C9, 0x0589 },
	{ 0x08027D38u, 0x0DC9, 0x0D89 },
	{ 0x08028084u, 0x05C0, 0x0580 },
	{ 0x08028086u, 0x0DC0, 0x0D80 },
	{ 0x080282A4u, 0x0A40, 0x0A80 },
	{ 0x08028308u, 0x0A40, 0x0A80 },
	{ 0x080283ACu, 0x01FF, 0x03FF },
	{ 0x0802855Cu, 0x0A40, 0x0A80 },
	{ 0x08028688u, 0x01FF, 0x03FF },
	{ 0x08028E78u, 0x0A41, 0x0A81 },
	{ 0x08028E7Au, 0x05C0, 0x0580 },
	{ 0x08028E7Cu, 0x0DC0, 0x0D80 },
	{ 0x08028EEEu, 0x0A41, 0x0A81 },
	{ 0x08028EF0u, 0x05C0, 0x0580 },
	{ 0x08028EF2u, 0x0DC0, 0x0D80 },
	{ 0x08028FF4u, 0x05C0, 0x0580 },
	{ 0x08028FF6u, 0x0DC0, 0x0D80 },
	{ 0x08029108u, 0x01FF, 0x03FF },
	{ 0x08029168u, 0x0A41, 0x0A81 },
	{ 0x0802916Au, 0x05C0, 0x0580 },
	{ 0x0802916Cu, 0x0DC0, 0x0D80 },
	{ 0x0802953Au, 0x0A4A, 0x0A8A },
	{ 0x0802953Cu, 0x05C9, 0x0589 },
	{ 0x0802953Eu, 0x0DC9, 0x0D89 },
	{ 0x0802A00Cu, 0x05C0, 0x0580 },
	{ 0x0802A00Eu, 0x0DC0, 0x0D80 },
	{ 0x0802A3B2u, 0x05C0, 0x0580 },
	{ 0x0802A3B4u, 0x0DC0, 0x0D80 },
	{ 0x0802A516u, 0x05C0, 0x0580 },
	{ 0x0802A518u, 0x0DC0, 0x0D80 },
	{ 0x0802A55Cu, 0x3785, 0x6D85 },
	{ 0x0802B708u, 0x027F, 0x02BF },
	{ 0x0802B846u, 0x05C2, 0x0582 },
	{ 0x0802B848u, 0x0DD2, 0x0D92 },
	{ 0x0802B850u, 0x0A41, 0x0A81 },
	{ 0x0810B466u, 0x05C4, 0x0584 },
	{ 0x0810B468u, 0x0DE4, 0x0DA4 },
	{ 0x0810BA32u, 0x05C4, 0x0584 },
	{ 0x0810BA34u, 0x0DE4, 0x0DA4 },
	{ 0x0810C88Au, 0x05C4, 0x0584 },
	{ 0x0810C88Cu, 0x0DE4, 0x0DA4 },
	{ 0x0810D102u, 0x05C4, 0x0584 },
	{ 0x0810D104u, 0x0DE4, 0x0DA4 },
	{ 0x0810D5C0u, 0x05C4, 0x0584 },
	{ 0x0810D5C2u, 0x0DE4, 0x0DA4 },
	{ 0x0810DB5Eu, 0x05C4, 0x0584 },
	{ 0x0810DB60u, 0x0DE4, 0x0DA4 },
	{ 0x0810E288u, 0x05C4, 0x0584 },
	{ 0x0810E28Au, 0x0DE4, 0x0DA4 },
	{ 0x0810EE2Eu, 0x05C4, 0x0584 },
	{ 0x0810EE30u, 0x0DE4, 0x0DA4 },
	{ 0x0810F688u, 0x05C4, 0x0584 },
	{ 0x0810F68Au, 0x0DE4, 0x0DA4 },
	{ 0x081104F4u, 0x05C4, 0x0584 },
	{ 0x081104F6u, 0x0DE4, 0x0DA4 },
	{ 0x081132ECu, 0x05C4, 0x0584 },
	{ 0x081132EEu, 0x0DE4, 0x0DA4 },
	{ 0x0811393Eu, 0x05C4, 0x0584 },
	{ 0x08113940u, 0x0DE4, 0x0DA4 },
	{ 0x08114516u, 0x05C4, 0x0584 },
	{ 0x08114518u, 0x0DE4, 0x0DA4 },
	{ 0x08122502u, 0x024B, 0x028B },
	{ 0x081225C0u, 0x024B, 0x028B },
	{ 0x0812EA88u, 0x05C0, 0x0580 },
	{ 0x0812EA8Au, 0x0DC0, 0x0D80 },
	{ 0x0812EAB2u, 0x05C0, 0x0580 },
	{ 0x0812EAB4u, 0x0DC0, 0x0D80 },
	{ 0x0812EB8Eu, 0x05C0, 0x0580 },
	{ 0x0812EB90u, 0x0DC0, 0x0D80 },
	{ 0x0812EB98u, 0x0E89, 0x0EC9 },
	{ 0x0812EBA4u, 0x05C0, 0x0580 },
	{ 0x0812EBA6u, 0x0DC0, 0x0D80 },
	{ 0x0812EBAEu, 0x0E89, 0x0EC9 },
	{ 0x0812F3B4u, 0x05C0, 0x0580 },
	{ 0x0812F3B6u, 0x0DC0, 0x0D80 },
	{ 0x0812F3BCu, 0x0E89, 0x0EC9 },
	{ 0x0812F64Cu, 0x0240, 0x0280 },
	/* (then, by hand, not the census's: the names' and descriptions'
	 * routines, BN6_CHIP_NAME_OF and BN6_CHIP_DESC_OF, taking an id's
	 * archive as id >> 8 of four (CHIPS_TABLES) where BN6 picks one of two
	 * by id > 255: cmp r1,#0xff; ble 1f; adds r0,#4; subs r1,#0xff; subs
	 * r1,#1; b 0b; 1: ldr r0,[r0]; mov pc,lr, over its padding, and its
	 * literal pointed at the four words) */
	ARCHIVE_LOOP(BN6_CHIP_NAME_OF), ARCHIVE_WORD(BN6_CHIP_NAME_OF, 0x08027D2Cu, CHIPS_TABLES),
	ARCHIVE_LOOP(BN6_CHIP_DESC_OF), ARCHIVE_WORD(BN6_CHIP_DESC_OF, 0x08027D50u, CHIPS_TABLES + 0x10u),
	/* (the folder editor's items, id << 6 | code in a halfword where BN6 has id << 7 | code, ids to 511: an item made,
	 * its id and its code taken, tools/chip_ids.py --table's second part, from the census's EDITOR_KEYS) */
	{ 0x08121AC4u, 0x01C4, 0x0184 },
	{ 0x08121B00u, 0x01C2, 0x0182 },
	{ 0x08121CB4u, 0x01DB, 0x019B },
	{ 0x08121CC2u, 0x01DB, 0x019B },
	{ 0x08121EE8u, 0x09C9, 0x0989 },
	{ 0x08121F94u, 0x09C9, 0x0989 },
	{ 0x08122192u, 0x09C0, 0x0980 },
	{ 0x081221E6u, 0x09C0, 0x0980 },
	{ 0x0812225Au, 0x09C8, 0x0988 },
	{ 0x08122260u, 0x247F, 0x243F },
	{ 0x081222B4u, 0x09C8, 0x0988 },
	{ 0x08122408u, 0x09C8, 0x0988 },
	{ 0x081224BCu, 0x0DC8, 0x0D88 },
	{ 0x081224C2u, 0x0249, 0x0289 },
	{ 0x081224C4u, 0x0E49, 0x0E89 },
	{ 0x081224F2u, 0x0DC8, 0x0D88 },
	{ 0x081224F4u, 0x0249, 0x0289 },
	{ 0x081224F6u, 0x0E49, 0x0E89 },
	{ 0x081225B6u, 0x0DC8, 0x0D88 },
	{ 0x081225B8u, 0x0249, 0x0289 },
	{ 0x081225BAu, 0x0E49, 0x0E89 },
	{ 0x0812F396u, 0x0DC0, 0x0D80 },
	{ 0x0812F398u, 0x217F, 0x213F },
	{ 0x0812F646u, 0x09C1, 0x0981 },
	{ 0x0812F648u, 0x227F, 0x223F },
	{ 0x08135EFAu, 0x0DC0, 0x0D80 },
	{ 0x08135EFCu, 0x217F, 0x213F },
	{ 0x081367DAu, 0x09C0, 0x0980 },
	{ 0x08136936u, 0x09C9, 0x0989 },
	{ 0x0813693Cu, 0x09D2, 0x0992 },
	{ 0x08136B8Eu, 0x09C9, 0x0989 },
	{ 0x08136F24u, 0x0DD2, 0x0D92 },
	{ 0x08136F7Au, 0x0DC0, 0x0D80 },
	{ 0x08136FB2u, 0x0DC0, 0x0D80 },
	{ 0x081372FCu, 0x09C0, 0x0980 },
	/* (by hand: the six readers that take a name's archive from a table of two words by the id's high byte, each one's
	 * literal pointed at the four; the folder editor's two load an empty entry's archive from their table's third word,
	 * the ninth here; and the two readers of a description that pick one of two, DESC_READ) */
	WORD(BN6_CHIP_NAMES_REF_RESULTS, BN6_CHIP_NAMES_REF_RESULTS + 4u, CHIPS_TABLES),
	WORD(BN6_CHIP_NAMES_REF_LINE, BN6_CHIP_NAMES_REF_LINE + 4u, CHIPS_TABLES),
	WORD(BN6_CHIP_NAMES_REF_LIST, BN6_CHIP_NAMES_REF_LIST + 4u, CHIPS_TABLES),
	WORD(BN6_CHIP_NAMES_REF_PACK, BN6_CHIP_NAMES_REF_PACK + 4u, CHIPS_TABLES),
	WORD(BN6_CHIP_NAMES_REF_PACK2, BN6_CHIP_NAMES_REF_PACK2 + 4u, CHIPS_TABLES),
	WORD(BN6_CHIP_NAMES_REF_LIBRARY, BN6_CHIP_NAMES_REF_LIBRARY + 4u, CHIPS_TABLES),
	{ BN6_CHIP_NAMES_NONE, 0x6880, 0x6A00 }, { BN6_CHIP_NAMES_NONE2, 0x6880, 0x6A00 },   /* (ldr r0,[r0,#8] to #0x20) */
	DESC_READ(BN6_CHIP_DESC_READ, 0x484C, 0x4849, 0x484B),
	DESC_READ(BN6_CHIP_DESC_READ2, 0x484F, 0x484C, 0x484E),
};
#define NCHANGES (int)(sizeof changes / sizeof *changes)

/* the preset folders GiveFolder copies (docs/rom/chips.md, Chip folders) */
static const uint32_t presets[] = {
	BN6_PRESET_FOLDERS, BN6_PRESET_FOLDERS + 0x3Cu,
	BN6_PRESET_FOLDERS2, BN6_PRESET_FOLDERS2 + 0x3Cu, BN6_PRESET_FOLDERS2 + 0x78u, BN6_PRESET_FOLDERS2 + 0xB4u,
};
#define NPRESETS (int)(sizeof presets / sizeof *presets)

static void put16(uint32_t a, uint16_t v) {
	const uint8_t b[2] = { (uint8_t)v, (uint8_t)(v >> 8) };
	emu_write(a, b, 2);
}

/* every change in one state: 0 BN6's (the core's copy and the player's
 * file), 1 ours, -1 neither or mixed */
static int changes_state(void) {
	int own = 0, ours = 0;
	for (int i = 0; i < NCHANGES; ++i) {
		uint16_t now = emu_read16(changes[i].at), file = rom_u16(changes[i].at - BUS);
		if (now == changes[i].was && file == changes[i].was) ++own;
		else if (now == changes[i].now) ++ours;
		else {
			fprintf(stderr, "chips: ten-bit ids not installed: %08x is %04x, neither BN6's nor ours\n", (unsigned)changes[i].at, now);
			return -1;
		}
	}
	return own == NCHANGES ? 0 : ours == NCHANGES ? 1 : -1;
}

/* The ownership marks and their keys hold ids to 511 (docs/rom/chips.md,
 * Chips owned): a chip past them is neither marked (BN6_PACK_MARK_STORE's
 * store skipped) nor taken for a cheat's (BN6_PACK_MARK_TEST's compare
 * made equal), where BN6 would write and read the bytes past the tables */
static HookAct mark_store(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] < BN6_PACK_EXT_IDS) return HOOK_CONTINUE;
	r->r[12] = (BN6_PACK_MARK_STORE + 2u) | 1u;   /* (on to its pop) */
	return HOOK_JUMP;
}

static HookAct mark_test(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] >= BN6_PACK_EXT_IDS) r->r[1] = r->r[2];
	return HOOK_CONTINUE;
}

/* A chip past 511 given: BN6 would set flag BN6_FLAG_LIBRARY + id, past
 * the Library's own 512 in another block of flags. It sets the held id's
 * instead (XREG_HELD, no chip's: nothing reads it), and the chip is told
 * to the director, which keeps the Library (r0 the id). */
static int given_kind;

void xchips_wide_given(int kind) { given_kind = kind; }

static HookAct library_flag(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] < BN6_FLAG_LIBRARY + BN6_FLAG_LIBRARY_IDS) return HOOK_CONTINUE;
	HookRegs told = *r;
	told.r[0] -= BN6_FLAG_LIBRARY;
	if (given_kind) hook_post(&told, given_kind);
	r->r[0] = BN6_FLAG_LIBRARY + XREG_HELD;
	return HOOK_CONTINUE;
}

/* A Program Advance past nine bits, as BN6 tests and sets its per-battle
 * bit (BN6's own bits hold ids to 383, and from 512 the byte it would
 * take is another's): its bit in the engine's own word, side 0's (side 1
 * forms from the link battles' list, BN6's own); formed already this
 * battle, r0 0, as BN6 answers */
static HookAct pa_bit(HookRegs *r, void *user) {
	(void)user;
	uint32_t id = r->r[0], bit = id - BN6_PA_WIDE_FIRST;
	if (id < BN6_PACK_WIDE_FIRST) return HOOK_CONTINUE;
	if (r->r[1] == 0 && bit < BN6_PA_WIDE_COUNT) {
		uint32_t formed = hook_read32(RAM_PA_WIDE);
		if (formed >> bit & 1u) r->r[0] = 0;
		else hook_write32(RAM_PA_WIDE, formed | 1u << bit);
	}
	return HOOK_RETURN;
}

/* The chat box's chip name: its command's kind 1 takes the names' archive
 * as its table's entry 1 plus the id's high byte, entries 3 and 4 other
 * texts' archives; an id from 512 its own archive of the four */
static HookAct chat_name(HookRegs *r, void *user) {
	(void)user;
	uint32_t kind = hook_read8(r->r[4] + 3) & 0xFu, at = r->r[2] >> 2;
	if (kind == BN6_CHAT_CHIP_NAME && at >= kind + 2u && at <= kind + 3u) r->r[0] = CHIPS_TABLES - 4u * kind;
	return HOOK_CONTINUE;
}

/* A reward's wide form (chip_entry.h) at BN6's split, where it takes one
 * for HP or BugFrags: a chip (r0 its kind), its id (r1) and code (r2) */
static HookAct reward_wide(HookRegs *r, void *user) {
	(void)user;
	uint16_t v = (uint16_t)r->r[0];
	if (!(v & REWARD_WIDE)) return HOOK_CONTINUE;
	r->r[0] = REWARD_CHIP;
	r->r[1] = reward_chip_id(v);
	r->r[2] = reward_chip_code(v);
	return HOOK_RETURN;   /* (as its mov pc,lr: the split pushes nothing, and every caller came by bl) */
}

bool xchips_wide_install(void) {
	if (CHIP_ID_BITS == 9) return true;
	int state = changes_state();
	if (state == 1) return true;
	if (state < 0) return false;
	claims_begin("ten-bit chip ids (xchips_wide.c)");
	for (int i = 0; i < NCHANGES; ++i) put16(changes[i].at, changes[i].now);
	for (int f = 0; f < NPRESETS; ++f)
		for (uint32_t k = 0; k < BN6_FOLDER_ENTRIES; ++k) {
			uint32_t a = presets[f] + 2 * k;
			put16(a, chip_entry_from9(rom_u16(a - BUS)));
		}
	/* (a core's own hooks, set with its copy's changes) */
	if (!emu_hook(BN6_PACK_MARK_STORE, mark_store, NULL) || !emu_hook(BN6_PACK_MARK_TEST, mark_test, NULL))
		fprintf(stderr, "chips: no hooks on the marks: a chip past 511 is taken for a cheat's\n");
	if (!emu_hook(BN6_GIVE_CHIPS_LIBRARY, library_flag, NULL) || !emu_hook(BN6_GIVE_FOLDER_LIBRARY, library_flag, NULL) ||
		!emu_hook(BN6_CHAT_NAME_ARCHIVE, chat_name, NULL) || !emu_hook(BN6_REWARD_SPLIT_HP, reward_wide, NULL) ||
		!emu_hook(BN6_PA_FORMED_FLAG, library_flag, NULL) || !emu_hook(BN6_PA_BATTLE_BIT, pa_bit, NULL))
		fprintf(stderr, "chips: no hooks for a chip past 511: given it sets another's flag, a chat names and a battle rewards another\n");
	claims_end();
	if (emu_debug_on()) fprintf(stderr, "chips: ten-bit ids, %d of BN6's halfwords and %d preset folders\n", NCHANGES, NPRESETS);
	return true;
}
