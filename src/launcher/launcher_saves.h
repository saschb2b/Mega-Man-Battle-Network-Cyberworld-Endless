/* What the ROMs screen asks of SAVES, which stands above it (docs/code/
 * layers.md): two hooks the launcher owns and SAVES fills (saves.c), so
 * that the launcher includes nothing of SAVES. */
#ifndef CW_LAUNCHER_SAVES_H
#define CW_LAUNCHER_SAVES_H

typedef struct {
	/* A .cwsave dropped on the ROMs screen: compared with this device's
	 * before anything changes (saves_import) */
	void (*import)(const char *path);
	/* Each frame of the ROMs screen: the copy's place holding saves not
	 * answered yet, compared (saves_tick) */
	void (*tick)(void);
} LauncherSaves;

extern const LauncherSaves launcher_saves;

#endif
