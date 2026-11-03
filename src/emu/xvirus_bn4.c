/* xvirus_bn4.c: BN4 Red Sun's virus families' recipes, written by tools/xrecipe_bn4.py from BN4's and BN6's ROMs
 * (python3 tools/xrecipe_bn4.py; --check says whether this is what it writes). Do not edit: change the generator (or
 * tools/bn4_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/multirom/families.md): addresses
 * in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA), counts and CRC32s. The families here are the generator's whole ones
 * (no decision open), installed only where BN4's ROM is read for research (xrom_research: --dev bn4families); the open
 * ones stay in its report (.build/xrecipe/bn4_report.md). */
#include "xvirus_bn4.h"

#include <stddef.h>

/* (none whole yet: Billy's open decisions are in the report) */
const XVirusFamily *const xvirus_bn4[1] = { NULL };
const int xvirus_bn4_count = 0;
