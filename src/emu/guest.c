/* Another game's battles on a second core (guest.h). The guest is BN5: its
 * ROM, read beside BN6's (rom.c, XR), padded to EMU_ROM_SIZE as BN6's copy
 * is, in an mGBA core of its own. It boots once to a playable state (its
 * title, NEW GAME, and the intro pressed through to Lan's room) kept in
 * the data directory, and runs only while BN6's core waits, on the main
 * thread and without hooks: its encounter roll, patched in its ROM copy,
 * returns the record a battle is given, and the battle's end is read from
 * its game state (docs/ROM_DATA.md, BN5 guest battles). */
#include "guest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "data.h"
#include "debug.h"
#include "emu.h"
#include "rom.h"

#if !defined(__3DS__) && !defined(__EMSCRIPTEN__)
#define CW_GUEST 1
#endif

/* ---- its battle records, from the ROM file ---- */

static uint32_t rom32(const uint8_t *d, uint32_t a) {
	if (a < 0x08000000u || a - 0x08000000u + 4 > ROM_SIZE) return 0;
	const uint8_t *p = d + (a - 0x08000000u);
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

/* the first record of net map (group, number), 0 for none */
static uint32_t records_at(int xrom, int group, int number) {
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data || group < 0x80 || number < 0 || number > 0xFF) return 0;
	const uint8_t *d = XR[xrom].data;
	uint32_t groups = rom32(d, BN5_BATTLE_TABLES + 4), maps = groups ? rom32(d, groups + 4u * (uint32_t)(group - 0x80)) : 0;
	return maps ? rom32(d, maps + 4u * (uint32_t)number) : 0;
}

/* enemy `id`'s HP and damage, from its game's tables; false for a Navi or
 * none */
static bool xenemy(const uint8_t *d, int id, int *hp, int *damage) {
	if (id <= 0 || id >= 0x200) return false;
	const uint8_t *e = d + (BN5_ENEMY_IDS - 0x08000000u) + 3u * (uint32_t)id;
	if (e[1] != 0) return false;   /* (type 1 a Navi, 2 an object) */
	uint32_t types = rom32(d, BN5_ENEMY_STATS + 4u * e[1]), ais = types ? rom32(d, types + 4u * e[2]) : 0;
	if (!ais || ais - 0x08000000u + 6u * e[0] + 6 > ROM_SIZE) return false;
	const uint8_t *r = d + (ais - 0x08000000u) + 6u * e[0];
	*hp = (r[0] | r[1] << 8) & 0xFFF;
	*damage = (r[4] | r[5] << 8) & 0xFFF;
	return true;
}

int guest_record_strength(int xrom, uint32_t record, int *hp, int *damage) {
	*hp = *damage = 0;
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data || record < 0x08000000u || record - 0x08000000u + 16 > ROM_SIZE) return 0;
	const uint8_t *d = XR[xrom].data, *rec = d + (record - 0x08000000u);
	if (rec[7]) return 0;
	uint32_t ents = rom32(d, record + 12);
	int n = 0;
	for (uint32_t a = ents, k = 0; a && k < 16 && a - 0x08000000u + 4 <= ROM_SIZE && d[a - 0x08000000u] != 0xF0; a += 4, ++k) {
		const uint8_t *e = d + (a - 0x08000000u);
		int h = 0, dm = 0;
		if (e[0] != 0x11) continue;
		if (!xenemy(d, e[2] | e[3] << 8, &h, &dm)) return 0;
		*hp += h;
		if (dm > *damage) *damage = dm;
		++n;
	}
	return n;
}

int guest_records(int xrom, int group, int number) {
	uint32_t r = records_at(xrom, group, number);
	int n = 0;
	while (r && n < 64 && r - 0x08000000u + 16u * (uint32_t)(n + 1) <= ROM_SIZE && XR[xrom].data[r - 0x08000000u + 16u * (uint32_t)n] != 0xFF) ++n;
	return n;
}

uint32_t guest_record(int xrom, int group, int number, int i) {
	return i >= 0 && i < guest_records(xrom, group, number) ? records_at(xrom, group, number) + 16u * (uint32_t)i : 0;
}

bool guest_dev_worried;
int guest_backdrop = -1;

