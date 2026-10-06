/* The iPhone's and the iPad's own, in UIKit (ios.h). The game's main is
 * SDL's here too: SDL_main.h names main.c's SDL_main, and this file's
 * main hands it to UIKit, as SDL2main's would. */
#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include "backup.h"
#include "ios.h"
#include "rom.h"

/* (SDL_main.h names every main SDL_main, this one too: SDL's own
 * SDL_uikit_main.c undoes it the same way) */
#ifdef main
#undef main
#endif

int main(int argc, char *argv[]) {
	return SDL_UIKitRunApp(argc, argv, SDL_main);
}

/* ---- the ROMs: what a look takes, and what it refuses ---- */

/* The ROMs the game takes, as android/.../RomActivity.java has them: their
 * header's game code, their SHA-1, their copy's name (IOS_ROM_BN6, then
 * IOS_ROM_BN5) */
static const struct { const char *code, *sha1, *file, *tag; } known[] = {
	{ "BR5E", "89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6", "bn6g.gba", "BN6 Cybeast Gregar (USA)" },
	{ "BRKE", "5f472f78d8de2df01d5039e045c043cb40969a39", "bn5c.gba", "BN5 Team Colonel (USA)" },
};
#define KNOWN ((int)(sizeof known / sizeof *known))
/* Battle Network 6's and 5's other versions, by their game code: a player
 * who has one is told which it is (rom.c names BN6's the same way) */
static const struct { const char *code, *why; } others[] = {
	{ "BR6E", "BN6 Cybeast Falzar, not Gregar" },
	{ "BR6P", "BN6 Cybeast Falzar (Europe), not Gregar (USA)" },
	{ "BR6J", "Rockman EXE 6 Falzar (Japan), not BN6 Gregar (USA)" },
	{ "BR5P", "BN6 Cybeast Gregar (Europe), not the USA version" },
	{ "BR5J", "Rockman EXE 6 Gregar (Japan), not the USA version" },
	{ "BRBE", "BN5 Team ProtoMan, not Team Colonel" },
	{ "BRBP", "BN5 Team ProtoMan (Europe), not Team Colonel (USA)" },
	{ "BRBJ", "Rockman EXE 5 Team of Blues (Japan), not BN5 Team Colonel (USA)" },
	{ "BRKP", "BN5 Team Colonel (Europe), not the USA version" },
	{ "BRKJ", "Rockman EXE 5 Team of Colonel (Japan), not the USA version" },
};
#define BN6_NAME "Mega Man Battle Network 6: Cybeast Gregar (USA)"
/* (a pick's folder whose .gba files are in iCloud, not on the phone: they
 * are sent for when there are this few, a ROM folder; a library's are not) */
#define CLOUD_FETCH 4

/* The kept folder (a bookmark), its name, and the files a look settled
 * ("path|size|date"), in the app's own preferences */
static NSString *const kFolder = @"romFolder", *const kFolderName = @"romFolderName", *const kSeen = @"romSeen";

/* What one look found: the ROMs it copied in, and each file it refused, with why */
@interface CWLook : NSObject
@property (nonatomic, copy) NSString *where;    /* the folder's name; nil for files picked */
@property (nonatomic) unsigned kept, had;       /* copied in now; offered, a copy kept already */
@property (nonatomic) int files, gba, bn;       /* files picked; .gba files seen; Battle Network ROMs refused (first in refused) */
@property (nonatomic, strong) NSMutableArray<NSString *> *refused, *zipped;
@property (nonatomic, strong) NSMutableArray<NSString *> *bnWhy;   /* why each Battle Network ROM was refused */
@property (nonatomic, strong) NSMutableArray<NSURL *> *cloud;      /* .gba files in iCloud, not on the phone */
@end

@implementation CWLook
- (instancetype)init {
	if ((self = [super init])) {
		_refused = [NSMutableArray new];
		_zipped = [NSMutableArray new];
		_bnWhy = [NSMutableArray new];
		_cloud = [NSMutableArray new];
	}
	return self;
}

- (void)refuse:(NSString *)name why:(NSString *)why battleNetwork:(BOOL)bn {
	NSString *line = [NSString stringWithFormat:@"%@: %@", name, why];
	if (bn) {
		[_refused insertObject:line atIndex:(NSUInteger)_bn++];
		[_bnWhy addObject:why];
	} else [_refused addObject:line];
}
@end

