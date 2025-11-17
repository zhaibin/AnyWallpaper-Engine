#import "MessageBridge.h"
#import "../Utils/Logger.h"

@interface MessageBridge ()

@property (nonatomic, strong) FlutterMethodChannel *channel;
@property (nonatomic, strong) NSMutableArray *pendingMessages;

@end

@implementation MessageBridge

- (instancetype)initWithChannel:(FlutterMethodChannel *)channel {
    self = [super init];
    if (self) {
        _channel = channel;
        _pendingMessages = [NSMutableArray array];
        [AWPLogger log:@"MessageBridge initialized"];
    }
    return self;
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
    // This needs to be connected to WallpaperManager to actually send to WebViews
    // For now, just log
    [AWPLogger log:[NSString stringWithFormat:@"Sending message to monitor %ld: %@",
                   (long)monitorIndex, message]];
    
    // TODO: Get wallpaper instances from WallpaperManager and send message
    // This will be implemented once we wire up the modules properly
    
    return YES;
}

- (NSArray *)getPendingMessages {
    @synchronized (self.pendingMessages) {
        NSArray *messages = [self.pendingMessages copy];
        [self.pendingMessages removeAllObjects];
        return messages;
    }
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
    // Load the unified TypeScript SDK (compiled from sdk/src/)
    // The SDK is platform-independent and automatically detects macOS
    // v2.2.0: SDK moved to top-level sdk/ directory
    
    // Try to load from bundle resources
    NSBundle *bundle = [NSBundle bundleForClass:[self class]];
    NSString *sdkPath = [bundle pathForResource:@"anywp_sdk" ofType:@"js"];
    
    if (sdkPath) {
        NSError *error = nil;
        NSString *sdkScript = [NSString stringWithContentsOfFile:sdkPath
                                                        encoding:NSUTF8StringEncoding
                                                           error:&error];
        if (error) {
            [AWPLogger error:[NSString stringWithFormat:@"Failed to load SDK from bundle: %@",
                            error.localizedDescription]];
            return [self fallbackSDKScript];
        }
        
        [AWPLogger log:@"Loaded unified TypeScript SDK from bundle (sdk/dist/)"];
        return sdkScript;
    }
    
    // If not found in bundle, return fallback minimal SDK
    [AWPLogger warn:@"SDK file not found in bundle, using fallback"];
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
                          @"    version: '2.2.0',\n"
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

