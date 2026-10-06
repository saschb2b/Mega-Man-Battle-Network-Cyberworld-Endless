/* The run's chips in the game: its folder and pack as BN6 keeps them, the
 * starting folder, the All * helper's records, the Library and the programs
 * found carried between the profile and the game, and the BugFrag
 * Trader's trade. */
#include "director_folder.h"

#include <string.h>

#include "bytes.h"
#include "chip_pool.h"
#include "data.h"
#include "devtools.h"
#include "director.h"
#include "director_board.h"
#include "director_see.h"
#include "director_state.h"
#include "emu.h"
#include "gamecall.h"
#include "loot.h"
#include "meta.h"
#include "save.h"
#include "save_blob.h"
#include "trader.h"

/* The run's folder as the game holds it (30 entries, chip | code << 9;
 * zeros where its data is not there) */
void folder_now(uint16_t folder[BN6_FOLDER_ENTRIES]) {
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	bool there = data >= BN6_EWRAM && data < BN6_EWRAM_END;
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i) folder[i] = there ? emu_read16(data + 2u * (uint32_t)i) : 0;
}

/* The folder as the layer was made: its codes are the run's (run.codes),
 * its copies of each chip beside the checkpoint ("run.folder"), so a
 * CONTINUE makes the same stock before the game's memory is back. */
static uint16_t folder_made[BN6_FOLDER_ENTRIES];
#define FOLDER_MADE_MAGIC 0x43464C44u   /* "CFLD" */

/* ... saved beside the checkpoint, and back on CONTINUE (none for a run
 * saved before it was kept, which made its stock without it) */
void folder_made_save(void) { save_write_blob("run.folder", FOLDER_MADE_MAGIC, folder_made, sizeof folder_made); }

void folder_made_load(void) {
	if (!save_read_blob("run.folder", FOLDER_MADE_MAGIC, folder_made, sizeof folder_made)) memset(folder_made, 0, sizeof folder_made);
	loot_folder_counts(folder_made, BN6_FOLDER_ENTRIES);
}

/* The folder's codes, for the layer about to be made (loot_fit_code): read
 * from the game as MegaMan moves on, kept with the run, so a checkpoint
 * rebuilds the layer as it was without the game's memory; and the
 * NaviCust's board, which the guardian's draft fits its programs beside;
 * and the Library, which a vault's lock counts (as the checkpoint after
 * keeps it, which a CONTINUE's rebuild reads). */
void note_folder_codes(void) {
	library_from_game();
	own_folder_chips();
	memset(run.programs, 0, sizeof run.programs);
	board_programs(run.programs, (int)sizeof run.programs);
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	if (data < BN6_EWRAM || data >= BN6_EWRAM_END) return;
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i) folder_made[i] = emu_read16(data + 2u * (uint32_t)i);
	loot_folder_codes(folder_made, BN6_FOLDER_ENTRIES, run.codes);
	loot_folder_counts(folder_made, BN6_FOLDER_ENTRIES);
}

/* The folder's chips owned, as BN6 marks a chip it gives (bn6f
 * encryption_applyPack, Gregar 0x08006E70): its byte in the table at
 * Toolkit+0x7C is its key (0x020008A0 + chip) XOR 0x17, and a chip whose
 * byte does not match is taken for a cheat's and drawn blank in battle, no
 * name and no effect. A chip only written into the folder had none: a
 * playtester's LongSwrd, PanlGrab and Barrier came up blank in every hand,
 * where the Standard folder's chips, given at NEW GAME, played (session
 * 30). Every fresh layer and CONTINUE mark the folder's chips again, so a
 * run saved before this is mended. */
void own_folder_chips(void) {
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS), marks = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIP_MARKS);
	if (data < BN6_EWRAM || data >= BN6_EWRAM_END || marks < BN6_EWRAM || marks >= BN6_EWRAM_END) return;
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		int id = emu_read16(data + 2u * (uint32_t)i) & 0x1FF;
		if (id > 0) emu_write8(marks + (uint32_t)id, (uint8_t)(emu_read8(BN6_CHIP_KEYS + (uint32_t)id) ^ BN6_CHIP_KEY_XOR));
	}
}

/* --dev pack=N: a copy of each of the first N chips in the pack, in its
 * first code (one more: outside the folder editor the counts hold the
 * folder's copies too), each owned as BN6 marks one it gives
 * (own_folder_chips: BN6's count of a chip unmarked reads 0, and a
 * request's hand-over found none) */
