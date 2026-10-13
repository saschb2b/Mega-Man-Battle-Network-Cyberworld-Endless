/* The NaviCust's board watched (docs/NAVICUST.md): the programs placed
 * and left off, what fits, its bugs as the RUN leaves them, the Spins and
 * the TagChip system the profile holds, compression codes and RegUps. */
#include "director_board.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "board_words.h"
#include "boss.h"
#include "cinema.h"
#include "devtools.h"
#include "director.h"
#include "director_folder.h"
#include "director_state.h"
#include "emu.h"
#include "meta.h"
#include "save.h"
#include "save_blob.h"
#include "talk.h"

static bool free_to_speak(void);

/* The programs seen on the board since the run went on (a variant a bit):
 * one taken off since was the player's choice, and MegaMan told a
 * playtester to install the SlipRunr he had just taken off, three times
 * (session 63). And those MegaMan has said all about, off the board:
 * after that, L's word is a line, and only where it fits as the board
 * stands (another heard the same five boxes on HP+100, which needed a
 * Spin, five times in two layers, after the PET, in L's words and after
 * each battle). A new run forgets both; a CONTINUE takes them back. */
static uint8_t placed_seen[47 * 4 / 8 + 1];
static uint8_t off_explained[47 * 4 / 8 + 1];

static bool bit_of(const uint8_t *set, int v) { return set[v >> 3] >> (v & 7) & 1; }
static void bit_set(uint8_t *set, int v) { set[v >> 3] |= (uint8_t)(1 << (v & 7)); }

/* Whether MegaMan has said all about program variant v off the board */
bool off_board_explained(int v) { return bit_of(off_explained, v); }
void off_board_explain(int v) { bit_set(off_explained, v); }

static void placed_note(void) {
	for (int e = 0; e < BN6_NAVICUST_PLACED_MAX; ++e) {
		int id = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)e * 8);
		if (!id) break;
		if (id > 0 && id < 47 * 4) bit_set(placed_seen, id);
	}
}

void off_board_forget(void) {
	no_room_forget();
	memset(placed_seen, 0, sizeof placed_seen);
	memset(off_explained, 0, sizeof off_explained);
}

/* ... kept beside the checkpoint, for the run whose seed it names: a
 * CONTINUE had forgotten both, and a playtester heard a program's whole
 * reminder again after each one (session 65) */
#define BOARD_NOTE_MAGIC 0x42524431u   /* "BRD1" */
typedef struct { uint32_t seed; uint8_t placed[sizeof placed_seen], explained[sizeof off_explained]; } BoardNote;
_Static_assert(sizeof(BoardNote) == 52 && offsetof(BoardNote, seed) == 0 && offsetof(BoardNote, placed) == 4 &&
	offsetof(BoardNote, explained) == 28, "Board save layout changed");

void off_board_save(void) {
	BoardNote b = { run.seed, { 0 }, { 0 } };
	memcpy(b.placed, placed_seen, sizeof b.placed);
	memcpy(b.explained, off_explained, sizeof b.explained);
	save_write_blob("run.board", BOARD_NOTE_MAGIC, &b, sizeof b);
}

void off_board_load(void) {
	BoardNote b;
	off_board_forget();
	if (!save_read_blob("run.board", BOARD_NOTE_MAGIC, &b, sizeof b) || b.seed != run.seed) return;
	memcpy(placed_seen, b.placed, sizeof placed_seen);
	memcpy(off_explained, b.explained, sizeof off_explained);
}

/* A program MegaMan has that is not on the NaviCust's board (the key
 * items count a program whether placed or not; the board's list holds the
 * placed ones): its name, "" for one outside the draft's pool, NULL for
 * none. A playtester played two acts believing a Guardian Data's UnderSht
 * was running. */
const char *program_off_board(int *variant) {
	static char name[16];
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	*variant = 0;
	for (int v = 4; v < 47 * 4; ++v) {
		int owned = emu_read8(items + BN6_PROGRAM_ITEMS + (uint32_t)v), placed = 0;
		if (!owned || bit_of(placed_seen, v)) continue;
		for (int e = 0; e < BN6_NAVICUST_PLACED_MAX; ++e) {
			int id = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)e * 8);
			if (!id) break;
			placed += id == v;
		}
		if (owned <= placed) continue;
		*variant = v;
		const char *about = navicust_about(v / 4);
		const char *colon = about ? strchr(about, ':') : NULL;
		snprintf(name, sizeof name, "%.*s", colon ? (int)(colon - about) : 0, colon ? about : "");
		return name;
	}
	return NULL;
}

