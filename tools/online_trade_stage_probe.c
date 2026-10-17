/* Candidate staging against readonly original fixtures; no expected effects. */
#include "online_trade_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/core.h>
#include <mgba/internal/arm/arm.h>

#include "online_trade_stage.h"

static bool output(const char *name, const uint8_t *bytes, size_t n) {
	FILE *file = trade_probe_output(name);
	if (!file) return false;
	bool ok = fwrite(bytes, 1, n, file) == n;
	return !fclose(file) && ok;
}

static uint8_t *input(const char *path, size_t *size) {
	FILE *file = fopen(path, "rb");
	if (!file) return NULL;
	uint8_t *bytes = NULL;
	if (!fseek(file, 0, SEEK_END)) {
		long n = ftell(file);
		if (n > 0 && n < 1024 * 1024 && !fseek(file, 0, SEEK_SET)) {
			bytes = malloc((size_t)n);
			if (bytes && fread(bytes, 1, (size_t)n, file) == (size_t)n) *size = (size_t)n;
			else { free(bytes); bytes = NULL; }
		}
	}
	if (fclose(file)) { free(bytes); bytes = NULL; }
	return bytes;
}

int main(int argc, char **argv) {
	if (argc != 6) { fprintf(stderr, "usage: %s ROM SOURCE OUTPUT GIVE RECEIVE\n", argv[0]); return 2; }
	if (!trade_probe_start(argv[1], argv[2])) return 2;
	trade_probe_directory(argv[3]);
	size_t size = 0, written = 0;
	uint8_t *source = input(argv[2], &size);
	OnlineTradePrepared prepared;
	OnlineTradeTerms terms = {.server={1}, .transaction={2}};
	for (unsigned i = 0; i < 2; ++i) {
		terms.side[i].identity[0] = (uint8_t)(i + 1);
		terms.side[i].run_id[0] = (uint8_t)(i + 3);
		terms.side[i].checkpoint[0] = 5;
		terms.side[i].chip = (uint16_t)strtoul(argv[4 + i], NULL, 10);
	}
	if (!source || !online_trade_capture(&prepared, true, 0, false) || !trade_probe_saved_state("live-before", false)) return 2;
	struct ARMCore *cpu = trade_probe_core->cpu;
	uint32_t regs[16]; memcpy(regs, cpu->gprs, sizeof regs);
	int32_t cpsr = cpu->cpsr.packed;
	uint8_t *staged = online_trade_stage(&prepared, &terms, 0, source, size, &written);
	bool ok = staged && written == size && output("candidate.state", staged, written) &&
		output("source-after.state", source, size) && trade_probe_saved_state("live-after", false);
	free(source); free(staged);
	trade_probe_core->deinit(trade_probe_core);
	if (!ok) return 2;
	char path[2048];
	if (snprintf(path, sizeof path, "%s/candidate.state", argv[3]) >= (int)sizeof path || !trade_probe_start(argv[1], path)) return 2;
	ok = trade_probe_saved_state("reloaded", false) && trade_probe_editor(regs, cpsr);
	trade_probe_core->deinit(trade_probe_core);
	return ok ? 0 : 2;
}
