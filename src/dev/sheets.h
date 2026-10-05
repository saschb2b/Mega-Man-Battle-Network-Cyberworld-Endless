/* --sheet: sprites drawn into a picture (sheets.c). */
#ifndef CW_SHEETS_H
#define CW_SHEETS_H

/* --sheet @CAT:FIRST:COUNT:PATH (frame 0 of many sprites) or
 * CAT:IDX:ANIM[:PAL]:PATH (every frame of one animation) drawn and saved;
 * the exit code */
int sheet_run(const char *spec);

#endif