/* The NaviCust's board as the game has it: the variants on it, then those
 * MegaMan has that are not, where they fit beside them (a program left
 * off because it cannot fit is left out); how many. */
int board_programs(uint8_t *out, int max) {
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	int n = 0, w, h;
	navicust_board(key_item(SCRIPTS_EXP_MEMORY), &w, &h);
	NaviShape s[10];
	int ns = 0;
	for (int e = 0; e < BN6_NAVICUST_PLACED_MAX && n < max; ++e) {
		int id = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)e * 8);
		if (!id) break;
		if (id >= 47 * 4) continue;
		out[n++] = (uint8_t)id;
		if (ns < 10 && navicust_shape(id, &s[ns])) ++ns;
	}
	if (items < BN6_EWRAM || items >= BN6_EWRAM_END) return n;
	for (int v = 4; v < 47 * 4 && n < max && ns < 10; ++v) {
		int owned = emu_read8(items + BN6_PROGRAM_ITEMS + (uint32_t)v), placed = 0;
		for (int i = 0; i < n; ++i) placed += out[i] == v;
		for (int k = placed; k < owned && n < max && ns < 10; ++k) {
			if (!navicust_shape(v, &s[ns]) || !navicust_pack(s, ns + 1, w, h)) break;
			out[n++] = (uint8_t)v;
			++ns;
		}
	}
	return n;
}

/* --dev programs=N: a copy of each of the first N programs, in the first
 * of its colours the ROM draws, given as BN6 gives one (its check beside
 * its count: the NaviCust lists no other); the NaviCustomizer's list, for
 * a capture */
void dev_programs(void) {
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS), check = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_CHECK);
	if (dev.programs <= 0 || items < BN6_EWRAM || items >= BN6_EWRAM_END || check < BN6_EWRAM || check >= BN6_EWRAM_END) return;
	for (int p = 1, given = 0; p < 47 && given < dev.programs; ++p)
		for (int v = p * 4; v < p * 4 + 4; ++v) {
			NaviShape s;
			if (!navicust_shape(v, &s) || !s.color) continue;
			uint32_t id = BN6_PROGRAM_ITEMS + (uint32_t)v;
			if (!emu_read8(items + id)) emu_write8(items + id, 1);
			emu_write8(check + id, (uint8_t)(emu_read8(BN6_KEY_ITEM_SEEDS + id) ^ 0x55));
			++given;
			break;
		}
}

/* A program variant's shape as the PET has it: compressed where its code
 * was entered (BN6's flags BN6_FLAG_COMPRESSED + variant, which the
 * NaviCust's shapes read; a playtester's two compressed Custom1 fit, and
 * MegaMan said the second would not: issue #54). Not the draft's, made
 * with the layer before a CONTINUE's state is back. */
static bool shape_now(int v, NaviShape *out) { return navicust_shape_as(v, flag_get(BN6_FLAG_COMPRESSED + v), out); }

/* Whether variant `v` fits the board beside the programs placed on it,
 * copies of it among them (a second Custom1 beside the first: issue #54),
 * compressed or not (fits_beside_placed: as the PET has it). */
bool fits_beside_as(int v, bool compressed) {
	int w, h, ns = 0;
	navicust_board(key_item(SCRIPTS_EXP_MEMORY), &w, &h);
	NaviShape s[10];
	for (int e = 0; e < BN6_NAVICUST_PLACED_MAX && ns < 9; ++e) {
		int id = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)e * 8);
		if (!id) break;
		if (shape_now(id, &s[ns])) ++ns;
	}
	if (!navicust_shape_as(v, compressed, &s[ns])) return true;
	return navicust_pack(s, ns + 1, w, h);
}

bool fits_beside_placed(int v) { return fits_beside_as(v, flag_get(BN6_FLAG_COMPRESSED + v)); }

/* The NaviCust's rotations (key items 0x50-0x55, one a colour: white,
 * yellow, pink, red, blue, green; "Lets you rotate white parts with the L
 * and R Button"): BN6 hands them out over its story; a run has those the
 * profile found in the net, one a run (docs/META.md), and no other: a
 * program of another colour lies as its record draws it, and the drafts
 * offer only what fits so (navicust_set_spins). */
