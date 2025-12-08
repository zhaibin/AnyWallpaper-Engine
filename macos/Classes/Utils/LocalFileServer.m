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

static NSString *_rootDirectory = nil;
static BOOL _isRunning = NO;

@implementation LocalFileProtocol

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
        
        // If path is relative, prepend root directory
        if (![filePath isAbsolutePath] && _rootDirectory) {
            filePath = [_rootDirectory stringByAppendingPathComponent:filePath];
        }
        
        [AWPLogger log:[NSString stringWithFormat:@"LocalFileServer: Loading %@", filePath]];
        
        // Check if file exists
        NSFileManager *fileManager = [NSFileManager defaultManager];
        if (![fileManager fileExistsAtPath:filePath]) {
            [self sendError:404 message:@"File not found"];
            return;
        }
        
        // Read file data
        NSError *readError = nil;
        NSData *data = [NSData dataWithContentsOfFile:filePath options:0 error:&readError];
        if (readError || !data) {
            [AWPLogger error:[NSString stringWithFormat:@"Failed to read file: %@", readError.localizedDescription]];
            [self sendError:500 message:@"Failed to read file"];
            return;
        }
        
        // Detect MIME type
        NSString *mimeType = [self detectMIMEType:filePath];
        
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
                       [filePath lastPathComponent], mimeType, (unsigned long)data.length]];
        
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
        if (_isRunning) {
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
        
        // Store root directory
        _rootDirectory = [rootDirectory copy];
        
        // Register custom URL protocol
        [NSURLProtocol registerClass:[LocalFileProtocol class]];
        
        _isRunning = YES;
        
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
        if (!_isRunning) {
            return;
        }
        
        // Unregister custom URL protocol
        [NSURLProtocol unregisterClass:[LocalFileProtocol class]];
        
        _rootDirectory = nil;
        _isRunning = NO;
        
        [AWPLogger log:@"LocalFileServer stopped"];
        
    } @catch (NSException *exception) {
        [AWPLogger error:[NSString stringWithFormat:@"Error stopping LocalFileServer: %@", exception.reason]];
    }
}

+ (BOOL)isRunning {
    return _isRunning;
}

+ (NSString *)rootDirectory {
    return _rootDirectory;
}

+ (NSString *)baseURL {
    return @"localfile://";
}

@end

