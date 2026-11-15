/* --chip-texts (chip_texts.c; tools/chip_texts.py compares it with the
 * ROMs, docs/devtools/chip-texts.md): every chip id's name and description
 * as the core's copy gives them to BN6's own readers, and the entry each
 * should be, its bytes in hex (into .build/ alone). */
#ifndef CW_CHIP_TEXTS_H
#define CW_CHIP_TEXTS_H

/* Writes it to `path`, the core started and the chips installed first (as
 * a run's core starts); 0 when written. */
int chip_texts_run(const char *path);

#endif