static NSString *changed(int i) {
	return [NSString stringWithFormat:@"%s, but changed: patched, trimmed or a bad dump", known[i].tag];
}

/* A copy kept in dir already (bn6g.gba or bn5c.gba, 8 MB) */
static BOOL kept_in(NSString *dir, int i) {
	NSDictionary *a = [NSFileManager.defaultManager attributesOfItemAtPath:[dir stringByAppendingPathComponent:@(known[i].file)] error:nil];
	return a && a.fileSize == (unsigned long long)ROM_SIZE;
}

/* url read as Files' providers want (coordinated), its path handed to `reader` */
static void coordinated(NSURL *url, void (^reader)(NSURL *at)) {
	NSError *err = nil;
	[[[NSFileCoordinator alloc] initWithFilePresenter:nil] coordinateReadingItemAtURL:url options:NSFileCoordinatorReadingWithoutChanges
		error:&err byAccessor:^(NSURL *at) { reader(at); }];
	if (err) NSLog(@"ROM look: %@ could not be read: %@", url.lastPathComponent, err.localizedDescription);
}

typedef struct { uint8_t bytes[0xB0]; int got; long size; } RomHead;

/* A file's header (its game code at 0xAC) and size */
static RomHead head_of(NSURL *url) {
	__block RomHead h = { .got = -1, .size = -1 };
	coordinated(url, ^(NSURL *at) {
		FILE *f = fopen(at.fileSystemRepresentation, "rb");
		if (!f) return;
		h.got = (int)fread(h.bytes, 1, sizeof h.bytes, f);
		if (fseek(f, 0, SEEK_END) == 0) h.size = ftell(f);
		fclose(f);
	});
	return h;
}

/* ROM i copied into dir from url, its SHA-1 checked first: nil when it is
 * that ROM, unchanged, else why not */
static NSString *copy_in(NSURL *url, int i, NSString *dir) {
	__block NSString *why = @"could not be read";
	NSString *to = [dir stringByAppendingPathComponent:@(known[i].file)], *part = [to stringByAppendingString:@".part"];
	[NSFileManager.defaultManager createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:nil];
	coordinated(url, ^(NSURL *at) {
		FILE *f = fopen(at.fileSystemRepresentation, "rb");
		if (!f) return;
		uint8_t *data = malloc(ROM_SIZE + 1);
		size_t n = data ? fread(data, 1, ROM_SIZE + 1, f) : 0;
		fclose(f);
		if (!data) return;
		char hex[41] = "";
		if (n == ROM_SIZE) sha1_hex(data, n, hex);
		if (n != ROM_SIZE || strcmp(hex, known[i].sha1)) why = changed(i);
		else {
			FILE *o = fopen(part.fileSystemRepresentation, "wb");
			BOOL ok = o && fwrite(data, 1, n, o) == n;
			if (o && fclose(o)) ok = NO;
			if (ok && rename(part.fileSystemRepresentation, to.fileSystemRepresentation) == 0) why = nil;
			else {
				remove(part.fileSystemRepresentation);
				why = @"the app could not keep it";
			}
		}
		free(data);
	});
	return why;
}

/* One file: told by its header's game code; BN6's or BN5's (those `want`
 * names) checked by their SHA-1 and copied into dir. YES when it is
 * settled (taken, refused, or the same as a copy kept), so a quick look
 * need not open it again; NO when it could not be read this time. */
static BOOL offer(NSURL *url, NSString *name, unsigned want, NSString *dir, CWLook *l) {
	RomHead h = head_of(url);
	if (h.got < 0) { [l refuse:name why:@"could not be read" battleNetwork:NO]; return NO; }
	if (h.got < (int)sizeof h.bytes) { [l refuse:name why:@"not a GBA ROM" battleNetwork:NO]; return YES; }
	for (int i = 0; i < KNOWN; ++i) {
		if (memcmp(h.bytes + 0xAC, known[i].code, 4)) continue;
		if (!(want & (1u << i)) || kept_in(dir, i)) { l.had |= 1u << i; return YES; }
		NSString *why = h.size >= 0 && h.size != ROM_SIZE ? changed(i) : copy_in(url, i, dir);
		if (why) [l refuse:name why:why battleNetwork:YES];
		else l.kept |= 1u << i;
		return YES;
	}
	for (size_t k = 0; k < sizeof others / sizeof *others; ++k)
		if (!memcmp(h.bytes + 0xAC, others[k].code, 4)) { [l refuse:name why:@(others[k].why) battleNetwork:YES]; return YES; }
	[l refuse:name why:@"not BN6 Gregar or BN5 Team Colonel" battleNetwork:NO];
	return YES;
}

