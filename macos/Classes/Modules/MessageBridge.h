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
 * Inject AnyWP SDK into WebView configuration
 * Should be called during WebView configuration setup
 */
- (void)injectSDKIntoConfiguration:(WKWebViewConfiguration *)configuration;

/**
 * Inject AnyWP SDK into an existing WebView (legacy method)
 */
- (void)injectSDKIntoWebView:(WKWebView *)webView;

/**
 * Send message directly to a specific WebView
 * Internal method, can be called by WallpaperManager
 */
- (BOOL)sendMessageToWebView:(WKWebView *)webView message:(NSString *)message;

@end

