/* Hooks (hook.h): a Thumb BKPT #0xCE (or an ARM one) written over an
 * instruction of the core's ROM copy, or of RAM. mGBA runs a BKPT's handler
 * at no cost in cycles, with the PC already past it (src/arm/isa-thumb.c);
 * the board's handler takes immediate 0 (the debugger) and 1 (the cheat
 * device), so ours comes first and hands it every other. ARMRunFake then puts
 * an instruction into the prefetch in the BKPT's place: the original, which
 * runs as if never replaced, as mGBA's own cheat hooks do; or bx lr, to
 * return, or bx r12, to go on elsewhere. Each core attached has its own
 * table and queue, found by the CPU that met the BKPT. */
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
#define HOOK_CORES   3     /* cores attached at once: BN6's, the guest's (the differential's), a test's */

typedef struct {
	uint32_t addr, original;
	bool arm;
	bool live;    /* false once removed: kept, so a BKPT the CPU had fetched still runs the original */
	EmuHook fn;   /* NULL: an event hook, queuing its kind */
	void *user;
	int kind;
} Hook;

/* A core's hooks: its CPU (NULL: the entry is free), the board's BKPT
 * handlers ours stand before, its table and the queue of its events */
struct HookCore {
	struct mCore *core;
	struct ARMCore *cpu;
	void (*board16)(struct ARMCore *, int);
	void (*board32)(struct ARMCore *, int);
	Hook hooks[MAX_HOOKS];
	int nhooks;
	HookEvent queue[QUEUE];
	uint32_t qhead, qtail;
};

static HookCore cores[HOOK_CORES];
static HookCore *first;     /* hook_attach's: the core the calls that name none take (BN6's) */
static HookCore *running;   /* the core whose hook runs now (one core's frame at a time: see hook.h) */
uint32_t hook_hits, hook_dropped, hook_strays;

/* the core a hook's reads and writes go to: the one whose hook runs, else the first */
static struct mCore *here(void) { return (running ? running : first)->core; }

uint8_t hook_read8(uint32_t a) { return (uint8_t)here()->rawRead8(here(), a, -1); }
uint16_t hook_read16(uint32_t a) { return (uint16_t)here()->rawRead16(here(), a, -1); }
uint32_t hook_read32(uint32_t a) { return here()->rawRead32(here(), a, -1); }
void hook_write8(uint32_t a, uint8_t v) { here()->rawWrite8(here(), a, -1, v); }
void hook_write16(uint32_t a, uint16_t v) { here()->rawWrite16(here(), a, -1, v); }
void hook_write32(uint32_t a, uint32_t v) { here()->rawWrite32(here(), a, -1, v); }

static Hook *find(HookCore *c, uint32_t addr) {
	for (int i = 0; i < c->nhooks; ++i)
		if (c->hooks[i].addr == addr) return &c->hooks[i];
	return NULL;
}

static HookCore *core_of(const struct ARMCore *cpu) {
	for (int i = 0; i < HOOK_CORES; ++i)
		if (cores[i].cpu == cpu) return &cores[i];
	return NULL;
}

static void enqueue(HookCore *c, uint32_t addr, int kind, const HookRegs *r) {
	if (c->qtail - c->qhead >= QUEUE) { ++hook_dropped; return; }
	HookEvent *e = &c->queue[c->qtail++ & (QUEUE - 1)];
	e->addr = addr;
	e->kind = kind;
	memcpy(e->r, r->r, sizeof e->r);
	e->lr = r->lr;
}

void hook_post(const HookRegs *r, int kind) { enqueue(running ? running : first, r->pc, kind, r); }

/* (as the BIOS's Halt: the CPU idle, its cycles skipped, until an enabled
 * interrupt is raised) */
void hook_halt(void) { GBAHalt((struct GBA *)here()->board); }

static void hit(HookCore *c, struct ARMCore *cpu, bool arm) {
	/* (the PC runs two instructions ahead of the one executing) */
	uint32_t at = (uint32_t)cpu->gprs[ARM_PC] - (arm ? 8u : 4u);
	Hook *h = find(c, at);
	if (!h || h->arm != arm) { ++hook_strays; return; }
	++hook_hits;
	HookAct act = HOOK_CONTINUE;
	if (h->live) {
		HookRegs r = { .sp = (uint32_t)cpu->gprs[ARM_SP], .lr = (uint32_t)cpu->gprs[ARM_LR], .pc = at };
		for (int i = 0; i < 13; ++i) r.r[i] = (uint32_t)cpu->gprs[i];
		HookCore *was = running;
		running = c;
		if (h->fn) act = h->fn(&r, h->user);
		else enqueue(c, h->addr, h->kind, &r);
		running = was;
		for (int i = 0; i < 13; ++i) cpu->gprs[i] = (int32_t)r.r[i];
		cpu->gprs[ARM_LR] = (int32_t)r.lr;
	}
	if (act == HOOK_RETURN) ARMRunFake(cpu, arm ? ARM_BX_LR : THUMB_BX_LR);
	else if (act == HOOK_JUMP) ARMRunFake(cpu, arm ? ARM_BX_R12 : THUMB_BX_R12);
	else ARMRunFake(cpu, h->original);
}