static BOOL zipped(NSString *name, CWLook *l) {
	NSString *ext = name.pathExtension.lowercaseString;
	if (![ext isEqualToString:@"zip"] && ![ext isEqualToString:@"7z"] && ![ext isEqualToString:@"rar"]) return NO;
	[l.zipped addObject:name];
	return YES;
}

/* iCloud Drive lists a file not on the phone as ".name.icloud": its name, else nil */
static NSString *cloud_name(NSString *name) {
	if (name.length > 8 && [name hasPrefix:@"."] && [name hasSuffix:@".icloud"]) return [name substringWithRange:NSMakeRange(1, name.length - 8)];
	return nil;
}

/* A file a provider has not brought onto the phone yet */
static BOOL not_here(NSURL *url) {
	NSString *status = nil;
	[url getResourceValue:&status forKey:NSURLUbiquitousItemDownloadingStatusKey error:nil];
	return [status isEqualToString:NSURLUbiquitousItemDownloadingStatusNotDownloaded];
}

/* The folder's files, and those of the folders directly in it, offered.
 * A quick look (at a start, or the ROM screen's every three seconds) opens
 * only an 8 MB .gba not settled before. */
static void walk(NSURL *folder, int depth, unsigned want, NSString *dir, BOOL quick, NSMutableSet<NSString *> *seen, CWLook *l) {
	NSArray<NSURLResourceKey> *keys = @[ NSURLIsDirectoryKey, NSURLFileSizeKey, NSURLContentModificationDateKey ];
	NSArray<NSURL *> *items = [NSFileManager.defaultManager contentsOfDirectoryAtURL:folder includingPropertiesForKeys:keys options:0 error:nil];
	for (NSURL *item in items) {
		NSNumber *isDir = nil, *bytes = nil;
		NSDate *date = nil;
		[item getResourceValue:&isDir forKey:NSURLIsDirectoryKey error:nil];
		NSString *name = item.lastPathComponent, *cloud = cloud_name(name);
		if (isDir.boolValue) {
			if (depth > 0) walk(item, depth - 1, want, dir, quick, seen, l);
			continue;
		}
		if (cloud) {
			if ([cloud.pathExtension caseInsensitiveCompare:@"gba"] == NSOrderedSame) [l.cloud addObject:[folder URLByAppendingPathComponent:cloud]];
			continue;
		}
		if (zipped(name, l) || [name.pathExtension caseInsensitiveCompare:@"gba"] != NSOrderedSame) continue;
		[item getResourceValue:&bytes forKey:NSURLFileSizeKey error:nil];
		[item getResourceValue:&date forKey:NSURLContentModificationDateKey error:nil];
		NSString *key = [NSString stringWithFormat:@"%@|%lld|%.0f", item.path, bytes.longLongValue, date.timeIntervalSince1970];
		if (not_here(item)) { [l.cloud addObject:item]; continue; }
		l.gba += 1;
		if (quick && (bytes.longLongValue != ROM_SIZE || [seen containsObject:key])) continue;
		if (offer(item, name, want, dir, l)) [seen addObject:key];
	}
}

/* A look at a folder, its scope entered by the caller */
static CWLook *look_in(NSURL *folder, NSString *where, unsigned want, NSString *dir, BOOL quick) {
	CWLook *l = [CWLook new];
	l.where = where;
	NSUserDefaults *prefs = NSUserDefaults.standardUserDefaults;
	NSMutableSet<NSString *> *seen = [NSMutableSet setWithArray:[prefs stringArrayForKey:kSeen] ?: @[]];
	NSUInteger before = seen.count;
	coordinated(folder, ^(NSURL *at) { walk(at, 1, want, dir, quick, seen, l); });
	if (seen.count != before) [prefs setObject:(seen.count > 400 ? @[] : seen.allObjects) forKey:kSeen];
	NSLog(@"ROM look in %@: %d .gba, kept %u, refused %@, %lu in iCloud", where, l.gba, l.kept, l.refused, (unsigned long)l.cloud.count);
	return l;
}

