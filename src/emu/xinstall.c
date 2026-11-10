/* The tracks' installs in their order (xinstall.h). */
#include "xinstall.h"

#include "emu.h"
#include "rom.h"
#include "soulproxy.h"
#include "soulunite.h"
#include "xbracket.h"
#include "xchips_code.h"
#include "xchips_install.h"
#include "xinstall_log.h"
#include "xnavi_bn4.h"
#include "xnavi_bn4_chip.h"
#include "xnavi_bn4_sounds.h"
#include "xnavi_install.h"
#include "xnavi_objects.h"
#include "xsea.h"
#include "xstrike.h"

/* before each frame where BN4's ROM is read: a Navi chip's Navi's rooms (xnavi_bn4_chip.h), then the pool's pointing */
static void bn4_frame(void) {
	xnavi_bn4_chip_frame();
	xnavi_bn4_sounds_frame();
}

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
	/* (BN4's chips' code where its ROM is read: its own region and numbers, after BN5's so both find the grown tables) */
	if (emu_ready() && XR[XROM_BN4_RED_SUN_US].data) xchips_bn4_code_install();
	xvirus_install(families);
	/* (BN4's Navis where its ROM is read, after its families: their art's and the dispatch's numbers past theirs; each
	 * installed once a core, xnavi_install's own) */
	if (families == XVIRUS_FAMILIES && navis != XNAVI_NONE && emu_ready() && XR[XROM_BN4_RED_SUN_US].data)
		for (int i = 0; i < xnavi_bn4_count; ++i) xnavi_install(xnavi_bn4[i]);
	/* (BN4's Navi chips' units after the Navis whose attack objects they run, xchips_code.h; without the Navis their entries
	 * stay the grown table's stub, a Navi chip that brings nobody) */
	if (families == XVIRUS_FAMILIES && navis != XNAVI_NONE && emu_ready() && XR[XROM_BN4_RED_SUN_US].data) xchips_bn4_navi_install();
	/* (their sounds' pool pointed at the battle's Navi's, or its late family's, before each frame, xnavi_bn4_sounds.h; a Navi
	 * chip's Navi's rooms filled before it, xnavi_bn4_chip.h) */
	if (families == XVIRUS_FAMILIES && emu_ready() && XR[XROM_BN4_RED_SUN_US].data) emu_before_frame(bn4_frame);
	xsea_install();   /* (BN5's sea's submerge, where its lava and sea came in with its families) */
	/* (BN5's obstacle strike, a Navi Colonel's side's as ColonelSoul's: the Souls' moved code first, its soldier among it, which
	 * spawns the other tracks' copies of the types it shares; the Souls' own install, soulunite.c, finds it there) */
	if (bn5 && navis == XNAVI_BN5) xstrike_install();
	xlog_end();
	/* (BN4's bracket screen, where its ROM is read: a main mode of its own, outside the battles' installs) */
	if (emu_ready() && XR[XROM_BN4_RED_SUN_US].data) xbracket_install();
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
