/* The Souls' proxy form (issue #82, the Souls track's S1; docs/multirom/souls.md,
 * Transplanted Souls): BN6's form 8, Falzar's TenguCross, which Gregar keeps
 * in every table by form and never offers, made an exact copy of HeatCross
 * (--dev soulproxy), so BN5's Souls can later stand behind one form of
 * BN6's own. tools/xproxy.py proves the copy plays as HeatCross frame for
 * frame. */
#ifndef CW_SOULPROXY_H
#define CW_SOULPROXY_H

#include <stdbool.h>

/* Who the Souls' bytes are (claims.h): the proxy form's, UNITE's and a Soul's life's */
#define SOULS_CLAIMS "the Souls (soulproxy.c, soulunite.c, soullife.c)"

/* Form 8 made HeatCross's copy in the core's ROM copy: entry 8 of each table
 * by form (bn6_forms.h) written from entry 1, the object id of its NameID
 * from HeatCross's, and hooks where Gregar's code reads the form itself
 * (HeatCross's chip bonus, his arm flash's look, the summon as he crosses)
 * answering form 8 as 1.
 * Once a core, before its boot; false (nothing written) where the ROM's
 * bytes are not Gregar's as expected. `breaks` leaves parts of it out
 * (SOULPROXY_BREAK_*, 0 none: tools/xproxy.py --mutations, each of which
 * must part the proxy from HeatCross). */
enum {
	SOULPROXY_BREAK_TABLES = 1,   /* entry 8 of the tables by form left Falzar's TenguCross's */
	SOULPROXY_BREAK_OBJECT_ID = 2, /* its NameID's object id left TenguCross's */
	SOULPROXY_BREAK_BONUS = 4,    /* the chip bonus by form not hooked */
	SOULPROXY_BREAK_FLASH = 8,    /* the arm flash's look not hooked */
	SOULPROXY_BREAK_SUMMON = 16,  /* the summon not hooked */
	SOULPROXY_BREAK_TENGU = 32,   /* TenguCross chosen on CROSSSELECT leaves form 8 the Souls' (HeatCross's copy) */
};
bool soulproxy_install(int breaks);
/* Whether form 8 is the proxy in this core (installed): Falzar's
 * TenguCross is then not offered (powers.c) */
bool soulproxy_installed(void);
/* Form 8 the Souls' (`soul`: HeatCross's copy, the Souls' stubs) or Gregar's
 * own TenguCross, written into its entries before the crossing that reads
 * them: a Soul's commit arms it (soulunite.c, liberation_battle.c),
 * TenguCross's on CROSSSELECT disarms it (this file's commit hook). Called
 * from a hook (hook_write8). Nothing where the proxy is not installed. */
void soulproxy_arm(bool soul);
/* Whether form 8 is the Souls' now (true where no proxy is installed: no
 * TenguCross stands on it to keep apart) */
bool soulproxy_armed(void);
/* The Souls' hooks as a run's core takes them at its start (game_hooks.c):
 * the proxy form and UNITE wherever BN5's Navis are (no_navis: they are
 * not), or always where forced (--dev soulproxy), each written down for
 * the Arena's cores (xinstall_log.h); the breaks as soulproxy_install's
 * and soulunite_install's. */
void soulproxy_install_run(bool forced, bool no_navis, int proxy_breaks, int unite_breaks, int soul_breaks);

/* (dev step "cross F", tools/xproxy.py) The next Cross MegaMan chooses on
 * CROSSSELECT is committed as form F (1-0x18) instead of its own, by
 * CROSSSELECT's own commit (its pending record, its turn, its row's used
 * bit): form 8 with the proxy installed is HeatCross's copy. False where F
 * is no form or the hook cannot be placed. */
bool soulproxy_cross(int form);

#endif
