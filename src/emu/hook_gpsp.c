/* hook.h's hooks on gpSP (CW_GPSP, docs/engine/emulation/psp-and-ds.md):
 * no BKPT in memory, a table instead, which gpSP's translator reads as it
 * translates a block (cw.h's cw_hooked), calling out (cw_hook_call) before
 * a hooked instruction runs. The hook gets the registers as on mGBA and
 * leaves them for the instruction; HOOK_RETURN and HOOK_JUMP send the CPU
 * on as bx lr and bx r12 would. A hook added or removed takes effect at the
 * next frame, its translations made afresh (emu_gpsp.c). One core: the
 * calls that name another find none. */
#include "hook_gpsp.h"

#include "hook.h"

#ifdef CW_GPSP

#include <string.h>

#include <cw.h>

#include "emu.h"

extern uint32_t reg[64];   /* gpSP's registers, as it runs (r0-r14) */

#define QUEUE   256   /* a power of two */
#define BUCKETS 256

typedef struct {
	uint32_t addr;
	bool arm, live;
	EmuHook fn;   /* NULL: an event hook, queuing its kind */
	void *user;
	int kind;
} Hook;

static Hook hooks[HOOK_MAX];
static int nhooks;
static uint8_t bucket[BUCKETS];   /* a slot + 1 by bucket_of(address), 0 none */
static HookEvent queue[QUEUE];
static uint32_t qhead, qtail;
static bool changed;
uint32_t hook_hits, hook_dropped, hook_strays;

static uint32_t bucket_of(uint32_t addr) { return (addr >> 1 ^ addr >> 9) & (BUCKETS - 1); }

static void buckets_make(void) {
	memset(bucket, 0, sizeof bucket);
	for (int i = 0; i < nhooks; ++i) bucket[bucket_of(hooks[i].addr)] = (uint8_t)(i + 1);
	changed = true;
}

static Hook *find(uint32_t addr) {
	int b = bucket[bucket_of(addr)];
	if (!b) return NULL;
	if (hooks[b - 1].addr == addr) return &hooks[b - 1];
	for (int i = 0; i < nhooks; ++i)
		if (hooks[i].addr == addr) return &hooks[i];
	return NULL;
}

bool hook_gpsp_hooked(uint32_t pc) {
	const Hook *h = find(pc);
	return h && h->live;
}

bool hook_gpsp_changed(void) {
	bool was = changed;
	changed = false;
	return was;
}

static void enqueue(uint32_t addr, int kind, const HookRegs *r) {
	if (qtail - qhead >= QUEUE) { ++hook_dropped; return; }
	HookEvent *e = &queue[qtail++ & (QUEUE - 1)];
	e->addr = addr;
	e->kind = kind;
	memcpy(e->r, r->r, sizeof e->r);
	e->lr = r->lr;
}

/* (on x86 gpSP's translated code calls with the stack as its own code left
 * it, not aligned as C's calls align it: the stack aligned again here, for
 * the hooks' SSE) */
#if defined(__x86_64__) || defined(__i386__)
__attribute__((force_align_arg_pointer))
#endif
uint32_t cw_hook_call(uint32_t pc) {
	Hook *h = find(pc);
	if (!h || !h->live) { ++hook_strays; return 0; }
	++hook_hits;
	HookRegs r = { .sp = reg[13], .lr = reg[14], .pc = pc };
	for (int i = 0; i < 13; ++i) r.r[i] = reg[i];
	HookAct act = HOOK_CONTINUE;
	if (h->fn) act = h->fn(&r, h->user);
	else enqueue(h->addr, h->kind, &r);
	for (int i = 0; i < 13; ++i) reg[i] = r.r[i];
	reg[14] = r.lr;
	/* (on as bx goes: bit 0 of the address says Thumb) */
	if (act == HOOK_RETURN) return r.lr;
	if (act == HOOK_JUMP) return r.r[12];
	return 0;
}

void hook_post(const HookRegs *r, int kind) { enqueue(r->pc, kind, r); }

static bool add(uint32_t addr, bool arm, EmuHook fn, void *user, int kind) {
	addr &= arm ? ~3u : ~1u;
	Hook *h = find(addr);
	if (h && h->live) return false;
	if (!h) {
		if (nhooks == HOOK_MAX) return false;
		h = &hooks[nhooks++];
	}
	*h = (Hook){ .addr = addr, .arm = arm, .live = true, .fn = fn, .user = user, .kind = kind };
	buckets_make();
	return true;
}

bool hook_add(uint32_t addr, EmuHook fn, void *user) { return fn && add(addr, false, fn, user, 0); }
bool hook_add_arm(uint32_t addr, EmuHook fn, void *user) { return fn && add(addr, true, fn, user, 0); }
bool hook_add_event(uint32_t addr, int kind) { return add(addr, false, NULL, NULL, kind); }

void hook_remove(uint32_t addr) {
	Hook *h = find(addr & ~1u);
	if (!h || !h->live) return;
	*h = hooks[--nhooks];
	buckets_make();
}

/* (nothing to write back: no BKPT is ever in memory) */
void hook_reapply(void) {}

int hook_drain(HookEvent *out, int max) {
	int n = 0;
	while (n < max && qhead != qtail) out[n++] = queue[qhead++ & (QUEUE - 1)];
	return n;
}

/* ---- one core: mGBA's cores, the Arena's and the guests', have none here ---- */
void hook_attach(struct mCore *core) { (void)core; }
HookCore *hook_core(struct mCore *core) { (void)core; return NULL; }
void hook_detach(struct mCore *core) { (void)core; }
bool hook_on(HookCore *core, uint32_t addr, EmuHook fn, void *user) { return !core && hook_add(addr, fn, user); }
int hook_slot(void) { return 0; }
int hook_slot_of(struct mCore *core) { (void)core; return -1; }
bool hook_keep_slots(void *array, size_t each) { (void)array; (void)each; return true; }
size_t hook_kept_size(void) { return 0; }
void hook_kept_save(int slot, void *out) { (void)slot; (void)out; }
void hook_kept_load(int slot, const void *in) { (void)slot; (void)in; }

/* (gpSP skips BN6's wait for VBlank itself, its idle loop: idle.c's hook,
 * the one that halts, is not set on gpSP) */
void hook_halt(void) {}

uint8_t hook_read8(uint32_t a) { return emu_read8(a); }
uint16_t hook_read16(uint32_t a) { return emu_read16(a); }
uint32_t hook_read32(uint32_t a) { return emu_read32(a); }
void hook_write8(uint32_t a, uint8_t v) { emu_write8(a, v); }
void hook_write16(uint32_t a, uint16_t v) {
	const uint8_t b[2] = { (uint8_t)v, (uint8_t)(v >> 8) };
	emu_write(a, b, 2);
}
void hook_write32(uint32_t a, uint32_t v) { emu_write32(a, v); }

#endif