static void dev_pack(void) {
	uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK), marks = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIP_MARKS);
	if (dev.pack <= 0 || pack < BN6_EWRAM || pack + CHIP_PACK_ENTRY * PACK_CHIPS >= BN6_EWRAM_END) return;
	for (uint32_t id = 1; (int)id <= dev.pack && id < PACK_CHIPS; ++id) {
		int c = emu_read8(pack + CHIP_PACK_ENTRY * id);
		if (c < 99) emu_write8(pack + CHIP_PACK_ENTRY * id, (uint8_t)(c + 1));
		if (marks >= BN6_EWRAM && marks < BN6_EWRAM_END) emu_write8(marks + id, (uint8_t)(emu_read8(BN6_CHIP_KEYS + id) ^ BN6_CHIP_KEY_XOR));
	}
}

/* (dev: folder=ID, the run's folder all chip ID in *, owned as the folder's
 * chips are: a capture of one chip's battles, the first hand all of it;
 * folder=ID/N its first N entries; pack=N, dev_pack) */
void dev_folder(void) {
	dev_pack();
	if (dev.zenny > 0) game_call(BN6_GIVE_ZENNY, (uint32_t)dev.zenny, 0);
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	if (dev.folder <= 0 || dev.folder >= 0x200 || data < BN6_EWRAM || data >= BN6_EWRAM_END) return;
	uint8_t e[2] = { (uint8_t)dev.folder, (uint8_t)(dev.folder >> 8 | CHIP_CODE_STAR << 1) };
	for (uint32_t i = 0; i < BN6_FOLDER_ENTRIES && (int)i < dev.folder_n; ++i) emu_write(data + 2 * i, e, sizeof e);
	own_folder_chips();
}

/* A folder entry's count in the pack (bn6f getOffsetToQuantityOfChipCode:
 * the code's place among the chip record's four, else the first) */
static uint32_t pack_count_at(uint32_t pack, int entry) {
	int id = entry & 0x1FF, code = entry >> 9, slot = 0;
	uint32_t rec = R.layout->chip_data + (uint32_t)id * 0x2C;
	for (int k = 0; k < 4; ++k) if (R.data[rec + (uint32_t)k] == code) { slot = k; break; }
	return pack + 12u * (uint32_t)id + (uint32_t)slot;
}

/* The run's starting folder in the game's first folder (docs/META.md): the
 * chosen one's 30 chips over the game's own (Standard keeps those), as
 * BN6's GiveFolder copies a folder in (bn6f sub_8021AB4). */
void set_start_folder(void) {
	const uint16_t *chips = meta_folder_chips(run.folder);
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS), pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	if (!chips || data < BN6_EWRAM || data >= BN6_EWRAM_END) return;
	/* the pack as if this folder had been given at NEW GAME, not the
	 * Standard one (GiveFolder counts a folder's chips in the pack): a
	 * playtester's Blade run kept the Standard folder's CrakShot and
	 * Cannons as spares, and the folder's promise with them (session 31) */
	bool counts = pack >= BN6_EWRAM && pack < BN6_EWRAM_END && R.data;
	for (int i = 0; i < BN6_FOLDER_ENTRIES && counts; ++i) {
		uint32_t at = pack_count_at(pack, emu_read16(data + 2u * (uint32_t)i));
		int n = emu_read8(at);
		if (n > 0) emu_write8(at, (uint8_t)(n - 1));
	}
	for (int i = 0; i < BN6_FOLDER_ENTRIES && counts; ++i) {
		uint32_t at = pack_count_at(pack, chips[i]);
		int n = emu_read8(at);
		if (n < 99) emu_write8(at, (uint8_t)(n + 1));
	}
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		uint8_t b[2] = { (uint8_t)chips[i], (uint8_t)(chips[i] >> 8) };
		emu_write(data + 2u * (uint32_t)i, b, 2);
		flag_set(BN6_FLAG_LIBRARY + (chips[i] & 0x1FF));   /* (in the Library, as GiveFolder puts them) */
	}
	own_folder_chips();
}

