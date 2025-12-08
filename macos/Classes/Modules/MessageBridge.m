#import "MessageBridge.h"
#import "../Utils/Logger.h"
#import "../Utils/EmbeddedSDK.h"
#import "WallpaperManager.h"

@interface MessageBridge ()

@property (nonatomic, strong) FlutterMethodChannel *channel;
@property (nonatomic, strong) NSMutableArray *pendingMessages;
@property (nonatomic, weak) WallpaperManager *wallpaperManager;  // Weak reference to avoid retain cycle

@end

@implementation MessageBridge

- (instancetype)initWithChannel:(FlutterMethodChannel *)channel {
    self = [super init];
    if (self) {
        _channel = channel;
        _pendingMessages = [NSMutableArray array];
        _wallpaperManager = nil;
        [AWPLogger log:@"MessageBridge initialized"];
    }
    return self;
}

- (void)setWallpaperManager:(WallpaperManager *)manager {
    _wallpaperManager = manager;  // Use instance variable directly to avoid recursion
    [AWPLogger log:@"WallpaperManager reference set in MessageBridge"];
}

#pragma mark - WKScriptMessageHandler

- (void)userContentController:(WKUserContentController *)userContentController
      didReceiveScriptMessage:(WKScriptMessage *)message {
    
    if (![message.name isEqualToString:@"anywpMessage"]) {
        return;
    }
    
    // Message from JavaScript
    NSString *messageBody = nil;
    
    if ([message.body isKindOfClass:[NSString class]]) {
        messageBody = (NSString *)message.body;
    } else if ([message.body isKindOfClass:[NSDictionary class]]) {
        // Convert dictionary to JSON string
        NSError *error = nil;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:message.body
                                                          options:0
                                                            error:&error];
        if (!error && jsonData) {
            messageBody = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        }
    }
    
    if (messageBody) {
        [AWPLogger log:[NSString stringWithFormat:@"Message received from JavaScript: %@", messageBody]];
        
        // Add to pending messages queue
        @synchronized (self.pendingMessages) {
            [self.pendingMessages addObject:messageBody];
        }
    } else {
        [AWPLogger error:@"Failed to process message from JavaScript"];
    }
}

#pragma mark - Public API

- (BOOL)sendMessage:(NSString *)message toMonitorIndex:(NSInteger)monitorIndex {
    if (!self.wallpaperManager) {
        [AWPLogger error:@"WallpaperManager not set, cannot send message"];
        return NO;
    }
    
    [AWPLogger log:[NSString stringWithFormat:@"Sending message to monitor %ld: %@",
                   (long)monitorIndex, message]];
    
    if (monitorIndex < 0) {
        // Send to all monitors
        [self.wallpaperManager sendMessageToAll:message];
        return YES;
    } else {
        // Send to specific monitor
        WallpaperInstance *instance = [self.wallpaperManager getInstanceForMonitor:monitorIndex];
        if (!instance || !instance.webView) {
            [AWPLogger error:[NSString stringWithFormat:@"No WebView found for monitor %ld",
                            (long)monitorIndex]];
            return NO;
        }
        
        return [self sendMessageToWebView:instance.webView message:message];
    }
}

- (BOOL)sendMessageToWebView:(WKWebView *)webView message:(NSString *)message {
    if (!webView) {
        [AWPLogger error:@"WebView is nil"];
        return NO;
    }
    
    // Escape the message string for JavaScript
    NSString *escapedMessage = [self escapeJavaScript:message];
    
    // Build JavaScript code to dispatch CustomEvent
    NSString *script = [NSString stringWithFormat:
        @"(function() {\n"
        @"  try {\n"
        @"    const messageStr = \"%@\";\n"
        @"    const message = JSON.parse(messageStr);\n"
        @"    const event = new CustomEvent('AnyWP:message', {\n"
        @"      detail: message,\n"
        @"      bubbles: true\n"
        @"    });\n"
        @"    window.dispatchEvent(event);\n"
        @"    console.log('[AnyWP Engine] Message dispatched:', message);\n"
        @"  } catch(e) {\n"
        @"    console.error('[AnyWP Engine] Failed to dispatch message:', e);\n"
        @"    console.error('[AnyWP Engine] Message string:', \"%@\");\n"
        @"  }\n"
        @"})();\n",
        escapedMessage, escapedMessage];
    
    // Execute script
    [webView evaluateJavaScript:script completionHandler:^(id result, NSError *error) {
        if (error) {
            [AWPLogger error:[NSString stringWithFormat:@"Failed to execute script: %@",
                            error.localizedDescription]];
        } else {
            [AWPLogger log:@"Message sent to WebView successfully"];
        }
    }];
    
    return YES;
}

- (NSString *)escapeJavaScript:(NSString *)string {
    if (!string) {
        return @"";
    }
    
    NSMutableString *escaped = [NSMutableString stringWithString:string];
    [escaped replaceOccurrencesOfString:@"\\" withString:@"\\\\" options:0 range:NSMakeRange(0, escaped.length)];
    [escaped replaceOccurrencesOfString:@"\"" withString:@"\\\"" options:0 range:NSMakeRange(0, escaped.length)];
    [escaped replaceOccurrencesOfString:@"\n" withString:@"\\n" options:0 range:NSMakeRange(0, escaped.length)];
    [escaped replaceOccurrencesOfString:@"\r" withString:@"\\r" options:0 range:NSMakeRange(0, escaped.length)];
    [escaped replaceOccurrencesOfString:@"\t" withString:@"\\t" options:0 range:NSMakeRange(0, escaped.length)];
    return escaped;
}

