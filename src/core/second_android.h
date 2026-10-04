/* The second screen on Android: a display beside the game's, the AYN
 * Thor's lower screen, which GameActivity's SecondScreen shows the layer's
 * map on (android/README.md). Java reports the display's size; the game
 * draws the map at a size picked for it (second_android.c) into pixels
 * Java copies from. Empty on every other target. */
#ifndef SECOND_ANDROID_H
#define SECOND_ANDROID_H

#include <stdbool.h>
#include <stdint.h>

/* The pixels to draw the second screen in (RGBA8888, w x h, w * 4 bytes a
 * row), or NULL: no display beside the game's, or its last picture not yet
 * copied by Java. */
uint32_t *second_android_begin(int *w, int *h);
/* ... drawn: a picture handed to Java (`on`), else the screen black. */
void second_android_end(bool on);
/* The second screen black, at once (the scene that drew it gone). */
void second_android_dark(void);

#endif
