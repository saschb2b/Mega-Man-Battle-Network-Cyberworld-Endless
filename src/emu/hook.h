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
 * emu_read and emu_write wait for the frame the hook runs in. */
#ifndef CW_HOOK_H
#define CW_HOOK_H

#include <stdbool.h>
#include <stdint.h>

struct mCore;

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

/* Our BKPT handler, before the board's: once, as the core is made. */
void hook_attach(struct mCore *core);

/* The table, for a core that is not running (emu.c's emu_hook and the
 * others wait for the frame first): a Thumb instruction at `addr` (its
 * address, bit 0 ignored), or an ARM one (hook_add_arm). False where none
 * fits: another hook there, or the table full. */
bool hook_add(uint32_t addr, EmuHook fn, void *user);
bool hook_add_arm(uint32_t addr, EmuHook fn, void *user);
bool hook_add_event(uint32_t addr, int kind);
void hook_remove(uint32_t addr);
/* Hooks in RAM written again, over what a state or a reset put there. */
void hook_reapply(void);
/* The queued events, oldest first; how many. */
int hook_drain(HookEvent *out, int max);
/* From an answer hook: an event of this kind with the registers it has,
 * where only some calls are worth one (a routine every object runs). */
void hook_post(const HookRegs *r, int kind);

/* The game's memory, from a hook. */
uint8_t hook_read8(uint32_t addr);
uint16_t hook_read16(uint32_t addr);
uint32_t hook_read32(uint32_t addr);
void hook_write8(uint32_t addr, uint8_t v);
void hook_write16(uint32_t addr, uint16_t v);
void hook_write32(uint32_t addr, uint32_t v);

/* Hits so far (CYBERWORLD_EMU_DEBUG's 30-frame line), events lost to a
 * full queue, and BKPTs of ours met where no hook is. */
extern uint32_t hook_hits, hook_dropped, hook_strays;

#endif
