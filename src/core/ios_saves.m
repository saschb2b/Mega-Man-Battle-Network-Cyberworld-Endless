#include "ios_saves.h"

#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include <stdio.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include "backup.h"

/* Independent of the ROM folder; an existing ROM bookmark is the default
 * until the player chooses a transfer folder. These never travel. */
static NSString *const folder_key = @"savesFolder", *const name_key = @"savesFolderName";
static SDL_atomic_t result;
static char result_text[1024];

const char *ios_saves_device(void) { return UIDevice.currentDevice.name.UTF8String; }

static void done(int status, NSString *text) {
	snprintf(result_text, sizeof result_text, "%s", text.UTF8String ?: "");
	SDL_AtomicSet(&result, status);
}

static BOOL stage(NSURL *url, const char *to) {
	__block BOOL ok = NO;
	NSFileCoordinator *c = [[NSFileCoordinator alloc] initWithFilePresenter:nil];
	[c coordinateReadingItemAtURL:url options:0 error:nil byAccessor:^(NSURL *at) {
		NSNumber *size = nil;
		[at getResourceValue:&size forKey:NSURLFileSizeKey error:nil];
		if (size && size.unsignedLongLongValue > BACKUP_MAX) return;
		NSData *data = [NSData dataWithContentsOfURL:at options:NSDataReadingMappedIfSafe error:nil];
		if (data.length && data.length <= BACKUP_MAX) ok = backup_write_file(to, data.bytes, data.length);
	}];
	return ok;
}

bool ios_saves_get(const char *to) {
	@autoreleasepool {
		backup_set_device(UIDevice.currentDevice.name.UTF8String);
		NSUserDefaults *prefs = NSUserDefaults.standardUserDefaults;
		NSData *mark = [prefs dataForKey:folder_key] ?: [prefs dataForKey:@"romFolder"];
		if (!mark) return false;
		BOOL stale = NO;
		NSURL *folder = [NSURL URLByResolvingBookmarkData:mark options:0 relativeToURL:nil bookmarkDataIsStale:&stale error:nil];
		if (!folder) return false;
		BOOL scoped = [folder startAccessingSecurityScopedResource];
		NSURL *file = [folder URLByAppendingPathComponent:@BACKUP_NAME];
		BOOL exists = [NSFileManager.defaultManager fileExistsAtPath:file.path];
		BOOL ok = exists && stage(file, to);
		if (exists && !ok) {
			static const uint8_t refused[] = "The incoming saves file could not be read.";
			ok = backup_write_file(to, refused, sizeof refused - 1);
		}
		if (scoped) [folder stopAccessingSecurityScopedResource];
		return ok;
	}
}

bool ios_saves_folder_name(char *out, size_t n) {
	NSUserDefaults *prefs = NSUserDefaults.standardUserDefaults;
	BOOL separate = [prefs dataForKey:folder_key] != nil;
	if (!separate && ![prefs dataForKey:@"romFolder"]) return false;
	NSString *name = [prefs stringForKey:separate ? name_key : @"romFolderName"] ?: @"your transfer folder";
	if (n) snprintf(out, n, "%s", name.UTF8String);
	return true;
}

@interface CWSavesPicker : NSObject <UIDocumentPickerDelegate>
@property (nonatomic) int kind;
@property (nonatomic, copy) NSString *dir;
@end

@implementation CWSavesPicker
- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
	(void)controller;
	NSURL *url = urls.firstObject;
	if (!url) { done(-1, @""); return; }
	BOOL scoped = [url startAccessingSecurityScopedResource];
	if (self.kind == 2) {
		NSData *mark = [url bookmarkDataWithOptions:NSURLBookmarkCreationMinimalBookmark includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
		if (mark) {
			[NSUserDefaults.standardUserDefaults setObject:mark forKey:folder_key];
			[NSUserDefaults.standardUserDefaults setObject:url.lastPathComponent forKey:name_key];
		}
		done(mark ? 1 : -2, mark ? url.lastPathComponent : @"The folder's access could not be kept.");
	} else if (self.kind == 0) {
		NSString *to = [self.dir stringByAppendingPathComponent:@"saves-import.cwsave"];
		BOOL ok = stage(url, to.UTF8String);
		done(ok ? 1 : -2, ok ? to : @"The saves file could not be read.");
	} else done(1, url.path ?: url.lastPathComponent);
	if (scoped) [url stopAccessingSecurityScopedResource];
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {
	(void)controller;
	done(-1, @"");
}
@end

static CWSavesPicker *delegate;

bool ios_saves_pick(void *window, const char *dir, int kind, const char *from) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (!window || !SDL_GetWindowWMInfo((SDL_Window *)window, &info) || info.subsystem != SDL_SYSWM_UIKIT) return false;
	UIWindow *ui = info.info.uikit.window;
	NSString *data_dir = @(dir), *file = from ? @(from) : nil;
	backup_set_device(UIDevice.currentDevice.name.UTF8String);
	if (kind == 1) {
		NSString *named = [NSTemporaryDirectory() stringByAppendingPathComponent:@BACKUP_NAME];
		NSData *data = [NSData dataWithContentsOfFile:file];
		if (!data || ![data writeToFile:named atomically:YES]) return false;
		file = named;
	}
	SDL_AtomicSet(&result, 0);
	dispatch_async(dispatch_get_main_queue(), ^{
		UIViewController *top = ui.rootViewController;
		while (top.presentedViewController) top = top.presentedViewController;
		if (!top) { done(-2, @"The Files picker could not be opened."); return; }
		if (!delegate) delegate = [CWSavesPicker new];
		delegate.kind = kind;
		delegate.dir = data_dir;
		UIDocumentPickerViewController *picker;
		if (kind == 1) picker = [[UIDocumentPickerViewController alloc] initForExportingURLs:@[ [NSURL fileURLWithPath:file] ] asCopy:YES];
		else picker = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[ kind == 2 ? UTTypeFolder : UTTypeData ]];
		picker.delegate = delegate;
		picker.allowsMultipleSelection = NO;
		[top presentViewController:picker animated:YES completion:nil];
	});
	return true;
}

int ios_saves_result(char *out, size_t n) {
	int status = SDL_AtomicGet(&result);
	if (!status) return 0;
	if (n) snprintf(out, n, "%s", result_text);
	SDL_AtomicSet(&result, 0);
	return status;
}
