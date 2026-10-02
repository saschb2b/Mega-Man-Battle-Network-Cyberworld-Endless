/* BN6's main loop waits for each frame by reading DISPSTAT until its
 * VBlank flag is set (bn6f main_awaitFrame): about half of the GBA's
 * cycles on a layer, more in battle, every one of them interpreted. mGBA's
 * own idle-loop removal leaves a loop that reads DISPSTAT alone (the flag
 * changes with no interrupt to wake a halted CPU), so a hook halts the CPU
 * there instead, while the VBlank interrupt is on to end the halt where the
 * flag would have ended the loop: the game's state each frame is the same,
 * at a third less of the core's time (docs/EMULATION.md). */
#include "idle.h"

#include <stdbool.h>

#include "bn6.h"
#include "emu.h"

#define IO_DISPSTAT 0x04000004u   /* bit 0 in VBlank, bit 3 its interrupt on */
#define IO_IE       0x04000200u   /* bit 0 VBlank */
#define IO_IME      0x04000208u   /* bit 0 interrupts on */

static HookAct await_frame(HookRegs *r, void *user) {
	(void)r;
	(void)user;
	uint16_t stat = hook_read16(IO_DISPSTAT);
	if (!(stat & 1) && (stat & 8) && (hook_read16(IO_IE) & 1) && (hook_read16(IO_IME) & 1)) hook_halt();
	return HOOK_CONTINUE;
}

void idle_install(void) {
	static bool done;
	if (done) return;
	done = true;
	emu_hook(BN6_AWAIT_FRAME_LOOP, await_frame, NULL);
}
