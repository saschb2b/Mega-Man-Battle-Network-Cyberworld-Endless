/* Hooks: a BKPT written over one instruction of the game's code calls the
 * engine's C right there, mid-frame, with the CPU's registers (docs/
 * EMULATION.md, Hooks; issue #27). The game's code stays the player's ROM's;
 * only the core's copy is patched.
 *
 * Two kinds, and neither touches the director's state, since a hook runs on
 * the core's own thread where it has one (the New 3DS, CYBERWORLD_EMU_THREAD):
 * - an event hook queues the registers it met, and the director takes the
 *   queue up after the frame (emu_hook_events);
 * - an answer hook returns, or changes, what the director prepared before the
 *   frame.
 * A hook reads and writes the game's memory with hook_read and hook_write:
 * emu_read and emu_write wait for the frame the hook runs in.
 *
 * Each core has a table of its own, found by the CPU that meets the BKPT:
 * BN6's (hook_attach, which the calls that name no core take) and, for the
 * differential alone (src/dev/xdiff.c), the guest's (hook_core). A hook
 * reads and writes the core it runs on. Two cores' hooks run one core's
 * frame at a time: the guest's hooks are a dev tool's, whose frames run on
 * the thread BN6's do. */
#ifndef CW_HOOK_H
#define CW_HOOK_H

#include <stdbool.h>
#include <stdint.h>

struct mCore;
typedef struct HookCore HookCore;   /* a core's hooks: its table and its queue */

/* A core's hooks at most: the game's and every installer's on BN6's core
 * (measured: 137 in a differential's Soul run with its own, the Navis',
 * the chips' flash and Chaos Unison's, issue #82's final engine round;
 * 128 was the room until then) with room to come, below the buckets'
 * byte (hook.c). A slot is 40 bytes (24 on a 32-bit build); a BKPT met
 * finds its hook through a bucket of its address, the table read through
 * only where two hooks share one. */
#define HOOK_MAX 192

/* What a hook reads, and may change: r0-r12 and lr go back into the CPU
 * (BN6's routines keep their object in r5); sp is read only; pc is the
 * hooked instruction's address. */
typedef struct {
	uint32_t r[13];
	uint32_t sp, lr, pc;
} HookRegs;

typedef enum {
	HOOK_CONTINUE,   /* the hooked instruction runs, with the registers as the hook left them */
	HOOK_RETURN,     /* bx lr, r0 and r1 the result: only at a routine's first instruction, before it pushes */
	HOOK_JUMP,       /* bx r12: on to r[12] (its bit 0 set for Thumb code) */
} HookAct;

typedef HookAct (*EmuHook)(HookRegs *regs, void *user);

/* What an event hook queued, or an answer hook posted (hook_post). */
typedef struct {
	uint32_t addr;   /* the hooked instruction */
	int kind;        /* the caller's tag (emu_hook_event) */
	uint32_t r[8], lr;   /* r0-r7 and lr as it met them */
} HookEvent;

/* Our BKPT handler, before the board's: once, as the core is made. Its
 * table is the one the calls below that name no core take. */
void hook_attach(struct mCore *core);
/* Another core's own table, our handler put before its board's at the
 * first call (NULL where every entry is taken): the guest's, for the
 * differential's hooks on BN5's code. */
HookCore *hook_core(struct mCore *core);
/* The core's table gone and its board's handler back, before the core is
 * let go (a test's cores). */
void hook_detach(struct mCore *core);

/* The table, for a core that is not running (emu.c's emu_hook and the
 * others wait for the frame first): a Thumb instruction at `addr` (its
 * address, bit 0 ignored), or an ARM one (hook_add_arm). False where none
 * fits: another hook there, or the table full. */
bool hook_add(uint32_t addr, EmuHook fn, void *user);
bool hook_add_arm(uint32_t addr, EmuHook fn, void *user);
bool hook_add_event(uint32_t addr, int kind);
/* hook_add on a core of its own (hook_core's) */
bool hook_on(HookCore *core, uint32_t addr, EmuHook fn, void *user);
void hook_remove(uint32_t addr);
/* Hooks in RAM written again, over what a state or a reset put there. */
void hook_reapply(void);
/* The queued events, oldest first; how many. */
int hook_drain(HookEvent *out, int max);
/* From an answer hook: an event of this kind with the registers it has,
 * where only some calls are worth one (a routine every object runs). */
void hook_post(const HookRegs *r, int kind);
/* Cores attached at once: BN6's, and the differential's guest or the
 * Arena's pair (src/online/arena_pair.c) or a test's */
#define HOOK_CORES 3
/* From a hook: its core's entry, 0 to HOOK_CORES - 1, for what a hook
 * keeps between two of its calls (a way back through a stub): one a core,
 * since two cores' hooks run an instruction at a time in turn there (the
 * Arena's pair) */
int hook_slot(void);
/* ... and a core's own, between frames (-1: none attached) */
int hook_slot_of(struct mCore *core);
/* From an answer hook: the CPU halted until the next interrupt it takes,
 * once the hooked instruction has run (a wait the game would spin through:
 * only where an interrupt will end it). */
void hook_halt(void);

/* The game's memory, from a hook. */
uint8_t hook_read8(uint32_t addr);
uint16_t hook_read16(uint32_t addr);
uint32_t hook_read32(uint32_t addr);
void hook_write8(uint32_t addr, uint8_t v);
void hook_write16(uint32_t addr, uint16_t v);
void hook_write32(uint32_t addr, uint32_t v);

/* Hits so far (CYBERWORLD_EMU_DEBUG's 30-frame line), events lost to a
 * full queue, and BKPTs of ours met where no hook is: every core's. */
extern uint32_t hook_hits, hook_dropped, hook_strays;

#endif
