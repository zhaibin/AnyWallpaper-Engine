//
// LocalFileServer.m
// AnyWP Engine - Local File Server for macOS
//

#import "LocalFileServer.h"
#import "Logger.h"

// ============================================================================
// LocalFileProtocol - Custom URL Protocol Handler
// ============================================================================

@interface LocalFileProtocol : NSURLProtocol
@end

// Thread-safe static variables with synchronization
static NSString *_rootDirectory = nil;
static BOOL _isRunning = NO;
static dispatch_queue_t _syncQueue = nil;

@implementation LocalFileProtocol

+ (void)initialize {
    if (self == [LocalFileProtocol class]) {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            _syncQueue = dispatch_queue_create("com.anywp.localfileserver.sync", DISPATCH_QUEUE_SERIAL);
        });
    }
}

+ (BOOL)canInitWithRequest:(NSURLRequest *)request {
    // Handle requests with "localfile" scheme
    return [request.URL.scheme isEqualToString:@"localfile"];
}

+ (NSURLRequest *)canonicalRequestForRequest:(NSURLRequest *)request {
    return request;
}

- (void)startLoading {
    @try {
        NSURL *url = self.request.URL;
        
        // Extract file path from URL
        // localfile:///path/to/file.html -> /path/to/file.html
        NSString *filePath = [url.path stringByRemovingPercentEncoding];
        
        // Thread-safe access to _rootDirectory
        __block NSString *rootDir = nil;
        dispatch_sync(_syncQueue, ^{
            rootDir = [_rootDirectory copy];
        });
        
        // Security: Validate and normalize path
        if (!rootDir) {
            [AWPLogger error:@"LocalFileServer: Root directory not set"];
            [self sendError:500 message:@"Server not properly configured"];
            return;
        }
        
        // Always prepend root directory for security (treat all paths as relative)
        // Remove leading slash if present to ensure proper path joining
        if ([filePath hasPrefix:@"/"]) {
            filePath = [filePath substringFromIndex:1];
        }
        
        NSString *fullPath = [rootDir stringByAppendingPathComponent:filePath];
        
        // Security: Ensure the resolved path is within root directory
        NSString *canonicalRoot = [rootDir stringByStandardizingPath];
        NSString *canonicalPath = [fullPath stringByStandardizingPath];
        
        if (![canonicalPath hasPrefix:canonicalRoot]) {
            [AWPLogger error:[NSString stringWithFormat:@"Security: Path traversal attempt blocked: %@", filePath]];
            [self sendError:403 message:@"Access denied"];
            return;
        }
        
        [AWPLogger log:[NSString stringWithFormat:@"LocalFileServer: Loading %@", canonicalPath]];
        
        // Check if file exists
        NSFileManager *fileManager = [NSFileManager defaultManager];
        if (![fileManager fileExistsAtPath:canonicalPath]) {
            [self sendError:404 message:@"File not found"];
            return;
        }
        
        // Read file data
        NSError *readError = nil;
        NSData *data = [NSData dataWithContentsOfFile:canonicalPath options:0 error:&readError];
        if (readError || !data) {
            [AWPLogger error:[NSString stringWithFormat:@"Failed to read file: %@", readError.localizedDescription]];
            [self sendError:500 message:@"Failed to read file"];
            return;
        }
        
        // Detect MIME type
        NSString *mimeType = [self detectMIMEType:canonicalPath];
        
        // Create response with CORS headers
        NSHTTPURLResponse *response = [[NSHTTPURLResponse alloc]
            initWithURL:url
            statusCode:200
            HTTPVersion:@"HTTP/1.1"
            headerFields:@{
                @"Content-Type": mimeType,
                @"Content-Length": [NSString stringWithFormat:@"%lu", (unsigned long)data.length],
                @"Access-Control-Allow-Origin": @"*",
                @"Access-Control-Allow-Methods": @"GET, POST, OPTIONS",
                @"Access-Control-Allow-Headers": @"*",
                @"Cache-Control": @"max-age=3600"
            }];
        
        // Send response
        [self.client URLProtocol:self didReceiveResponse:response cacheStoragePolicy:NSURLCacheStorageAllowed];
        [self.client URLProtocol:self didLoadData:data];
        [self.client URLProtocolDidFinishLoading:self];
        
        [AWPLogger log:[NSString stringWithFormat:@"LocalFileServer: Served %@ (%@, %lu bytes)",
                       [canonicalPath lastPathComponent], mimeType, (unsigned long)data.length]];
        
    } @catch (NSException *exception) {
        [AWPLogger error:[NSString stringWithFormat:@"Exception in LocalFileProtocol: %@", exception.reason]];
        [self sendError:500 message:@"Internal server error"];
    }
}

- (void)stopLoading {
    // Nothing to do
}

