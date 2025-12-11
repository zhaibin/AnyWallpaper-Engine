#import "AnyWPEnginePlugin.h"
#import "Modules/WallpaperManager.h"
#import "Modules/MonitorManager.h"
#import "Modules/PowerManager.h"
#import "Modules/MessageBridge.h"
#import "Utils/Logger.h"
#import "Utils/StatePersistence.h"
#import "Utils/AWPCustomSchemeHandler.h"
#import "Utils/LocalFileServer.h"

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
        
        // IMPORTANT: Set wallpaper manager reference in message bridge (for bidirectional communication)
        [_messageBridge setWallpaperManager:_wallpaperManager];
        
        [AWPLogger log:@"AnyWP Engine Plugin initialized"];
    }
    return self;
}

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSString *method = call.method;
    
    @try {
        // ========== Version Info ==========
        if ([method isEqualToString:@"getVersion"]) {
            // Read version from Info.plist dynamically
            NSBundle *bundle = [NSBundle bundleForClass:[self class]];
            NSString *version = [bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
            if (!version) {
                version = @"Unknown";
            }
            result(version);
        }
        else if ([method isEqualToString:@"getSDKVersion"]) {
            // SDK version is independent from plugin version
            result(@"2.5.0");
        }
        
        // ========== Debug & Logging ==========
        else if ([method isEqualToString:@"setLogLevel"]) {
            NSDictionary *args = call.arguments;
            NSNumber *levelNum = args[@"level"];
            if (levelNum) {
                AWPLogLevel level = (AWPLogLevel)[levelNum integerValue];
                [AWPLogger setLogLevel:level];
                [AWPLogger debug:[NSString stringWithFormat:@"Log level changed to: %ld", (long)level]];
                result(@YES);
            } else {
                [AWPLogger error:@"setLogLevel: missing 'level' parameter"];
                result(@NO);
            }
        }
        else if ([method isEqualToString:@"getLogLevel"]) {
            AWPLogLevel currentLevel = [AWPLogger logLevel];
            result(@(currentLevel));
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
        
        // ========== Bundle Resources ==========
        else if ([method isEqualToString:@"getBundleResourcePath"]) {
            [self handleGetBundleResourcePath:call result:result];
        }
        
        // ========== Interactive Mode ==========
        else if ([method isEqualToString:@"setInteractiveMode"]) {
            [self handleSetInteractiveMode:call result:result];
        }
        
        // ========== Local File Server ==========
        else if ([method isEqualToString:@"startFileServer"]) {
            [self handleStartFileServer:call result:result];
        }
        else if ([method isEqualToString:@"stopFileServer"]) {
            [self handleStopFileServer:call result:result];
        }
        else if ([method isEqualToString:@"isFileServerRunning"]) {
            [self handleIsFileServerRunning:call result:result];
        }
        
        // ========== File Access Control ==========
        else if ([method isEqualToString:@"setAllowedAccessPath"]) {
            [self handleSetAllowedAccessPath:call result:result];
        }
        else if ([method isEqualToString:@"getDefaultLibraryPath"]) {
            [self handleGetDefaultLibraryPath:call result:result];
        }
        else if ([method isEqualToString:@"getApplicationSupportPath"]) {
            [self handleGetApplicationSupportPath:call result:result];
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
    NSString *allowedAccessPath = args[@"allowedAccessPath"];  // Optional custom access path
    
    BOOL success;
    if (allowedAccessPath && allowedAccessPath.length > 0) {
        // Use custom allowed access path
        success = [self.wallpaperManager initializeWallpaperOnMonitor:url
                                                         monitorIndex:[monitorIndex intValue]
                                                    allowedAccessPath:allowedAccessPath];
    } else {
        // Use default (global or Library path)
        success = [self.wallpaperManager initializeWallpaperOnMonitor:url
                                                         monitorIndex:[monitorIndex intValue]];
    }
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
    // Get all wallpaper instances from WallpaperManager
    NSArray *instances = [self.wallpaperManager getAllInstances];
    
    // Pass instances to PowerManager for optimization
    [self.powerManager optimizeMemory:instances];
    
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
    
    if (!sourcePath || !destPath) {
        [AWPLogger error:@"EncryptFile - Missing sourcePath or destPath"];
        result([FlutterError errorWithCode:@"INVALID_ARGUMENTS"
                                   message:@"sourcePath and destPath are required"
                                   details:nil]);
        return;
    }
    
    NSError *error = nil;
    BOOL success = [AWPCustomSchemeHandler encryptFile:sourcePath 
                                         toDestination:destPath 
                                                 error:&error];
    
    if (success) {
        [AWPLogger log:[NSString stringWithFormat:@"File encrypted successfully: %@ -> %@", 
                       sourcePath, destPath]];
        result(@YES);
    } else {
        [AWPLogger error:[NSString stringWithFormat:@"File encryption failed: %@", error.localizedDescription]];
        result([FlutterError errorWithCode:@"ENCRYPTION_FAILED"
                                   message:error.localizedDescription
                                   details:nil]);
    }
}

- (void)handleDecryptFile:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *encryptedPath = args[@"encryptedPath"];
    NSString *destPath = args[@"destPath"];
    
    if (!encryptedPath || !destPath) {
        [AWPLogger error:@"DecryptFile - Missing encryptedPath or destPath"];
        result([FlutterError errorWithCode:@"INVALID_ARGUMENTS"
                                   message:@"encryptedPath and destPath are required"
                                   details:nil]);
        return;
    }
    
    NSError *error = nil;
    BOOL success = [AWPCustomSchemeHandler decryptFile:encryptedPath 
                                         toDestination:destPath 
                                                 error:&error];
    
    if (success) {
        [AWPLogger log:[NSString stringWithFormat:@"File decrypted successfully: %@ -> %@", 
                       encryptedPath, destPath]];
        result(@YES);
    } else {
        [AWPLogger error:[NSString stringWithFormat:@"File decryption failed: %@", error.localizedDescription]];
        result([FlutterError errorWithCode:@"DECRYPTION_FAILED"
                                   message:error.localizedDescription
                                   details:nil]);
    }
}

#pragma mark - Bundle Resources

- (void)handleGetBundleResourcePath:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *resourceName = args[@"resourceName"];
    NSString *resourceType = args[@"type"] ?: @"html";
    
    if (!resourceName || resourceName.length == 0) {
        result([FlutterError errorWithCode:@"INVALID_ARGUMENT"
                                   message:@"Resource name is required"
                                   details:nil]);
        return;
    }
    
    // Get the plugin bundle
    NSBundle *bundle = [NSBundle bundleForClass:[self class]];
    
    // Find resource path
    NSString *resourcePath = [bundle pathForResource:resourceName ofType:resourceType];
    
    if (resourcePath) {
        [AWPLogger log:[NSString stringWithFormat:@"Found bundle resource: %@", resourcePath]];
        result(resourcePath);
    } else {
        [AWPLogger warn:[NSString stringWithFormat:@"Bundle resource not found: %@.%@", 
                        resourceName, resourceType]];
        result([FlutterError errorWithCode:@"RESOURCE_NOT_FOUND"
                                   message:[NSString stringWithFormat:@"Resource %@.%@ not found in bundle", 
                                           resourceName, resourceType]
                                   details:nil]);
    }
}

#pragma mark - Interactive Mode

- (void)handleSetInteractiveMode:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSNumber *monitorIndex = args[@"monitorIndex"];
    NSNumber *interactive = args[@"interactive"];
    
    if (!monitorIndex || !interactive) {
        result([FlutterError errorWithCode:@"INVALID_ARGUMENT"
                                   message:@"monitorIndex and interactive are required"
                                   details:nil]);
        return;
    }
    
    // Set interactive mode
    BOOL success = [self.wallpaperManager setInteractiveMode:[interactive boolValue]
                                                   forMonitor:[monitorIndex intValue]];
    
    if (success) {
        // Notify Web SDK about mode change
        NSString *message = [NSString stringWithFormat:
            @"{\"type\":\"interactiveMode\",\"data\":{\"interactive\":%@,\"monitorIndex\":%@}}",
            interactive, monitorIndex];
        [self.messageBridge sendMessage:message toMonitorIndex:[monitorIndex intValue]];
    }
    
    result(@(success));
}

