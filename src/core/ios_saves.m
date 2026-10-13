#include "ios_saves.h"

#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include <stdio.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include "backup.h"
#include "ios.h"

/* Independent of the ROM folder; the ROM folder's bookmark (ios.m) is the
 * default until the player chooses a saves folder. These never travel. */
static NSString *const folder_key = @"savesFolder", *const name_key = @"savesFolderName";
static NSString *const rom_key = @"romFolder", *const rom_name_key = @"romFolderName";
static SDL_atomic_t result;
static char result_text[1024];

/* The kept folder resolved, and the bookmark it came from: resolved again
 * only when that bookmark changes (a folder chosen, the ROM folder's anew) */
static NSURL *folder_url;
static NSData *folder_mark;

static void forget(void) {
	@synchronized (folder_key) {
		folder_url = nil;
		folder_mark = nil;
	}
}

/* The saves folder, else the ROM folder; nil for none, or one that cannot
 * be opened. A stale bookmark is made afresh, as ios.m does. */
static NSURL *kept_folder(void) {
	NSUserDefaults *prefs = NSUserDefaults.standardUserDefaults;
	NSString *key = [prefs dataForKey:folder_key] ? folder_key : rom_key;
	NSData *mark = [prefs dataForKey:key];
	if (!mark) return nil;
	@synchronized (folder_key) {
		if (folder_url && [mark isEqualToData:folder_mark]) return folder_url;
		BOOL stale = NO;
		NSError *err = nil;
		NSURL *url = [NSURL URLByResolvingBookmarkData:mark options:0 relativeToURL:nil bookmarkDataIsStale:&stale error:&err];
		if (!url) {
			NSLog(@"saves: the folder can't be opened: %@", err.localizedDescription);
			return nil;
		}
		if (stale) {
			BOOL scoped = [url startAccessingSecurityScopedResource];
			NSData *fresh = [url bookmarkDataWithOptions:NSURLBookmarkCreationMinimalBookmark includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
			if (scoped) [url stopAccessingSecurityScopedResource];
			if (fresh) {
				[prefs setObject:fresh forKey:key];
				mark = fresh;
			}
		}
		folder_url = url;
		folder_mark = mark;
		return url;
	}
}

/* The folder's copy, its scope entered by the caller: 1 on the phone, 0
 * none, -1 in iCloud and not here as its latest (sent for). iCloud Drive
 * lists a file not on the phone as ".name.icloud". */
static int copy_state(NSURL *folder, NSURL *file) {
	NSFileManager *fm = NSFileManager.defaultManager;
	if (![fm fileExistsAtPath:file.path]) {
		if (![fm fileExistsAtPath:[folder URLByAppendingPathComponent:@"." BACKUP_NAME ".icloud"].path]) return 0;
	} else {
		NSNumber *cloud = nil;
		NSString *status = nil;
		[file getResourceValue:&cloud forKey:NSURLIsUbiquitousItemKey error:nil];
		if (!cloud.boolValue) return 1;
		[file getResourceValue:&status forKey:NSURLUbiquitousItemDownloadingStatusKey error:nil];
		if (!status || [status isEqualToString:NSURLUbiquitousItemDownloadingStatusCurrent]) return 1;
	}
	[fm startDownloadingUbiquitousItemAtURL:file error:nil];
	return -1;
}

static void done(int status, NSString *text) {
	snprintf(result_text, sizeof result_text, "%s", text.UTF8String ?: "");
	SDL_AtomicSet(&result, status);
}

/* url read as Files' providers want (coordinated) into `to`, whole */
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

int ios_saves_get(const char *to) {
	@autoreleasepool {
		int got = 0;
		NSURL *folder = kept_folder();
		if (folder && to) {
			BOOL scoped = [folder startAccessingSecurityScopedResource];
			NSURL *file = [folder URLByAppendingPathComponent:@BACKUP_NAME];
			got = copy_state(folder, file);
			if (got > 0 && !stage(file, to)) got = -1;
			if (scoped) [folder stopAccessingSecurityScopedResource];
		}
		if (got < 1 && to) remove(to);
		return got;
	}
}

/* (ios.h's: `from` into the saves folder, whole or not at all; false where
 * none is kept, it refused, or its copy there is in iCloud and not on the
 * phone as its latest, sent for and not written over) */
bool ios_saves_put(const char *from) {
	@autoreleasepool {
		NSData *data = from ? [NSData dataWithContentsOfFile:@(from)] : nil;
		NSURL *folder = kept_folder();
		if (!folder || !data.length) return false;
		BOOL scoped = [folder startAccessingSecurityScopedResource];
		NSURL *file = [folder URLByAppendingPathComponent:@BACKUP_NAME];
		__block BOOL ok = NO;
		NSError *err = nil;
		/* (written as Files' providers want it: coordinated, whole; a copy
		 * in iCloud not here as its latest is sent for, not written over) */
		if (copy_state(folder, file) >= 0)
			[[[NSFileCoordinator alloc] initWithFilePresenter:nil] coordinateWritingItemAtURL:file
				options:NSFileCoordinatorWritingForReplacing error:&err byAccessor:^(NSURL *at) { ok = [data writeToURL:at atomically:YES]; }];
		if (scoped) [folder stopAccessingSecurityScopedResource];
		if (!ok) NSLog(@"saves: the copy in the saves folder was not written: %@", err.localizedDescription ?: @"in iCloud, not here yet");
		return ok;
	}
}

bool ios_saves_folder_name(char *out, size_t n) {
	@autoreleasepool {
		NSUserDefaults *prefs = NSUserDefaults.standardUserDefaults;
		BOOL separate = [prefs dataForKey:folder_key] != nil;
		if (!separate && ![prefs dataForKey:rom_key]) return false;
		NSString *name = [prefs stringForKey:separate ? name_key : rom_name_key] ?: @"your transfer folder";
		if (n) snprintf(out, n, "%s", name.UTF8String ?: "");
		return true;
	}
}

bool ios_saves_folder_scope(char *out, size_t n) {
	@autoreleasepool {
		NSURL *folder = n ? kept_folder() : nil;
		if (!folder) return false;
		int length = snprintf(out, n, "%s", folder.absoluteString.UTF8String ?: "");
		return length > 0 && (size_t)length < n;
	}
}

/* ---- the pickers ---- */

@interface CWSavesPicker : NSObject <UIDocumentPickerDelegate>
@property (nonatomic) int kind;
@property (nonatomic, copy) NSString *dir;
@property (nonatomic, copy) NSString *offered;   /* the export's copy in tmp */
- (void)tidy;
@end

/* The folder picked kept as the saves folder */
static void keep_folder(NSURL *url) {
	BOOL scoped = [url startAccessingSecurityScopedResource];
	NSData *mark = [url bookmarkDataWithOptions:NSURLBookmarkCreationMinimalBookmark includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
	if (scoped) [url stopAccessingSecurityScopedResource];
	if (!mark) { done(-2, @"The folder could not be kept."); return; }
	NSUserDefaults *prefs = NSUserDefaults.standardUserDefaults;
	[prefs setObject:mark forKey:folder_key];
	[prefs setObject:url.lastPathComponent forKey:name_key];
	forget();
	done(1, url.lastPathComponent);
}

/* The file picked (the picker's copy, in the app's tmp: read, then gone)
 * staged as `dir`/saves-import.cwsave */
static void take_file(NSURL *url, NSString *dir) {
	NSString *to = [dir stringByAppendingPathComponent:@"saves-import.cwsave"];
	BOOL ok = stage(url, to.UTF8String);
	[NSFileManager.defaultManager removeItemAtURL:url error:nil];
	done(ok ? 1 : -2, ok ? to : @"The file could not be read.");
}

@implementation CWSavesPicker
/* (the export's copy in tmp, gone once the picker is done with it) */
- (void)tidy {
	if (self.offered) [NSFileManager.defaultManager removeItemAtPath:self.offered error:nil];
	self.offered = nil;
}

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
	(void)controller;
	@autoreleasepool {
		NSURL *url = urls.firstObject;
		[self tidy];
		if (!url) done(-1, @"");
		else if (self.kind == 2) keep_folder(url);
		else if (self.kind == 0) take_file(url, self.dir);
		else done(1, url.path ?: url.lastPathComponent);
	}
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {
	(void)controller;
	[self tidy];
	done(-1, @"");
}
@end

static CWSavesPicker *delegate;

static UIWindow *window_of(void *window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (!window || !SDL_GetWindowWMInfo((SDL_Window *)window, &info) || info.subsystem != SDL_SYSWM_UIKIT) return nil;
	return info.info.uikit.window;
}

/* (on the main queue) the picker for `kind` over the topmost screen */
static void present(UIWindow *ui, int kind, NSString *dir, NSString *offered) {
	UIViewController *top = ui.rootViewController;
	while (top.presentedViewController) top = top.presentedViewController;
	if (!delegate) delegate = [CWSavesPicker new];
	delegate.kind = kind;
	delegate.dir = dir;
	delegate.offered = offered;
	if (!top) {
		[delegate tidy];
		done(-2, @"The Files picker could not be opened.");
		return;
	}
	UIDocumentPickerViewController *picker;
	/* (an import as a copy the picker brings onto the phone; the folder opened in place, its scope kept) */
	if (kind == 1) picker = [[UIDocumentPickerViewController alloc] initForExportingURLs:@[ [NSURL fileURLWithPath:offered] ] asCopy:YES];
	else if (kind == 0) picker = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[ UTTypeData ] asCopy:YES];
	else picker = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[ UTTypeFolder ]];
	picker.delegate = delegate;
	picker.allowsMultipleSelection = NO;
	[top presentViewController:picker animated:YES completion:nil];
}

bool ios_saves_pick(void *window, const char *dir, int kind, const char *from) {
	@autoreleasepool {
		UIWindow *ui = window_of(window);
		if (!ui || !dir) return false;
		NSString *data_dir = @(dir), *offered = nil;
		if (kind == 1) {
			/* (offered under its own name) */
			offered = [NSTemporaryDirectory() stringByAppendingPathComponent:@BACKUP_NAME];
			NSData *data = from ? [NSData dataWithContentsOfFile:@(from)] : nil;
			if (!data.length || ![data writeToFile:offered atomically:YES]) return false;
		}
		SDL_AtomicSet(&result, 0);
		dispatch_async(dispatch_get_main_queue(), ^{ present(ui, kind, data_dir, offered); });
		return true;
	}
}

int ios_saves_result(char *out, size_t n) {
	@autoreleasepool {
		int status = SDL_AtomicGet(&result);
		if (!status) return 0;
		if (n) snprintf(out, n, "%s", result_text);
		SDL_AtomicSet(&result, 0);
		return status;
	}
}