const char *guest_dark_name(int k) {
	static char name[16];
	if (k < 0 || k >= GUEST_DARK_KINDS) return "";
	xrom_text(XROM_BN5_COLONEL_US, BN5_CHIP_NAMES_LOW, BN5_DARK_FIRST + k, name, sizeof name);
	return name;
}

#ifdef CW_GUEST

#include <mgba/core/blip_buf.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba-util/vfs.h>

#include "compat.h"
#include "game.h"
#include "xchips.h"

/* ---- the run's chips as its game's: by name, from both ROMs ---- */

#define BN6_CHIPS 314   /* BN6's standard, Mega and Giga chips, those a folder holds */
static uint16_t to_bn5[BN6_CHIPS], from_bn5[BN5_CHIPS];

/* Pairs BN6's chips with BN5's of the same name (xchips.c); how many
 * paired. */
static int chips_pair(int xrom) {
	static char names5[BN5_CHIPS][XCHIP_NAME], names6[BN6_CHIPS][XCHIP_NAME];
	for (int i = 1; i < BN5_CHIPS; ++i)
		xrom_text(xrom, i < 256 ? BN5_CHIP_NAMES_LOW : BN5_CHIP_NAMES_HIGH, i < 256 ? i : i - 256, names5[i], sizeof names5[i]);
	for (int id = 1; id < BN6_CHIPS; ++id) {
		ChipInfo ci;
		chip_info(id, &ci);
		snprintf(names6[id], sizeof names6[id], "%s", ci.name);
	}
	return xchips_pair((const char (*)[XCHIP_NAME])names6, BN6_CHIPS, (const char (*)[XCHIP_NAME])names5, BN5_CHIPS, to_bn5, from_bn5);
}

static struct mCore *core;
static bool ready, failed, active, result_due;
static uint32_t video[EMU_W * EMU_H];
static GuestResult result;
static int frames;
static int recoded, recode_ex[3];   /* chips that went in with another code, and the first: BN6 id, code, its code there */
static uint8_t dark_in[GUEST_DARK_KINDS];   /* the run's DarkChips as the battle began */
static bool dark_used;                       /* ... and one was used in it (latched from BN5_DARK_USED) */
static bool buster_told;                     /* (debug) its battle's buster was printed */

/* a battle's course: asked for, begun */
enum { PH_IDLE, PH_ASKED, PH_BATTLE };
static int phase;

static uint8_t rd8(uint32_t a) { return (uint8_t)core->rawRead8(core, a, -1); }
static uint16_t rd16(uint32_t a) { return (uint16_t)core->rawRead16(core, a, -1); }
static uint32_t rd32(uint32_t a) { return core->rawRead32(core, a, -1); }
static void wr16(uint32_t a, uint16_t v) { core->rawWrite16(core, a, -1, v); }

/* The deck's compaction, replaced (BN5_COMPACT): as BN5's, the live chips
 * to the front, 0xFFFF after; but a DarkChip stays live only where BN5's
 * worried rule could have put it (at the hand's end while MegaMan is
 * worried or dark, or in the hand after the battle's first Custom screen),
 * the others shelved behind the padding, where no draw reaches and the
 * rule, which reads all 30, still finds them: DarkChips come only by its
 * rule (docs/META.md). Thumb, its literals 0x08012705 (emotion(side)),
 * the deck 0x0203C830 and 0xFFFF; called with r5 the Custom screen's state
 * and sl the toolkit, as BN5's (written and tested in romlab: the worried
 * MegaMan offered the shelved DarkCirc in his hand's last slot, the calm
 * one none). */
