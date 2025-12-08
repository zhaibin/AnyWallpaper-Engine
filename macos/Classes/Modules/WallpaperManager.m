#import "WallpaperManager.h"
#import "../Utils/Logger.h"
#import "../Utils/AWPCustomSchemeHandler.h"

@implementation WallpaperInstance
@end

@interface WallpaperManager ()

@property (nonatomic, strong) MonitorManager *monitorManager;
@property (nonatomic, strong) MessageBridge *messageBridge;
@property (nonatomic, strong) NSMutableArray<WallpaperInstance *> *instances;
@property (nonatomic, strong) WKWebViewConfiguration *webViewConfig;

@end

@implementation WallpaperManager

- (instancetype)initWithMonitorManager:(MonitorManager *)monitorManager
                        messageBridge:(MessageBridge *)messageBridge {
    self = [super init];
    if (self) {
        _monitorManager = monitorManager;
        _messageBridge = messageBridge;
        _instances = [NSMutableArray array];
        
        // Setup WebView configuration
        [self setupWebViewConfiguration];
        
        [AWPLogger log:@"WallpaperManager initialized"];
    }
    return self;
}

- (void)setupWebViewConfiguration {
    self.webViewConfig = [[WKWebViewConfiguration alloc] init];
    
    // Enable JavaScript
    WKPreferences *preferences = [[WKPreferences alloc] init];
    [preferences setValue:@YES forKey:@"javaScriptEnabled"];
    [preferences setValue:@YES forKey:@"javaScriptCanOpenWindowsAutomatically"];
    
    // Enable Web Inspector in debug mode
    #ifdef DEBUG
    [preferences setValue:@YES forKey:@"developerExtrasEnabled"];
    [AWPLogger log:@"Web Inspector enabled (Debug mode)"];
    #endif
    
    self.webViewConfig.preferences = preferences;
    
    // Setup user content controller for message handling
    WKUserContentController *userContentController = [[WKUserContentController alloc] init];
    [userContentController addScriptMessageHandler:self.messageBridge name:@"anywpMessage"];
    self.webViewConfig.userContentController = userContentController;
    
    // Inject AnyWP SDK at document start
    [self.messageBridge injectSDKIntoConfiguration:self.webViewConfig];
    
    // Allow local file access (important for loading local HTML files)
    [self.webViewConfig.preferences setValue:@YES forKey:@"allowFileAccessFromFileURLs"];
    [self.webViewConfig setValue:@YES forKey:@"allowUniversalAccessFromFileURLs"];
    
    // Register custom URL scheme handler for anywp:// protocol
    AWPCustomSchemeHandler *schemeHandler = [[AWPCustomSchemeHandler alloc] init];
    [self.webViewConfig setURLSchemeHandler:schemeHandler forURLScheme:@"anywp"];
    [AWPLogger log:@"Custom URL scheme handler registered for anywp://"];
    
    [AWPLogger log:@"WebView configuration setup complete"];
}

- (BOOL)initializeWallpaper:(NSString *)url {
    return [self initializeWallpaperOnMonitor:url monitorIndex:0];
}

- (BOOL)stopWallpaper {
    return [self stopWallpaperOnMonitor:0];
}

- (BOOL)navigateToUrl:(NSString *)url {
    return [self navigateToUrlOnMonitor:url monitorIndex:0];
}

