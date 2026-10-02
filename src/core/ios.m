/* The iPhone's and the iPad's own, in UIKit (ios.h). The game's main is
 * SDL's here too: SDL_main.h names main.c's SDL_main, and this file's
 * main hands it to UIKit, as SDL2main's would. */
#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include "ios.h"

/* (SDL_main.h names every main SDL_main, this one too: SDL's own
 * SDL_uikit_main.c undoes it the same way) */
#ifdef main
#undef main
#endif

int main(int argc, char *argv[]) {
	return SDL_UIKitRunApp(argc, argv, SDL_main);
}

static int picked;          /* (ios_pick_result) */
static NSString *pick_dir;

@interface CWRomPicker : NSObject <UIDocumentPickerDelegate>
@end

@implementation CWRomPicker
- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
	(void)controller;
	NSURL *from = urls.firstObject;
	if (!from || !pick_dir) { picked = -1; return; }
	NSFileManager *files = NSFileManager.defaultManager;
	[files createDirectoryAtPath:pick_dir withIntermediateDirectories:YES attributes:nil error:nil];
	NSString *to = [pick_dir stringByAppendingPathComponent:from.lastPathComponent];
	[files removeItemAtPath:to error:nil];
	/* (a copy the picker made for the game, asCopy; the scope in case) */
	BOOL scoped = [from startAccessingSecurityScopedResource];
	BOOL ok = [files copyItemAtURL:from toURL:[NSURL fileURLWithPath:to] error:nil];
	if (scoped) [from stopAccessingSecurityScopedResource];
	picked = ok ? 1 : -1;
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {
	(void)controller;
	picked = -1;
}
@end

static CWRomPicker *picker_delegate;

static UIWindow *window_of(void *window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (!window || !SDL_GetWindowWMInfo((SDL_Window *)window, &info) || info.subsystem != SDL_SYSWM_UIKIT) return nil;
	return info.info.uikit.window;
}

void ios_pick_rom(void *window, const char *dir) {
	UIViewController *top = window_of(window).rootViewController;
	while (top.presentedViewController) top = top.presentedViewController;
	if (!top) { picked = -1; return; }
	pick_dir = [NSString stringWithUTF8String:dir];
	if (!picker_delegate) picker_delegate = [CWRomPicker new];
	UIDocumentPickerViewController *files = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[ UTTypeData ] asCopy:YES];
	files.delegate = picker_delegate;
	files.allowsMultipleSelection = NO;
	picked = 0;
	[top presentViewController:files animated:YES completion:nil];
}

int ios_pick_result(void) {
	int r = picked;
	picked = 0;
	return r;
}

void ios_haptic(void) {
	static UIImpactFeedbackGenerator *tick;
	if (!tick) tick = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
	[tick impactOccurred];
}

void ios_safe_insets(void *window, float *top, float *left, float *bottom, float *right) {
	UIEdgeInsets e = window_of(window).safeAreaInsets;
	*top = (float)e.top;
	*left = (float)e.left;
	*bottom = (float)e.bottom;
	*right = (float)e.right;
}
