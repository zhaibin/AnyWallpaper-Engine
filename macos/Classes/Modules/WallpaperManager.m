#import "WallpaperManager.h"
#import "../Utils/Logger.h"

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
    preferences.javaScriptEnabled = YES;
    self.webViewConfig.preferences = preferences;
    
    // Setup user content controller for message handling
    WKUserContentController *userContentController = [[WKUserContentController alloc] init];
    [userContentController addScriptMessageHandler:self.messageBridge name:@"anywpMessage"];
    self.webViewConfig.userContentController = userContentController;
    
    // Allow file access
    [self.webViewConfig.preferences setValue:@YES forKey:@"allowFileAccessFromFileURLs"];
    
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
        
        NSWindow *window = [[NSWindow alloc] initWithContentRect:screenFrame
                                                       styleMask:NSWindowStyleMaskBorderless
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO
                                                          screen:screen];
        
        // Configure window to be wallpaper-like
        [window setLevel:CGWindowLevelForKey(kCGDesktopWindowLevelKey)];  // Desktop level
        [window setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces |
                                       NSWindowCollectionBehaviorStationary |
                                       NSWindowCollectionBehaviorIgnoresCycle];
        [window setOpaque:YES];
        [window setBackgroundColor:[NSColor blackColor]];
        [window setIgnoresMouseEvents:YES];  // Mouse transparent by default (Simple Mode)
        [window setAcceptsMouseMovedEvents:NO];
        [window setHidesOnDeactivate:NO];
        [window setReleasedWhenClosed:NO];
        
        instance.window = window;
        
        // Create WebView
        WKWebView *webView = [[WKWebView alloc] initWithFrame:screenFrame
                                                 configuration:self.webViewConfig];
        webView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        
        // Set webView as window content
        [window.contentView addSubview:webView];
        
        instance.webView = webView;
        
        // Load URL
        NSURL *nsurl = [NSURL URLWithString:url];
        if (!nsurl) {
            // Try as file path
            nsurl = [NSURL fileURLWithPath:url];
        }
        
        NSURLRequest *request = [NSURLRequest requestWithURL:nsurl];
        [webView loadRequest:request];
        
        // Show window
        [window orderFront:nil];
        [window orderBack:nil];  // Send to back (below other windows)
        
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
        
        NSURL *nsurl = [NSURL URLWithString:url];
        if (!nsurl) {
            nsurl = [NSURL fileURLWithPath:url];
        }
        
        NSURLRequest *request = [NSURLRequest requestWithURL:nsurl];
        [instance.webView loadRequest:request];
        
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
    NSString *script = [NSString stringWithFormat:@"window.postMessage(%@, '*');", message];
    
    @synchronized (self.instances) {
        for (WallpaperInstance *instance in self.instances) {
            [instance.webView evaluateJavaScript:script completionHandler:^(id result, NSError *error) {
                if (error) {
                    [AWPLogger error:[NSString stringWithFormat:@"Failed to send message: %@",
                                    error.localizedDescription]];
                }
            }];
        }
    }
}

- (void)dealloc {
    [self stopAllWallpapers];
}

@end

