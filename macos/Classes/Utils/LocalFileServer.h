//
// LocalFileServer.h
// AnyWP Engine - Local File Server for macOS
//
// Provides a simple local file server to solve CORS issues
// Uses custom URL scheme: localfile://
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Local File Server - Lightweight implementation using NSURLProtocol
 * 
 * Features:
 * - Custom URL scheme: localfile://
 * - Automatic MIME type detection
 * - CORS headers support
 * - No external dependencies
 * 
 * Usage:
 * 1. Start server: [LocalFileServer startWithRootDirectory:@"/path/to/files"]
 * 2. Load in WebView: localfile:///path/to/file.html
 * 3. Stop server: [LocalFileServer stop]
 */
@interface LocalFileServer : NSObject

/**
 * Start the local file server
 * @param rootDirectory Root directory for serving files
 * @return YES if started successfully
 */
+ (BOOL)startWithRootDirectory:(NSString *)rootDirectory;

/**
 * Stop the local file server
 */
+ (void)stop;

/**
 * Check if server is running
 */
+ (BOOL)isRunning;

/**
 * Get the root directory
 */
+ (nullable NSString *)rootDirectory;

/**
 * Get the base URL for accessing files
 * @return Base URL (e.g., localfile://)
 */
+ (NSString *)baseURL;

@end

NS_ASSUME_NONNULL_END

