/* xdiff_navi4.h: the code of BN4's Navi chips the differential's logs watch. */
#include "xdiff_navi4.h"

#include <string.h>

#include "chip_entry.h"
#include "xchips_bn4.h"
#include "xchips_registry.h"
#include "xnavi_bn4_art.h"
#include "xnavi_recipe.h"

static bool used[XC4_UNITS];

/* (a chip of BN4's whose row is a moved Navi chip's: XREG_MOVED with a twin, its subfamily the unit's number) */
static void units_of(int id) {
	const XChipReg4 *r = xreg4_at(id);
	for (int u = 0; r && r->how == XREG_MOVED && r->twin && u < xchips_bn4_count && u < XC4_UNITS; ++u)
		if (xchips_bn4[u].kind == XCHIP_NAVI && xchips_bn4[u].number == r->sub) used[u] = true;
}

void xd_navi4_units(const uint16_t *folder, int n) {
	memset(used, 0, sizeof used);
	for (int i = 0; i < n; ++i) units_of((int)chip_entry_id(folder[i]));
}

/* whether `m` is one of a recipe's blocks */
static bool block_of(const XVirusFamily *f, const XvMoved *m) {
	for (int i = 0; f && i < f->nblocks; ++i)
		if (f->blocks[i].start == m->from && f->blocks[i].at == m->to) return true;
	return false;
}

bool xd_navi4_code(const XvMoved *m) {
	if (!((m->to >= XC4_CODE && m->to < XC4_ART) || (m->to >= XN_BN4_CODE && m->to < XN_BN4_ART))) return false;
	for (int u = 0; u < xchips_bn4_count && u < XC4_UNITS; ++u) {
		if (!used[u]) continue;
		const XNavi *n = xchips_bn4[u].navis ? xnavi_bn4_seat(xchips_bn4[u].navis - 1) : NULL;
		if (block_of(&xchips_bn4[u].code, m) || (n && block_of(&n->code, m))) return true;
	}
	return false;
}
