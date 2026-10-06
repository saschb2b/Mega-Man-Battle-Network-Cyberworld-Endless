/* The statistics' requests on Apple's systems (analytics_net.h), macOS's
 * and iOS's: NSURLSession, which sends on its own queue and never waits
 * in the game's frame. An ephemeral session: no cookies, no cache, nothing
 * kept on the device. Objective-C with ARC, built for TARGET=macos and
 * ios alone (Makefile). */
#import <Foundation/Foundation.h>

#include <string.h>

#include "analytics_net.h"

bool analytics_apple_post(const char *url, const char *agent, const char *body, void (*done)(bool ok)) {
	static NSURLSession *session;
	static dispatch_once_t once;
	dispatch_once(&once, ^(void) {
		NSURLSessionConfiguration *c = [NSURLSessionConfiguration ephemeralSessionConfiguration];
		c.timeoutIntervalForRequest = 10;
		c.timeoutIntervalForResource = 15;
		c.HTTPCookieAcceptPolicy = NSHTTPCookieAcceptPolicyNever;
		c.HTTPShouldSetCookies = NO;
		c.URLCache = nil;
		session = [NSURLSession sessionWithConfiguration:c];
	});
	@autoreleasepool {
		NSString *address = url ? [NSString stringWithUTF8String:url] : nil;
		NSURL *to = address ? [NSURL URLWithString:address] : nil;
		NSString *who = agent ? [NSString stringWithUTF8String:agent] : nil;
		if (!session || !to || !who || !body) return false;
		NSMutableURLRequest *r = [NSMutableURLRequest requestWithURL:to];
		r.HTTPMethod = @"POST";
		[r setValue:@"application/json" forHTTPHeaderField:@"Content-Type"];
		[r setValue:who forHTTPHeaderField:@"User-Agent"];
		r.HTTPBody = [NSData dataWithBytes:body length:strlen(body)];
		NSURLSessionDataTask *t = [session dataTaskWithRequest:r completionHandler:^(NSData *data, NSURLResponse *answer, NSError *error) {
			(void)data;
			NSInteger status = [answer isKindOfClass:[NSHTTPURLResponse class]] ? ((NSHTTPURLResponse *)answer).statusCode : 0;
			done(!error && status >= 200 && status < 300);
		}];
		if (!t) return false;
		[t resume];
		return true;
	}
}
