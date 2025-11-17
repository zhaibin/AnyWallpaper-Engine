#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

/**
 * Monitor management module
 * Handles multi-monitor detection and tracking
 */
@interface MonitorManager : NSObject

/**
 * Get list of all available monitors
 * Returns array of dictionaries with monitor info:
 * - index: Monitor index (0-based)
 * - deviceName: Device identifier
 * - left, top, width, height: Monitor bounds
 * - isPrimary: Whether this is the primary monitor
 */
- (NSArray<NSDictionary *> *)getMonitors;

/**
 * Get monitor count
 */
- (NSInteger)getMonitorCount;

/**
 * Get monitor at specific index
 * Returns nil if index is out of bounds
 */
- (NSScreen *)getMonitorAtIndex:(NSInteger)index;

/**
 * Get primary monitor
 */
- (NSScreen *)getPrimaryMonitor;

/**
 * Setup display change notifications
 */
- (void)setupDisplayChangeNotifications:(void (^)(void))callback;

/**
 * Remove display change notifications
 */
- (void)removeDisplayChangeNotifications;

@end

