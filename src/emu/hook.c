/* Hooks (hook.h): a Thumb BKPT #0xCE (or an ARM one) written over an
 * instruction of the core's ROM copy, or of RAM. mGBA runs a BKPT's handler
 * at no cost in cycles, with the PC already past it (src/arm/isa-thumb.c);
 * the board's handler takes immediate 0 (the debugger) and 1 (the cheat
 * device), so ours comes first and hands it every other. ARMRunFake then puts
 * an instruction into the prefetch in the BKPT's place: the original, which
 * runs as if never replaced, as mGBA's own cheat hooks do; or bx lr, to
 * return, or bx r12, to go on elsewhere. */
#include "hook.h"

#include <string.h>

#include <mgba/core/core.h>
#include <mgba/internal/arm/arm.h>
#include <mgba/internal/gba/gba.h>

#define HOOK_IMM     0xCE
#define THUMB_BKPT   (0xBE00u | HOOK_IMM)
#define ARM_BKPT     (0xE1200070u | (HOOK_IMM & 0xFu) | ((HOOK_IMM & 0xFFF0u) << 4))
#define THUMB_BX_LR  0x4770u
#define THUMB_BX_R12 0x4760u
#define ARM_BX_LR    0xE12FFF1Eu
#define ARM_BX_R12   0xE12FFF1Cu
#define MAX_HOOKS    48
#define QUEUE        256   /* a power of two */

typedef struct {
	uint32_t addr, original;
	bool arm;
	bool live;    /* false once removed: kept, so a BKPT the CPU had fetched still runs the original */
	EmuHook fn;   /* NULL: an event hook, queuing its kind */
	void *user;
	int kind;
} Hook;

static struct mCore *core;
static void (*board16)(struct ARMCore *, int);
static void (*board32)(struct ARMCore *, int);
static Hook hooks[MAX_HOOKS];
static int nhooks;
static HookEvent queue[QUEUE];
static uint32_t qhead, qtail;
uint32_t hook_hits, hook_dropped, hook_strays;

uint8_t hook_read8(uint32_t a) { return (uint8_t)core->rawRead8(core, a, -1); }
uint16_t hook_read16(uint32_t a) { return (uint16_t)core->rawRead16(core, a, -1); }
uint32_t hook_read32(uint32_t a) { return core->rawRead32(core, a, -1); }
void hook_write8(uint32_t a, uint8_t v) { core->rawWrite8(core, a, -1, v); }
void hook_write16(uint32_t a, uint16_t v) { core->rawWrite16(core, a, -1, v); }
void hook_write32(uint32_t a, uint32_t v) { core->rawWrite32(core, a, -1, v); }

static Hook *find(uint32_t addr) {
	for (int i = 0; i < nhooks; ++i)
		if (hooks[i].addr == addr) return &hooks[i];
	return NULL;
}

static void enqueue(uint32_t addr, int kind, const HookRegs *r) {
	if (qtail - qhead >= QUEUE) { ++hook_dropped; return; }
	HookEvent *e = &queue[qtail++ & (QUEUE - 1)];
	e->addr = addr;
	e->kind = kind;
	memcpy(e->r, r->r, sizeof e->r);
	e->lr = r->lr;
}

void hook_post(const HookRegs *r, int kind) { enqueue(r->pc, kind, r); }

/* (as the BIOS's Halt: the CPU idle, its cycles skipped, until an enabled
 * interrupt is raised) */
void hook_halt(void) { GBAHalt((struct GBA *)core->board); }