void grant_spins(void) {
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS), check = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_CHECK);
	if (items < BN6_EWRAM || items >= BN6_EWRAM_END || check < BN6_EWRAM || check >= BN6_EWRAM_END) return;
	/* (the count and the item's check, its seed ^ 0x55, as the game's own
	 * giving writes them: a count without it reads as none) */
	unsigned held = meta_spins();
	for (uint32_t c = 1; c <= 6; ++c) {
		uint32_t id = 0x4F + c;
		uint8_t want = (uint8_t)(emu_read8(BN6_KEY_ITEM_SEEDS + id) ^ 0x55);
		if (held >> (c - 1) & 1) {
			if (!emu_read8(items + id) || emu_read8(check + id) != want) {
				emu_write8(items + id, 1);
				emu_write8(check + id, want);
			}
		} else if (emu_read8(items + id)) emu_write8(items + id, 0);
	}
	/* (and the TagChip system, Chaud's first clearance's: issue #51) */
	uint8_t want = (uint8_t)(emu_read8(BN6_KEY_ITEM_SEEDS + SCRIPTS_TAG_CHIP) ^ 0x55);
	if (rival_clearance() >= 1 && (!emu_read8(items + SCRIPTS_TAG_CHIP) || emu_read8(check + SCRIPTS_TAG_CHIP) != want)) {
		emu_write8(items + SCRIPTS_TAG_CHIP, 1);
		emu_write8(check + SCRIPTS_TAG_CHIP, want);
	}
}

/* The Spins as the profile has them, in the game's key items and the
 * drafts: as a layer is made and after a checkpoint's state (a run saved
 * by an older build held all six) */
void spins_sync(void) {
	navicust_set_spins(meta_spins());
	grant_spins();
}

/* A key item the game gave (EV_ITEM_GIVEN): a ScrtData, which MegaMan
 * says is for; the run's Spin, from its Mystery Data, which the profile
 * keeps and MegaMan says what it does. */
void item_given(int item) {
	if (item >= SCRIPTS_REG_UP1 && item <= SCRIPTS_REG_UP1 + 2) D.reg_due = true;
	if (item == SCRIPTS_SECRET_DATA) {
		int n = key_item(SCRIPTS_SECRET_DATA);
		if (n > run.fragments && D.objs.fragment_found >= 0) D.fragment_due = true;
		run.fragments = n;
	}
	int c = D.objs.spin_colour;
	if (c && item == 0x4F + c && !(meta_spins() >> (c - 1) & 1)) {
		meta_spin_found(c);
		navicust_set_spins(meta_spins());
		D.spin_due = D.objs.spin_found >= 0;
	}
}

void spin_watch(void) {
	if (D.spin_due && talk_script(D.objs.archive, D.objs.spin_found)) D.spin_due = false;
}

/* MegaMan's words on Reg memory as a RegUp is found (issue #51): what it
 * is for, the first time in any run, else how much there is now; and once,
 * for a profile whose Chaud's clearance came before the TagChip system did,
 * what that does (a first clearance has Chaud say it). Said once the
 * PET's return has calmed, as code_watch's. */
void reg_watch(void) {
	if (D.pet_seen || !free_to_speak()) return;
	int reg = emu_read8(BN6_NAVI_REG), kind;
	if (D.reg_due && !profile.reg_taught) kind = REG_LESSON;
	else if (D.reg_due) kind = REG_UP;
	else if (rival_clearance() >= 1 && !profile.tag_taught) kind = REG_TAGCHIP;
	else return;
	if (!talk_start(reg_words(kind, reg), FACE_MEGAMAN)) return;
	if (D.reg_due) profile.reg_taught = 1;
	else profile.tag_taught = 1;
	D.reg_due = false;
	profile_save();
}

/* A compression code entered on the NaviCust screen (BN6 sets the flags of
 * the program's four colours at once), the first time in any run: into the
 * profile's codebook, Dad's Compression mail made again with it, and MegaMan
 * says where it is kept (issue #50). */
void code_watch(void) {
	for (int p = 1; p < NAVICUST_PROGRAMS; ++p) {
		if (profile_code_entered(p) || !navicust_in_pool(p) || !flag_get(BN6_FLAG_COMPRESSED + p * 4)) continue;
		profile_code_note(p);
		profile_save();
		D.pet_refreshed = false;
		D.code_due = p;
	}
	/* (once bug_watch has taken the PET's return, its frames calm: said as
	 * the PET closed, the box was drawn over its fading screen, garbled) */
	const char *about = D.code_due ? navicust_about(D.code_due) : NULL;
	if (!about || D.pet_seen || !free_to_speak()) return;
	if (talk_start(compressed_words(about), FACE_MEGAMAN)) D.code_due = 0;
}

static NaviPart board_parts[BN6_NAVICUST_SLOTS];
static uint8_t board_grid[NAVICUST_GRID * NAVICUST_GRID];

/* The game's board: its grid and its programs from their records, the
 * number of slots read (0 without a ROM's records). */
