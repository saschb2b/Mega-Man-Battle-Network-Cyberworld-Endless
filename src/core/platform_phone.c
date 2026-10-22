/* platform_phone.h. */
#include "platform_phone.h"

/* A phone's own, before SDL starts: Android's and the iPhone's hints. */
void platform_phone_hints(void) {
#ifdef __ANDROID__
	/* a phone turns (the touch controls go under the picture or beside it),
	 * and Back is Escape (the quit prompt), not the end of the app */
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight Portrait PortraitUpsideDown");
	SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
	/* (a phone's tilt is no controller) */
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#endif
#ifdef CW_IOS
	/* an iPhone turns as an Android phone does; the home indicator fades
	 * till a swipe at the screen's foot (two swipes to leave), and the
	 * phone's tilt is no controller */
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight Portrait");
	SDL_SetHint(SDL_HINT_IOS_HIDE_HOME_INDICATOR, "2");
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#endif
}

/* ... and its window's flags: SDL locks a window that cannot resize to
 * the way the phone is held at the start, whatever the hint allows; on
 * iOS, the screen's own pixels, not its points */
Uint32 platform_phone_window_flags(void) {
#if defined(__ANDROID__)
	return SDL_WINDOW_RESIZABLE;
#elif defined(CW_IOS)
	return SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#else
	return 0;
#endif
}
