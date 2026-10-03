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

/* ---- the run's chips as its game's: by name, from both ROMs ---- */

#define BN6_CHIPS 314   /* BN6's standard, Mega and Giga chips, those a folder holds */
static uint16_t to_bn5[BN6_CHIPS], from_bn5[BN5_CHIPS];

/* Pairs BN6's chips with BN5's of the same name; how many paired. */
static int chips_pair(int xrom) {
	static char names[BN5_CHIPS][16];
	for (int i = 1; i < BN5_CHIPS; ++i)
		xrom_text(xrom, i < 256 ? BN5_CHIP_NAMES_LOW : BN5_CHIP_NAMES_HIGH, i < 256 ? i : i - 256, names[i], sizeof names[i]);
	int paired = 0;
	for (int id = 1; id < BN6_CHIPS; ++id) {
		ChipInfo ci;
		chip_info(id, &ci);
		to_bn5[id] = 0;
		for (int j = 1; j < BN5_CHIPS && ci.name[0] && !to_bn5[id]; ++j)
			if (!strcmp(names[j], ci.name)) to_bn5[id] = (uint16_t)j;
		if (to_bn5[id] && !from_bn5[to_bn5[id]]) from_bn5[to_bn5[id]] = (uint16_t)id;
		paired += to_bn5[id] != 0;
	}
	return paired;
}

static struct mCore *core;
static bool ready, failed, active, result_due;
static uint32_t video[EMU_W * EMU_H];
static GuestResult result;
static int frames, sat_out;
static uint8_t dark_in[GUEST_DARK_KINDS];   /* the run's DarkChips as the battle began */
static bool dark_used;                       /* ... and one was used in it (latched from BN5_DARK_USED) */

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

/* the run's DarkChips into the guest's folder (three at most, one of each,
 * as BN5's folder takes them), each in its own code, where chips sat out:
 * one empty slot stays, the shelf's edge */
