/* Private-core observation helpers shared by the two proof executables. */
#include "online_trade_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/blip_buf.h>
#include <mgba/core/config.h>
#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba/core/serialize.h>
#include <mgba/internal/arm/arm.h>
#include <mgba/internal/arm/isa-inlines.h>
#include <mgba-util/vfs.h>

#include "data.h"
#include "emu.h"
#include "rom.h"
#include "run.h"

Run run;
struct mCore *trade_probe_core;
static const char *directory;

void trade_probe_directory(const char *path) { directory = path; }

uint8_t emu_read8(uint32_t address) { return trade_probe_core->rawRead8(trade_probe_core, address, -1); }
uint16_t emu_read16(uint32_t address) { return trade_probe_core->rawRead16(trade_probe_core, address, -1); }
uint32_t emu_read32(uint32_t address) { return trade_probe_core->rawRead32(trade_probe_core, address, -1); }
void emu_write8(uint32_t address, uint8_t value) { trade_probe_core->rawWrite8(trade_probe_core, address, -1, value); }

static void quiet(struct mLogger *logger, int category, enum mLogLevel level, const char *format, va_list args) {
	(void)logger; (void)category; (void)level; (void)format; (void)args;
}
static struct mLogger logger = { .log = quiet };

FILE *trade_probe_output(const char *name) {
	char path[2048];
	if (snprintf(path, sizeof path, "%s/%s", directory, name) >= (int)sizeof path) return NULL;
	return fopen(path, "wb");
}

bool trade_probe_saved_state(const char *name, bool loading) {
	char path[2048];
	if (snprintf(path, sizeof path, "%s/%s.state", directory, name) >= (int)sizeof path) return false;
	struct VFile *file = VFileOpen(path, loading ? O_RDONLY : O_CREAT | O_TRUNC | O_RDWR);
	if (!file) return false;
	bool ok = loading ? mCoreLoadStateNamed(trade_probe_core, file, SAVESTATE_SAVEDATA | SAVESTATE_RTC) :
		mCoreSaveStateNamed(trade_probe_core, file, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	file->close(file);
	return ok;
}

unsigned trade_probe_program_counter(void) {
	struct ARMCore *cpu = trade_probe_core->cpu;
	return (unsigned)cpu->gprs[ARM_PC] - (cpu->cpsr.t ? WORD_SIZE_THUMB : WORD_SIZE_ARM);
}

unsigned trade_probe_chip_code(unsigned id, unsigned slot) {
	return R.data[R.layout->chip_data + id * BN6_CHIP_RECORD_SIZE + BN6_CHIP_CODES + slot];
}

static void trade_probe_advance(unsigned frames, unsigned keys) {
	trade_probe_core->setKeys(trade_probe_core, keys);
	for (unsigned i = 0; i < frames; ++i) trade_probe_core->runFrame(trade_probe_core);
}

static void trade_probe_tap(unsigned keys) { trade_probe_advance(2, keys); trade_probe_advance(8, 0); }

/* Restore only the interrupted map caller's registers to resume the original
 * editor. given.state keeps the calls' CPU and stack effects; inventory is
 * never restored or copied from an expected-result buffer. */
bool trade_probe_editor(const uint32_t regs[16], int32_t cpsr) {
	struct ARMCore *cpu = trade_probe_core->cpu;
	memcpy(cpu->gprs, regs, 16 * sizeof *regs);
	cpu->cpsr.packed = cpsr;
	_ARMReadCPSR(cpu);
	cpu->gprs[ARM_PC] = (int32_t)BN6_OW_HOOK;
	cpu->cycles += ThumbWritePC(cpu);
	trade_probe_advance(30, 0); trade_probe_tap(KEY_START); trade_probe_advance(40, 0); trade_probe_tap(KEY_A); trade_probe_advance(50, 0);
	trade_probe_tap(KEY_A); trade_probe_advance(30, 0); trade_probe_tap(KEY_A); trade_probe_advance(60, 0);
	uint32_t menu = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU);
	if (emu_read8(emu_read32(BN6_TOOLKIT)) != BN6_MODE_SUBMENU || emu_read8(menu) != BN6_SUBMENU_EDIT) return false;
	uint8_t counts[512 * 27] = {0};
	uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	for (unsigned id = 1; id < BN6_PACK_IDS; ++id) {
		for (unsigned slot = 0; slot < ONLINE_TRADE_CODES; ++slot) {
			unsigned code = trade_probe_chip_code(id, slot);
			if (code <= CHIP_CODE_STAR) counts[id * 27 + code] = emu_read8(pack + id * BN6_PACK_ENTRY + slot);
		}
	}
	FILE *file = trade_probe_output("editor.counts");
	return file && fwrite(counts, 1, sizeof counts, file) == sizeof counts && !fclose(file);
}

bool trade_probe_start(const char *rom, const char *state) {
	char message[512];
	if (!rom_load_file(rom, message, sizeof message)) return false;
	mLogSetDefaultLogger(&logger);
	trade_probe_core = mCoreFind(rom);
	if (!trade_probe_core || !trade_probe_core->init(trade_probe_core)) return false;
	mCoreInitConfig(trade_probe_core, NULL);
	static color_t video[EMU_W * EMU_H];
	trade_probe_core->setVideoBuffer(trade_probe_core, video, EMU_W);
	if (!mCoreLoadFile(trade_probe_core, rom)) return false;
	trade_probe_core->loadSave(trade_probe_core, VFileMemChunk(NULL, 0));
	trade_probe_core->setAudioBufferSize(trade_probe_core, 1024);
	for (int i = 0; i < 2; ++i) blip_set_rates(trade_probe_core->getAudioChannel(trade_probe_core, i), trade_probe_core->frequency(trade_probe_core), 48000);
	trade_probe_core->reset(trade_probe_core);
	struct VFile *file = VFileOpen(state, O_RDONLY);
	if (!file) return false;
	bool ok = mCoreLoadStateNamed(trade_probe_core, file, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	file->close(file);
	for (unsigned steps = 0; ok && trade_probe_program_counter() != BN6_OW_HOOK && steps < 1000000; ++steps) trade_probe_core->step(trade_probe_core);
	return ok && trade_probe_program_counter() == BN6_OW_HOOK;
}