- (NSArray *)getPendingMessages {
    @synchronized (self.pendingMessages) {
        NSArray *messages = [self.pendingMessages copy];
        [self.pendingMessages removeAllObjects];
        return messages;
    }
}

- (void)injectSDKIntoConfiguration:(WKWebViewConfiguration *)configuration {
    // Load SDK script
    NSString *sdkScript = [self loadSDKScript];
    if (!sdkScript) {
        [AWPLogger error:@"Failed to load SDK script"];
        return;
    }
    
    // Inject at document start
    WKUserScript *userScript = [[WKUserScript alloc] initWithSource:sdkScript
                                                      injectionTime:WKUserScriptInjectionTimeAtDocumentStart
                                                   forMainFrameOnly:NO];
    
    [configuration.userContentController addUserScript:userScript];
    
    [AWPLogger log:@"AnyWP SDK injected into WebView configuration"];
}

- (void)injectSDKIntoWebView:(WKWebView *)webView {
    // Load SDK script
    NSString *sdkScript = [self loadSDKScript];
    if (!sdkScript) {
        [AWPLogger error:@"Failed to load SDK script"];
        return;
    }
    
    // Inject at document start
    WKUserScript *userScript = [[WKUserScript alloc] initWithSource:sdkScript
                                                      injectionTime:WKUserScriptInjectionTimeAtDocumentStart
                                                   forMainFrameOnly:NO];
    
    [webView.configuration.userContentController addUserScript:userScript];
    
    [AWPLogger log:@"AnyWP SDK injected into WebView"];
}

- (NSString *)loadSDKScript {
    // v2.6.0: Use embedded SDK (similar to Windows DLL resource approach)
    // Priority 1: Load from embedded source (production, recommended)
    // Priority 2: Load from bundle resources (development fallback)
    // Priority 3: Minimal fallback SDK (emergency fallback)
    
    // Strategy 1: Load embedded SDK (recommended for production)
    NSString *embeddedSDK = [EmbeddedSDK getSDKScript];
    if (embeddedSDK && embeddedSDK.length > 0) {
        [AWPLogger log:[NSString stringWithFormat:@"Using embedded SDK v%@ (%lu bytes)",
                       [EmbeddedSDK getSDKVersion],
                       (unsigned long)embeddedSDK.length]];
        return embeddedSDK;
    }
    
    [AWPLogger warn:@"Embedded SDK not available, trying bundle resources..."];
    
    // Strategy 2: Load from bundle resources (fallback for development)
    NSBundle *bundle = [NSBundle bundleForClass:[self class]];
    NSString *sdkPath = [bundle pathForResource:@"anywp_sdk" ofType:@"js"];
    
    if (sdkPath) {
        NSError *error = nil;
        NSString *sdkScript = [NSString stringWithContentsOfFile:sdkPath
                                                        encoding:NSUTF8StringEncoding
                                                           error:&error];
        if (!error && sdkScript.length > 0) {
            [AWPLogger log:@"Loaded SDK from bundle resources (development mode)"];
            return sdkScript;
        }
        
        [AWPLogger error:[NSString stringWithFormat:@"Failed to load SDK from bundle: %@",
                        error.localizedDescription]];
    }
    
    // Strategy 3: Minimal fallback SDK (emergency)
    [AWPLogger warn:@"Using minimal fallback SDK (limited functionality)"];
    return [self fallbackSDKScript];
}

- (NSString *)fallbackSDKScript {
    // Minimal fallback SDK for development/testing
    // In production, the full SDK should be bundled
    
    NSString *sdkScript = @"(function() {\n"
                          @"  if (window.AnyWP) {\n"
                          @"    console.log('[AnyWP SDK] Already initialized');\n"
                          @"    return;\n"
                          @"  }\n"
                          @"  window.AnyWP = {\n"
                          @"    version: '2.5.0',\n"
                          @"    platform: 'macos',\n"
                          @"    sendMessage: function(message) {\n"
                          @"      if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.anywpMessage) {\n"
                          @"        var msg = typeof message === 'string' ? message : JSON.stringify(message);\n"
                          @"        window.webkit.messageHandlers.anywpMessage.postMessage(msg);\n"
                          @"      }\n"
                          @"    },\n"
                          @"    ready: function(name) {\n"
                          @"      this.sendMessage({ type: 'ready', name: name });\n"
                          @"    },\n"
                          @"    log: function(message) {\n"
                          @"      console.log('[AnyWP]', message);\n"
                          @"      this.sendMessage({ type: 'log', message: message });\n"
                          @"    }\n"
                          @"  };\n"
                          @"  console.log('[AnyWP SDK] Fallback SDK initialized for macOS v2.2.0');\n"
                          @"})();";
    
    return sdkScript;
}

@end

