/* hook.h's hooks on gpSP (hook_gpsp.c, CW_GPSP): what emu_gpsp.c asks of
 * them. */
#ifndef CW_HOOK_GPSP_H
#define CW_HOOK_GPSP_H

#include <stdbool.h>
#include <stdint.h>

/* Whether the instruction at pc is hooked, live (gpSP's translator asks,
 * cw.h's cw_hooked) */
bool hook_gpsp_hooked(uint32_t pc);
/* Whether a hook was added or removed since the last call: the
 * translations are made afresh before the next frame */
bool hook_gpsp_changed(void);

#endif
