#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>
#import <FlutterMacOS/FlutterMacOS.h>

// Forward declaration to avoid circular dependency
@class WallpaperManager;

/**
 * Message bridge module
 * Handles bidirectional communication between Flutter and WebView
 */
@interface MessageBridge : NSObject<WKScriptMessageHandler>

/**
 * Initialize with Flutter method channel
 */
- (instancetype)initWithChannel:(FlutterMethodChannel *)channel;

/**
 * Set wallpaper manager reference (called after WallpaperManager is initialized)
 * v2.2.0: Added to enable message sending to WebViews
 */
- (void)setWallpaperManager:(WallpaperManager *)manager;

/**
 * Send message to WebView(s)
 * If monitorIndex is -1, send to all monitors
 * v2.2.0: Now fully functional with WallpaperManager integration
 */
- (BOOL)sendMessage:(NSString *)message toMonitorIndex:(NSInteger)monitorIndex;

/**
 * Get pending messages from JavaScript
 * Returns array of message strings (JSON)
 */
- (NSArray *)getPendingMessages;

/**
 * Inject AnyWP SDK into WebView
 */
- (void)injectSDKIntoWebView:(WKWebView *)webView;

@end