- (BOOL)initializeWallpaperOnMonitor:(NSString *)url monitorIndex:(NSInteger)monitorIndex {
    @try {
        [AWPLogger log:[NSString stringWithFormat:@"Initializing wallpaper on monitor %ld with URL: %@",
                       (long)monitorIndex, url]];
        
        // Get monitor
        NSScreen *screen = [self.monitorManager getMonitorAtIndex:monitorIndex];
        if (!screen) {
            [AWPLogger error:[NSString stringWithFormat:@"Monitor %ld not found", (long)monitorIndex]];
            return NO;
        }
        
        // Check if wallpaper already exists for this monitor
        WallpaperInstance *existing = [self getInstanceForMonitor:monitorIndex];
        if (existing) {
            [AWPLogger warn:[NSString stringWithFormat:@"Wallpaper already exists on monitor %ld, stopping it first",
                           (long)monitorIndex]];
            [self stopWallpaperOnMonitor:monitorIndex];
        }
        
        // Create wallpaper instance
        WallpaperInstance *instance = [[WallpaperInstance alloc] init];
        instance.monitorIndex = monitorIndex;
        instance.currentURL = url;
        
        // Create window for this monitor
        NSRect screenFrame = screen.frame;
        
        [AWPLogger log:[NSString stringWithFormat:@"Screen frame: origin=(%f, %f) size=(%f x %f)",
                       screenFrame.origin.x, screenFrame.origin.y,
                       screenFrame.size.width, screenFrame.size.height]];
        
        NSWindow *window = [[NSWindow alloc] initWithContentRect:screenFrame
                                                       styleMask:NSWindowStyleMaskBorderless
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO
                                                          screen:screen];
        
        // Explicitly set frame origin to ensure correct positioning on HiDPI displays
        // This fixes an issue where secondary monitors get positioned at 2x the expected location
        [window setFrameOrigin:screenFrame.origin];
        
        [AWPLogger log:[NSString stringWithFormat:@"Window frame after creation: origin=(%f, %f) size=(%f x %f)",
                       window.frame.origin.x, window.frame.origin.y,
                       window.frame.size.width, window.frame.size.height]];
        
        // Configure window to be wallpaper-like (below desktop icons)
        // Use a window level that's below the desktop but visible
        // CGWindowLevelForKey(kCGDesktopIconWindowLevelKey) is the desktop icons level
        // We need to be below that
        NSInteger desktopIconLevel = CGWindowLevelForKey(kCGDesktopIconWindowLevelKey);
        [window setLevel:desktopIconLevel - 1];  // Below desktop icons
        
        [window setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces |
                                       NSWindowCollectionBehaviorStationary |
                                       NSWindowCollectionBehaviorIgnoresCycle];
        [window setOpaque:YES];
        [window setBackgroundColor:[NSColor blackColor]];
        // Start in simple mode (mouse transparent)
        [window setIgnoresMouseEvents:YES];
        [window setAcceptsMouseMovedEvents:NO];
        [window setHidesOnDeactivate:NO];
        [window setReleasedWhenClosed:NO];
        
        [AWPLogger log:[NSString stringWithFormat:@"Window created with level: %ld (desktop icons level: %ld), mouse transparent: YES",
                       (long)window.level, (long)desktopIconLevel]];
        
        instance.window = window;
        
        // Create WebView
        WKWebView *webView = [[WKWebView alloc] initWithFrame:screenFrame
                                                 configuration:self.webViewConfig];
        webView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        webView.navigationDelegate = self;  // Set navigation delegate for load tracking
        
        // Set webView as window content
        [window.contentView addSubview:webView];
        
        instance.webView = webView;
        
        // Load URL
        NSURL *nsurl = nil;
        if ([url hasPrefix:@"http://"] || [url hasPrefix:@"https://"]) {
            // HTTP/HTTPS URL
            nsurl = [NSURL URLWithString:url];
            NSURLRequest *request = [NSURLRequest requestWithURL:nsurl];
            [webView loadRequest:request];
        } else if ([url hasPrefix:@"file://"]) {
            // File URL
            NSString *filePath = [url substringFromIndex:7];  // Remove "file://"
            nsurl = [NSURL fileURLWithPath:filePath];
            // Load file URL with read access to directory
            NSURL *directoryURL = [nsurl URLByDeletingLastPathComponent];
            [webView loadFileURL:nsurl allowingReadAccessToURL:directoryURL];
            [AWPLogger log:[NSString stringWithFormat:@"Loading local file: %@ with read access to: %@",
                           nsurl.path, directoryURL.path]];
        } else {
            // Assume file path
            nsurl = [NSURL fileURLWithPath:url];
            NSURL *directoryURL = [nsurl URLByDeletingLastPathComponent];
            [webView loadFileURL:nsurl allowingReadAccessToURL:directoryURL];
            [AWPLogger log:[NSString stringWithFormat:@"Loading local file: %@ with read access to: %@",
                           nsurl.path, directoryURL.path]];
        }
        
        // Show window
        [window makeKeyAndOrderFront:nil];
        [AWPLogger log:@"Window displayed"];
        
        // Add instance to array
        @synchronized (self.instances) {
            [self.instances addObject:instance];
        }
        
        [AWPLogger log:[NSString stringWithFormat:@"Wallpaper initialized on monitor %ld",
                       (long)monitorIndex]];
        return YES;
    }
    @catch (NSException *exception) {
        [AWPLogger error:[NSString stringWithFormat:@"Failed to initialize wallpaper: %@",
                        exception.reason]];
        return NO;
    }
}