#pragma mark - Local File Server

- (void)handleStartFileServer:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *rootPath = args[@"rootPath"];
    
    if (!rootPath || rootPath.length == 0) {
        result([FlutterError errorWithCode:@"INVALID_ARGUMENT"
                                   message:@"Root path is required"
                                   details:nil]);
        return;
    }
    
    BOOL success = [LocalFileServer startWithRootDirectory:rootPath];
    
    if (success) {
        NSString *baseURL = [LocalFileServer baseURL];
        result(@{
            @"success": @YES,
            @"baseURL": baseURL,
            @"rootPath": rootPath
        });
    } else {
        result(@{
            @"success": @NO,
            @"error": @"Failed to start file server"
        });
    }
}

- (void)handleStopFileServer:(FlutterMethodCall*)call result:(FlutterResult)result {
    [LocalFileServer stop];
    result(@YES);
}

- (void)handleIsFileServerRunning:(FlutterMethodCall*)call result:(FlutterResult)result {
    BOOL isRunning = [LocalFileServer isRunning];
    NSString *rootDirectory = [LocalFileServer rootDirectory];
    
    if (isRunning && rootDirectory) {
        result(@{
            @"running": @YES,
            @"baseURL": [LocalFileServer baseURL],
            @"rootPath": rootDirectory
        });
    } else {
        result(@{
            @"running": @NO
        });
    }
}

#pragma mark - File Access Control

- (void)handleSetAllowedAccessPath:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    NSString *path = args[@"path"];
    
    [self.wallpaperManager setGlobalAllowedAccessPath:path];
    
    // Return the current allowed access path (or default)
    NSString *currentPath = [self.wallpaperManager globalAllowedAccessPath];
    if (!currentPath || currentPath.length == 0) {
        currentPath = [WallpaperManager defaultLibraryPath];
    }
    
    result(@{
        @"success": @YES,
        @"currentPath": currentPath ?: @""
    });
}

- (void)handleGetDefaultLibraryPath:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSString *path = [WallpaperManager defaultLibraryPath];
    result(path ?: @"");
}

- (void)handleGetApplicationSupportPath:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSString *path = [WallpaperManager applicationSupportPath];
    result(path ?: @"");
}

- (void)dealloc {
    [AWPLogger log:@"AnyWP Engine Plugin deallocated"];
}

@end