static const uint8_t shelf[188] = {
	0xf0, 0xb5, 0x84, 0xb0, 0x50, 0x46, 0x80, 0x69, 0xc1, 0x79, 0x00, 0x22, 0x01, 0x29, 0x00, 0xd9, 0x01, 0x22, 0x03, 0x92, 0x40, 0x7b,
	0x26, 0x4b, 0x00, 0xf0, 0x49, 0xf8, 0x01, 0x28, 0x01, 0xd0, 0x05, 0x28, 0x03, 0xd1, 0x03, 0x9a, 0x02, 0x21, 0x0a, 0x43, 0x03, 0x92,
	0xab, 0x79, 0x5b, 0x00, 0x20, 0x4c, 0x00, 0x26, 0x00, 0x27, 0x00, 0x20, 0x02, 0x90, 0xa0, 0x5b, 0x1e, 0x49, 0x88, 0x42, 0x1b, 0xd0,
	0xc1, 0x05, 0xc9, 0x0d, 0xbb, 0x29, 0x15, 0xd3, 0xc6, 0x29, 0x13, 0xd8, 0x03, 0x9a, 0x9e, 0x42, 0x04, 0xd3, 0x06, 0xd1, 0x02, 0x21,
	0x0a, 0x42, 0x0c, 0xd1, 0x02, 0xe0, 0x01, 0x21, 0x0a, 0x42, 0x08, 0xd1, 0x02, 0x99, 0x04, 0x29, 0x05, 0xd2, 0x4a, 0x00, 0x01, 0x31,
	0x02, 0x91, 0x69, 0x46, 0x88, 0x52, 0x01, 0xe0, 0xe0, 0x53, 0x02, 0x37, 0x02, 0x36, 0x3c, 0x2e, 0xdc, 0xd3, 0x02, 0x99, 0x1e, 0x22,
	0x52, 0x1a, 0x52, 0x00, 0x0b, 0x48, 0x97, 0x42, 0x02, 0xd2, 0xe0, 0x53, 0x02, 0x37, 0xfa, 0xe7, 0x00, 0x26, 0x02, 0x98, 0x40, 0x00,
	0x86, 0x42, 0x05, 0xd2, 0x69, 0x46, 0x88, 0x5b, 0xe0, 0x53, 0x02, 0x37, 0x02, 0x36, 0xf5, 0xe7, 0x04, 0xb0, 0xf0, 0xbd, 0x18, 0x47,
	0x05, 0x27, 0x01, 0x08, 0x30, 0xc8, 0x03, 0x02, 0xff, 0xff, 0x00, 0x00,
};
#define SHELF_AT (BN5_FREE + 0x100)

/* BN5's compaction pointed at the shelf (ldr r3, =shelf + 1; bx r3; nop) */
static void shelf_install(void) {
	for (uint32_t i = 0; i < sizeof shelf; i += 2) core->rawWrite16(core, SHELF_AT + i, -1, (uint16_t)(shelf[i] | shelf[i + 1] << 8));
	static const uint16_t jump[3] = { 0x4B01, 0x4718, 0x46C0 };
	for (uint32_t i = 0; i < 3; ++i) core->rawWrite16(core, BN5_COMPACT + 2 * i, -1, jump[i]);
	core->rawWrite32(core, BN5_COMPACT + 6, -1, SHELF_AT + 1);
}

/* The All * helper in the guest (docs/META.md): its chips' records hold *
 * alone in its ROM copy, as BN6's then do, so the run's chips come into its
 * Custom screen in * (a code its record lacks it draws as nothing), and its
 * check of a Program Advance of codes in a row takes three * (as BN6's,
 * BN5_PA_STAR_LIMIT); else its ROM's own again, where a run before had it. */
static bool starred;
static void star_records(bool on) {
	if (on == starred) return;
	starred = on;
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	static const uint8_t star[4] = { CHIP_CODE_STAR, 0xFF, 0xFF, 0xFF };
	for (uint32_t x = 1; x < BN5_CHIPS; ++x)
		for (uint32_t i = 0, rec = BN5_CHIP_RECORDS + 0x2Cu * x; i < 4; ++i)
			core->rawWrite8(core, 0x08000000u + rec + i, -1, on ? star[i] : d[rec + i]);
	uint32_t at = BN5_PA_STAR_LIMIT - 0x08000000u;
	uint16_t limit = (uint16_t)(d[at] | d[at + 1] << 8);
	wr16(BN5_PA_STAR_LIMIT, on && limit == 0x2A01 ? 0x2A03 : limit);
}

/* the run's DarkChips into the guest's folder (three at most, one of each,
 * as BN5's folder takes them), each in its own code (in * with All *),
 * where chips sat out: one empty slot stays, the shelf's edge */
