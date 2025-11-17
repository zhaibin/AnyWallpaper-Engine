#import "AnyWPEnginePlugin.h"
#import "Modules/WallpaperManager.h"
#import "Modules/MonitorManager.h"
#import "Modules/PowerManager.h"
#import "Modules/MessageBridge.h"
#import "Utils/Logger.h"
#import "Utils/StatePersistence.h"

@interface AnyWPEnginePlugin ()

@property (nonatomic, strong) WallpaperManager *wallpaperManager;
@property (nonatomic, strong) MonitorManager *monitorManager;
@property (nonatomic, strong) PowerManager *powerManager;
@property (nonatomic, strong) MessageBridge *messageBridge;
@property (nonatomic, strong) StatePersistence *statePersistence;
@property (nonatomic, strong) FlutterMethodChannel *channel;

@end

@implementation AnyWPEnginePlugin

+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
    FlutterMethodChannel* channel = [FlutterMethodChannel
        methodChannelWithName:@"anywp_engine"
              binaryMessenger:[registrar messenger]];
    AnyWPEnginePlugin* instance = [[AnyWPEnginePlugin alloc] initWithChannel:channel];
    [registrar addMethodCallDelegate:instance channel:channel];
    
    [AWPLogger log:@"AnyWP Engine Plugin registered for macOS"];
}

- (instancetype)initWithChannel:(FlutterMethodChannel *)channel {
    self = [super init];
    if (self) {
        _channel = channel;
        
        // Initialize modules
        _monitorManager = [[MonitorManager alloc] init];
        _statePersistence = [[StatePersistence alloc] init];
        _messageBridge = [[MessageBridge alloc] initWithChannel:channel];
        _powerManager = [[PowerManager alloc] init];
        _wallpaperManager = [[WallpaperManager alloc] initWithMonitorManager:_monitorManager
                                                              messageBridge:_messageBridge];
        
        [AWPLogger log:@"AnyWP Engine Plugin initialized"];
    }
    return self;
}

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSString *method = call.method;
    
    @try {
        // ========== Version Info ==========
        if ([method isEqualToString:@"getVersion"]) {
            result(@"2.2.0");
        }
        else if ([method isEqualToString:@"getSDKVersion"]) {
            result(@"2.2.0");
        }
        
        // ========== Wallpaper Initialization ==========
        else if ([method isEqualToString:@"initializeWallpaper"]) {
            [self handleInitializeWallpaper:call result:result];
        }
        else if ([method isEqualToString:@"stopWallpaper"]) {
            [self handleStopWallpaper:call result:result];
        }
        else if ([method isEqualToString:@"navigateToUrl"]) {
            [self handleNavigateToUrl:call result:result];
        }
        
        // ========== Multi-Monitor Support ==========
        else if ([method isEqualToString:@"getMonitors"]) {
            [self handleGetMonitors:call result:result];
        }
        else if ([method isEqualToString:@"initializeWallpaperOnMonitor"]) {
            [self handleInitializeWallpaperOnMonitor:call result:result];
        }
        else if ([method isEqualToString:@"stopWallpaperOnMonitor"]) {
            [self handleStopWallpaperOnMonitor:call result:result];
        }
        else if ([method isEqualToString:@"navigateToUrlOnMonitor"]) {
            [self handleNavigateToUrlOnMonitor:call result:result];
        }
        
        // ========== Power Management ==========
        else if ([method isEqualToString:@"pauseWallpaper"]) {
            [self handlePauseWallpaper:call result:result];
        }
        else if ([method isEqualToString:@"resumeWallpaper"]) {
            [self handleResumeWallpaper:call result:result];
        }
        else if ([method isEqualToString:@"setAutoPowerSaving"]) {
            [self handleSetAutoPowerSaving:call result:result];
        }
        else if ([method isEqualToString:@"getPowerState"]) {
            [self handleGetPowerState:call result:result];
        }
        else if ([method isEqualToString:@"getMemoryUsage"]) {
            [self handleGetMemoryUsage:call result:result];
        }
        else if ([method isEqualToString:@"optimizeMemory"]) {
            [self handleOptimizeMemory:call result:result];
        }
        else if ([method isEqualToString:@"getPendingPowerStateChanges"]) {
            [self handleGetPendingPowerStateChanges:call result:result];
        }
        
        // ========== Configuration ==========
        else if ([method isEqualToString:@"setIdleTimeout"]) {
            [self handleSetIdleTimeout:call result:result];
        }
        else if ([method isEqualToString:@"setMemoryThreshold"]) {
            [self handleSetMemoryThreshold:call result:result];
        }
        else if ([method isEqualToString:@"setCleanupInterval"]) {
            [self handleSetCleanupInterval:call result:result];
        }
        else if ([method isEqualToString:@"getConfiguration"]) {
            [self handleGetConfiguration:call result:result];
        }
        
        // ========== State Persistence ==========
        else if ([method isEqualToString:@"saveState"]) {
            [self handleSaveState:call result:result];
        }
        else if ([method isEqualToString:@"loadState"]) {
            [self handleLoadState:call result:result];
        }
        else if ([method isEqualToString:@"clearState"]) {
            [self handleClearState:call result:result];
        }
        else if ([method isEqualToString:@"setApplicationName"]) {
            [self handleSetApplicationName:call result:result];
        }
        else if ([method isEqualToString:@"getStoragePath"]) {
            [self handleGetStoragePath:call result:result];
        }
        
        // ========== Bidirectional Communication ==========
        else if ([method isEqualToString:@"sendMessage"]) {
            [self handleSendMessage:call result:result];
        }
        else if ([method isEqualToString:@"getPendingMessages"]) {
            [self handleGetPendingMessages:call result:result];
        }
        
        // ========== File Encryption (Custom Scheme) ==========
        else if ([method isEqualToString:@"encryptFile"]) {
            [self handleEncryptFile:call result:result];
        }
        else if ([method isEqualToString:@"decryptFile"]) {
            [self handleDecryptFile:call result:result];
        }
        
        // ========== Unknown Method ==========
        else {
            result(FlutterMethodNotImplemented);
        }
    }
    @catch (NSException *exception) {
        [AWPLogger error:[NSString stringWithFormat:@"Exception in %@: %@", method, exception.reason]];
        result([FlutterError errorWithCode:@"EXCEPTION"
                                   message:exception.reason
                                   details:nil]);
    }
}

