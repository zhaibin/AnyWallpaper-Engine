#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>
#import "MonitorManager.h"
#import "MessageBridge.h"

/**
 * Wallpaper instance for a specific monitor
 */
@interface WallpaperInstance : NSObject

@property (nonatomic, assign) NSInteger monitorIndex;
@property (nonatomic, strong) NSWindow *window;
@property (nonatomic, strong) WKWebView *webView;
@property (nonatomic, strong) NSString *currentURL;
@property (nonatomic, assign) BOOL isPaused;
@property (nonatomic, strong) NSString *allowedAccessPath;  // Custom allowed access path for file loading

@end

/**
 * Wallpaper management module
 * Handles wallpaper window creation, WebView management, and lifecycle
 */
@interface WallpaperManager : NSObject <WKNavigationDelegate>

/**
 * Initialize with required dependencies
 */
- (instancetype)initWithMonitorManager:(MonitorManager *)monitorManager
                        messageBridge:(MessageBridge *)messageBridge;

/**
 * Initialize wallpaper on primary monitor
 */
- (BOOL)initializeWallpaper:(NSString *)url;

/**
 * Stop wallpaper on primary monitor
 */
- (BOOL)stopWallpaper;

/**
 * Navigate to new URL on primary monitor
 */
- (BOOL)navigateToUrl:(NSString *)url;

/**
 * Initialize wallpaper on specific monitor
 */
- (BOOL)initializeWallpaperOnMonitor:(NSString *)url monitorIndex:(NSInteger)monitorIndex;

/**
 * Initialize wallpaper on specific monitor with custom allowed access path
 * @param url The URL to load
 * @param monitorIndex Target monitor index
 * @param allowedAccessPath Custom path for file access authorization (nil = use default)
 */
- (BOOL)initializeWallpaperOnMonitor:(NSString *)url 
                        monitorIndex:(NSInteger)monitorIndex 
                   allowedAccessPath:(NSString *)allowedAccessPath;

/**
 * Stop wallpaper on specific monitor
 */
- (BOOL)stopWallpaperOnMonitor:(NSInteger)monitorIndex;

/**
 * Navigate to new URL on specific monitor
 */
- (BOOL)navigateToUrlOnMonitor:(NSString *)url monitorIndex:(NSInteger)monitorIndex;

/**
 * Stop all wallpapers
 */
- (void)stopAllWallpapers;

/**
 * Get wallpaper instance for monitor
 */
- (WallpaperInstance *)getInstanceForMonitor:(NSInteger)monitorIndex;

/**
 * Get all wallpaper instances
 */
- (NSArray<WallpaperInstance *> *)getAllInstances;

/**
 * Set interactive mode for wallpaper
 * @param monitorIndex Monitor index
 * @param interactive YES for interactive mode (captures mouse), NO for simple mode (transparent)
 */
- (BOOL)setInteractiveMode:(BOOL)interactive forMonitor:(NSInteger)monitorIndex;

/**
 * Pause all wallpapers (hide windows, stop rendering)
 */
- (void)pauseAllWallpapers;

/**
 * Resume all wallpapers
 */
- (void)resumeAllWallpapers;

/**
 * Send message to all WebViews
 */
- (void)sendMessageToAll:(NSString *)message;

/**
 * Set global allowed access path for file loading
 * This path will be used as the base for file access authorization
 * @param path The path to authorize (nil = use Library directory)
 */
- (void)setGlobalAllowedAccessPath:(NSString *)path;

/**
 * Get current global allowed access path
 */
- (NSString *)globalAllowedAccessPath;

/**
 * Get the default Library directory path
 */
+ (NSString *)defaultLibraryPath;

/**
 * Get Application Support directory path
 */
+ (NSString *)applicationSupportPath;

@end

