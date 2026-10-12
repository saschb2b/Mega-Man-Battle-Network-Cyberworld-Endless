/* xnavi_objects.h: BN5's engine objects moved once, their recipe carried
 * out on the core's ROM copy by the families' code mover. */
#include "xnavi_objects.h"

#include <stdio.h>

#include "emu.h"
#include "xnavi_bn5.h"
#include "xnavi_room.h"
#include "xnavi_recipe.h"
#include "xvirus_dispatch.h"
#include "xvirus_move.h"
#include "xvirus_tx.h"

bool xnavi_objects_have(int kind, int type) {
	uint32_t twin = xvirus_dispatch_twin_at(kind, type), moved = xvirus_dispatch_moved_at(kind, type);
	return twin && moved && emu_read8(twin) == XV_TYPE_MOVED && emu_read32(moved) != (XV_STUB_NONE | 1u);
}

bool xnavi_objects_install(void) {
	static bool done;
	const XVirusFamily *f = &xnavi_objects_bn5;
	if (done) return true;
	char why[200];
	if (!xn_grow()) {
		fprintf(stderr, "xnavi: %s not installed: BN6's tables not grown\n", f->name);
		return false;
	}
	xv_tx_begin();
	if (!emu_ready()) xv_fail("no core");
	else if (!xnavi_objects_ok(f, why, sizeof why)) xv_fail("%s", why);
	else if (xv_move_begin(f)) {
		xv_move_code();
		xv_move_art();
	}
	if (xv_failed()) {
		fprintf(stderr, "xnavi: %s not installed: %s\n", f->name, xv_why());
		xv_tx_end();
		return false;
	}
	uint32_t used = xv_used();
	xv_commit_by(xn_writer);
	xv_tx_end();
	for (int i = 0; i < f->nblocks; ++i) xv_moved_add(f->blocks[i].start, f->blocks[i].at, f->blocks[i].end - f->blocks[i].start);
	done = true;
	printf("xnavi: %s: %d blocks, %d sprites, %u bytes written\n", f->name, f->nblocks, f->nsprites, (unsigned)used);
	return true;
}
