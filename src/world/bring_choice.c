/* bring_choice.h. BN6's Chip Trader screen (bn6f ChipTraderControl) takes
 * as many chips as its script's command says, lists the Pack (bn6f
 * sub_811FE7C), asks once they are in, and on Yes takes them, draws a
 * prize, gives it and saves the game. BN6's machines take 3 or 10; a
 * screen for one is a Bring request's alone, and three hooks make it the
 * choice (docs/rom/world.md, The Chip Trader's screen as a Bring's
 * choice):
 *
 * - the list made (BN6_TRADER_LISTED): the entries whose chip is not of
 *   the element asked are left out, before BN6 sorts it, so the screen
 *   opens over the fitting chips alone and its SORT keeps to them;
 * - its question (BN6_TRADER_SAY): "Add these 10?", BN6's for an amount
 *   but 3, is MegaMan's of ours (FREE_BRING_ASK, job_words.c);
 * - the Yes (BN6_TRADER_TRADE): the chip put in is kept here, and the
 *   screen left by BN6's own way out with no trade, B's (BN6_TRADER_LEFT):
 *   no chip taken, no prize, no save, and the map's check finds no prize
 *   to show (a trade's own way out leaves the map black for the prize's
 *   cutscene).
 *
 * B leaves as BN6's own does, nothing taken. The director then has the
 * asker go on (director_jobs.c). */
#include "bring_choice.h"

#include <stdint.h>
#include <string.h>

#include "bn6.h"
#include "chip_entry.h"
#include "data.h"
#include "emu.h"
#include "free_space.h"
#include "hook.h"
#include "job_words.h"
#include "rom.h"
#include "text.h"

#define ITEM_CODE_BITS (16 - CHIP_ID_BITS)   /* the list's item: id << ITEM_CODE_BITS | code (xchips_wide.h) */
#define IDS (1u << CHIP_ID_BITS)

/* set between frames: the element asked (0 none) and the chips of it */
static int element;
static uint8_t fits[IDS / 8];
/* set by the hooks: a screen for one opened, and the chip its Yes put in (0 none) */
static bool opened;
static uint16_t chosen;

static bool fitting(unsigned id) { return id < IDS && fits[id / 8] >> (id % 8) & 1u; }

/* the screen open is a Bring's: one chip, while one is asked (BN6's own
 * take 3 or 10; its command's 1 is the BugFrag Trader's, which opens no
 * screen, so the asker's script opens it for BRING_OPEN and the list made
 * sets one) */
static bool ours(void) { return element && hook_read32(BN6_TRADER_AMOUNT) == 1; }

static HookAct listed(HookRegs *r, void *user) {
	(void)user;
	if (!element || hook_read32(BN6_TRADER_AMOUNT) != BRING_OPEN) return HOOK_CONTINUE;
	hook_write32(BN6_TRADER_AMOUNT, 1);
	uint32_t n = r->r[0] & 0xFFFFu, kept = 0;
	for (uint32_t i = 0; i < n; ++i) {
		uint32_t at = BN6_TRADER_LIST + BN6_TRADER_ENTRY * i;
		if (!fitting((unsigned)hook_read16(at + BN6_TRADER_ITEM) >> ITEM_CODE_BITS)) continue;
		if (kept != i)
			for (uint32_t b = 0; b < BN6_TRADER_ENTRY; b += 4) hook_write32(BN6_TRADER_LIST + BN6_TRADER_ENTRY * kept + b, hook_read32(at + b));
		++kept;
	}
	for (uint32_t i = kept; i < n; ++i)
		for (uint32_t b = 0; b < BN6_TRADER_ENTRY; b += 4) hook_write32(BN6_TRADER_LIST + BN6_TRADER_ENTRY * i + b, 0);
	r->r[0] = kept;
	opened = true;
	chosen = 0;
	return HOOK_CONTINUE;
}

static HookAct say(HookRegs *r, void *user) {
	(void)user;
	if (!ours() || r->r[1] != BN6_TRADER_ASK_OTHER) return HOOK_CONTINUE;
	r->r[0] = FREE_BRING_ASK;
	r->r[1] = 0;
	r->r[12] = BN6_TRADER_SAY_RUN | 1u;
	return HOOK_JUMP;
}

static HookAct trade(HookRegs *r, void *user) {
	(void)user;
	if (!ours()) return HOOK_CONTINUE;
	chosen = hook_read16(BN6_TRADER_PUT + BN6_TRADER_ITEM);
	r->r[12] = BN6_TRADER_LEFT | 1u;
	return HOOK_JUMP;
}

/* each hooked instruction as Gregar has it, checked before it is hooked */
static const struct { uint32_t at; uint16_t was; EmuHook fn; } hooks[] = {
	{ BN6_TRADER_LISTED, 0x8528, listed },   /* strh r0,[r5,#0x28] */
	{ BN6_TRADER_SAY, 0x4801, say },         /* ldr r0,[pc,#4] */
	{ BN6_TRADER_TRADE, 0xF000, trade },     /* bl's first half */
};

/* (once, as the first request asks: hooks in the ROM copy stay, hook.h) */
static void install(void) {
	static bool done;
	if (done) return;
	done = true;
	for (size_t i = 0; i < sizeof hooks / sizeof *hooks; ++i)
		if (rom_u16(rom_off(hooks[i].at)) != hooks[i].was) return;
	for (size_t i = 0; i < sizeof hooks / sizeof *hooks; ++i) emu_hook(hooks[i].at, hooks[i].fn, NULL);
}

void bring_choice_arm(int elem) {
	if (elem) install();
	element = elem;
	opened = false;
	memset(fits, 0, sizeof fits);
	if (!elem) return;
	for (unsigned id = 1; id < IDS; ++id)
		if (chip_hits_with((int)id) == elem) fits[id / 8] |= (uint8_t)(1u << (id % 8));
	static TextArchive t;
	static uint8_t bytes[TEXT_ARCHIVE_MAX];
	ta_begin(&t);
	job_bring_ask(&t);
	int n = ta_build(&t, bytes);
	if (n > 0 && (uint32_t)n <= FREE_BRING_ASK_END - FREE_BRING_ASK) emu_write(FREE_BRING_ASK, bytes, (size_t)n);
}

BringResult bring_choice_result(int *id, int *code) {
	if (!opened || emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_TRADER) return BRING_NONE;
	opened = false;
	if (!chosen) return BRING_KEPT;
	*id = chosen >> ITEM_CODE_BITS;
	*code = chosen & ((1u << ITEM_CODE_BITS) - 1u);
	chosen = 0;
	return BRING_CHOSEN;
}
