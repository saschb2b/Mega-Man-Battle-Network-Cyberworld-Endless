/* The PET's words for a chip of BN5's without its ROM (xchips_text.h). */
#include "xchips_text.h"

#include <stdio.h>

void xchips_text_name(int bn5, char *out, size_t n) { snprintf(out, n, "BN5-%d", bn5); }

const char *const xchips_text_desc[3] = { "Needs BN5", "ROM back", "to battle" };