#pragma mark - Wallpaper Initialization

- (void)handleInitializeWallpaper:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *url = args[@"url"];
    
    BOOL success = [self.wallpaperManager initializeWallpaper:url];
    result(@(success));
}

- (void)handleStopWallpaper:(FlutterMethodCall*)call result:(FlutterResult)result {
    BOOL success = [self.wallpaperManager stopWallpaper];
    result(@(success));
}

- (void)handleNavigateToUrl:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *url = args[@"url"];
    
    BOOL success = [self.wallpaperManager navigateToUrl:url];
    result(@(success));
}

#pragma mark - Multi-Monitor Support

- (void)handleGetMonitors:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSArray *monitors = [self.monitorManager getMonitors];
    result(monitors);
}

- (void)handleInitializeWallpaperOnMonitor:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *url = args[@"url"];
    NSNumber *monitorIndex = args[@"monitorIndex"];
    
    BOOL success = [self.wallpaperManager initializeWallpaperOnMonitor:url
                                                          monitorIndex:[monitorIndex intValue]];
    result(@(success));
}

- (void)handleStopWallpaperOnMonitor:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSNumber *monitorIndex = args[@"monitorIndex"];
    
    BOOL success = [self.wallpaperManager stopWallpaperOnMonitor:[monitorIndex intValue]];
    result(@(success));
}

