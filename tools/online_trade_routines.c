/* Original BN6 chip calls, observed in a private core. Local evidence only. */
#include "online_trade_runtime.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/core.h>
#include <mgba/internal/arm/arm.h>
#include <mgba/internal/arm/isa-inlines.h>

#include "data.h"
#include "emu.h"

static void write16(uint32_t address, uint16_t value) { trade_probe_core->rawWrite16(trade_probe_core, address, -1, value); }

static FILE *calls;

typedef struct { unsigned give, receive, slot, outgoing, incoming, counter; } Fixture;
static const Fixture fixtures[] = {
	{2, 3, 0, 3, 0, 0x7fff}, {3, 2, 1, 3, 0, 0x7fff},
	{2, 3, 0, 1, 0, 0}, {3, 2, 1, 3, 98, 1}
};

static void registers(FILE *file, const uint32_t regs[16]) {
	fputc('[', file);
	for (unsigned i = 0; i < 16; ++i) fprintf(file, "%s%u", i ? "," : "", regs[i]);
	fputc(']', file);
}

static bool original_call(const char *name, unsigned function, unsigned chip, unsigned code, unsigned count) {
	struct ARMCore *cpu = trade_probe_core->cpu;
	uint32_t before[16], after[16];
	memcpy(before, cpu->gprs, sizeof before);
	unsigned back = trade_probe_program_counter(), low_sp = (unsigned)cpu->gprs[ARM_SP];
	cpu->gprs[0] = (int32_t)chip; cpu->gprs[1] = (int32_t)code; cpu->gprs[2] = (int32_t)count;
	cpu->gprs[ARM_LR] = (int32_t)(back | 1u);
	_ARMSetMode(cpu, MODE_THUMB);
	cpu->gprs[ARM_PC] = (int32_t)(function & ~1u);
	cpu->cycles += ThumbWritePC(cpu);
	unsigned steps = 0;
	while (trade_probe_program_counter() != back && steps < 100000) {
		if ((unsigned)cpu->gprs[ARM_SP] < low_sp) low_sp = (unsigned)cpu->gprs[ARM_SP];
		trade_probe_core->step(trade_probe_core); ++steps;
	}
	memcpy(after, cpu->gprs, sizeof after);
	fprintf(calls, "%s\"%s\":{\"function\":%u,\"arguments\":[%u,%u,%u],\"return_pc\":%u,\"steps\":%u,\"minimum_sp\":%u,\"before\":",
		!strcmp(name, "noop") ? "{" : ",", name, function, chip, code, count, trade_probe_program_counter(), steps, low_sp);
	registers(calls, before); fputs(",\"after\":", calls); registers(calls, after); fputc('}', calls);
	return trade_probe_program_counter() == back && steps > 0 && steps < 100000;
}

static bool fixture(const Fixture *f, OnlineTradePrepared *prepared) {
	uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	uint32_t marks = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIP_MARKS);
	unsigned give_code = trade_probe_chip_code(f->give, f->slot), receive_code = trade_probe_chip_code(f->receive, f->slot);
	if (give_code > CHIP_CODE_STAR || receive_code > CHIP_CODE_STAR) return false;
	emu_write8(pack + f->give * BN6_PACK_ENTRY + f->slot, (uint8_t)f->outgoing);
	emu_write8(pack + f->receive * BN6_PACK_ENTRY + f->slot, (uint8_t)f->incoming);
	emu_write8(marks + f->give, emu_read8(BN6_CHIP_KEYS + f->give) ^ BN6_CHIP_KEY_XOR);
	emu_write8(marks + f->receive, emu_read8(BN6_CHIP_KEYS + f->receive) ^ BN6_CHIP_KEY_XOR ^ (f->incoming ? 0 : 1));
	unsigned flag = BN6_FLAG_LIBRARY + f->receive;
	uint32_t at = BN6_EVENT_FLAGS + flag / 8;
	unsigned mask = 0x80u >> (flag & 7);
	unsigned value = emu_read8(at);
	emu_write8(at, (uint8_t)(f->incoming ? value | mask : value & ~mask));
	write16(pack + f->receive * BN6_PACK_ENTRY + BN6_PACK_ORDER + 2 * f->slot, 0xbeef);
	write16(BN6_CHIP_ORDER_COUNTER, (uint16_t)f->counter);
	if (!online_trade_capture(prepared, true, 0, false)) return false;
	OnlineTradeInventory view = online_trade_inventory_view(prepared);
	return online_trade_check(&view, (uint16_t)(f->give | give_code << 9),
		(uint16_t)(f->receive | receive_code << 9), false) == ONLINE_TRADE_VALID;
}

int main(int argc, char **argv) {
	if (argc != 5) { fprintf(stderr, "usage: %s ROM STATE OUTPUT CASE(0-3)\n", argv[0]); return 2; }
	char *end;
	errno = 0;
	unsigned long which = strtoul(argv[4], &end, 10);
	if (errno || !*argv[4] || *end || which >= sizeof fixtures / sizeof *fixtures || !trade_probe_start(argv[1], argv[2])) return 2;
	trade_probe_directory(argv[3]);
	const Fixture *f = &fixtures[which];
	OnlineTradePrepared prepared;
	if (!fixture(f, &prepared) || !trade_probe_saved_state("before", false)) return 2;
	struct ARMCore *cpu = trade_probe_core->cpu;
	uint32_t regs[16]; memcpy(regs, cpu->gprs, sizeof regs);
	int32_t cpsr = cpu->cpsr.packed;
	calls = trade_probe_output("calls.json");
	if (!calls) return 2;
	bool ok = original_call("noop", BN6_TAKE_CHIPS, f->give, trade_probe_chip_code(f->give, f->slot), 0) &&
		trade_probe_saved_state("noop", false) && trade_probe_saved_state("before", true) &&
		original_call("take", BN6_TAKE_CHIPS, f->give, trade_probe_chip_code(f->give, f->slot), 1) && trade_probe_saved_state("taken", false) &&
		original_call("give", BN6_GIVE_CHIPS, f->receive, trade_probe_chip_code(f->receive, f->slot), 1) && trade_probe_saved_state("given", false);
	fputs("}\n", calls);
	if (fclose(calls)) ok = false;
	if (ok) ok = trade_probe_editor(regs, cpsr);
	trade_probe_core->deinit(trade_probe_core);
	return ok ? 0 : 2;
}
