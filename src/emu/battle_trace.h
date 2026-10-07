/* CYBERWORLD_BTRACE=1: each frame of a battle, the game's memory (EWRAM and
 * IWRAM) into the data folder's btrace.bin, as what changed since the frame
 * before, for tools/xcontrol.py: the control family's differential (issue
 * #82; --dev xcontrol, docs/MULTIROM.md, Transplanted families).
 *
 * The file: "BTRC", a version (1), the count of ranges moved (xvirus.h,
 * XvMoved: from, to, len, three u32 each) and the ranges; then a record a
 * frame: "FRAM", its number among the traced frames, the battle's timer
 * (BN6_BATTLE_TIMER), the count of runs, and the runs (an offset into the
 * image, its length, its bytes), where the image is EWRAM's 0x40000 bytes
 * and then IWRAM's 0x8000. All u32 little-endian. */
#ifndef CW_BATTLE_TRACE_H
#define CW_BATTLE_TRACE_H

/* After each frame of the game (emu_debug_frame): a record where a battle is on and the trace asked for. */
void battle_trace_frame(void);

#endif