/* The All * helper in the game (docs/META.md, issue #18): every chip's
 * record holds * alone in the core's ROM copy, so the pack keeps a chip in
 * * whatever code it was given in (bn6f getOffsetToQuantityOfChipCode
 * counts a code the record lacks as its first), the folder's EDIT moves it
 * in in *, and the check of a Program Advance of one chip in codes in a
 * row takes all three in * (BN6's own takes one). Without it the ROM's own
 * again, where a run before had it: the copy lasts the session. */
void star_records(bool on) {
	uint32_t first = R.layout->chip_data + 0x2Cu;   /* (Cannon's: A B C *) */
	uint16_t limit = rom_u16(BN6_PA_STAR_LIMIT - 0x08000000u);
	if (on && limit == 0x2A01) limit = BN6_PA_STAR_ANY;
	if (emu_read8(0x08000000u + first) == (on ? CHIP_CODE_STAR : R.data[first]) && emu_read16(BN6_PA_STAR_LIMIT) == limit) return;
	static const uint8_t star[4] = { CHIP_CODE_STAR, 0xFF, 0xFF, 0xFF };
	for (uint32_t id = 1; id < PACK_CHIPS; ++id) {
		uint32_t rec = R.layout->chip_data + 0x2Cu * id;
		emu_write(0x08000000u + rec, on ? star : R.data + rec, sizeof star);
	}
	uint8_t b[2] = { (uint8_t)limit, (uint8_t)(limit >> 8) };
	emu_write(BN6_PA_STAR_LIMIT, b, sizeof b);
}

/* (before the game's own NEW GAME, which counts the folder it gives in the
 * pack by the records, and a boot state keeps it) */
void director_before_boot(void) { star_records(false); }

/* ... and the run's folder and pack in * as it begins (the folder it
 * brought, the pack BN6's NEW GAME counted it in): each entry's code *,
 * each chip's counts in its one code's */
void star_folder_pack(void) {
	if (!run_all_star()) return;
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS), pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	for (uint32_t i = 0; data >= BN6_EWRAM && data < BN6_EWRAM_END && i < BN6_FOLDER_ENTRIES; ++i) {
		uint16_t e = chip_entry_star(emu_read16(data + 2 * i));
		uint8_t b[2] = { (uint8_t)e, (uint8_t)(e >> 8) };
		emu_write(data + 2 * i, b, sizeof b);
	}
	if (pack < BN6_EWRAM || pack + CHIP_PACK_ENTRY * PACK_CHIPS >= BN6_EWRAM_END) return;
	for (uint32_t id = 1; id < PACK_CHIPS; ++id) {
		uint8_t e[CHIP_PACK_ENTRY], was[CHIP_PACK_ENTRY];
		for (uint32_t k = 0; k < CHIP_PACK_ENTRY; ++k) e[k] = was[k] = emu_read8(pack + CHIP_PACK_ENTRY * id + k);
		chip_pack_star(e);
		if (memcmp(e, was, sizeof e)) emu_write(pack + CHIP_PACK_ENTRY * id, e, sizeof e);
	}
}

/* The profile's Library (docs/META.md) in the run's game: the PET's Library
 * shows every chip held in any run, and a Chip Trader's prize, new to the
 * Library first, is new across runs. */
void library_to_game(void) {
	for (int id = 1; id < 8 * (int)sizeof profile.library; ++id)
		if (meta_library_has(id)) flag_set(BN6_FLAG_LIBRARY + id);
}

/* ... and back: the chips the run's game has put in its Library since. */
void library_from_game(void) {
	bool added = false;
	/* (a run begun by a build before the Library: its count starts here,
	 * not at 0; a playtester's chips of earlier runs read "+32" at its end) */
	if (run.active && profile.library_run != run.seed) {
		profile.library_start = (uint16_t)meta_library_count(-1);
		profile.library_run = run.seed;
		added = true;
	}
	for (int id = 1; id < 8 * (int)sizeof profile.library; ++id)
		if (chip_pool_class(id) >= 0 && flag_get(BN6_FLAG_LIBRARY + id)) added |= meta_library_add(id);
	if (added) profile_save();
}

/* ... and the programs MegaMan has, into the programs found, which later
 * runs' NaviCust vendors keep (docs/NAVICUST.md, 7). */
void programs_from_game(void) {
	uint8_t now[10];
	int n = board_programs(now, (int)sizeof now);
	bool added = false;
	for (int i = 0; i < n; ++i) {
		int p = now[i] / 4;
		if (p <= 0 || p >= 64 || shop_program_found(p)) continue;
		profile.programs_found[p / 8] |= (uint8_t)(1u << (p % 8));
		added = true;
	}
	if (added) profile_save();
}

