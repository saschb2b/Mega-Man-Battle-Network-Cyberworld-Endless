/* Native acquisition + native folder editor: independent loadout oracle. */
#include "online_carried_read.h"

#include <stdlib.h>
#include <string.h>
#include <mgba/core/serialize.h>
#include <mgba/internal/arm/arm.h>
#include <mgba/internal/arm/isa-inlines.h>
#include <mgba-util/vfs.h>
#include "online_frame_pair_internal.h"
#include "online_frame_wire.h"

static bool advance(OnlineFramePair *p, unsigned frames, unsigned keys) {
	unsigned target[2];
	for (unsigned side = 0; side < 2; ++side) {
		target[side] = p->core[side]->frameCounter(p->core[side]) + frames;
		p->core[side]->setKeys(p->core[side], keys);
	}
	uint64_t deadline = online_frame_now_ms() + 30000;
	for (uint64_t limit = (uint64_t)frames * 2000000 + 1000000; limit; --limit) {
		if (!(limit & 0x3FFFu) && online_frame_now_ms() >= deadline) return false;
		for (unsigned side = 0; side < 2; ++side) {
			if (!side && p->waiting) continue;
			p->running = (int)side;
			p->core[side]->step(p->core[side]);
		}
		if (p->core[0]->frameCounter(p->core[0]) >= target[0] &&
			p->core[1]->frameCounter(p->core[1]) >= target[1]) return true;
	}
	return false;
}

static unsigned pc(struct mCore *core) {
	struct ARMCore *cpu = core->cpu;
	return (unsigned)cpu->gprs[ARM_PC] - (cpu->cpsr.t ? WORD_SIZE_THUMB : WORD_SIZE_ARM);
}

static bool give(struct mCore *core, unsigned side) {
	unsigned steps = 0;
	while (pc(core) != BN6_OW_HOOK && ++steps < 1000000) core->step(core);
	if (pc(core) != BN6_OW_HOOK) return false;
	struct ARMCore *cpu = core->cpu;
	int32_t regs[16], cpsr = cpu->cpsr.packed;
	memcpy(regs, cpu->gprs, sizeof regs);
	cpu->gprs[0] = 1; cpu->gprs[1] = 0; cpu->gprs[2] = 1;
	cpu->gprs[ARM_LR] = BN6_OW_HOOK | 1;
	_ARMSetMode(cpu, MODE_THUMB);
	cpu->gprs[ARM_PC] = BN6_GIVE_CHIPS & ~1u;
	cpu->cycles += ThumbWritePC(cpu);
	steps = 0;
	while (pc(core) != BN6_OW_HOOK && ++steps < 100000) core->step(core);
	if (pc(core) != BN6_OW_HOOK) return false;
	printf("native-give side=%u id=1 code=0 count=1 steps=%u return_pc=%u\n", side, steps, pc(core));
	/* Only resume the interrupted caller; retain every native inventory write. */
	memcpy(cpu->gprs, regs, sizeof regs);
	cpu->cpsr.packed = cpsr; _ARMReadCPSR(cpu);
	cpu->gprs[ARM_PC] = BN6_OW_HOOK;
	cpu->cycles += ThumbWritePC(cpu);
	return true;
}

static bool menu(OnlineFramePair *p) {
	static const unsigned entry[][2] = {
		{30, 0}, {4, 8}, {40, 0}, {4, 1}, {50, 0}, {4, 1}, {30, 0}, {4, 1}, {60, 0}
	};
	for (unsigned i = 0; i < sizeof entry / sizeof *entry; ++i)
		if (!advance(p, entry[i][0], entry[i][1])) return false;
	for (unsigned i = 0; i < 9; ++i)
		if (!advance(p, 4, 128) || !advance(p, 8, 0)) return false;
	static const unsigned exchange[][2] = {{4, 1}, {40, 0}, {4, 16}, {40, 0}, {4, 1}, {20, 0}};
	for (unsigned i = 0; i < sizeof exchange / sizeof *exchange; ++i)
		if (!advance(p, exchange[i][0], exchange[i][1])) return false;
	for (unsigned i = 0; i < 3; ++i)
		if (!advance(p, 4, 2) || !advance(p, 60, 0)) return false;
	return true;
}

static bool write_loadout(struct mCore *core, const char *prefix, const char *name) {
	OnlineLoadout loadout;
	if (!online_carried_read(core, &loadout)) return false;
	char path[1024];
	if (snprintf(path, sizeof path, "%s-%s.json", prefix, name) >= (int)sizeof path) return false;
	FILE *out = fopen(path, "w");
	if (!out) return false;
	bool ok = online_carried_write(out, &loadout);
	if (fclose(out) || !ok) return false;
	/* Local input for the future independent installer, never a core state. */
	if (snprintf(path, sizeof path, "%s-%s.loadout", prefix, name) >= (int)sizeof path) return false;
	out = fopen(path, "wb");
	if (!out) return false;
	ok = fwrite(&loadout, 1, sizeof loadout, out) == sizeof loadout;
	return !fclose(out) && ok;
}

static bool save_fixture(OnlineFramePair *p, const char *prefix) {
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *core = p->core[side];
		if (core->rawRead8(core, core->rawRead32(core, BN6_TOOLKIT, -1), -1) != BN6_MODE_GAME) return false;
		char path[1024];
		if (snprintf(path, sizeof path, "%s-%u.state", prefix, side) >= (int)sizeof path) return false;
		struct VFile *file = VFileOpen(path, O_CREAT | O_TRUNC | O_RDWR);
		if (!file) return false;
		bool ok = mCoreSaveStateNamed(core, file, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
		file->close(file);
		if (!ok) return false;
	}
	return write_loadout(p->core[0], prefix, "native");
}

int main(int argc, char **argv) {
	if (argc != 4) { fprintf(stderr, "usage: %s ROM BOOT_STATE OUTPUT_PREFIX\n", argv[0]); return 2; }
	OnlineFramePair *pair = online_frame_pair_open(argv[1], argv[2], argv[3]);
	bool ok = pair && write_loadout(pair->core[0], argv[3], "starter") && advance(pair, 30, 0) &&
		give(pair->core[0], 0) && give(pair->core[1], 1) && menu(pair) && save_fixture(pair, argv[3]);
	online_frame_pair_close(pair);
	return ok ? 0 : 1;
}
