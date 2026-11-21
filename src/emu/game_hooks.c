/* game_hooks.h. Each hook's own file says what it hooks and why
 * (docs/engine/emulation/); this one keeps their order. */
#include "game_hooks.h"

#include "cross_rows.h"
#include "darkinvs.h"
#include "devtools.h"
#include "encounter.h"
#include "events.h"
#include "gamecall.h"
#include "idle.h"
#include "npc.h"
#include "order_sort.h"
#include "party_field.h"
#include "party_play.h"
#include "pet.h"
#include "pet_text.h"
#include "protoman_cross.h"
#include "scripts.h"
#include "soulproxy.h"
#include "sound_quiet.h"
#include "xchips_install.h"

void game_hooks_install(void) {
	/* (BN6's chip tables grown, BN5's chips in them or their placeholders,
	 * and the hook on the deck's deal: before the boot, whose NEW GAME
	 * counts its folder by the records; the same at every boot,
	 * docs/multirom/chip-registry.md), BN5's chips' moved code with
	 * them where its ROM is read (xchips_code.h) */
	xchips_install();
	gamecall_install();
#ifndef CW_GPSP
	/* (gpSP skips BN6's wait for VBlank as its own idle loop) */
	idle_install();
#endif
	/* (the music or effects off: Options' Audio) */
	sound_quiet_install();
	npc_reach_install();
	chat_marks_install();
	pet_install();
	order_sort_install();
	pet_text_install();
	/* (BN5's Souls on the proxy form in BN6's battles wherever BN5's Navis
	 * are, every battle of a run: the run's Souls reach them as each battle
	 * begins, encounter.c; docs/multirom/souls.md. And the
	 * proxy form for its own test, --dev soulproxy) */
	soulproxy_install_run(dev.soulproxy, dev.nonavis || dev.xnavicontrol, dev.proxybreak, dev.unitebreak, dev.soulbreak);
	/* (CROSSSELECT listing Falzar's five Crosses beside Gregar's, their rows
	 * made from the ROM's own: issue #706, docs/rom/souls.md) */
	cross_rows_install();
	/* (BN5's chip AI for the player entered in each battle: DarkInvs's machine, installed with the Souls; docs/multirom/) */
	darkinvs_dev(dev.chipai);
}

void game_hooks_after_boot(void) {
	emu_encounters_install();
	protoman_cross_install();
	events_install();
	/* (a party on one field: the shared dive's spike, party_field.h) */
	if (dev.party > 1) party_field_install(dev.party, dev.party_hp, dev.party_end);
	if (dev.party_core > 1 || dev.party_end > 0 || dev.party_bot || dev.party_roll || dev.party_delay || dev.party_lead)
		party_play_dev(&(PartyDev){ dev.party_core, dev.party_twin, dev.party_roll, dev.party_end, dev.party_bot, dev.party_delay, dev.party_lead,
			dev.party_del });
}
