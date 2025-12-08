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
@property (nonatomic, strong) NSString *globalAllowedAccessPath;  // Global allowed access path for file loading

@end

@implementation WallpaperManager

// Synthesize the globalAllowedAccessPath property to generate the instance variable
@synthesize globalAllowedAccessPath = _globalAllowedAccessPath;

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
    // Use the global allowed access path if set, otherwise use default (nil triggers Library path usage)
    return [self initializeWallpaperOnMonitor:url 
                                 monitorIndex:monitorIndex 
                            allowedAccessPath:self.globalAllowedAccessPath];
}

- (BOOL)initializeWallpaperOnMonitor:(NSString *)url 
                        monitorIndex:(NSInteger)monitorIndex 
                   allowedAccessPath:(NSString *)allowedAccessPath {
    @try {
        [AWPLogger log:[NSString stringWithFormat:@"Initializing wallpaper on monitor %ld with URL: %@",
                       (long)monitorIndex, url]];
        if (allowedAccessPath) {
            [AWPLogger log:[NSString stringWithFormat:@"Custom allowed access path: %@", allowedAccessPath]];
        }
        
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
        
        // Configure window level for wallpaper display
        // The window should be just below desktop icons so it acts as a true wallpaper
        // 
        // Root cause of previous rendering issue was WebView frame using screen coordinates
        // instead of window-relative coordinates. Now that's fixed, we can use proper level.
        NSInteger desktopIconLevel = CGWindowLevelForKey(kCGDesktopIconWindowLevelKey);  // -2147483603
        NSInteger wallpaperLevel = desktopIconLevel - 1;  // Just below icons
        
        [window setLevel:wallpaperLevel];
        
        [AWPLogger log:[NSString stringWithFormat:@"Window level set to: %ld (icon level: %ld, wallpaper: below icons)",
                       (long)window.level, (long)desktopIconLevel]];
        
        [window setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces |
                                       NSWindowCollectionBehaviorStationary |
                                       NSWindowCollectionBehaviorIgnoresCycle];
        [window setOpaque:YES];  // Opaque window to show content
        [window setBackgroundColor:[NSColor blackColor]];  // Black background as base
        [window setHasShadow:NO];  // No shadow for wallpaper
        // Start in simple mode (mouse transparent)
        [window setIgnoresMouseEvents:YES];
        [window setAcceptsMouseMovedEvents:NO];
        [window setHidesOnDeactivate:NO];
        [window setReleasedWhenClosed:NO];
        
        [AWPLogger log:[NSString stringWithFormat:@"Window configured - Level: %ld (below desktop icons), mouse transparent: YES",
                       (long)window.level]];
        
        instance.window = window;
        
        // Create WebView with frame relative to window (not screen)
        // IMPORTANT: Use bounds (0, 0, width, height), not screen coordinates
        NSRect webViewFrame = NSMakeRect(0, 0, screenFrame.size.width, screenFrame.size.height);
        WKWebView *webView = [[WKWebView alloc] initWithFrame:webViewFrame
                                                 configuration:self.webViewConfig];
        webView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        webView.navigationDelegate = self;  // Set navigation delegate for load tracking
        
        // Configure WebView for proper rendering
        webView.wantsLayer = YES;
        
        [AWPLogger log:[NSString stringWithFormat:@"WebView frame: origin=(%f, %f) size=(%f x %f)",
                       webViewFrame.origin.x, webViewFrame.origin.y,
                       webViewFrame.size.width, webViewFrame.size.height]];
        
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
            // Determine allowed access path
            NSURL *accessURL = [self determineAllowedAccessURL:allowedAccessPath forFileURL:nsurl];
            [webView loadFileURL:nsurl allowingReadAccessToURL:accessURL];
            [AWPLogger log:[NSString stringWithFormat:@"Loading local file: %@ with read access to: %@",
                           nsurl.path, accessURL.path]];
            // Save allowed access path to instance
            instance.allowedAccessPath = allowedAccessPath;
        } else {
            // Assume file path
            nsurl = [NSURL fileURLWithPath:url];
            // Determine allowed access path
            NSURL *accessURL = [self determineAllowedAccessURL:allowedAccessPath forFileURL:nsurl];
            [webView loadFileURL:nsurl allowingReadAccessToURL:accessURL];
            [AWPLogger log:[NSString stringWithFormat:@"Loading local file: %@ with read access to: %@",
                           nsurl.path, accessURL.path]];
            // Save allowed access path to instance
            instance.allowedAccessPath = allowedAccessPath;
        }
        
        // Show window at wallpaper level (below desktop icons)
        // Use orderBack to ensure it stays behind other windows
        [window setAlphaValue:1.0];  // Ensure fully opaque window
        [window orderBack:nil];
        
        // Force display update
        [window display];
        [webView setNeedsDisplay:YES];
        
        [AWPLogger log:[NSString stringWithFormat:@"Window ordered to back (wallpaper level), alpha: %f, isVisible: %d",
                       window.alphaValue, window.isVisible]];
        
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
            // Use instance's allowed access path or determine from global/default
            NSURL *accessURL = [self determineAllowedAccessURL:instance.allowedAccessPath forFileURL:nsurl];
            [instance.webView loadFileURL:nsurl allowingReadAccessToURL:accessURL];
            [AWPLogger log:[NSString stringWithFormat:@"Navigating with read access to: %@", accessURL.path]];
        } else {
            // Assume file path
            nsurl = [NSURL fileURLWithPath:url];
            // Use instance's allowed access path or determine from global/default
            NSURL *accessURL = [self determineAllowedAccessURL:instance.allowedAccessPath forFileURL:nsurl];
            [instance.webView loadFileURL:nsurl allowingReadAccessToURL:accessURL];
            [AWPLogger log:[NSString stringWithFormat:@"Navigating with read access to: %@", accessURL.path]];
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
            // Interactive mode: Raise above icons and enable mouse
            [instance.window setLevel:desktopIconLevel + 1];  // Above desktop icons
            [instance.window setIgnoresMouseEvents:NO];
            [instance.window setAcceptsMouseMovedEvents:YES];
            [instance.window makeKeyAndOrderFront:nil];
            [AWPLogger log:[NSString stringWithFormat:@"Monitor %ld: Interactive mode (level: %ld, above icons, mouse enabled)", 
                           (long)monitorIndex, (long)(desktopIconLevel + 1)]];
        } else {
            // Simple mode: Below icons and mouse transparent (true wallpaper behavior)
            [instance.window setLevel:desktopIconLevel - 1];  // Below desktop icons
            [instance.window setIgnoresMouseEvents:YES];
            [instance.window setAcceptsMouseMovedEvents:NO];
            [instance.window orderBack:nil];
            [AWPLogger log:[NSString stringWithFormat:@"Monitor %ld: Simple mode (level: %ld, below icons, mouse disabled)", 
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

#pragma mark - Allowed Access Path Management

- (NSURL *)determineAllowedAccessURL:(NSString *)customPath forFileURL:(NSURL *)fileURL {
    // Priority: 1. Custom path specified for this call
    //           2. Global allowed access path
    //           3. Library directory (default, expanded access)
    
    if (customPath && customPath.length > 0) {
        // Use custom path specified in the call
        return [NSURL fileURLWithPath:customPath isDirectory:YES];
    }
    
    if (self.globalAllowedAccessPath && self.globalAllowedAccessPath.length > 0) {
        // Use global allowed access path
        return [NSURL fileURLWithPath:self.globalAllowedAccessPath isDirectory:YES];
    }
    
    // Default: Use Library directory for expanded access
    // This allows access to ~/Library and all subdirectories (Application Support, Caches, etc.)
    NSString *libraryPath = [WallpaperManager defaultLibraryPath];
    if (libraryPath) {
        [AWPLogger log:[NSString stringWithFormat:@"Using default Library path for access: %@", libraryPath]];
        return [NSURL fileURLWithPath:libraryPath isDirectory:YES];
    }
    
    // Fallback: Use the file's parent directory
    [AWPLogger warn:@"Could not determine Library path, falling back to file's parent directory"];
    return [fileURL URLByDeletingLastPathComponent];
}

- (void)setGlobalAllowedAccessPath:(NSString *)path {
    if (path && path.length > 0) {
        // Validate the path exists
        BOOL isDirectory = NO;
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDirectory];
        
        if (!exists) {
            [AWPLogger warn:[NSString stringWithFormat:@"Allowed access path does not exist: %@", path]];
        } else if (!isDirectory) {
            [AWPLogger warn:[NSString stringWithFormat:@"Allowed access path is not a directory: %@", path]];
        }
        
        // Use instance variable directly to avoid infinite recursion
        _globalAllowedAccessPath = path;
        [AWPLogger log:[NSString stringWithFormat:@"Global allowed access path set to: %@", path]];
    } else {
        // Use instance variable directly to avoid infinite recursion
        _globalAllowedAccessPath = nil;
        [AWPLogger log:@"Global allowed access path cleared (will use default Library path)"];
    }
}

- (NSString *)globalAllowedAccessPath {
    return _globalAllowedAccessPath;
}

+ (NSString *)defaultLibraryPath {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    return paths.firstObject;
}

+ (NSString *)applicationSupportPath {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    return paths.firstObject;
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

