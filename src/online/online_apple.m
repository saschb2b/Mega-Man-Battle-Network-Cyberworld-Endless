/* iOS's system WebSocket and TLS, driven by the SDL network worker. */
#include "online_socket.h"

#import <Foundation/Foundation.h>

#include <string.h>

#include "protocol.h"

@interface CWOnlineSocket : NSObject <NSURLSessionWebSocketDelegate>
@property(nonatomic, strong) NSLock *lock;
@property(nonatomic, strong) NSMutableArray<NSData *> *messages;
@property(nonatomic, strong) NSURLSession *session;
@property(nonatomic, strong) NSURLSessionWebSocketTask *task;
@property(nonatomic) int state;
- (void)receive;
- (void)finish;
@end

@implementation CWOnlineSocket
- (instancetype)init {
	self = [super init];
	if (self) { _lock = [NSLock new]; _messages = [NSMutableArray new]; }
	return self;
}

- (void)finish {
	[self.lock lock];
	self.state = -1;
	[self.lock unlock];
}

- (void)receive {
	[self.task receiveMessageWithCompletionHandler:^(NSURLSessionWebSocketMessage *message, NSError *error) {
		[self.lock lock];
		BOOL valid = !error && message.type == NSURLSessionWebSocketMessageTypeData && message.data.length > 0 &&
		             message.data.length <= ONLINE_MAX_MESSAGE && self.messages.count < 32;
		BOOL again = self.state == 1 && valid;
		if (again) [self.messages addObject:message.data];
		else self.state = -1;
		[self.lock unlock];
		/* (the worker's rest ended: online_socket_wait) */
		online_socket_wake();
		if (again) [self receive];
		else [self.task cancelWithCloseCode:NSURLSessionWebSocketCloseCodeProtocolError reason:nil];
	}];
}

- (void)URLSession:(NSURLSession *)session webSocketTask:(NSURLSessionWebSocketTask *)task didOpenWithProtocol:(NSString *)protocol {
	(void)session; (void)task;
	[self.lock lock];
	BOOL accepted = self.state == 0 && protocol && !strcmp(protocol.UTF8String, ONLINE_SUBPROTOCOL);
	self.state = accepted ? 1 : -1;
	[self.lock unlock];
	if (accepted) [self receive];
	else [task cancelWithCloseCode:NSURLSessionWebSocketCloseCodeProtocolError reason:nil];
}

- (void)URLSession:(NSURLSession *)session webSocketTask:(NSURLSessionWebSocketTask *)task
 didCloseWithCode:(NSURLSessionWebSocketCloseCode)code reason:(NSData *)reason {
	(void)session; (void)task; (void)code; (void)reason;
	[self finish];
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
	(void)session; (void)task; (void)error;
	[self finish];
}

/* A different server must get its own identity and explicit consent. */
- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
 willPerformHTTPRedirection:(NSHTTPURLResponse *)response newRequest:(NSURLRequest *)request
 completionHandler:(void (^)(NSURLRequest *))completionHandler {
	(void)session; (void)task; (void)response; (void)request;
	completionHandler(nil);
}
@end

static CWOnlineSocket *connection;
static SDL_atomic_t *cancelled;

bool online_socket_open(const char *url, SDL_atomic_t *stop, char *error, size_t size) {
	@autoreleasepool {
		online_socket_close();
		NSURL *address = [NSURL URLWithString:[NSString stringWithUTF8String:url]];
		if (!address || !([address.scheme isEqualToString:@"wss"] || [address.scheme isEqualToString:@"ws"])) return false;
		connection = [CWOnlineSocket new];
		cancelled = stop;
		NSURLSessionConfiguration *config = [NSURLSessionConfiguration ephemeralSessionConfiguration];
		config.timeoutIntervalForRequest = 7;
		config.HTTPCookieAcceptPolicy = NSHTTPCookieAcceptPolicyNever;
		config.HTTPShouldSetCookies = NO;
		config.URLCache = nil;
		connection.session = [NSURLSession sessionWithConfiguration:config delegate:connection delegateQueue:nil];
		NSArray<NSString *> *protocols = @[@ONLINE_SUBPROTOCOL];
		connection.task = [connection.session webSocketTaskWithURL:address protocols:protocols];
		connection.task.maximumMessageSize = ONLINE_MAX_MESSAGE;
		[connection.task resume];
		uint32_t started = SDL_GetTicks();
		while (!SDL_AtomicGet(stop) && SDL_GetTicks() - started < 7000) {
			[connection.lock lock];
			int state = connection.state;
			[connection.lock unlock];
			if (state == 1) return true;
			if (state < 0) break;
			SDL_Delay(10);
		}
		SDL_strlcpy(error, "Connection failed or timed out", size);
		return false;
	}
}

bool online_socket_send(const uint8_t *data, size_t size) {
	@autoreleasepool {
		if (!connection || !size || size > ONLINE_MAX_MESSAGE) return false;
		dispatch_semaphore_t completed = dispatch_semaphore_create(0);
		__block BOOL sent = NO;
		NSURLSessionWebSocketMessage *message = [[NSURLSessionWebSocketMessage alloc] initWithData:[NSData dataWithBytes:data length:size]];
		[connection.task sendMessage:message completionHandler:^(NSError *error) {
			sent = error == nil;
			dispatch_semaphore_signal(completed);
		}];
		uint32_t started = SDL_GetTicks();
		while (!SDL_AtomicGet(cancelled) && SDL_GetTicks() - started < 2000) {
			if (!dispatch_semaphore_wait(completed, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_MSEC))) return sent;
		}
		[connection.task cancelWithCloseCode:NSURLSessionWebSocketCloseCodeGoingAway reason:nil];
		return false;
	}
}

int online_socket_receive(uint8_t *data, size_t size) {
	@autoreleasepool {
		if (!connection) return -1;
		[connection.lock lock];
		NSData *message = connection.messages.firstObject;
		int count = connection.state < 0 ? -1 : 0;
		if (message) {
			count = message.length <= size ? (int)message.length : -1;
			if (count > 0) memcpy(data, message.bytes, (size_t)count);
			[connection.messages removeObjectAtIndex:0];
		}
		[connection.lock unlock];
		return count;
	}
}

void online_socket_close(void) {
	@autoreleasepool {
		[connection finish];
		[connection.task cancelWithCloseCode:NSURLSessionWebSocketCloseCodeGoingAway reason:nil];
		[connection.session invalidateAndCancel];
		connection = nil;
		cancelled = NULL;
	}
}

/* the worker's rest (online_socket.h): a semaphore made once and kept for
 * the process, posted by the game's thread and by each message's
 * completion; NSURLSessionWebSocketTask lets no TCP option be set
 * (docs/online/arena/rollback/quicker-on-the-wire.md) */
static SDL_atomic_t wake_made;
static SDL_sem *wake;

void online_socket_wait(uint32_t ms) {
	if (!SDL_AtomicGet(&wake_made) && (wake = SDL_CreateSemaphore(0))) SDL_AtomicSet(&wake_made, 1);
	if (SDL_AtomicGet(&wake_made)) SDL_SemWaitTimeout(wake, ms);
	else SDL_Delay(1);
}

void online_socket_wake(void) {
	if (SDL_AtomicGet(&wake_made)) SDL_SemPost(wake);
}
