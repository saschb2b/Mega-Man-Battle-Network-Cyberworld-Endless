/* Actual BN6 link experiment. ROMs and optional boot states remain local.
 * Compile with NDEBUG to match the pinned Release mGBA lockstep ABI. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mgba/core/blip_buf.h>
#include <mgba/core/config.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/io.h>
#include <mgba/internal/gba/sio/lockstep.h>
#include <mgba-util/vfs.h>

#include "bn6.h"
#include "online_link_script.h"
#include "online_link_ipc.h"

typedef struct {
	struct GBASIOLockstep cable;
	struct GBASIOLockstepNode node[2];
	struct mCore *core[2];
	color_t video[2][240 * 160];
	int32_t credit[2];
	unsigned waiting, transfers;
	int running;
	bool scripted;
	unsigned script_start[2], traced[2];
	FILE *trace;
} BattleProbe;

static bool signal_peer(struct mLockstep *lock, unsigned mask) {
	BattleProbe *p = lock->context;
	if (p->running != 0) p->waiting &= ~mask;
	return true;
}
static bool wait_peer(struct mLockstep *lock, unsigned mask) {
	BattleProbe *p = lock->context;
	p->waiting = mask;
	return true;
}
static void add_cycles(struct mLockstep *lock, int id, int32_t cycles) {
	BattleProbe *p = lock->context;
	p->credit[id == 0 ? 1 : id] += cycles;
}
static int32_t use_cycles(struct mLockstep *lock, int id, int32_t cycles) {
	BattleProbe *p = lock->context;
	p->credit[id] -= cycles;
	return p->credit[id];
}
static int32_t unused_cycles(struct mLockstep *lock, int id) {
	BattleProbe *p = lock->context;
	return p->credit[id];
}
static void unload_peer(struct mLockstep *lock, int id) {
	BattleProbe *p = lock->context;
	p->waiting &= ~(1u << (unsigned)id);
}
static void quiet_log(struct mLogger *logger, int category, enum mLogLevel level, const char *format, va_list args) {
	(void)logger; (void)category; (void)level; (void)format; (void)args;
}
static struct mLogger quiet = { .log = quiet_log };

static uint64_t now_ms(void) {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (uint64_t)now.tv_sec * 1000 + (uint64_t)now.tv_nsec / 1000000;
}

static bool open_core(BattleProbe *p, int i, const char *rom, const char *state) {
	struct mCore *c = mCoreFind(rom);
	if (!c || !c->init(c)) return false;
	p->core[i] = c;
	mCoreInitConfig(c, NULL);
	c->setVideoBuffer(c, p->video[i], 240);
	if (!mCoreLoadFile(c, rom)) return false;
	c->loadSave(c, VFileMemChunk(NULL, 0));
	c->setAudioBufferSize(c, 1024);
	blip_set_rates(c->getAudioChannel(c, 0), c->frequency(c), 48000);
	blip_set_rates(c->getAudioChannel(c, 1), c->frequency(c), 48000);
	c->reset(c);
	if (state) {
		struct VFile *vf = VFileOpen(state, O_RDONLY);
		bool ok = vf && mCoreLoadStateNamed(c, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
		if (vf) vf->close(vf);
		if (!ok) return false;
	}
	GBASIOLockstepNodeCreate(&p->node[i]);
	if (!GBASIOLockstepAttachNode(&p->cable, &p->node[i])) return false;
	GBASIOSetDriver(&((struct GBA *)c->board)->sio, &p->node[i].d, SIO_MULTI);
	return true;
}

static bool advance(BattleProbe *p, unsigned frames, unsigned k0, unsigned k1) {
	unsigned target[2];
	for (int i = 0; i < 2; ++i) {
		target[i] = p->core[i]->frameCounter(p->core[i]) + frames;
		p->core[i]->setKeys(p->core[i], i ? k1 : k0);
	}
	unsigned long long limit = (unsigned long long)frames * 2000000 + 1000000;
	uint64_t deadline = now_ms() + 30000;
	while (limit--) {
		if (!(limit & 0x3FFFu) && now_ms() >= deadline) break;
		for (int i = 0; i < 2; ++i) {
			if (i == 0 && p->waiting) continue;
			p->running = i;
			unsigned frame = p->core[i]->frameCounter(p->core[i]);
			if (p->scripted) online_link_battle_input(p->core[i], (unsigned)i, frame - p->script_start[i], NULL);
			enum mLockstepPhase phase = p->cable.d.transferActive;
			p->core[i]->step(p->core[i]);
			if (phase != TRANSFER_IDLE && p->cable.d.transferActive == TRANSFER_IDLE) ++p->transfers;
			if (p->trace && frame != p->traced[i]) {
				uint32_t sample[8];
				online_link_battle_sample(p->core[i], (unsigned)i, sample, NULL);
				fprintf(p->trace, "%d,%u,%u,%u,%u,%u,%u,%u,%u\n", i, frame, sample[2], sample[0],
					sample[3] & 65535, sample[3] >> 16, sample[4], sample[6], sample[7]);
				p->traced[i] = frame;
			}
		}
		if (p->core[0]->frameCounter(p->core[0]) >= target[0] &&
			p->core[1]->frameCounter(p->core[1]) >= target[1]) return true;
	}
	fprintf(stderr, "frame timeout waiting=%u phase=%d credit=%d\n", p->waiting, p->cable.d.transferActive, p->credit[1]);
	return false;
}

static void status(BattleProbe *p) {
	for (int i = 0; i < 2; ++i) {
		struct mCore *c = p->core[i];
		uint32_t main = c->rawRead32(c, BN6_TOOLKIT, -1);
		uint32_t menu = c->rawRead32(c, BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU, -1);
		printf("side=%d frame=%u main=%02x pet=%02x submenu=%08x battle=%08x custom=%08x sio=%04x send=%04x recv=%08x transfers=%u\n", i,
			c->frameCounter(c), c->rawRead8(c, main, -1), c->rawRead8(c, BN6_PET_CURSOR, -1),
			c->rawRead32(c, menu, -1), c->rawRead32(c, BN6_BATTLE_STATE, -1), c->rawRead32(c, BN6_CUSTOM_SCREEN, -1),
			c->busRead16(c, 0x04000000u + REG_SIOCNT), c->busRead16(c, 0x04000000u + REG_SIOMLT_SEND),
			c->busRead32(c, 0x04000000u + REG_SIOMULTI0), p->transfers);
		printf("side=%d battle_side=%u timer=%u result=%u cursor=%u", i,
			c->rawRead8(c, BN6_BATTLE_SIDE, -1), c->rawRead32(c, BN6_BATTLE_TIMER, -1),
			c->rawRead8(c, BN6_BATTLE_RESULT, -1), c->rawRead8(c, BN6_CUSTOM_CURSOR, -1));
		for (unsigned j = 0; j < 2; ++j) {
			uint32_t obj = BN6_T1_OBJECTS + j * BN6_T1_SIZE;
			printf(" obj%u=%u,%u,%u,%u,%u", j, c->rawRead16(c, obj + BN6_T1_NAME_ID, -1),
				c->rawRead16(c, obj + BN6_T1_HP, -1), c->rawRead8(c, obj + BN6_T1_ALLIANCE, -1),
				c->rawRead8(c, obj + BN6_T1_PANEL_X, -1), c->rawRead8(c, obj + BN6_T1_PANEL_Y, -1));
		}
		putchar('\n');
		printf("side=%d hand=", i);
		for (unsigned j = 0; j < 5; ++j)
			printf("%s%04x", j ? "," : "", c->rawRead16(c, BN6_BATTLE_DECK + j * 2, -1));
		putchar('\n');
	}
	fflush(stdout);
}

static bool screenshot(BattleProbe *p, const char *prefix) {
	for (int side = 0; side < 2; ++side) {
		char path[520];
		snprintf(path, sizeof path, "%s-%d.ppm", prefix, side);
		FILE *file = fopen(path, "wb");
		if (!file) return false;
		fprintf(file, "P6\n240 160\n255\n");
		for (unsigned i = 0; i < 240 * 160; ++i) {
			uint32_t color = p->video[side][i];
			uint8_t rgb[] = {color & 255, (color >> 8) & 255, (color >> 16) & 255};
			fwrite(rgb, 1, sizeof rgb, file);
		}
		fclose(file);
	}
	return true;
}

static bool practice(BattleProbe *p, const char *prefix) {
	unsigned count;
	const OnlineLinkInput *steps = online_link_entry_steps(&count);
	for (unsigned i = 0; i < count; ++i)
		if (!advance(p, steps[i].frames, steps[i].keys[0], steps[i].keys[1])) return false;
	for (unsigned i = 0; i < 2; ++i) {
		if (p->core[i]->rawRead8(p->core[i], BN6_BATTLE_PHASE, -1) != BN6_PHASE_CUSTOM) return false;
		p->script_start[i] = p->core[i]->frameCounter(p->core[i]);
	}
	char path[600];
	snprintf(path, sizeof path, "%s-start", prefix);
	if (!screenshot(p, path)) return false;
	snprintf(path, sizeof path, "%s-trace.csv", prefix);
	p->trace = fopen(path, "w");
	if (!p->trace) return false;
	fputs("core,frame,battle_timer,result,hp0,hp1,positions,menu,battle_state\n", p->trace);
	p->scripted = true;
	bool ok = advance(p, online_link_battle_frames(), 0, 0);
	p->scripted = false;
	fclose(p->trace);
	p->trace = NULL;
	uint32_t sample[2][8];
	for (unsigned i = 0; i < 2; ++i) online_link_battle_sample(p->core[i], i, sample[i], NULL);
	ok = ok && online_link_battle_agrees(sample[0], sample[1]);
	snprintf(path, sizeof path, "%s-result", prefix);
	ok = screenshot(p, path) && ok;
	printf("practice passed=%s transfers=%u results=%u,%u timers=%u,%u hp=%u,%u\n", ok ? "yes" : "NO",
		p->transfers, sample[0][0], sample[1][0], sample[0][2], sample[1][2], sample[0][3] & 65535, sample[0][3] >> 16);
	snprintf(path, sizeof path, "%s-report.json", prefix);
	FILE *report = fopen(path, "w");
	if (!report) return false;
	fprintf(report, "{\"passed\":%s,\"cores\":2,\"processes\":1,\"serial_transfers\":%u,\"winner\":0,\"results\":[%u,%u],"
		"\"battle_timers\":[%u,%u],\"global_hp\":[%u,%u],\"battle_agreement\":%s,\"entry\":\"PET Comm > Link Cable > NetBattle > Single > Practice\","
		"\"inputs_only\":true,\"rom_patches\":false}\n", ok ? "true" : "false", p->transfers, sample[0][0], sample[1][0],
		sample[0][2], sample[1][2], sample[0][3] & 65535, sample[0][3] >> 16, ok ? "true" : "false");
	fclose(report);
	return ok;
}

static bool ipc_battle(BattleProbe *p, const char *line) {
	bool active = !strncmp(line, "ipcactive ", 10);
	bool full = active || !strncmp(line, "ipcfull ", 8);
	struct OnlineLinkIpcConfig config = {
		.initial_credit = {p->credit[0], p->credit[1]}, .initial_waiting = p->waiting,
		.frames = full ? online_link_full_frames() : online_link_battle_frames(),
		.timeout_ms = 5000, .wall_limit_ms = 60000,
		.delay_after_frames = active ? 0 : full ? online_link_entry_frames() : 0,
		.delay_ready = active ? online_link_battle_active : NULL,
		.frame = full ? online_link_full_input : online_link_battle_input, .sample = online_link_battle_sample,
		.trace_path = getenv("ONLINE_LINK_IPC_TRACE")
	};
	if (sscanf(line + (active ? 10 : full ? 8 : 4), "%u %u %u %u", &config.delay_ms, &config.wall_limit_ms,
		&config.disconnect_after, &config.corrupt_after) < 2) return false;
	struct OnlineLinkIpcResult result;
	bool ran = online_link_ipc_run(&p->cable, p->core, &config, &result);
	bool agrees = online_link_battle_agrees(result.sample[0], result.sample[1]);
	printf("{\"ipc\":true,\"status\":\"%s\",\"delay_ms\":%u,\"elapsed_ms\":%llu,\"pids\":[%d,%d],"
		"\"frames\":[%u,%u],\"transfers\":[%u,%u],\"delayed_transfers\":%llu,\"applied_delay_ms\":%llu,"
		"\"battle_agreement\":%s,\"results\":[%u,%u],\"timers\":[%u,%u],\"hp\":[%u,%u],\"transcripts\":[\"%016llx\",\"%016llx\"],"
		"\"delay_scope\":\"%s\",\"delay_start_frames\":[%u,%u],\"delay_start_transfers\":[%u,%u],\"delay_start_elapsed_ms\":%llu}\n",
		online_link_ipc_status(result.status), config.delay_ms, (unsigned long long)result.elapsed_ms,
		result.pids[0], result.pids[1], result.frames[0], result.frames[1], result.transfers[0], result.transfers[1],
		(unsigned long long)result.delayed_transfers, (unsigned long long)result.applied_delay_ms, agrees ? "true" : "false",
		result.sample[0][0], result.sample[1][0], result.sample[0][2], result.sample[1][2],
		result.sample[0][3] & 65535, result.sample[0][3] >> 16,
		(unsigned long long)result.transcript[0], (unsigned long long)result.transcript[1],
		active ? "active-battle-onward" : "custom-screen-onward", result.delay_start_frames[0], result.delay_start_frames[1],
		result.delay_start_transfers[0], result.delay_start_transfers[1], (unsigned long long)result.delay_start_elapsed_ms);
	fflush(stdout);
	return ran && (result.status != ONLINE_LINK_IPC_COMPLETE || agrees);
}

int main(int argc, char **argv) {
	bool automatic = argc == 5 && !strcmp(argv[1], "--practice");
	if (!automatic && (argc < 2 || argc > 3)) {
		fprintf(stderr, "usage: %s ROM [BOOT_STATE] < commands\n       %s --practice ROM BOOT_STATE OUTPUT_PREFIX\n", argv[0], argv[0]);
		return 2;
	}
	mLogSetDefaultLogger(&quiet);
	BattleProbe *p = calloc(1, sizeof *p);
	if (!p) return 1;
	mLockstepInit(&p->cable.d);
	GBASIOLockstepInit(&p->cable);
	p->cable.d.context = p;
	p->cable.d.signal = signal_peer;
	p->cable.d.wait = wait_peer;
	p->cable.d.addCycles = add_cycles;
	p->cable.d.useCycles = use_cycles;
	p->cable.d.unusedCycles = unused_cycles;
	p->cable.d.unload = unload_peer;
	const char *rom = argv[automatic ? 2 : 1], *state = automatic ? argv[3] : argc == 3 ? argv[2] : NULL;
	bool ok = open_core(p, 0, rom, state) && open_core(p, 1, rom, state);
	if (ok && automatic) ok = practice(p, argv[4]);
	char line[1024];
	while (ok && !automatic && fgets(line, sizeof line, stdin)) {
		unsigned frames = 0, k0 = 0, k1 = 0;
		char path[512];
		if (sscanf(line, "run %u %x %x", &frames, &k0, &k1) >= 1) ok = advance(p, frames, k0, k1);
		else if (sscanf(line, "shot %511s", path) == 1) ok = screenshot(p, path);
		else if (!strncmp(line, "ipc ", 4) || !strncmp(line, "ipcfull ", 8) || !strncmp(line, "ipcactive ", 10)) ok = ipc_battle(p, line);
		else if (strncmp(line, "status", 6)) { fprintf(stderr, "Unknown command: %s", line); ok = false; }
		status(p);
	}
	for (int i = 0; i < 2; ++i) if (p->core[i]) {
		GBASIOSetDriver(&((struct GBA *)p->core[i]->board)->sio, NULL, SIO_MULTI);
		GBASIOLockstepDetachNode(&p->cable, &p->node[i]);
		p->core[i]->deinit(p->core[i]);
	}
	free(p);
	return ok ? 0 : 1;
}
