#import <Foundation/Foundation.h>

/**
 * Log level enumeration
 */
typedef NS_ENUM(NSInteger, AWPLogLevel) {
    AWPLogLevelDebug = 0,   // Verbose debugging information
    AWPLogLevelInfo = 1,    // General informational messages
    AWPLogLevelWarn = 2,    // Warning messages
    AWPLogLevelError = 3,   // Error messages
    AWPLogLevelNone = 4     // Disable all logging
};

/**
 * Centralized logging utility for AnyWP Engine
 * Provides consistent logging across all modules with log level control
 */
@interface AWPLogger : NSObject

/**
 * Set the minimum log level (default: Info in Release, Debug in Debug builds)
 */
+ (void)setLogLevel:(AWPLogLevel)level;

/**
 * Get the current log level
 */
+ (AWPLogLevel)logLevel;

/**
 * Log a debug message (verbose, only for development)
 */
+ (void)debug:(NSString *)message;

/**
 * Log an informational message
 */
+ (void)log:(NSString *)message;

/**
 * Log a warning message
 */
+ (void)warn:(NSString *)message;

/**
 * Log an error message
 */
+ (void)error:(NSString *)message;

@end