static void on_bkpt16(struct ARMCore *cpu, int imm) {
	HookCore *c = core_of(cpu);
	if (c && imm == HOOK_IMM) hit(c, cpu, false);
	else if (c && c->board16) c->board16(cpu, imm);
}

static void on_bkpt32(struct ARMCore *cpu, int imm) {
	HookCore *c = core_of(cpu);
	if (c && imm == HOOK_IMM) hit(c, cpu, true);
	else if (c && c->board32) c->board32(cpu, imm);
}

/* the core's entry, made where it has none (NULL: every entry taken); its
 * table and queue emptied */
static HookCore *attach(struct mCore *core) {
	struct ARMCore *cpu = core->cpu;
	HookCore *c = core_of(cpu);
	if (!c) c = core_of(NULL);
	if (!c) return NULL;
	/* (the board installs its handler once, in GBAInit; a reset keeps ours) */
	if (cpu->irqh.bkpt16 != on_bkpt16) { c->board16 = cpu->irqh.bkpt16; cpu->irqh.bkpt16 = on_bkpt16; }
	if (cpu->irqh.bkpt32 != on_bkpt32) { c->board32 = cpu->irqh.bkpt32; cpu->irqh.bkpt32 = on_bkpt32; }
	c->core = core;
	c->cpu = cpu;
	c->nhooks = 0;
	c->qhead = c->qtail = 0;
	return c;
}

void hook_attach(struct mCore *core) {
	HookCore *c = attach(core);
	if (c) first = c;
}

HookCore *hook_core(struct mCore *core) {
	HookCore *c = core_of(core->cpu);
	return c ? c : attach(core);
}

static void put_original(HookCore *c, const Hook *h) {
	if (h->arm) c->core->rawWrite32(c->core, h->addr, -1, h->original);
	else c->core->rawWrite16(c->core, h->addr, -1, (uint16_t)h->original);
}

void hook_detach(struct mCore *core) {
	struct ARMCore *cpu = core->cpu;
	HookCore *c = core_of(cpu);
	if (!c) return;
	for (int i = 0; i < c->nhooks; ++i)
		if (c->hooks[i].live) put_original(c, &c->hooks[i]);
	cpu->irqh.bkpt16 = c->board16;
	cpu->irqh.bkpt32 = c->board32;
	if (first == c) first = NULL;
	memset(c, 0, sizeof *c);
}

static void put_bkpt(HookCore *c, Hook *h) {
	struct mCore *core = c->core;
	if (h->arm) {
		h->original = core->rawRead32(core, h->addr, -1);
		core->rawWrite32(core, h->addr, -1, ARM_BKPT);
	} else {
		h->original = (uint16_t)core->rawRead16(core, h->addr, -1);
		core->rawWrite16(core, h->addr, -1, (uint16_t)THUMB_BKPT);
	}
}

static bool add(HookCore *c, uint32_t addr, bool arm, EmuHook fn, void *user, int kind) {
	if (!c) return false;
	addr &= arm ? ~3u : ~1u;
	Hook *h = find(c, addr);
	if (h && h->live) return false;
	/* (a removed hook's slot again: its original is back in memory) */
	if (!h) {
		if (c->nhooks == MAX_HOOKS) return false;
		h = &c->hooks[c->nhooks++];
	}
	*h = (Hook){ .addr = addr, .arm = arm, .live = true, .fn = fn, .user = user, .kind = kind };
	put_bkpt(c, h);
	return true;
}

bool hook_add(uint32_t addr, EmuHook fn, void *user) { return fn && add(first, addr, false, fn, user, 0); }
bool hook_add_arm(uint32_t addr, EmuHook fn, void *user) { return fn && add(first, addr, true, fn, user, 0); }
bool hook_add_event(uint32_t addr, int kind) { return add(first, addr, false, NULL, NULL, kind); }
bool hook_on(HookCore *c, uint32_t addr, EmuHook fn, void *user) { return fn && add(c, addr, false, fn, user, 0); }

void hook_remove(uint32_t addr) {
	HookCore *c = first;
	Hook *h = c ? find(c, addr & ~1u) : NULL;
	if (!h || !h->live) return;
	put_original(c, h);
	/* (kept where the CPU may have fetched its BKPT already: the two
	 * instructions at and before the PC) */
	uint32_t pc = (uint32_t)c->cpu->gprs[ARM_PC];
	if (h->addr <= pc && pc - h->addr <= (h->arm ? 4u : 2u)) { h->live = false; return; }
	*h = c->hooks[--c->nhooks];
}

void hook_reapply(void) {
	HookCore *c = first;
	for (int i = 0; c && i < c->nhooks; ++i) {
		Hook *h = &c->hooks[i];
		if (!h->live || h->addr >= 0x08000000u) continue;
		uint32_t now = h->arm ? c->core->rawRead32(c->core, h->addr, -1) : (uint16_t)c->core->rawRead16(c->core, h->addr, -1);
		if (now != (h->arm ? ARM_BKPT : THUMB_BKPT)) put_bkpt(c, h);
	}
}

int hook_drain(HookEvent *out, int max) {
	HookCore *c = first;
	int n = 0;
	while (c && n < max && c->qhead != c->qtail) out[n++] = c->queue[c->qhead++ & (QUEUE - 1)];
	return n;
}
