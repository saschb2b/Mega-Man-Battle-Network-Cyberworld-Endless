/* The tracks' installs in their order (xinstall.h). */
#include "xinstall.h"

#include "emu.h"
#include "rom.h"
#include "xchips_code.h"
#include "xnavi_objects.h"
#include "xsea.h"
#include "xstrike.h"

void xinstall_core(int families, int navis) {
	bool bn5 = emu_ready() && XR[XROM_BN5_COLONEL_US].data;
	/* (BN5's engine objects moved once, before any install whose code runs one: the Navis', the chips' muzzle flashes, WuNote's
	 * image) */
	if (bn5 && (families == XVIRUS_FAMILIES || navis == XNAVI_BN5)) xnavi_objects_install();
	xnavi_install_all(navis);
	/* (BN5's chips' code where its ROM is read, before the families that spawn its copies; again, from the chip registry's
	 * install, it does nothing) */
	if (bn5) xchips_code_install();
	xvirus_install(families);
	xsea_install();   /* (BN5's sea's submerge, where its lava and sea came in with its families) */
	/* (BN5's obstacle strike, a Navi Colonel's side's as ColonelSoul's: the Souls' moved code first, its soldier among it, which
	 * spawns the other tracks' copies of the types it shares; the Souls' own install, soulunite.c, finds it there) */
	if (bn5 && navis == XNAVI_BN5) xstrike_install();
}
