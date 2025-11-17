#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>
#import <FlutterMacOS/FlutterMacOS.h>

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
 * Send message to WebView(s)
 * If monitorIndex is -1, send to all monitors
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

