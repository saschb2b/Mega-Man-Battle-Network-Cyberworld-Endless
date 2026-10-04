/* A QR code (ISO/IEC 18004) for one short text, such as the project's
 * address: byte mode, versions 1 to 10, error correction L (M where it fits
 * in the same size), the mask the standard's penalty rules prefer. The
 * site's web/assets/qr.js, module for module. */
#ifndef CW_QR_H
#define CW_QR_H

#include <stdint.h>

#define QR_MAX 57   /* version 10's modules a side */

/* The code for `text` into m, row by row, `size` modules a row, 1 dark,
 * without the quiet zone: its size, 0 where the text does not fit. */
int qr_make(const char *text, uint8_t m[QR_MAX * QR_MAX]);

#endif