static void dark_in_folder(uint32_t folders, uint32_t marks) {
	static const uint16_t entry[GUEST_DARK_KINDS] = { 0x22BB, 0x32BC, 0x10BD, 0x34BE, 0x2CBF, 0x26C0, 0x18C1, 0x0EC2, 0x0AC3, 0x06C4, 0x26C5, 0x08C6 };
	int empty[30], ne = 0, put = 0;
	for (int i = 29; i >= 0; --i) if (rd16(folders + 2u * (uint32_t)i) == 0xFFFF) empty[ne++] = i;
	for (int k = 0; k < GUEST_DARK_KINDS && put < 3 && put + 1 < ne; ++k) {
		if (!dark_in[k]) continue;
		wr16(folders + 2u * (uint32_t)empty[put++], entry[k]);
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
 * rewards are). Its entities follow the record's pointer at +0xC, four
 * bytes each (0x11 an enemy, its id in the last two), 0xF0 ending them. */
static void rows_fit(uint32_t record) {
	uint32_t e = rd32(record + 0xC);
	for (int k = 0; k < 16 && e >= 0x08000000u && e < 0x08800000u; ++k, e += 4) {
		uint8_t kind = rd8(e);
		if (kind == 0xF0) break;
		if (kind != 0x11) continue;
		int id = rd16(e + 2);
		if (id <= 0 || id >= 0x200) continue;
		for (uint32_t i = 0; i < 20; ++i) {
			uint32_t a = BN5_REWARD_ROWS + (uint32_t)id * 0x28 + 2 * i;
			uint16_t v = rd16(a);
			if (v == 0xFFFF || v >> 14 || (int)(v & 0x1FF) >= BN5_CHIPS || from_bn5[v & 0x1FF]) continue;
			wr16(a, (uint16_t)(1u << 14 | REWARD_ZENNY));
		}
	}
}

static uint32_t record_copy(uint32_t record) {
	for (uint32_t i = 0; i < 16; i += 2) {
		uint16_t v = rd16(record + i);
		if (i == 8) v = (uint16_t)(v & ~BN5_OPT_GAME_OVER);
		wr16(BN5_FREE + i, v);
	}
	return BN5_FREE;
}

/* The run's folder as the guest's first, each chip as its game's of the
 * same name (its own code), owned; a chip it has none of sits out, its
 * slot empty. How many went in. */
static int folder_in(const uint16_t *folder) {
	uint32_t folders = rd32(BN5_TOOLKIT + BN5_TOOLKIT_CHIPS), marks = rd32(BN5_TOOLKIT + BN5_TOOLKIT_CHIP_MARKS);
	if (folders < 0x02000000u || folders >= 0x02040000u || marks < 0x02000000u || marks >= 0x02040000u) return 0;
	int in = 0;
	for (uint32_t i = 0; i < 30; ++i) {
		int id = folder[i] & 0x1FF, x = id > 0 && id < BN6_CHIPS ? to_bn5[id] : 0;
		core->rawWrite16(core, folders + 2 * i, -1, x ? (uint16_t)(x | (folder[i] >> 9) << 9) : 0xFFFF);
		if (!x) continue;
		core->rawWrite8(core, marks + (uint32_t)x, -1, (uint8_t)(rd8(BN5_CHIP_KEYS + (uint32_t)x) ^ BN5_CHIP_KEY_XOR));
		++in;
	}
	core->rawWrite8(core, BN5_NAVI_FOLDER, -1, 0);
	dark_in_folder(folders, marks);
	return in;
}

bool guest_battle(uint32_t record, int hp, int max_hp, const uint16_t *folder, const uint8_t dark[GUEST_DARK_KINDS]) {
	if (!ready || active || !record) return false;
	memcpy(dark_in, dark, sizeof dark_in);
	emu_sync();   /* (BN6's frame done first, where its core has a thread) */
	if (!on_map() && !load_boot()) return false;
	/* MegaMan as the run has him: his HP (BN5 copies it back after a battle
	 * whose options carry 0x40, as its random battles' do) */
	wr16(BN5_NAVI_BASE_MAX_HP, (uint16_t)max_hp);
	wr16(BN5_NAVI_MAX_HP, (uint16_t)max_hp);
	wr16(BN5_NAVI_HP, (uint16_t)(hp < 1 ? 1 : hp > max_hp ? max_hp : hp));
	/* (calm at its start, as each of the run's battles begins: BN5's dark
	 * meter and mood would carry its last DarkChip's darkness on) */
	core->rawWrite8(core, BN5_NAVI_MOOD, -1, 0x80);
	wr16(BN5_NAVI_METER, 500);
	uint32_t check = rd32(BN5_TOOLKIT + BN5_TOOLKIT_METER_CHECK);
	if (check >= 0x02000000u && check < 0x02040000u) core->rawWrite32(core, check, -1, 500u ^ rd32(BN5_METER_KEY));
	dark_used = false;
	int in = folder ? folder_in(folder) : 0, held = 0;
	for (int i = 0; folder && i < 30; ++i) held += (folder[i] & 0x1FF) != 0;
	sat_out = held - in;
	rows_fit(record);
	if (emu_debug_on()) fprintf(stderr, "guest: battle %08X, HP %d/%d, %d of the folder's 30 in\n", record, hp, max_hp, in);
	patch_roll(record_copy(record));
	active = true;
	phase = PH_ASKED;
	frames = 0;
	return true;
}

bool guest_active(void) { return active; }

bool guest_custom_screen(void) { return active && phase == PH_BATTLE && rd8(BN5_BATTLE_STATE + 1) == BN5_PHASE_CUSTOM; }

bool guest_on_screen(void) { return active && phase == PH_BATTLE && sub_mode() == BN5_SUB_BATTLE; }

bool guest_fight_hp(int *hp, int *max) {
	if (!guest_on_screen()) return false;
	*hp = rd16(BN5_FIGHT_HP);
	*max = rd16(BN5_FIGHT_HP + 2);
	return true;
}

static void finish(int outcome) {
	result = (GuestResult){ outcome, frames, phase == PH_BATTLE ? rd16(BN5_BATTLE_HP) : rd16(BN5_NAVI_HP), 0, 0, 0, sat_out, false, { 0 } };
	memcpy(result.dark, dark_in, sizeof result.dark);   /* (BN5 keeps a DarkChip once used: the run's stay) */
	result.dark_used = dark_used;
	/* (what its results screen gave, as the run's: a chip by its name, or
	 * zenny) */
	uint16_t v = outcome == GUEST_WON ? rd16(BN5_REWARD) : 0;
	if (v && v >> 14 == 1) result.zenny = v & 0x3FFF;
	else if (v && v >> 14 == 0 && (v & 0x1FF) < BN5_CHIPS) {
		result.chip = from_bn5[v & 0x1FF];
		result.code = v >> 9 & 0x1F;
		if (!result.chip) result.zenny = REWARD_ZENNY;
	}
	result_due = true;
	patch_roll(0);
	active = false;
	phase = PH_IDLE;
}

void guest_frame(uint32_t keys) {
	if (!active) return;
	core->setKeys(core, keys);
	core->runFrame(core);
	emu_audio_from(core);
	++frames;
	int mode = main_mode(), sub = sub_mode();
	if (phase == PH_ASKED && (sub == BN5_SUB_BATTLE_INIT || sub == BN5_SUB_BATTLE)) {
		phase = PH_BATTLE;
		core->rawWrite32(core, BN5_ROLL + 8, -1, 0);   /* (one battle: the roll answers none again) */
		core->rawWrite8(core, BN5_BATTLE_RESULT + 1, -1, 0);
		wr16(BN5_REWARD, 0);
	} else if (phase == PH_ASKED && frames > 600) finish(GUEST_ESCAPED);   /* (never began: nothing happened) */
	/* (a DarkChip used: latched while its battle runs, as leaving it wipes
	 * the counter) */
	else if (phase == PH_BATTLE && sub == BN5_SUB_BATTLE && rd8(BN5_DARK_USED + 8u * rd8(BN5_BATTLE_SIDE))) dark_used = true;
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
bool guest_battle(uint32_t record, int hp, int max_hp, const uint16_t *folder, const uint8_t dark[GUEST_DARK_KINDS]) {
	(void)record; (void)hp; (void)max_hp; (void)folder; (void)dark;
	return false;
}
bool guest_active(void) { return false; }
bool guest_custom_screen(void) { return false; }
bool guest_on_screen(void) { return false; }
bool guest_fight_hp(int *hp, int *max) { (void)hp; (void)max; return false; }
void guest_frame(uint32_t keys) { (void)keys; }
const uint32_t *guest_video(void) { return NULL; }
bool guest_take_result(GuestResult *out) { (void)out; return false; }

#endif
