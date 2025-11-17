#import "PowerManager.h"
#import "../Utils/Logger.h"
#import <mach/mach.h>

typedef NS_ENUM(NSInteger, AWPPowerState) {
    AWPPowerStateActive,
    AWPPowerStateIdle,
    AWPPowerStateScreenOff,
    AWPPowerStateLocked,
    AWPPowerStateFullscreenApp,
    AWPPowerStatePaused
};

@interface PowerManager ()

@property (nonatomic, assign) AWPPowerState currentState;
@property (nonatomic, assign) BOOL autoPowerSavingEnabled;
@property (nonatomic, assign) NSInteger idleTimeoutSeconds;
@property (nonatomic, assign) NSInteger memoryThresholdMB;
@property (nonatomic, assign) NSInteger cleanupIntervalMinutes;
@property (nonatomic, strong) NSMutableArray *pendingStateChanges;
@property (nonatomic, strong) NSTimer *monitoringTimer;

@end

@implementation PowerManager

- (instancetype)init {
    self = [super init];
    if (self) {
        _currentState = AWPPowerStateActive;
        _autoPowerSavingEnabled = YES;
        _idleTimeoutSeconds = 300;  // 5 minutes default
        _memoryThresholdMB = 200;   // 200MB default (WKWebView uses more than WebView2)
        _cleanupIntervalMinutes = 15;
        _pendingStateChanges = [NSMutableArray array];
        
        // Setup system notifications
        [self setupSystemNotifications];
        
        // Start monitoring timer
        [self startMonitoring];
        
        [AWPLogger log:@"PowerManager initialized"];
    }
    return self;
}

- (void)setupSystemNotifications {
    NSNotificationCenter *center = [[NSWorkspace sharedWorkspace] notificationCenter];
    
    // Screen sleep/wake notifications
    [center addObserver:self
               selector:@selector(handleScreenSleep:)
                   name:NSWorkspaceScreensDidSleepNotification
                 object:nil];
    
    [center addObserver:self
               selector:@selector(handleScreenWake:)
                   name:NSWorkspaceScreensDidWakeNotification
                 object:nil];
    
    // Session lock/unlock (requires screen recording permission)
    [center addObserver:self
               selector:@selector(handleSessionLock:)
                   name:NSWorkspaceSessionDidResignActiveNotification
                 object:nil];
    
    [center addObserver:self
               selector:@selector(handleSessionUnlock:)
                   name:NSWorkspaceSessionDidBecomeActiveNotification
                 object:nil];
    
    [AWPLogger log:@"System notifications setup"];
}

- (void)startMonitoring {
    // Check every 10 seconds for idle state, memory usage, etc.
    self.monitoringTimer = [NSTimer scheduledTimerWithTimeInterval:10.0
                                                           repeats:YES
                                                             block:^(NSTimer * _Nonnull timer) {
        [self checkSystemState];
    }];
    
    [AWPLogger log:@"Power monitoring started"];
}

- (void)checkSystemState {
    if (!self.autoPowerSavingEnabled) {
        return;
    }
    
    // Check idle time
    CFTimeInterval idleTime = CGEventSourceSecondsSinceLastEventType(kCGEventSourceStateHIDSystemState,
                                                                     kCGAnyInputEventType);
    
    if (idleTime > self.idleTimeoutSeconds && self.currentState == AWPPowerStateActive) {
        [self transitionToState:AWPPowerStateIdle];
    } else if (idleTime <= self.idleTimeoutSeconds && self.currentState == AWPPowerStateIdle) {
        [self transitionToState:AWPPowerStateActive];
    }
    
    // Check memory usage
    NSInteger memoryMB = [self getMemoryUsage];
    if (memoryMB > self.memoryThresholdMB) {
        [AWPLogger warn:[NSString stringWithFormat:@"Memory usage (%ld MB) exceeds threshold (%ld MB)",
                        (long)memoryMB, (long)self.memoryThresholdMB]];
        [self optimizeMemory];
    }
}

- (void)transitionToState:(AWPPowerState)newState {
    if (self.currentState == newState) {
        return;
    }
    
    NSString *oldStateStr = [self stateToString:self.currentState];
    NSString *newStateStr = [self stateToString:newState];
    
    [AWPLogger log:[NSString stringWithFormat:@"Power state transition: %@ -> %@",
                   oldStateStr, newStateStr]];
    
    // Add to pending changes
    @synchronized (self.pendingStateChanges) {
        [self.pendingStateChanges addObject:@{
            @"oldState": oldStateStr,
            @"newState": newStateStr
        }];
    }
    
    self.currentState = newState;
    
    // Trigger wallpaper pause/resume if needed
    // This would be handled by WallpaperManager, but we notify about the state change
}