- (BOOL)stopWallpaperOnMonitor:(NSInteger)monitorIndex {
    @try {
        WallpaperInstance *instance = [self getInstanceForMonitor:monitorIndex];
        if (!instance) {
            [AWPLogger warn:[NSString stringWithFormat:@"No wallpaper on monitor %ld to stop",
                           (long)monitorIndex]];
            return NO;
        }
        
        // Close window
        [instance.window close];
        instance.window = nil;
        instance.webView = nil;
        
        // Remove from instances
        @synchronized (self.instances) {
            [self.instances removeObject:instance];
        }
        
        [AWPLogger log:[NSString stringWithFormat:@"Wallpaper stopped on monitor %ld",
                       (long)monitorIndex]];
        return YES;
    }
    @catch (NSException *exception) {
        [AWPLogger error:[NSString stringWithFormat:@"Failed to stop wallpaper: %@",
                        exception.reason]];
        return NO;
    }
}

- (BOOL)navigateToUrlOnMonitor:(NSString *)url monitorIndex:(NSInteger)monitorIndex {
    @try {
        WallpaperInstance *instance = [self getInstanceForMonitor:monitorIndex];
        if (!instance) {
            [AWPLogger error:[NSString stringWithFormat:@"No wallpaper on monitor %ld",
                           (long)monitorIndex]];
            return NO;
        }
        
        NSURL *nsurl = nil;
        if ([url hasPrefix:@"http://"] || [url hasPrefix:@"https://"]) {
            // HTTP/HTTPS URL
            nsurl = [NSURL URLWithString:url];
            NSURLRequest *request = [NSURLRequest requestWithURL:nsurl];
            [instance.webView loadRequest:request];
        } else if ([url hasPrefix:@"file://"]) {
            // File URL
            NSString *filePath = [url substringFromIndex:7];  // Remove "file://"
            nsurl = [NSURL fileURLWithPath:filePath];
            NSURL *directoryURL = [nsurl URLByDeletingLastPathComponent];
            [instance.webView loadFileURL:nsurl allowingReadAccessToURL:directoryURL];
        } else {
            // Assume file path
            nsurl = [NSURL fileURLWithPath:url];
            NSURL *directoryURL = [nsurl URLByDeletingLastPathComponent];
            [instance.webView loadFileURL:nsurl allowingReadAccessToURL:directoryURL];
        }
        
        instance.currentURL = url;
        
        [AWPLogger log:[NSString stringWithFormat:@"Navigated to URL on monitor %ld: %@",
                       (long)monitorIndex, url]];
        return YES;
    }
    @catch (NSException *exception) {
        [AWPLogger error:[NSString stringWithFormat:@"Failed to navigate: %@",
                        exception.reason]];
        return NO;
    }
}

- (void)stopAllWallpapers {
    @synchronized (self.instances) {
        NSArray *instancesCopy = [self.instances copy];
        for (WallpaperInstance *instance in instancesCopy) {
            [self stopWallpaperOnMonitor:instance.monitorIndex];
        }
    }
    [AWPLogger log:@"All wallpapers stopped"];
}

- (WallpaperInstance *)getInstanceForMonitor:(NSInteger)monitorIndex {
    @synchronized (self.instances) {
        for (WallpaperInstance *instance in self.instances) {
            if (instance.monitorIndex == monitorIndex) {
                return instance;
            }
        }
    }
    return nil;
}

- (NSArray<WallpaperInstance *> *)getAllInstances {
    @synchronized (self.instances) {
        return [self.instances copy];
    }
}

- (BOOL)setInteractiveMode:(BOOL)interactive forMonitor:(NSInteger)monitorIndex {
    @try {
        WallpaperInstance *instance = [self getInstanceForMonitor:monitorIndex];
        if (!instance || !instance.window) {
            [AWPLogger error:[NSString stringWithFormat:@"No wallpaper on monitor %ld", (long)monitorIndex]];
            return NO;
        }
        
        NSInteger desktopIconLevel = CGWindowLevelForKey(kCGDesktopIconWindowLevelKey);
        
        if (interactive) {
            // Interactive mode: 
            // 1. Raise window level ABOVE desktop icons to capture mouse events
            // 2. Enable mouse event capture
            [instance.window setLevel:desktopIconLevel + 1];  // Above desktop icons
            [instance.window setIgnoresMouseEvents:NO];
            [instance.window setAcceptsMouseMovedEvents:YES];
            // Make window key to receive events
            [instance.window makeKeyAndOrderFront:nil];
            [AWPLogger log:[NSString stringWithFormat:@"Monitor %ld: Interactive mode enabled (level: %ld)", 
                           (long)monitorIndex, (long)(desktopIconLevel + 1)]];
        } else {
            // Simple mode: 
            // 1. Lower window level BELOW desktop icons (wallpaper-like)
            // 2. Make mouse transparent
            [instance.window setLevel:desktopIconLevel - 1];  // Below desktop icons
            [instance.window setIgnoresMouseEvents:YES];
            [instance.window setAcceptsMouseMovedEvents:NO];
            // Order back to avoid covering desktop
            [instance.window orderBack:nil];
            [AWPLogger log:[NSString stringWithFormat:@"Monitor %ld: Simple mode enabled (level: %ld)", 
                           (long)monitorIndex, (long)(desktopIconLevel - 1)]];
        }
        
        return YES;
    }
    @catch (NSException *exception) {
        [AWPLogger error:[NSString stringWithFormat:@"Failed to set interactive mode: %@", exception.reason]];
        return NO;
    }
}

