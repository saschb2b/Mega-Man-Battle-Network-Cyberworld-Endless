#include "scene_emu_start.h"

#include <stdio.h>

#include "boot.h"
#include "cinema.h"
#include "devtools.h"
#include "director.h"
#include "emu.h"
#include "game.h"
#include "game_hooks.h"
#include "gfx.h"
#include "network.h"
#include "online_scene.h"
#include "online_visit.h"
#include "platform.h"
#include "rom.h"
#include "xinstall.h"
#ifdef __3DS__
#include "start_3ds.h"
#endif

bool scene_emu_start(void) {
	/* (making a run's net takes a while on a slow machine: twenty seconds
	 * of black on a 3DS read as a hang) */
	platform_begin_frame();
	fill_rect(0, 0, P.w, P.h, BLACK);
	text_draw(P.w / 2, P.h / 2 - 8, "Building the net...", WHITE, TEXT_CENTER);
	platform_present_now();
	if (!emu_init(R.data, ROM_SIZE)) { fprintf(stderr, "the GBA core did not start (too little memory?)\n"); return false; }
	/* (other games' virus families in BN6's battle engine first: the ROM
	 * copy's patches, the same at every boot; docs/MULTIROM.md) */
	xinstall_core(dev.xcontrol ? XVIRUS_CONTROL : dev.nofamilies ? XVIRUS_NONE : XVIRUS_FAMILIES,
		dev.xnavicontrol ? XNAVI_CONTROL : dev.nonavis ? XNAVI_NONE : XNAVI_BN5);
#ifdef __3DS__
	/* (what the core's copy and the installs left of the heap, BN5's or
	 * not: 3ds/README.md, Testing on a 3DS) */
	char heap[120];
	start_3ds_heap(heap, sizeof heap);
	printf("3ds: the core started, the installs done; %s\n", heap);
#endif
	game_hooks_install();
	cinema_reset();
	if (online_visit_active()) {
		char error[128];
		if (!online_visit_prepare(error, sizeof error)) {
			fprintf(stderr, "online: %s\n", error);
			online_visit_shutdown();
			network_message(error);
			return false;
		}
	} else if (emu_resume_requested) {
		emu_resume_requested = false;
		game_hooks_after_boot();
		online_visit_returned(director_resume());
	} else {
		director_before_boot();
		emu_boot();
		game_hooks_after_boot();
		if (online_scene_enabled()) { director_stop(); online_scene_start(); }
		else if (emu_start_in_town) director_start_run();
		else director_start_layer();
		if (emu_start_at_home && !online_scene_enabled()) director_dev_home();
		emu_start_in_town = emu_start_at_home = false;
	}
	return true;
}