static void dark_in_folder(uint32_t folders, uint32_t marks) {
	static const uint16_t entry[GUEST_DARK_KINDS] = { 0x22BB, 0x32BC, 0x10BD, 0x34BE, 0x2CBF, 0x26C0, 0x18C1, 0x0EC2, 0x0AC3, 0x06C4, 0x26C5, 0x08C6 };
	int empty[30], ne = 0, put = 0;
	for (int i = 29; i >= 0; --i) if (rd16(folders + 2u * (uint32_t)i) == 0xFFFF) empty[ne++] = i;
	for (int k = 0; k < GUEST_DARK_KINDS && put < 3 && put + 1 < ne; ++k) {
		if (!dark_in[k]) continue;
		wr16(folders + 2u * (uint32_t)empty[put++], starred ? chip_entry_star(entry[k]) : entry[k]);
		uint32_t id = entry[k] & 0x1FF;
		core->rawWrite8(core, marks + id, -1, (uint8_t)(rd8(BN5_CHIP_KEYS + id) ^ BN5_CHIP_KEY_XOR));
	}
}
static int main_mode(void) { return rd8(rd32(BN5_TOOLKIT)); }
static int sub_mode(void) { return rd8(BN5_GAMESTATE); }
static bool on_map(void) { return main_mode() == BN5_MODE_GAME && sub_mode() == BN5_SUB_MAP; }

static void run(int n, uint32_t keys) {
	core->setKeys(core, keys);
	for (int i = 0; i < n; ++i) core->runFrame(core);
}

static void tap(uint32_t key) {
	run(2, key);
	run(8, 0);
}

/* The roll answers `record` (0: no battle): ldr r0,=record; tst r0,r0;
 * bx lr, the record in the literal after it. */
static void patch_roll(uint32_t record) {
	static const uint16_t code[4] = { 0x4801, 0x4200, 0x4770, 0x46C0 };
	for (uint32_t i = 0; i < 4; ++i) core->rawWrite16(core, BN5_ROLL + 2 * i, -1, code[i]);
	core->rawWrite32(core, BN5_ROLL + 8, -1, record);
}

/* bump the number when the boot changes */
static void state_path(char *out, size_t n) { snprintf(out, n, "%s/guest-bn5-1.state", g_data_dir); }

static bool load_boot(void) {
	char path[600];
	state_path(path, sizeof path);
	struct VFile *vf = VFileOpen(path, O_RDONLY);
	if (!vf) return false;
	bool ok = mCoreLoadStateNamed(core, vf, SAVESTATE_SAVEDATA);
	vf->close(vf);
	return ok && on_map();
}

/* Its title, NEW GAME, then A pressed through the intro until Lan stands
 * in his room: the same frames every time (emulation is deterministic) */
static bool boot(void) {
	if (load_boot()) return true;
	core->reset(core);
	run(600, 0);
	for (int i = 0; i < 6; ++i) {
		tap(KEY_START);
		run(60, 0);
	}
	tap(KEY_A);
	run(120, 0);
	for (int i = 0; i < 120; ++i) {
		tap(KEY_A);
		run(40, 0);
	}
	run(60, 0);
	if (!on_map()) return false;
	char path[600], tmp[640];
	state_path(path, sizeof path);
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	struct VFile *vf = VFileOpen(tmp, O_CREAT | O_TRUNC | O_RDWR);
	if (vf) {
		bool ok = mCoreSaveStateNamed(core, vf, SAVESTATE_SAVEDATA);
		vf->close(vf);
		if (ok) cw_rename(tmp, path);
		else remove(tmp);
	}
	return true;
}

bool guest_start(int xrom) {
	if (ready || failed) return ready;
	if (xrom != XROM_BN5_COLONEL_US || !XR[xrom].data) return false;
	failed = true;
	uint8_t *copy = malloc(EMU_ROM_SIZE);
	if (!copy) return false;
	memcpy(copy, XR[xrom].data, ROM_SIZE);
	memset(copy + ROM_SIZE, 0xFF, EMU_ROM_SIZE - ROM_SIZE);
	struct VFile *vf = VFileMemChunk(copy, EMU_ROM_SIZE);
	free(copy);
	if (!vf) return false;
	core = mCoreFindVF(vf);
	if (!core || !core->init(core)) { vf->close(vf); core = NULL; return false; }
	mCoreInitConfig(core, NULL);
	core->setVideoBuffer(core, (color_t *)video, EMU_W);
	if (!core->loadROM(core, vf)) { core->deinit(core); core = NULL; return false; }
	core->loadSave(core, VFileMemChunk(NULL, 0));
	core->setAudioBufferSize(core, 1024);
	blip_set_rates(core->getAudioChannel(core, 0), core->frequency(core), emu_audio_out_rate());
	blip_set_rates(core->getAudioChannel(core, 1), core->frequency(core), emu_audio_out_rate());
	core->reset(core);
	if (!boot()) {
		fprintf(stderr, "guest: %s did not reach its map\n", XR[xrom].layout->tag);
		return false;
	}
	patch_roll(0);
	shelf_install();
	failed = false;
	ready = true;
	int paired = chips_pair(xrom);
	printf("guest: %s ready for battles, %d of BN6's chips by name\n", XR[xrom].layout->tag, paired);
	return true;
}