void director_folder_now(uint16_t *folder) { folder_now(folder); }

int director_pack_now(uint16_t *entry, uint8_t *count, int most) {
	uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	if (pack < BN6_EWRAM || pack + CHIP_PACK_ENTRY * PACK_CHIPS >= BN6_EWRAM_END) return 0;
	int n = 0;
	for (uint32_t id = 1; id < PACK_CHIPS && n < most; ++id) {
		ChipInfo ci;
		bool known = false;
		for (uint32_t k = 0; k < 4 && n < most; ++k) {
			int c = emu_read8(pack + CHIP_PACK_ENTRY * id + k);
			if (!c) continue;
			if (!known) chip_info((int)id, &ci);
			known = true;
			int code = (int)k < ci.ncodes ? (ci.codes[k] == '*' ? 26 : ci.codes[k] - 'A') : 26;
			entry[n] = (uint16_t)(id | (unsigned)code << 9);
			count[n++] = (uint8_t)c;
		}
	}
	return n;
}

/* A BugFrag Trader's trade (issue #12). After Yes, BN6's script holds
 * (ts_wait_hold) for the trader machine on the Undernet's map, which rolls
 * the prize, gives it, takes the ten BugFrags, saves and runs the script
 * that shows it (bn6f sub_809A078); after No it closes the box and holds
 * for the machine's scene to end the chat. A layer's trader stands without
 * the machine: its chat held for good after Yes, and after No MegaMan
 * walked with the chat still open, the PET shut. The director does what
 * the machine does where the chat holds, but takes the ten BugFrags before
 * it gives: the script's "not enough" line holds as its Yes does, and
 * read as a Yes it gave a chip for none, TakeBugfrags taking nothing
 * (issue #21: A mashed through "Try again?" traded on at 0 BugFrags). */
void bugfrag_trade(void) {
	static bool howl;
	if (!emu_read8(BN6_CHATBOX)) {
		/* (the machine clears its howl as it shows the prize) */
		if (howl) flag_clear(BN6_FLAG_TRADER_HOWL);
		howl = false;
		return;
	}
	uint32_t f = emu_read32(BN6_CHATBOX_FLAGS);
	if (D.objs.trader_kind != TRADER_BUGFRAG || !(f & 0x80) || !(f & 0x08) ||
	    emu_read32(BN6_CHATBOX_ARCHIVE) != BN6_TRADER_TEXT) return;
	/* (after No the box is closed: the chat ends as the Chip Trader's No
	 * ends it, with its script 5, a bare end) */
	if (f & 7) { game_call(BN6_CHAT_RUN_SCRIPT, BN6_TRADER_TEXT, 5); return; }
	uint32_t prize[2], took[2];
	game_call(BN6_TRADER_RESET, 0, 0);
	if (!game_call_ret(BN6_TRADER_PRIZE, 0, 0, 0, prize)) return;
	/* (TakeBugfrags: 0 where it took them, else it took none) */
	if (!game_call_ret(BN6_TAKE_BUGFRAGS, 10, 0, 0, took) || took[0]) {
		game_call(BN6_CHAT_RUN_SCRIPT, BN6_TRADER_TEXT, 5);
		return;
	}
	uint16_t chip = (uint16_t)prize[0], code = (uint16_t)prize[1];
	uint8_t v[4];
	put16(v, chip);
	put16(v + 2, code);
	emu_write(BN6_TRADER_STATE_PRIZE, v, 4);
	game_call_ret(BN6_GIVE_CHIPS, chip, code, 1, NULL);
	/* (the map saved on) */
	put16(v, emu_read16(BN6_MAP_ID));
	emu_write(BN6_LAST_MAP, v, 2);
	game_call(BN6_SAVE_GAME, 0, 0);
	/* (the script names the prize from the chat box's two words) */
	emu_write32(BN6_CHATBOX_WORD0, chip);
	emu_write32(BN6_CHATBOX_WORD1, code);
	game_call(BN6_CHAT_RUN_SCRIPT, BN6_TRADER_TEXT, 15);
	memset(v, 0, sizeof v);
	emu_write(BN6_TRADER_STATE_PRIZE, v, 4);
	emu_write(BN6_TRADER_STATE_30, v, 2);
	howl = true;
}
