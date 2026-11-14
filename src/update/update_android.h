/* The update's work on Android (update.h), done by GameActivity's Java
 * (android/.../Updater.java): Android has no HTTP client of the game's
 * (analytics_http.c), so update.json and the APK are fetched there, the
 * APK checked against its size and SHA-256 and handed to Android's
 * installer, which updates the app without a tap from Android 12 on once
 * the player allowed installs from it, else shows its prompt. Each call
 * from the game's thread; the work runs on Java's, polled each frame. */
#ifndef CW_UPDATE_ANDROID_H
#define CW_UPDATE_ANDROID_H

#include <stdbool.h>
#include <stddef.h>

/* (update_android_progress beyond a download's share) */
enum { UPDATE_ANDROID_FAILED = -1, UPDATE_ANDROID_INSTALLING = 101 };

/* What an earlier update left (the APK, the installer's sessions) removed */
void update_android_tidy(void);
/* update.json's GET begun: false where Java has no updater */
bool update_android_look(const char *url, const char *agent);
/* ... its answer: 1 with the text in `text`, 0 none came, -1 under way */
int update_android_looked(char *text, size_t n);
/* The APK's download begun, then its check and the installer */
bool update_android_fetch(const char *url, const char *agent, long long size, const char *sha256);
/* ... its share (0-100), UPDATE_ANDROID_INSTALLING or UPDATE_ANDROID_FAILED */
int update_android_progress(void);
/* A download under way stopped (the updates switched off) */
void update_android_stop(void);

#endif
