/* The roots of trust the statistics' requests carry where the system's
 * own may not reach Umami's host (analytics_http.c): Let's Encrypt's,
 * whose chain the host serves (its certificate from YR2, under ISRG Root
 * YR, which ISRG Root X1 signs too). The 3DS has no store of its own that
 * knows them; a handheld's libcurl may lack its CA file, and tries again
 * with these. */
#ifndef CW_ANALYTICS_ROOTS_H
#define CW_ANALYTICS_ROOTS_H

/* ISRG Root X1 (to 2035, SHA-256 96:BC:EC:06:26:49:76:F3:...:BD:DF:08:C6)
 * and ISRG Root YR (to 2045, E5:7B:7E:6F:15:0C:41:91:...:EB:F4:A8:6F), as
 * PEM, from letsencrypt.org/certificates; "" where no request needs them */
extern char analytics_roots[];

#endif