- (void)pauseAllWallpapers {
    @synchronized (self.instances) {
        for (WallpaperInstance *instance in self.instances) {
            if (!instance.isPaused) {
                [instance.window orderOut:nil];  // Hide window
                instance.isPaused = YES;
            }
        }
    }
    [AWPLogger log:@"All wallpapers paused"];
}

- (void)resumeAllWallpapers {
    @synchronized (self.instances) {
        for (WallpaperInstance *instance in self.instances) {
            if (instance.isPaused) {
                [instance.window orderFront:nil];
                [instance.window orderBack:nil];  // Send to back
                instance.isPaused = NO;
            }
        }
    }
    [AWPLogger log:@"All wallpapers resumed"];
}

- (void)sendMessageToAll:(NSString *)message {
    @synchronized (self.instances) {
        for (WallpaperInstance *instance in self.instances) {
            if (instance.webView) {
                // Use MessageBridge to properly dispatch CustomEvent
                [self.messageBridge sendMessageToWebView:instance.webView message:message];
            }
        }
    }
}

- (void)dealloc {
    [self stopAllWallpapers];
}

#pragma mark - WKNavigationDelegate

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation {
    [AWPLogger log:@"🔄 WebView started loading"];
}

- (void)webView:(WKWebView *)webView didCommitNavigation:(WKNavigation *)navigation {
    [AWPLogger log:@"✅ WebView committed navigation"];
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation {
    [AWPLogger log:@"✅ WebView finished loading successfully"];
    
    // Get page title for verification
    [webView evaluateJavaScript:@"document.title" completionHandler:^(id result, NSError *error) {
        if (!error && result) {
            [AWPLogger log:[NSString stringWithFormat:@"   Page title: %@", result]];
        }
    }];
    
    // Send initial interactive mode state (default: Simple Mode = false)
    // Find which monitor this webView belongs to
    @synchronized (self.instances) {
        for (WallpaperInstance *instance in self.instances) {
            if (instance.webView == webView) {
                // Send initial state to WebView via MessageBridge
                NSString *initMessage = [NSString stringWithFormat:
                    @"{\"type\":\"interactiveMode\",\"data\":{\"interactive\":false,\"monitorIndex\":%ld}}",
                    (long)instance.monitorIndex];
                
                [self.messageBridge sendMessage:initMessage toMonitorIndex:instance.monitorIndex];
                [AWPLogger log:[NSString stringWithFormat:@"Sent initial interactive mode state (Simple Mode) to monitor %ld",
                               (long)instance.monitorIndex]];
                break;
            }
        }
    }
}

- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation withError:(NSError *)error {
    [AWPLogger error:[NSString stringWithFormat:@"❌ WebView navigation failed: %@ (Code: %ld)",
                     error.localizedDescription, (long)error.code]];
    [AWPLogger error:[NSString stringWithFormat:@"   Error domain: %@", error.domain]];
    if (error.userInfo) {
        [AWPLogger error:[NSString stringWithFormat:@"   User info: %@", error.userInfo]];
    }
}

- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(WKNavigation *)navigation withError:(NSError *)error {
    [AWPLogger error:[NSString stringWithFormat:@"❌ WebView provisional navigation failed: %@ (Code: %ld)",
                     error.localizedDescription, (long)error.code]];
    [AWPLogger error:[NSString stringWithFormat:@"   Error domain: %@", error.domain]];
    if (error.userInfo) {
        [AWPLogger error:[NSString stringWithFormat:@"   User info: %@", error.userInfo]];
    }
}

- (void)webViewWebContentProcessDidTerminate:(WKWebView *)webView {
    [AWPLogger error:@"❌ WebView content process terminated"];
}

@end

