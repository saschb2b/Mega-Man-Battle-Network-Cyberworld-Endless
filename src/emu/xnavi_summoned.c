/* xnavi_summoned.h: a summoned enemy's state 0, BN4's summoned appear,
 * whatever its family. */
#include "xnavi_summoned.h"

#include <stdio.h>

#include "bn4_navis.h"
#include "bn6.h"
#include "bn6_families.h"
#include "bytes.h"
#include "emu.h"
#include "hook.h"
#include "xvirus.h"
#include "xvirus_bn4.h"
#include "xvirus_tx.h"

static uint32_t word_at;   /* the summon's word (0 none), read as the hook runs: what the install staged there, or a poke's */

/* a family's state table, one of the homes' (the word as the install writes it; anything else the hook takes as none) */
static bool a_table(uint32_t t) {
	return t >= XV_HOMES && t < XV_HOMES + XV_FAMILIES * XV_HOME_SIZE && (t - XV_HOMES) % XV_HOME_SIZE == XV_HOME_STATES;
}

/* the dispatcher's call of the state runner, r5 the virus and r0 its state table: a moved family's enemy the summon
 * marked, in its state 0, given the summon's word's table instead (an answer prepared by the install) */
static HookAct summoned_run(HookRegs *r, void *user) {
	(void)user;
	const uint32_t v = r->r[5];
	if (!word_at || hook_read8(v + BN6_T1_ACTION) || hook_read8(v + BN6_T1_SPAWN_KIND) != BN4_SUMMON_MARK) return HOOK_CONTINUE;
	const uint32_t ai = hook_read8(hook_read32(v + BN6_T1_AI_DATA) + BN6_AI_INDEX), table = hook_read32(word_at);
	if (ai >= BN6_FAMILIES && ai < XV_AIS && a_table(table)) r->r[0] = table;
	return HOOK_CONTINUE;
}

/* the installed family moved for a summon alone among the ids it spawns, NULL none */
static const XVirusFamily *summoned_family(const XnSummon *m) {
	for (int i = 0; i < xvirus_bn4_count; ++i) {
		const XVirusFamily *f = xvirus_bn4[i];
		if (!f->summoned || xvirus_installed(f->game, f->src_ai) != f) continue;
		for (int k = 0; k < m->nids; ++k)
			if (m->ids[k].to >= f->first_id && m->ids[k].to < f->first_id + XV_VERSIONS) return f;
	}
	return NULL;
}

bool xnavi_summoned_set(const XnSummon *m, uint32_t at) {
	static bool hooked;
	const XVirusFamily *f = summoned_family(m);
	if (!f) return true;
	uint8_t *p = (at & 3u) == 0 ? xv_put(at, 4, true) : NULL;
	if (!p) return xv_fail("no room for its summon's word at %08x", (unsigned)at);
	put32(p, f->home + XV_HOME_STATES);
	if (!hooked) hooked = emu_hook(BN6_VIRUS_STATE_CALL, summoned_run, NULL);
	if (!hooked) return xv_fail("no hook at %08x for its summoned enemies' state 0", (unsigned)BN6_VIRUS_STATE_CALL);
	word_at = at;
	printf("xnavi: its summoned enemies' state 0 from %s's table (%08x, the summon's word at %08x)\n", f->name,
	       (unsigned)(f->home + XV_HOME_STATES), (unsigned)at);
	return true;
}