- (void)handleNavigateToUrlOnMonitor:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *url = args[@"url"];
    NSNumber *monitorIndex = args[@"monitorIndex"];
    
    BOOL success = [self.wallpaperManager navigateToUrlOnMonitor:url
                                                    monitorIndex:[monitorIndex intValue]];
    result(@(success));
}

#pragma mark - Power Management

- (void)handlePauseWallpaper:(FlutterMethodCall*)call result:(FlutterResult)result {
    [self.powerManager pauseWallpaper];
    result(@YES);
}

- (void)handleResumeWallpaper:(FlutterMethodCall*)call result:(FlutterResult)result {
    [self.powerManager resumeWallpaper];
    result(@YES);
}

- (void)handleSetAutoPowerSaving:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSNumber *enabled = args[@"enabled"];
    
    [self.powerManager setAutoPowerSaving:[enabled boolValue]];
    result(@YES);
}

- (void)handleGetPowerState:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSString *state = [self.powerManager getPowerState];
    result(state);
}

- (void)handleGetMemoryUsage:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSInteger memoryMB = [self.powerManager getMemoryUsage];
    result(@(memoryMB));
}

- (void)handleOptimizeMemory:(FlutterMethodCall*)call result:(FlutterResult)result {
    [self.powerManager optimizeMemory];
    result(@YES);
}

- (void)handleGetPendingPowerStateChanges:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSArray *changes = [self.powerManager getPendingPowerStateChanges];
    result(changes);
}

#pragma mark - Configuration

- (void)handleSetIdleTimeout:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSNumber *seconds = args[@"seconds"];
    
    [self.powerManager setIdleTimeout:[seconds intValue]];
    result(@YES);
}

- (void)handleSetMemoryThreshold:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSNumber *thresholdMB = args[@"thresholdMB"];
    
    [self.powerManager setMemoryThreshold:[thresholdMB intValue]];
    result(@YES);
}

- (void)handleSetCleanupInterval:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSNumber *minutes = args[@"minutes"];
    
    [self.powerManager setCleanupInterval:[minutes intValue]];
    result(@YES);
}

- (void)handleGetConfiguration:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *config = [self.powerManager getConfiguration];
    result(config);
}

#pragma mark - State Persistence

- (void)handleSaveState:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *key = args[@"key"];
    NSString *value = args[@"value"];
    
    BOOL success = [self.statePersistence saveState:value forKey:key];
    result(@(success));
}

- (void)handleLoadState:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *key = args[@"key"];
    
    NSString *value = [self.statePersistence loadStateForKey:key];
    result(value ? value : @"");
}

- (void)handleClearState:(FlutterMethodCall*)call result:(FlutterResult)result {
    BOOL success = [self.statePersistence clearState];
    result(@(success));
}

- (void)handleSetApplicationName:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *name = args[@"name"];
    
    [self.statePersistence setApplicationName:name];
    result(@YES);
}

- (void)handleGetStoragePath:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSString *path = [self.statePersistence getStoragePath];
    result(path);
}

#pragma mark - Bidirectional Communication

- (void)handleSendMessage:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *message = args[@"message"];
    NSNumber *monitorIndex = args[@"monitorIndex"];
    
    BOOL success = [self.messageBridge sendMessage:message
                                     toMonitorIndex:monitorIndex ? [monitorIndex intValue] : -1];
    result(@(success));
}

- (void)handleGetPendingMessages:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSArray *messages = [self.messageBridge getPendingMessages];
    result(messages);
}

#pragma mark - File Encryption

- (void)handleEncryptFile:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *sourcePath = args[@"sourcePath"];
    NSString *destPath = args[@"destPath"];
    
    // TODO: Implement file encryption
    // For now, return not implemented
    result(@NO);
}

- (void)handleDecryptFile:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *encryptedPath = args[@"encryptedPath"];
    NSString *destPath = args[@"destPath"];
    
    // TODO: Implement file decryption
    // For now, return not implemented
    result(@NO);
}

- (void)dealloc {
    [AWPLogger log:@"AnyWP Engine Plugin deallocated"];
}

@end

