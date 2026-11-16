/* ROM-free checks of the updater (src/update/): update.json as
 * tools/update_manifest.py writes it, versions compared, SHA-256, the
 * Switch's and the 3DS's swap of their own file, the PortMaster port's own
 * update folder and launcher; their failure count. */
#ifndef CW_TEST_UPDATE_H
#define CW_TEST_UPDATE_H

int test_update(void);

#endif
