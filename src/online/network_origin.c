#include "network_origin.h"

#include <stdio.h>

#include "emu.h"
#include "game.h"
#include "pc_text.h"
#include "save.h"

static bool from_checkpoint;

void network_origin_set(bool checkpoint) { from_checkpoint = checkpoint; }

void network_origin_back(char *error, size_t cap) {
	if (!from_checkpoint) { title_menu = true; scene_set(&scene_title); return; }
	if (!load_run()) { snprintf(error, cap, "%s", pc_restore_failed); return; }
	emu_resume_requested = true;
	emu_start_in_town = emu_start_at_home = false;
	fprintf(stderr, "online: Network cancelled; returning to PC checkpoint\n");
	scene_set(&scene_emu);
}