/* `record` copied past BN5's ROM without its GAME OVER: a loss ends the
 * battle on the map with the result 2, and the run's own GAME OVER follows
 * (BN6's); its copy's address. Its entities stay the original's. */
/* The zenny a reward row's chip pays where BN6 has none of its name: the
 * results screen shows what the run gets (docs/MULTIROM.md, Guest
 * battles) */
#define REWARD_ZENNY 200

/* The reward rows of the enemies in `record` rewritten where a chip has no
 * BN6 chip of its name: zenny instead (its game's own encoding, as BN6's
 * rewards are); with All * the others in * (its results screen shows the
 * chip as the run gets it), else as its ROM has them. Its entities follow
 * the record's pointer at +0xC, four bytes each (0x11 an enemy, its id in
 * the last two), 0xF0 ending them. */
static void rows_fit(uint32_t record) {
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data;
	uint32_t e = rd32(record + 0xC);
	for (int k = 0; k < 16 && e >= 0x08000000u && e < 0x08800000u; ++k, e += 4) {
		uint8_t kind = rd8(e);
		if (kind == 0xF0) break;
		if (kind != 0x11) continue;
		/* (a byte at a time: a list can start at an odd address, and a
		 * halfword read there takes the even one below, the entry's
		 * panel byte for its id's low half; CanGard's MrkCan1 S showed on
		 * a playtester's results screen, came back as zenny, session 66) */
		int id = rd8(e + 2) | rd8(e + 3) << 8;
		if (emu_debug_on()) fprintf(stderr, "guest: the reward rows of enemy %#x fitted\n", id);
		if (id <= 0 || id >= 0x200) continue;
		for (uint32_t i = 0; i < 20; ++i) {
			uint32_t a = BN5_REWARD_ROWS + (uint32_t)id * 0x28 + 2 * i, o = a - 0x08000000u;
			uint16_t v = (uint16_t)(d[o] | d[o + 1] << 8);
			if (v == 0xFFFF || v >> 14 || (int)(v & 0x1FF) >= BN5_CHIPS) continue;
			wr16(a, !from_bn5[v & 0x1FF] ? (uint16_t)(1u << 14 | REWARD_ZENNY) : starred ? chip_entry_star(v) : v);
		}
	}
}

/* (and its background the dressed area's where it leaves it to the map:
 * every guest battle stood in front of the room its boot state stands in,
 * yellow rings for each area) */
static uint32_t record_copy(uint32_t record) {
	for (uint32_t i = 0; i < 16; i += 2) {
		uint16_t v = rd16(record + i);
		if (i == 8) v = (uint16_t)(v & ~BN5_OPT_GAME_OVER);
		if (i == BN5_RECORD_BACKDROP && (v & 0xFF) == 0xFF && guest_backdrop >= 0) v = (uint16_t)((v & 0xFF00) | (guest_backdrop & 0xFF));
		wr16(BN5_FREE + i, v);
	}
	return BN5_FREE;
}

/* The run's folder as the guest's first, each chip as its game's of the
 * same name (its own code), owned; a chip it has none of sits out, its
 * slot empty. How many went in. */
/* A chip's code as BN5's chip of its name has it: the same letter where it
 * has that one, else its '*', else its first. BN5's Custom screen draws a
 * chip with a code it lacks as nothing, which can be chosen and does
 * nothing (a playtester's Storm folder had nine of them, its Thunder S
 * among them: BN5's Thunder, its chip 19, has B, L, P and *; session 65;
 * the Storm folder's ElcPuls1 and DolThdr1 BN5 has none of). */
