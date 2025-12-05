#import <Foundation/Foundation.h>

/**
 * State persistence utility using UserDefaults and file system
 * Provides application-isolated storage similar to Windows Registry approach
 */
@interface StatePersistence : NSObject

/**
 * Set application name for storage isolation
 */
- (void)setApplicationName:(NSString *)name;

/**
 * Get storage path for this application
 */
- (NSString *)getStoragePath;

/**
 * Save state value for key
 */
- (BOOL)saveState:(NSString *)value forKey:(NSString *)key;

/**
 * Load state value for key
 * Returns nil if key doesn't exist
 */
- (NSString *)loadStateForKey:(NSString *)key;

/**
 * Clear all saved state
 */
- (BOOL)clearState;

@end

