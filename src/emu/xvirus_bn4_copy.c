/* xvirus_bn4_copy.h: BN6's types copied for BN4's families, their recipe
 * carried out by the families' code mover. */
#include "xvirus_bn4_copy.h"

#include <stdio.h>

#include "emu.h"
#include "xvirus.h"
#include "xvirus_grow.h"
#include "xvirus_tx.h"

bool xvirus_bn4_copy_install(void) {
	static bool done;
	const XVirusFamily *f = &xvirus_bn4_bn6copy;
	static const XvRegion region = XV_BN4_COPY_REGION;
	if (done) return true;
	if (!xg_grow()) {
		fprintf(stderr, "xvirus: %s not installed: BN6's tables not grown\n", f->name);
		return false;
	}
	xv_tx_begin();
	if (!emu_ready()) xv_fail("no core");
	else xvirus_code_stage(f, &region, xvirus_bn4_bn6copy_veneers, xvirus_bn4_bn6copy_nveneers);
	if (xv_failed()) {
		fprintf(stderr, "xvirus: %s not installed: %s\n", f->name, xv_why());
		xv_tx_end();
		return false;
	}
	uint32_t used = xv_used();
	xv_commit();
	xv_tx_end();
	for (int i = 0; i < f->nblocks; ++i) xv_moved_add(f->blocks[i].start, f->blocks[i].at, f->blocks[i].end - f->blocks[i].start);
	done = true;
	printf("xvirus: %s: %d blocks, %u bytes written\n", f->name, f->nblocks, (unsigned)used);
	return true;
}
