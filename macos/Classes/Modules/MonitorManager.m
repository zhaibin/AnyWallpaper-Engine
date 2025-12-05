#import "MonitorManager.h"
#import "../Utils/Logger.h"

@interface MonitorManager ()
@property (nonatomic, copy) void (^displayChangeCallback)(void);
@property (nonatomic, strong) id displayChangeObserver;
@end

@implementation MonitorManager

- (instancetype)init {
    self = [super init];
    if (self) {
        [AWPLogger log:@"MonitorManager initialized"];
    }
    return self;
}

- (NSArray<NSDictionary *> *)getMonitors {
    NSArray<NSScreen *> *screens = [NSScreen screens];
    NSMutableArray<NSDictionary *> *monitors = [NSMutableArray array];
    
    NSScreen *mainScreen = [NSScreen mainScreen];
    
    for (NSInteger i = 0; i < screens.count; i++) {
        NSScreen *screen = screens[i];
        NSRect frame = screen.frame;
        CGFloat scaleFactor = screen.backingScaleFactor;  // Retina scale
        
        // Get device description
        NSDictionary *deviceDescription = screen.deviceDescription;
        NSString *deviceName = [deviceDescription objectForKey:@"NSScreenNumber"] ?
                              [NSString stringWithFormat:@"Display_%@",
                               [deviceDescription objectForKey:@"NSScreenNumber"]] :
                              [NSString stringWithFormat:@"Display_%ld", (long)i];
        
        // Calculate physical dimensions
        CGSize physicalSize = screen.frame.size;
        CGFloat dpi = 72.0 * scaleFactor;  // macOS base DPI is 72
        
        NSDictionary *monitorInfo = @{
            @"index": @(i),
            @"deviceName": deviceName,
            @"left": @((int)frame.origin.x),
            @"top": @((int)frame.origin.y),
            @"width": @((int)frame.size.width),
            @"height": @((int)frame.size.height),
            @"isPrimary": @(screen == mainScreen),
            @"scaleFactor": @(scaleFactor),  // 1.0 for standard, 2.0 for Retina
            @"dpi": @((int)dpi),
            @"physicalWidth": @((int)(physicalSize.width * scaleFactor)),
            @"physicalHeight": @((int)(physicalSize.height * scaleFactor))
        };
        
        [monitors addObject:monitorInfo];
    }
    
    [AWPLogger log:[NSString stringWithFormat:@"Found %ld monitors", (long)monitors.count]];
    return monitors;
}

- (NSInteger)getMonitorCount {
    return [[NSScreen screens] count];
}

- (NSScreen *)getMonitorAtIndex:(NSInteger)index {
    NSArray<NSScreen *> *screens = [NSScreen screens];
    
    if (index < 0 || index >= screens.count) {
        [AWPLogger error:[NSString stringWithFormat:@"Monitor index %ld out of bounds", (long)index]];
        return nil;
    }
    
    return screens[index];
}

- (NSScreen *)getPrimaryMonitor {
    return [NSScreen mainScreen];
}

- (void)setupDisplayChangeNotifications:(void (^)(void))callback {
    self.displayChangeCallback = callback;
    
    // Register for screen configuration change notifications
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    
    self.displayChangeObserver = [center addObserverForName:NSApplicationDidChangeScreenParametersNotification
                                                      object:nil
                                                       queue:[NSOperationQueue mainQueue]
                                                  usingBlock:^(NSNotification * _Nonnull note) {
        [AWPLogger log:@"Display configuration changed"];
        if (self.displayChangeCallback) {
            self.displayChangeCallback();
        }
    }];
    
    [AWPLogger log:@"Display change notifications setup"];
}

- (void)removeDisplayChangeNotifications {
    if (self.displayChangeObserver) {
        [[NSNotificationCenter defaultCenter] removeObserver:self.displayChangeObserver];
        self.displayChangeObserver = nil;
        [AWPLogger log:@"Display change notifications removed"];
    }
}

- (void)dealloc {
    [self removeDisplayChangeNotifications];
}

@end