/* What the ROM screen says after a look that found no BN6: what was
 * there, and why each .gba was refused (the minifont's characters) */
static NSString *words(CWLook *l, BOOL sent) {
	NSMutableString *s = [NSMutableString new];
	if (l.where) [s appendString:l.gba == 0 && !l.cloud.count ? [NSString stringWithFormat:@"No .gba file in %@ or the folders in it.", l.where]
		: [NSString stringWithFormat:@"No " BN6_NAME " in %@.", l.where]];
	else [s appendString:l.files == 1 ? @"That is not " BN6_NAME "." : @"None of those is " BN6_NAME "."];
	NSUInteger shown = 0;
	for (NSString *r in l.refused) {
		if (shown == 4) break;
		[s appendFormat:@"\n%@", r];
		++shown;
	}
	if (l.refused.count > shown) [s appendFormat:@"\nand %lu more .gba files.", (unsigned long)(l.refused.count - shown)];
	for (NSUInteger k = 0; k < l.zipped.count && k < 2; ++k) [s appendFormat:@"\n%@ is zipped: unzip it first.", l.zipped[k]];
	if (l.cloud.count == 1) [s appendFormat:@"\n%@ is in iCloud, not on this phone yet: %@", l.cloud[0].lastPathComponent,
		sent ? @"downloading it. This screen looks again by itself." : @"download it in Files first."];
	else if (l.cloud.count) [s appendFormat:@"\n%lu .gba files are in iCloud, not on this phone yet: %@", (unsigned long)l.cloud.count,
		sent ? @"downloading them. This screen looks again by itself." : @"download BN6's in Files first."];
	if ((l.kept & IOS_ROM_BN5) || (l.had & IOS_ROM_BN5)) [s appendFormat:@"\n%s is kept, for when BN6 is here.", known[1].tag];
	return s;
}

static char pick_msg[1024];   /* (ios_pick_result) */
static char note[256];        /* (ios_rom_note) */
static BOOL fetching;         /* a pick sent for its folder's files in iCloud */
static BOOL picked_saves;     /* (ios_pick_saves) */

/* What a look that took BN6 says: the ROMs copied in, and from where */
static NSString *took_words(CWLook *l) {
	NSString *what = (l.kept & IOS_ROM_BN5) ? [NSString stringWithFormat:@"%s and %s", known[0].tag, known[1].tag] : @(known[0].tag);
	return l.where ? [NSString stringWithFormat:@"Took %@ from %@.", what, l.where] : [NSString stringWithFormat:@"Took %@.", what];
}

/* The saves' copy in the folder (a reinstall's), copied to `to`: whether there was one */
static BOOL fetch_saves(NSURL *folder, NSString *to) {
	__block BOOL ok = NO;
	NSURL *saves = [folder URLByAppendingPathComponent:@BACKUP_NAME];
	if (![NSFileManager.defaultManager fileExistsAtPath:saves.path]) return NO;
	coordinated(saves, ^(NSURL *at) {
		NSData *data = [NSData dataWithContentsOfURL:at];
		if (data.length && data.length <= BACKUP_MAX) ok = [data writeToFile:to atomically:YES];
	});
	return ok;
}

static void to_c(NSString *s, char *out, size_t n) {
	if (n) snprintf(out, n, "%s", s.UTF8String ?: "");
}

/* With BN6 kept, a Battle Network ROM refused beside it (BN5 Team ProtoMan,
 * say) is said over the game as it starts: the title says nothing of it */
static void leave_note(CWLook *l, BOOL bn6) {
	if (!bn6 || l.bn <= 0) return;
	NSString *s = [NSString stringWithFormat:@"Not taken: %@%@", l.bnWhy[0], l.bn > 1 ? [NSString stringWithFormat:@" (and %d more)", l.bn - 1] : @""];
	to_c(s, note, sizeof note);
}