- (NSString *)stateToString:(AWPPowerState)state {
    switch (state) {
        case AWPPowerStateActive:
            return @"ACTIVE";
        case AWPPowerStateIdle:
            return @"IDLE";
        case AWPPowerStateScreenOff:
            return @"SCREEN_OFF";
        case AWPPowerStateLocked:
            return @"LOCKED";
        case AWPPowerStateFullscreenApp:
            return @"FULLSCREEN_APP";
        case AWPPowerStatePaused:
            return @"PAUSED";
    }
}

#pragma mark - System Notification Handlers

- (void)handleScreenSleep:(NSNotification *)notification {
    [AWPLogger log:@"Screen sleep detected"];
    [self transitionToState:AWPPowerStateScreenOff];
}

- (void)handleScreenWake:(NSNotification *)notification {
    [AWPLogger log:@"Screen wake detected"];
    [self transitionToState:AWPPowerStateActive];
}

- (void)handleSessionLock:(NSNotification *)notification {
    [AWPLogger log:@"Session locked"];
    [self transitionToState:AWPPowerStateLocked];
}

- (void)handleSessionUnlock:(NSNotification *)notification {
    [AWPLogger log:@"Session unlocked"];
    [self transitionToState:AWPPowerStateActive];
}

#pragma mark - Public API

- (void)pauseWallpaper {
    [AWPLogger log:@"Manual wallpaper pause"];
    [self transitionToState:AWPPowerStatePaused];
}

- (void)resumeWallpaper {
    [AWPLogger log:@"Manual wallpaper resume"];
    [self transitionToState:AWPPowerStateActive];
}

- (void)setAutoPowerSaving:(BOOL)enabled {
    self.autoPowerSavingEnabled = enabled;
    [AWPLogger log:[NSString stringWithFormat:@"Auto power saving: %@", enabled ? @"enabled" : @"disabled"]];
}

- (NSString *)getPowerState {
    return [self stateToString:self.currentState];
}

- (NSInteger)getMemoryUsage {
    struct task_basic_info info;
    mach_msg_type_number_t size = TASK_BASIC_INFO_COUNT;
    kern_return_t kerr = task_info(mach_task_self(),
                                   TASK_BASIC_INFO,
                                   (task_info_t)&info,
                                   &size);
    
    if (kerr == KERN_SUCCESS) {
        return (NSInteger)(info.resident_size / 1024 / 1024);  // Convert to MB
    }
    
    return 0;
}

- (void)optimizeMemory {
    [AWPLogger log:@"Memory optimization triggered"];
    
    // Clear caches
    [[NSURLCache sharedURLCache] removeAllCachedResponses];
    
    // TODO: Trigger WebView garbage collection if possible
    
    [AWPLogger log:@"Memory optimization complete"];
}

- (NSArray *)getPendingPowerStateChanges {
    @synchronized (self.pendingStateChanges) {
        NSArray *changes = [self.pendingStateChanges copy];
        [self.pendingStateChanges removeAllObjects];
        return changes;
    }
}

- (void)setIdleTimeout:(NSInteger)seconds {
    if (seconds < 60) {
        seconds = 60;  // Minimum 60 seconds
    }
    self.idleTimeoutSeconds = seconds;
    [AWPLogger log:[NSString stringWithFormat:@"Idle timeout set to %ld seconds", (long)seconds]];
}

- (void)setMemoryThreshold:(NSInteger)thresholdMB {
    if (thresholdMB < 100) {
        thresholdMB = 100;  // Minimum 100MB
    }
    self.memoryThresholdMB = thresholdMB;
    [AWPLogger log:[NSString stringWithFormat:@"Memory threshold set to %ld MB", (long)thresholdMB]];
}

- (void)setCleanupInterval:(NSInteger)minutes {
    if (minutes < 10) {
        minutes = 10;  // Minimum 10 minutes
    }
    self.cleanupIntervalMinutes = minutes;
    [AWPLogger log:[NSString stringWithFormat:@"Cleanup interval set to %ld minutes", (long)minutes]];
}

- (NSDictionary *)getConfiguration {
    return @{
        @"idleTimeoutSeconds": @(self.idleTimeoutSeconds),
        @"memoryThresholdMB": @(self.memoryThresholdMB),
        @"cleanupIntervalMinutes": @(self.cleanupIntervalMinutes),
        @"autoPowerSavingEnabled": @(self.autoPowerSavingEnabled)
    };
}

- (void)dealloc {
    [self.monitoringTimer invalidate];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end