static int read_board(void) {
	static char names[BN6_NAVICUST_SLOTS][16];
	if (!R.data || !R.layout || !R.layout->navicust_programs) return 0;
	int n = 0;
	for (int i = 0; i < BN6_NAVICUST_SLOTS; ++i) {
		int v = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)i * 8);
		const char *about = v > 0 && v < 47 * 4 ? navicust_about(v / 4) : NULL, *colon = about ? strchr(about, ':') : NULL;
		board_parts[i] = (NaviPart){ NULL, -1, 0 };
		if (v <= 0 || v >= 47 * 4) continue;
		const uint8_t *rec = R.data + R.layout->navicust_programs + (uint32_t)v * 16;
		snprintf(names[i], sizeof names[0], "%.*s", colon ? (int)(colon - about) : 0, colon ? about : "");
		board_parts[i] = (NaviPart){ colon ? names[i] : NULL, rec[1], rec[3] };
		n = i + 1;
	}
	for (int i = 0; i < NAVICUST_GRID * NAVICUST_GRID; ++i) board_grid[i] = emu_read8(BN6_NAVICUST_GRID + (uint32_t)i);
	return n;
}

/* Why the board bugs, from the game's own grid and its programs'
 * records (navicust_bug_cause), or NULL. */
const char *bug_cause(void) {
	if (!R.data || !R.layout || !R.layout->navicust_programs) return NULL;
	int n = read_board(), w, h;
	navicust_board(key_item(SCRIPTS_EXP_MEMORY), &w, &h);
	return navicust_bug_cause(board_grid, board_parts, n, w, h);
}

/* Whether program variant `v` fits the board's free cells as its programs
 * stand (true where that cannot be read) */
bool fits_free_as(int v, bool compressed) {
	NaviShape s;
	int n = read_board(), w, h;
	if (!R.data || !R.layout || !R.layout->navicust_programs || !navicust_shape_as(v, compressed, &s)) return true;
	navicust_board(key_item(SCRIPTS_EXP_MEMORY), &w, &h);
	return navicust_fits_free(board_grid, board_parts, n, &s, w, h);
}

bool fits_as_it_stands(int v) { return fits_free_as(v, flag_get(BN6_FLAG_COMPRESSED + v)); }

const char *director_board_bug(void) { return bug_cause(); }

bool director_board_fits(int v) { return fits_as_it_stands(v); }

/* Whether each program of the guardian's draft fits the board's free
 * space as it stands, in the flags its Guardian Data's lines read
 * (LAYER_DRAFT_FIT_FLAG + k), kept while it waits: said before the pick,
 * where two playtesters took a program MegaMan then said would not fit
 * (session 63). */
void draft_fit_watch(void) {
	if (!D.objs.guardian.navi || boss_done() || D.frame % 16) return;
	for (int k = 0; k < 3; ++k) {
		int v = D.objs.guardian.draft[k];
		bool fits = v && fits_free_as(v, false);
		if (fits && !flag_get(LAYER_DRAFT_FIT_FLAG + k)) flag_set(LAYER_DRAFT_FIT_FLAG + k);
		else if (!fits && flag_get(LAYER_DRAFT_FIT_FLAG + k)) flag_clear(LAYER_DRAFT_FIT_FLAG + k);
	}
}

/* A program just come into the PET (a Guardian Data's, a vendor's) that
 * fits the board only once others move: said at once, where a playtester
 * met it in the NaviCust and moved two programs to fit Custom1 (session
 * 55); NULL for none. */
bool in_draft(int v) {
	for (int k = 0; k < 3; ++k) if (v && D.objs.guardian.draft[k] == v) return true;
	return false;
}

/* the programs in the PET, placed or not */
static int programs_owned(void) {
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	int n = 0;
	for (int v = 4; v < 47 * 4; ++v) n += emu_read8(items + BN6_PROGRAM_ITEMS + (uint32_t)v);
	return n;
}

/* MegaMan free to speak of the NaviCust: no talk, staging or chat box, and
 * the game not holding him for lines of its own. A Chip Trader's lines go
 * on on the map after its trade screen: a talk begun in between took their
 * box, held the D-pad so that "Try again?" never reached No, and A traded
 * again (issue #25). */
static bool free_to_speak(void) {
	return !talk_busy() && !cinema_busy() && !emu_read8(BN6_CHATBOX) && !emu_read8(BN6_DIALOGUE_LOCK) &&
		flag_get(BN6_FLAG_PLAYER_CAN_MOVE);
}

/* A program just come into the PET without the PET (a Guardian Data's):
 * whether the board's free space takes it, said once the chat is done
 * (from the PET, off_board_words says it); true where it spoke. */
