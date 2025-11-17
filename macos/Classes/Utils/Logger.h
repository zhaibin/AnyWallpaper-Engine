#import <Foundation/Foundation.h>

/**
 * Centralized logging utility for AnyWP Engine
 * Provides consistent logging across all modules
 */
@interface AWPLogger : NSObject

/**
 * Log an informational message
 */
+ (void)log:(NSString *)message;

/**
 * Log an error message
 */
+ (void)error:(NSString *)message;

/**
 * Log a warning message
 */
+ (void)warn:(NSString *)message;

/**
 * Log a debug message (only in debug builds)
 */
+ (void)debug:(NSString *)message;

@end

