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
    // The SDK is platform-independent JavaScript
    // We can use the same SDK from windows/anywp_sdk.js
    
    // For now, return a minimal SDK placeholder
    // In production, this would load the actual SDK file
    
    NSString *sdkScript = @"(function() {\n"
                          @"  window.AnyWP = {\n"
                          @"    sendMessage: function(message) {\n"
                          @"      if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.anywpMessage) {\n"
                          @"        window.webkit.messageHandlers.anywpMessage.postMessage(message);\n"
                          @"      }\n"
                          @"    },\n"
                          @"    getMonitorInfo: function() {\n"
                          @"      return {\n"
                          @"        width: screen.width,\n"
                          @"        height: screen.height\n"
                          @"      };\n"
                          @"    }\n"
                          @"  };\n"
                          @"  console.log('[AnyWP SDK] Initialized for macOS');\n"
                          @"})();";
    
    return sdkScript;
}

@end