int ios_rom_folder_look(const char *dir, unsigned want, char *msg, size_t msglen) {
	@autoreleasepool {
		NSUserDefaults *prefs = NSUserDefaults.standardUserDefaults;
		NSData *mark = [prefs dataForKey:kFolder];
		if (!mark) return -1;
		NSString *where = [prefs stringForKey:kFolderName] ?: @"your ROM folder";
		BOOL stale = NO;
		NSError *err = nil;
		NSURL *folder = [NSURL URLByResolvingBookmarkData:mark options:0 relativeToURL:nil bookmarkDataIsStale:&stale error:&err];
		if (!folder) {
			/* (moved off the phone, or deleted: forgotten; the ROMs kept are copies) */
			NSLog(@"ROM look: the folder %@ can't be opened any more: %@", where, err.localizedDescription);
			[prefs removeObjectForKey:kFolder];
			to_c([NSString stringWithFormat:@"The folder %@ can't be opened any more: choose it again.", where], msg, msglen);
			return -1;
		}
		BOOL scoped = [folder startAccessingSecurityScopedResource];
		if (stale) {
			NSData *fresh = [folder bookmarkDataWithOptions:NSURLBookmarkCreationMinimalBookmark includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
			if (fresh) [prefs setObject:fresh forKey:kFolder];
		}
		CWLook *l = look_in(folder, where, want, @(dir), YES);
		if (scoped) [folder stopAccessingSecurityScopedResource];
		leave_note(l, !(want & IOS_ROM_BN6) || (l.kept & IOS_ROM_BN6));
		/* (the screen's words change only for a file newly refused) */
		if ((want & IOS_ROM_BN6) && !(l.kept & IOS_ROM_BN6) && l.refused.count) to_c(words(l, fetching), msg, msglen);
		return (int)l.kept;
	}
}

/* ---- the pickers ---- */

static int picked;   /* (ios_pick_result) */

@interface CWRomPicker : NSObject <UIDocumentPickerDelegate>
@property (nonatomic) int what;
@property (nonatomic, copy) NSString *dir;
@end

@implementation CWRomPicker
- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
	(void)controller;
	@autoreleasepool {
		unsigned want = IOS_ROM_BN6 | IOS_ROM_BN5;
		CWLook *l;
		fetching = NO;
		if (self.what == IOS_PICK_FOLDER) {
			NSURL *folder = urls.firstObject;
			if (!folder) { picked = -1; return; }
			BOOL scoped = [folder startAccessingSecurityScopedResource];
			/* (kept for the looks at later starts: a bookmark made in its scope) */
			NSUserDefaults *prefs = NSUserDefaults.standardUserDefaults;
			NSString *where = folder.lastPathComponent;
			NSData *mark = [folder bookmarkDataWithOptions:NSURLBookmarkCreationMinimalBookmark includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
			if (mark) {
				[prefs setObject:mark forKey:kFolder];
				[prefs setObject:where forKey:kFolderName];
			}
			l = look_in(folder, where, want, self.dir, NO);
			/* (the saves a reinstall left there, for the launcher to offer back) */
			picked_saves = fetch_saves(folder, [[self.dir stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"found.cwsave"]);
			/* (a ROM folder's files still in iCloud are sent for; a library's are not) */
			if (l.cloud.count && (NSUInteger)l.gba + l.cloud.count <= CLOUD_FETCH) {
				for (NSURL *u in l.cloud) [NSFileManager.defaultManager startDownloadingUbiquitousItemAtURL:u error:nil];
				fetching = YES;
			}
			if (scoped) [folder stopAccessingSecurityScopedResource];
		} else {
			/* (the picker's copies, in the app's tmp: looked at, then gone) */
			l = [CWLook new];
			l.files = (int)urls.count;
			for (NSURL *u in urls) {
				if (!zipped(u.lastPathComponent, l)) {
					l.gba += 1;
					offer(u, u.lastPathComponent, want, self.dir, l);
				}
				[NSFileManager.defaultManager removeItemAtURL:u error:nil];
			}
			NSLog(@"ROM look at %d files picked: kept %u, refused %@", l.files, l.kept, l.refused);
		}
		to_c(l.kept & IOS_ROM_BN6 ? took_words(l) : words(l, fetching), pick_msg, sizeof pick_msg);
		leave_note(l, l.kept & IOS_ROM_BN6);
		picked = 1;
	}
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

void ios_pick_roms(void *window, const char *dir, int what) {
	UIViewController *top = window_of(window).rootViewController;
	while (top.presentedViewController) top = top.presentedViewController;
	if (!top) { picked = -1; return; }
	if (!picker_delegate) picker_delegate = [CWRomPicker new];
	picker_delegate.dir = @(dir);
	picker_delegate.what = what;
	/* (a folder opened in place, its scope the app's to enter; files as copies) */
	UIDocumentPickerViewController *files = what == IOS_PICK_FOLDER
		? [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[ UTTypeFolder ]]
		: [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[ UTTypeData ] asCopy:YES];
	files.delegate = picker_delegate;
	files.allowsMultipleSelection = what == IOS_PICK_FILES;
	picked = 0;
	picked_saves = NO;
	pick_msg[0] = 0;
	[top presentViewController:files animated:YES completion:nil];
}

int ios_pick_result(char *msg, size_t msglen) {
	int r = picked;
	picked = 0;
	if (r && msglen) snprintf(msg, msglen, "%s", r > 0 ? pick_msg : "");
	return r;
}

bool ios_pick_saves(void) { return picked_saves; }

bool ios_rom_folder_name(char *out, size_t n) {
	NSUserDefaults *prefs = NSUserDefaults.standardUserDefaults;
	if (![prefs dataForKey:kFolder]) return false;
	to_c([prefs stringForKey:kFolderName] ?: @"your ROM folder", out, n);
	return true;
}

bool ios_saves_put(const char *from) {
	@autoreleasepool {
		NSData *mark = [NSUserDefaults.standardUserDefaults dataForKey:kFolder];
		NSData *data = [NSData dataWithContentsOfFile:@(from)];
		if (!mark || !data.length) return false;
		BOOL stale = NO;
		NSURL *folder = [NSURL URLByResolvingBookmarkData:mark options:0 relativeToURL:nil bookmarkDataIsStale:&stale error:nil];
		if (!folder) return false;
		BOOL scoped = [folder startAccessingSecurityScopedResource];
		__block BOOL ok = NO;
		NSError *err = nil;
		/* (written as Files' providers want it: coordinated, whole) */
		[[[NSFileCoordinator alloc] initWithFilePresenter:nil] coordinateWritingItemAtURL:[folder URLByAppendingPathComponent:@BACKUP_NAME]
			options:NSFileCoordinatorWritingForReplacing error:&err byAccessor:^(NSURL *at) { ok = [data writeToURL:at atomically:YES]; }];
		if (scoped) [folder stopAccessingSecurityScopedResource];
		if (!ok) NSLog(@"saves: the copy in the ROM folder could not be written: %@", err.localizedDescription);
		return ok;
	}
}

void ios_rom_note(void *window) {
	UIView *view = window_of(window).rootViewController.view;
	if (!note[0] || !view) return;
	UILabel *label = [UILabel new];
	label.text = @(note);
	note[0] = 0;
	label.numberOfLines = 0;
	label.textAlignment = NSTextAlignmentCenter;
	label.textColor = UIColor.whiteColor;
	label.font = [UIFont boldSystemFontOfSize:15];
	label.backgroundColor = [UIColor colorWithRed:16 / 255.0 green:54 / 255.0 blue:74 / 255.0 alpha:0.92];
	label.layer.cornerRadius = 8;
	label.layer.masksToBounds = YES;
	label.userInteractionEnabled = NO;
	CGRect safe = UIEdgeInsetsInsetRect(view.bounds, view.safeAreaInsets);
	CGSize fit = [label sizeThatFits:CGSizeMake(safe.size.width - 48, CGFLOAT_MAX)];
	label.frame = CGRectMake(safe.origin.x + (safe.size.width - fit.width - 24) / 2, CGRectGetMaxY(safe) - fit.height - 40, fit.width + 24, fit.height + 16);
	[view addSubview:label];
	[UIView animateWithDuration:0.4 delay:4.0 options:0 animations:^{ label.alpha = 0; } completion:^(BOOL done) { (void)done; [label removeFromSuperview]; }];
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
