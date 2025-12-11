#import <Foundation/Foundation.h>

/**
 * Power management module
 * Handles power saving, memory optimization, and system state monitoring
 */
@interface PowerManager : NSObject

/**
 * Pause wallpaper (manual)
 */
- (void)pauseWallpaper;

/**
 * Resume wallpaper
 */
- (void)resumeWallpaper;

/**
 * Set auto power saving enabled/disabled
 */
- (void)setAutoPowerSaving:(BOOL)enabled;

/**
 * Get current power state
 * Returns: ACTIVE, IDLE, SCREEN_OFF, LOCKED, FULLSCREEN_APP, PAUSED
 */
- (NSString *)getPowerState;

/**
 * Get current memory usage in MB
 */
- (NSInteger)getMemoryUsage;

/**
 * Manually trigger memory optimization
 * @param instances Array of WallpaperInstance objects to optimize (pass nil for all)
 */
- (void)optimizeMemory:(NSArray *)instances;

/**
 * Get pending power state changes
 * Returns array of dictionaries with oldState and newState
 */
- (NSArray *)getPendingPowerStateChanges;

/**
 * Set idle timeout in seconds
 */
- (void)setIdleTimeout:(NSInteger)seconds;

/**
 * Set memory threshold in MB
 */
- (void)setMemoryThreshold:(NSInteger)thresholdMB;

/**
 * Set cleanup interval in minutes
 */
- (void)setCleanupInterval:(NSInteger)minutes;

/**
 * Get current configuration
 */
- (NSDictionary *)getConfiguration;

@end