static int bn5_code(int x, int code) {
	/* (its record as its ROM copy has it: in * alone with All *) */
	uint8_t rec[4];
	for (uint32_t i = 0; i < 4; ++i) rec[i] = rd8(0x08000000u + BN5_CHIP_RECORDS + 0x2Cu * (uint32_t)x + i);
	return xchips_code(rec, code);
}

/* ... and a chip won there as BN6's chip of its name has it, the same way */
static int bn6_code(int id, int code) {
	ChipInfo ci;
	chip_info(id, &ci);
	uint8_t rec[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	for (int i = 0; i < ci.ncodes && i < 4; ++i) rec[i] = ci.codes[i] == '*' ? 26 : (uint8_t)(ci.codes[i] - 'A');
	return xchips_code(rec, code);
}

static int folder_in(const uint16_t *folder) {
	uint32_t folders = rd32(BN5_TOOLKIT + BN5_TOOLKIT_CHIPS), marks = rd32(BN5_TOOLKIT + BN5_TOOLKIT_CHIP_MARKS);
	if (folders < 0x02000000u || folders >= 0x02040000u || marks < 0x02000000u || marks >= 0x02040000u) return 0;
	int in = 0;
	recoded = 0;
	for (uint32_t i = 0; i < 30; ++i) {
		int id = folder[i] & 0x1FF, x = id > 0 && id < BN6_CHIPS ? to_bn5[id] : 0, code = folder[i] >> 9, there = x ? bn5_code(x, code) : code;
		core->rawWrite16(core, folders + 2 * i, -1, x ? (uint16_t)(x | there << 9) : 0xFFFF);
		if (!x) continue;
		if (there != code && !recoded++) { recode_ex[0] = id; recode_ex[1] = code; recode_ex[2] = there; }
		core->rawWrite8(core, marks + (uint32_t)x, -1, (uint8_t)(rd8(BN5_CHIP_KEYS + (uint32_t)x) ^ BN5_CHIP_KEY_XOR));
		++in;
	}
	core->rawWrite8(core, BN5_NAVI_FOLDER, -1, 0);
	dark_in_folder(folders, marks);
	return in;
}

int guest_sitting_out(const uint16_t *folder, uint16_t *out, int max) {
	return ready ? xchips_out(folder, 30, to_bn5, BN6_CHIPS, out, max) : 0;
}

/* (debug: the folder's chips that sat out, slot by slot) */
static void out_tell(const uint16_t *folder) {
	char line[600];
	int k = 0, n = 0;
	for (int i = 0; folder && i < 30 && k < (int)sizeof line - 24; ++i) {
		int id = folder[i] & 0x1FF, code = folder[i] >> 9;
		if (!id || (id < BN6_CHIPS && to_bn5[id])) continue;
		ChipInfo ci;
		chip_info(id, &ci);
		k += snprintf(line + k, sizeof line - (size_t)k, "%s%s %c", n++ ? ", " : "", ci.name, code == 26 ? '*' : code < 26 ? 'A' + code : '?');
	}
	if (n) fprintf(stderr, "guest: %d sat out: %s\n", n, line);
}

/* His buster as the run's NaviCust makes it: BN5's boot state's fired 1 a
 * shot and 10 a charge, whatever programs the run had run */
static void buster_in(const uint8_t buster[3]) {
	static const uint32_t at[3] = { BN5_NAVI_ATTACK, BN5_NAVI_SPEED, BN5_NAVI_CHARGE };
	for (int i = 0; i < 3; ++i) core->rawWrite8(core, at[i], -1, buster[i] > BN5_BUSTER_MAX ? BN5_BUSTER_MAX : buster[i]);
	buster_told = !emu_debug_on();
}

/* (debug: the buster the battle took, from its own copy, once its Custom
 * screen is up) */
static void buster_tell(void) {
	if (buster_told || phase != PH_BATTLE || sub_mode() != BN5_SUB_BATTLE || rd8(BN5_BATTLE_STATE + 1) != BN5_PHASE_CUSTOM) return;
	buster_told = true;
	uint32_t s = BN5_BATTLE_NAVI + BN5_BATTLE_NAVI_SIZE * rd8(BN5_BATTLE_SIDE);
	fprintf(stderr, "guest: its battle's buster Attack %d, Speed %d, Charge %d\n", rd8(s + (BN5_NAVI_ATTACK - BN5_NAVI_STATS)) + 1,
		rd8(s + (BN5_NAVI_SPEED - BN5_NAVI_STATS)) + 1, rd8(s + (BN5_NAVI_CHARGE - BN5_NAVI_STATS)) + 1);
}

bool guest_battle(uint32_t record, const GuestMegaMan *mm) {
	if (!ready || active || !record) return false;
	memcpy(dark_in, mm->dark, sizeof dark_in);
	emu_sync();   /* (BN6's frame done first, where its core has a thread) */
	if (!on_map() && !load_boot()) return false;
	star_records(mm->star);
	/* MegaMan as the run has him: his HP (BN5 copies it back after a battle
	 * whose options carry 0x40, as its random battles' do) */
	int hp = mm->hp, max_hp = mm->max_hp;
	wr16(BN5_NAVI_BASE_MAX_HP, (uint16_t)max_hp);
	wr16(BN5_NAVI_MAX_HP, (uint16_t)max_hp);
	wr16(BN5_NAVI_HP, (uint16_t)(hp < 1 ? 1 : hp > max_hp ? max_hp : hp));
	buster_in(mm->buster);
	/* (calm at its start, as each of the run's battles begins: BN5's dark
	 * meter and mood would carry its last DarkChip's darkness on) */
	core->rawWrite8(core, BN5_NAVI_MOOD, -1, 0x80);
	wr16(BN5_NAVI_METER, 500);
	uint32_t check = rd32(BN5_TOOLKIT + BN5_TOOLKIT_METER_CHECK);
	if (check >= 0x02000000u && check < 0x02040000u) core->rawWrite32(core, check, -1, 500u ^ rd32(BN5_METER_KEY));
	dark_used = false;
	const uint16_t *folder = mm->folder;
	int in = folder ? folder_in(folder) : 0;
	rows_fit(record);
	if (emu_debug_on()) {
		fprintf(stderr, "guest: battle %08X, HP %d/%d, %d of the folder's 30 in, buster Attack %d, Speed %d, Charge %d\n", record, hp, max_hp, in,
			rd8(BN5_NAVI_ATTACK) + 1, rd8(BN5_NAVI_SPEED) + 1, rd8(BN5_NAVI_CHARGE) + 1);
		out_tell(folder);
	}
	patch_roll(record_copy(record));
	active = true;
	phase = PH_ASKED;
	frames = 0;
	return true;
}

bool guest_active(void) { return active; }

bool guest_custom_screen(void) { return active && phase == PH_BATTLE && rd8(BN5_BATTLE_STATE + 1) == BN5_PHASE_CUSTOM; }

bool guest_on_screen(void) { return active && phase == PH_BATTLE && sub_mode() == BN5_SUB_BATTLE; }

int guest_custom_gauge(void) {
	return guest_on_screen() && rd8(BN5_BATTLE_STATE + 1) == BN5_PHASE_FIGHT ? rd16(BN5_CUSTOM_GAUGE) : -1;
}

bool guest_fight_hp(int *hp, int *max) {
	if (!guest_on_screen()) return false;
	*hp = rd16(BN5_FIGHT_HP);
	*max = rd16(BN5_FIGHT_HP + 2);
	return *max > 0;   /* (0/0 in the battle's first frames, before it sets MegaMan) */
}

static void finish(int outcome) {
	result = (GuestResult){ .outcome = outcome, .frames = frames, .hp = phase == PH_BATTLE ? rd16(BN5_BATTLE_HP) : rd16(BN5_NAVI_HP), .recoded = recoded,
		.recode_chip = recode_ex[0], .recode_from = recode_ex[1], .recode_to = recode_ex[2], .reward_from = -1 };
	memcpy(result.dark, dark_in, sizeof result.dark);   /* (BN5 keeps a DarkChip once used: the run's stay) */
	result.dark_used = dark_used;
	/* (what its results screen gave, as the run's: a chip by its name, or
	 * zenny) */
	uint16_t v = outcome == GUEST_WON ? rd16(BN5_REWARD) : 0;
	if (v && v >> 14 == 1) result.zenny = v & 0x3FFF;
	else if (v && v >> 14 == 0 && (v & 0x1FF) < BN5_CHIPS) {
		result.chip = from_bn5[v & 0x1FF];
		result.code = result.chip ? bn6_code(result.chip, v >> 9 & 0x1F) : 0;
		if (result.chip && result.code != (v >> 9 & 0x1F)) result.reward_from = v >> 9 & 0x1F;
		if (!result.chip) result.zenny = REWARD_ZENNY;
	}
	result_due = true;
	patch_roll(0);
	active = false;
	phase = PH_IDLE;
}

static void step(uint32_t keys, bool quiet);

void guest_frame(uint32_t keys) { step(keys, false); }
void guest_frame_quiet(uint32_t keys) { step(keys, true); }

static void step(uint32_t keys, bool quiet) {
	if (!active) return;
	core->setKeys(core, keys);
	core->runFrame(core);
	if (quiet) emu_audio_drop_from(core);
	else emu_audio_from(core);
	++frames;
	buster_tell();
	int mode = main_mode(), sub = sub_mode();
	if (phase == PH_ASKED && (sub == BN5_SUB_BATTLE_INIT || sub == BN5_SUB_BATTLE)) {
		phase = PH_BATTLE;
		core->rawWrite32(core, BN5_ROLL + 8, -1, 0);   /* (one battle: the roll answers none again) */
		core->rawWrite8(core, BN5_BATTLE_RESULT + 1, -1, 0);
		wr16(BN5_REWARD, 0);
	} else if (phase == PH_ASKED && frames > 600) finish(GUEST_ESCAPED);   /* (never began: nothing happened) */
	/* (a DarkChip used: latched while its battle is fought, as leaving it
	 * wipes the flag; not in the battle's first frames, which hold the last
	 * battle's bytes until BN5 clears them: every battle had cost 20 max HP) */
	else if (phase == PH_BATTLE && sub == BN5_SUB_BATTLE && rd8(BN5_BATTLE_STATE + 1) == BN5_PHASE_FIGHT &&
		rd8(BN5_DARK_USED + 8u * rd8(BN5_BATTLE_SIDE)))
		dark_used = true;
	if (guest_dev_worried && phase == PH_BATTLE && sub == BN5_SUB_BATTLE && rd8(BN5_BATTLE_MOOD) > 0x40) core->rawWrite8(core, BN5_BATTLE_MOOD, -1, 0x20);
	/* (back on the map: how it ended, from BN5's own result, and MegaMan's
	 * HP as the battle left it: BN5 copies it back to his NaviStats only on
	 * its own maps' terms, which a forced battle does not meet) */
	else if (phase == PH_BATTLE && mode == BN5_MODE_GAME && sub == BN5_SUB_MAP) {
		int r = rd8(BN5_BATTLE_RESULT + 1), hp = rd16(BN5_BATTLE_HP);
		finish(r == BN5_RESULT_LOST || hp == 0 ? GUEST_LOST : r == BN5_RESULT_ESCAPED ? GUEST_ESCAPED : GUEST_WON);
	}
	/* (a GAME OVER all the same: its records' copies leave it out) */
	else if (phase == PH_BATTLE && mode == BN5_MODE_GAME_OVER) finish(GUEST_LOST);
}

const uint32_t *guest_video(void) { return video; }

bool guest_take_result(GuestResult *out) {
	if (!result_due) return false;
	*out = result;
	result_due = false;
	return true;
}

#else   /* (one ROM: the 3DS, the browser) */

bool guest_start(int xrom) { (void)xrom; return false; }
bool guest_battle(uint32_t record, const GuestMegaMan *mm) {
	(void)record; (void)mm;
	return false;
}
int guest_sitting_out(const uint16_t *folder, uint16_t *out, int max) { (void)folder; (void)out; (void)max; return 0; }
bool guest_active(void) { return false; }
bool guest_custom_screen(void) { return false; }
bool guest_on_screen(void) { return false; }
bool guest_fight_hp(int *hp, int *max) { (void)hp; (void)max; return false; }
int guest_custom_gauge(void) { return -1; }
void guest_frame(uint32_t keys) { (void)keys; }
void guest_frame_quiet(uint32_t keys) { (void)keys; }
const uint32_t *guest_video(void) { return NULL; }
bool guest_take_result(GuestResult *out) { (void)out; return false; }

#endif