static bool program_watch(void) {
	int owned = programs_owned();
	if (D.pet_seen || owned < D.programs_seen) { D.programs_seen = owned; return false; }
	if (owned == D.programs_seen || !free_to_speak()) return false;
	const char *say = cramped_words();
	if (!say || talk_start(say, FACE_MEGAMAN)) D.programs_seen = owned;
	return say != NULL;
}

/* The board as the player sets it in the NaviCust: each slot's program,
 * column, row and turns, and the grid they fill. */
static uint32_t board_hash(void) {
	uint32_t h = 2166136261u;
	for (int i = 0; i < BN6_NAVICUST_SLOTS; ++i) {
		uint32_t e = BN6_NAVICUST_PLACED + (uint32_t)i * 8;
		h = (h ^ (emu_read32(e) & 0xFF00FFFFu)) * 16777619u;
		h = (h ^ emu_read16(e + 4)) * 16777619u;
	}
	for (int i = 0; i < NAVICUST_GRID * NAVICUST_GRID; ++i) h = (h ^ emu_read8(BN6_NAVICUST_GRID + (uint32_t)i)) * 16777619u;
	return h;
}

#define BUG_CALM 10
/* The NaviCust's bugs, named in MegaMan's words when they change: after the
 * player runs the NaviCust in the PET, or an ExpMemry grows the board
 * (docs/NAVICUST.md). A bug the player can read is a price they chose; the
 * game only says that there is one. Read on the layer's first quiet frame
 * without a word, so a layer entered bugged does not repeat it; spoken
 * BUG_CALM frames after the map is back, before a step (issue #13: a
 * second's wait, kept since a talk opened straight out of the PET had
 * drawn its letters as noise, let MegaMan walk first; a talk opened six
 * frames after the PET closed, from its menu and from the NaviCust, drew
 * them whole). */
void bug_watch(void) {
	/* (a reading the game has held BUG_CALM frames: it rewrites the counts
	 * in passes that span frames, and its save scrambles them for eight) */
	static int last, calm;
	static uint8_t held[NAVICUST_BUGS];
	static uint32_t held_board;
	static int held_size;
	uint8_t now[NAVICUST_BUGS];
	for (int t = 0; t < NAVICUST_BUGS; ++t) now[t] = emu_read8(BN6_NAVICUST_BUGS + (uint32_t)t);
	uint32_t board = board_hash();
	int size = key_item(SCRIPTS_EXP_MEMORY);
	bool same = !memcmp(now, held, sizeof now) && board == held_board && size == held_size;
	calm = D.frame == last + 1 && same ? calm + 1 : 0;
	last = D.frame;
	memcpy(held, now, sizeof now);
	held_board = board;
	held_size = size;
	if (calm < BUG_CALM) return;
	placed_note();
	if (!D.bugs_known) {
		memcpy(D.bugs, now, sizeof now);
		D.board = board;
		D.board_size = size;
		D.bugs_known = true;
		D.programs_seen = programs_owned();
		return;
	}
	if (program_watch()) return;
	/* (back from the PET with a program left off the board: said at once,
	 * where L said it only on the next layer; a playtester ran the NaviCust
	 * without placing his Guardian Data's HP+100) */
	if (D.pet_seen && free_to_speak()) {
		D.pet_seen = false;
		const char *say = off_board_words();
		if (say && talk_start(say, FACE_MEGAMAN)) { D.off_told = true; return; }
	}
	/* (the counts alone changed, the board as it was: the game's own work.
	 * Every reload of MegaMan's stats counts them anew, as a trade screen
	 * opens and closes, and with BugStop zeroes them; MegaMan said a
	 * player's NaviCust ran clean after each trade, then named its bug
	 * again; issue #25) */
	if (board == D.board && size == D.board_size) { memcpy(D.bugs, now, sizeof now); return; }
	if (!memcmp(D.bugs, now, sizeof now)) { D.board = board; D.board_size = size; return; }
	if (!free_to_speak()) return;
	bool had = false;
	for (int t = 0; t < NAVICUST_BUGS; ++t) had |= D.bugs[t] != 0;
	/* (a board placed anew is the NaviCust's RUN; an ExpMemry only grows it) */
	const char *words = navicust_bug_words(now, board != D.board, bug_cause());
	if (*words ? talk_start(words, FACE_MEGAMAN) : !had || talk_start(clean_words(), FACE_MEGAMAN)) {
		memcpy(D.bugs, now, sizeof now);
		D.board = board;
		D.board_size = size;
	}
}
