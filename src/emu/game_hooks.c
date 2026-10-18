/* game_hooks.h. Each hook's own file says what it hooks and why
 * (docs/EMULATION.md); this one keeps their order. */
#include "game_hooks.h"

#include "claims.h"
#include "darkinvs.h"
#include "debug.h"
#include "devtools.h"
#include "encounter.h"
#include "events.h"
#include "gamecall.h"
#include "idle.h"
#include "npc.h"
#include "order_sort.h"
#include "pet.h"
#include "pet_text.h"
#include "protoman_cross.h"
#include "rom.h"
#include "scripts.h"
#include "soulproxy.h"
#include "soulunite.h"
#include "xchips_install.h"
#include "xinstall_log.h"

void game_hooks_install(void) {
	/* (BN6's chip tables grown, BN5's chips in them or their placeholders,
	 * and the hook on the deck's deal: before the boot, whose NEW GAME
	 * counts its folder by the records; the same at every boot,
	 * docs/MULTIROM.md, The chip registry), BN5's chips' moved code with
	 * them where its ROM is read (xchips_code.h) */
	xchips_install();
	gamecall_install();
	idle_install();
	npc_reach_install();
	chat_marks_install();
	pet_install();
	order_sort_install();
	pet_text_install();
	/* (BN5's Souls on the proxy form in BN6's battles wherever BN5's Navis
	 * are, every battle of a run: the run's Souls reach them as each battle
	 * begins, encounter.c; docs/MULTIROM.md, Transplanted Souls. And the
	 * proxy form for its own test, --dev soulproxy) */
	bool souls = XR[XROM_BN5_COLONEL_US].data && !dev.nonavis && !dev.xnavicontrol;
	if (dev.soulproxy || souls) {
		/* (written down for the Arena's cores as they are made: xinstall_log.h) */
		xlog_begin();
		bool proxy = soulproxy_install(dev.proxybreak);
		if (proxy) soulunite_install(dev.unitebreak, dev.soulbreak);
		xlog_end();
	}
	/* (BN5's chip AI for the player entered in each battle: DarkInvs's machine, installed with the Souls; docs/MULTIROM.md) */
	darkinvs_dev(dev.chipai);
	if (emu_debug_on()) claims_report();
}

void game_hooks_after_boot(void) {
	emu_encounters_install();
	protoman_cross_install();
	events_install();
}
