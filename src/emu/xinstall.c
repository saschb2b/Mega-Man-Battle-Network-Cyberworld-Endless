/* The tracks' installs in their order (xinstall.h). */
#include "xinstall.h"

#include "emu.h"
#include "rom.h"
#include "soulproxy.h"
#include "soulunite.h"
#include "xchips_code.h"
#include "xchips_install.h"
#include "xinstall_log.h"
#include "xnavi_bn4.h"
#include "xnavi_install.h"
#include "xnavi_objects.h"
#include "xsea.h"
#include "xstrike.h"

void xinstall_core(int families, int navis) {
	/* (written down for the Arena's cores as they are made: xinstall_log.h) */
	xlog_begin();
	bool bn5 = emu_ready() && XR[XROM_BN5_COLONEL_US].data;
	/* (BN5's engine objects moved once, before any install whose code runs one: the Navis', the chips' muzzle flashes, WuNote's
	 * image) */
	if (bn5 && (families == XVIRUS_FAMILIES || navis == XNAVI_BN5)) xnavi_objects_install();
	xnavi_install_all(navis);
	/* (BN5's chips' code where its ROM is read, before the families that spawn its copies; again, from the chip registry's
	 * install, it does nothing) */
	if (bn5) xchips_code_install();
	xvirus_install(families);
	/* (BN4's Navis where its ROM is read, after its families: their art's and the dispatch's numbers past theirs; each
	 * installed once a core, xnavi_install's own) */
	if (families == XVIRUS_FAMILIES && navis != XNAVI_NONE && emu_ready() && XR[XROM_BN4_RED_SUN_US].data)
		for (int i = 0; i < xnavi_bn4_count; ++i) xnavi_install(xnavi_bn4[i]);
	xsea_install();   /* (BN5's sea's submerge, where its lava and sea came in with its families) */
	/* (BN5's obstacle strike, a Navi Colonel's side's as ColonelSoul's: the Souls' moved code first, its soldier among it, which
	 * spawns the other tracks' copies of the types it shares; the Souls' own install, soulunite.c, finds it there) */
	if (bn5 && navis == XNAVI_BN5) xstrike_install();
	xlog_end();
}

bool xinstall_bn5(void) {
	xinstall_core(XVIRUS_FAMILIES, XNAVI_BN5);
	if (!xchips_install()) return false;
	/* (the Souls' as game_hooks.c installs them, written down too) */
	xlog_begin();
	if (soulproxy_install(0)) soulunite_install(0, 0);
	xlog_end();
	return true;
}