- (void)sendError:(NSInteger)statusCode message:(NSString *)message {
    NSString *html = [NSString stringWithFormat:
        @"<!DOCTYPE html><html><head><title>Error %ld</title></head>"
        @"<body><h1>Error %ld</h1><p>%@</p></body></html>",
        (long)statusCode, (long)statusCode, message];
    
    NSData *data = [html dataUsingEncoding:NSUTF8StringEncoding];
    
    NSHTTPURLResponse *response = [[NSHTTPURLResponse alloc]
        initWithURL:self.request.URL
        statusCode:statusCode
        HTTPVersion:@"HTTP/1.1"
        headerFields:@{
            @"Content-Type": @"text/html; charset=utf-8",
            @"Content-Length": [NSString stringWithFormat:@"%lu", (unsigned long)data.length]
        }];
    
    [self.client URLProtocol:self didReceiveResponse:response cacheStoragePolicy:NSURLCacheStorageNotAllowed];
    [self.client URLProtocol:self didLoadData:data];
    [self.client URLProtocolDidFinishLoading:self];
}

- (NSString *)detectMIMEType:(NSString *)filePath {
    NSString *extension = [[filePath pathExtension] lowercaseString];
    
    // Common MIME types
    NSDictionary *mimeTypes = @{
        @"html": @"text/html",
        @"htm": @"text/html",
        @"css": @"text/css",
        @"js": @"application/javascript",
        @"json": @"application/json",
        @"xml": @"application/xml",
        @"txt": @"text/plain",
        @"jpg": @"image/jpeg",
        @"jpeg": @"image/jpeg",
        @"png": @"image/png",
        @"gif": @"image/gif",
        @"svg": @"image/svg+xml",
        @"webp": @"image/webp",
        @"mp4": @"video/mp4",
        @"webm": @"video/webm",
        @"mp3": @"audio/mpeg",
        @"wav": @"audio/wav",
        @"ogg": @"audio/ogg",
        @"pdf": @"application/pdf",
        @"zip": @"application/zip",
        @"woff": @"font/woff",
        @"woff2": @"font/woff2",
        @"ttf": @"font/ttf",
        @"otf": @"font/otf"
    };
    
    return mimeTypes[extension] ?: @"application/octet-stream";
}

@end

// ============================================================================
// LocalFileServer - Public API
// ============================================================================

@implementation LocalFileServer

+ (BOOL)startWithRootDirectory:(NSString *)rootDirectory {
    @try {
        // Thread-safe check if already running
        __block BOOL alreadyRunning = NO;
        dispatch_sync(_syncQueue, ^{
            alreadyRunning = _isRunning;
        });
        
        if (alreadyRunning) {
            [AWPLogger warn:@"LocalFileServer already running"];
            return YES;
        }
        
        // Validate root directory
        NSFileManager *fileManager = [NSFileManager defaultManager];
        BOOL isDirectory = NO;
        if (![fileManager fileExistsAtPath:rootDirectory isDirectory:&isDirectory] || !isDirectory) {
            [AWPLogger error:[NSString stringWithFormat:@"Invalid root directory: %@", rootDirectory]];
            return NO;
        }
        
        // Thread-safe update of static variables
        dispatch_sync(_syncQueue, ^{
            _rootDirectory = [rootDirectory copy];
            _isRunning = YES;
        });
        
        // Register custom URL protocol
        [NSURLProtocol registerClass:[LocalFileProtocol class]];
        
        [AWPLogger log:[NSString stringWithFormat:@"LocalFileServer started - Root: %@", rootDirectory]];
        [AWPLogger log:@"LocalFileServer: Use URL scheme: localfile:///path/to/file.html"];
        
        return YES;
        
    } @catch (NSException *exception) {
        [AWPLogger error:[NSString stringWithFormat:@"Failed to start LocalFileServer: %@", exception.reason]];
        return NO;
    }
}

+ (void)stop {
    @try {
        // Thread-safe check and update
        __block BOOL wasRunning = NO;
        dispatch_sync(_syncQueue, ^{
            wasRunning = _isRunning;
            if (_isRunning) {
                _rootDirectory = nil;
                _isRunning = NO;
            }
        });
        
        if (!wasRunning) {
            return;
        }
        
        // Unregister custom URL protocol
        [NSURLProtocol unregisterClass:[LocalFileProtocol class]];
        
        [AWPLogger log:@"LocalFileServer stopped"];
        
    } @catch (NSException *exception) {
        [AWPLogger error:[NSString stringWithFormat:@"Error stopping LocalFileServer: %@", exception.reason]];
    }
}

+ (BOOL)isRunning {
    __block BOOL running = NO;
    dispatch_sync(_syncQueue, ^{
        running = _isRunning;
    });
    return running;
}

+ (NSString *)rootDirectory {
    __block NSString *rootDir = nil;
    dispatch_sync(_syncQueue, ^{
        rootDir = [_rootDirectory copy];
    });
    return rootDir;
}

+ (NSString *)baseURL {
    return @"localfile://";
}

@end