static void hit(struct ARMCore *cpu, bool arm) {
	/* (the PC runs two instructions ahead of the one executing) */
	uint32_t at = (uint32_t)cpu->gprs[ARM_PC] - (arm ? 8u : 4u);
	Hook *h = find(at);
	if (!h || h->arm != arm) { ++hook_strays; return; }
	++hook_hits;
	HookAct act = HOOK_CONTINUE;
	if (h->live) {
		HookRegs r = { .sp = (uint32_t)cpu->gprs[ARM_SP], .lr = (uint32_t)cpu->gprs[ARM_LR], .pc = at };
		for (int i = 0; i < 13; ++i) r.r[i] = (uint32_t)cpu->gprs[i];
		if (h->fn) act = h->fn(&r, h->user);
		else enqueue(h->addr, h->kind, &r);
		for (int i = 0; i < 13; ++i) cpu->gprs[i] = (int32_t)r.r[i];
		cpu->gprs[ARM_LR] = (int32_t)r.lr;
	}
	if (act == HOOK_RETURN) ARMRunFake(cpu, arm ? ARM_BX_LR : THUMB_BX_LR);
	else if (act == HOOK_JUMP) ARMRunFake(cpu, arm ? ARM_BX_R12 : THUMB_BX_R12);
	else ARMRunFake(cpu, h->original);
}

static void on_bkpt16(struct ARMCore *cpu, int imm) {
	if (imm == HOOK_IMM) hit(cpu, false);
	else if (board16) board16(cpu, imm);
}

static void on_bkpt32(struct ARMCore *cpu, int imm) {
	if (imm == HOOK_IMM) hit(cpu, true);
	else if (board32) board32(cpu, imm);
}

void hook_attach(struct mCore *c) {
	core = c;
	struct ARMCore *cpu = c->cpu;
	/* (the board installs its handler once, in GBAInit; a reset keeps ours) */
	if (cpu->irqh.bkpt16 != on_bkpt16) { board16 = cpu->irqh.bkpt16; cpu->irqh.bkpt16 = on_bkpt16; }
	if (cpu->irqh.bkpt32 != on_bkpt32) { board32 = cpu->irqh.bkpt32; cpu->irqh.bkpt32 = on_bkpt32; }
	nhooks = 0;
	qhead = qtail = 0;
}

static void put_bkpt(Hook *h) {
	if (h->arm) {
		h->original = hook_read32(h->addr);
		hook_write32(h->addr, ARM_BKPT);
	} else {
		h->original = hook_read16(h->addr);
		hook_write16(h->addr, (uint16_t)THUMB_BKPT);
	}
}

static bool add(uint32_t addr, bool arm, EmuHook fn, void *user, int kind) {
	if (!core) return false;
	addr &= arm ? ~3u : ~1u;
	Hook *h = find(addr);
	if (h && h->live) return false;
	/* (a removed hook's slot again: its original is back in memory) */
	if (!h) {
		if (nhooks == MAX_HOOKS) return false;
		h = &hooks[nhooks++];
	}
	*h = (Hook){ .addr = addr, .arm = arm, .live = true, .fn = fn, .user = user, .kind = kind };
	put_bkpt(h);
	return true;
}

bool hook_add(uint32_t addr, EmuHook fn, void *user) { return fn && add(addr, false, fn, user, 0); }
bool hook_add_arm(uint32_t addr, EmuHook fn, void *user) { return fn && add(addr, true, fn, user, 0); }
bool hook_add_event(uint32_t addr, int kind) { return add(addr, false, NULL, NULL, kind); }

void hook_remove(uint32_t addr) {
	Hook *h = find(addr & ~1u);
	if (!h || !h->live) return;
	if (h->arm) hook_write32(h->addr, h->original);
	else hook_write16(h->addr, (uint16_t)h->original);
	/* (kept where the CPU may have fetched its BKPT already: the two
	 * instructions at and before the PC) */
	uint32_t pc = (uint32_t)((struct ARMCore *)core->cpu)->gprs[ARM_PC];
	if (h->addr <= pc && pc - h->addr <= (h->arm ? 4u : 2u)) { h->live = false; return; }
	*h = hooks[--nhooks];
}

void hook_reapply(void) {
	for (int i = 0; i < nhooks; ++i) {
		Hook *h = &hooks[i];
		if (!h->live || h->addr >= 0x08000000u) continue;
		uint32_t now = h->arm ? hook_read32(h->addr) : hook_read16(h->addr);
		if (now != (h->arm ? ARM_BKPT : THUMB_BKPT)) put_bkpt(h);
	}
}

int hook_drain(HookEvent *out, int max) {
	int n = 0;
	while (n < max && qhead != qtail) out[n++] = queue[qhead++ & (QUEUE - 1)];
	return n;
}
