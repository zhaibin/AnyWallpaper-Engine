#import "StatePersistence.h"
#import "Logger.h"

@interface StatePersistence ()
@property (nonatomic, strong) NSString *applicationName;
@property (nonatomic, strong) NSString *storagePath;
@end

@implementation StatePersistence

- (instancetype)init {
    self = [super init];
    if (self) {
        _applicationName = @"AnyWPEngine";
        [self updateStoragePath];
    }
    return self;
}

- (void)setApplicationName:(NSString *)name {
    if (!name || name.length == 0) {
        [AWPLogger warn:@"Invalid application name, using default"];
        return;
    }
    
    // Sanitize name: replace spaces with underscores, keep only alphanumeric
    NSCharacterSet *allowed = [NSCharacterSet characterSetWithCharactersInString:@"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"];
    NSString *sanitized = [[name componentsSeparatedByCharactersInSet:[allowed invertedSet]]
                          componentsJoinedByString:@"_"];
    
    self.applicationName = sanitized;
    [self updateStoragePath];
    
    [AWPLogger log:[NSString stringWithFormat:@"Application name set to: %@", sanitized]];
}

- (void)updateStoragePath {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *appSupport = [paths firstObject];
    
    self.storagePath = [[appSupport stringByAppendingPathComponent:@"AnyWPEngine"]
                       stringByAppendingPathComponent:self.applicationName];
    
    // Create directory if needed
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error = nil;
    if (![fileManager fileExistsAtPath:self.storagePath]) {
        [fileManager createDirectoryAtPath:self.storagePath
               withIntermediateDirectories:YES
                                attributes:nil
                                     error:&error];
        if (error) {
            [AWPLogger error:[NSString stringWithFormat:@"Failed to create storage directory: %@",
                            error.localizedDescription]];
        }
    }
}

- (NSString *)getStoragePath {
    return self.storagePath;
}

- (NSString *)stateFilePath {
    return [self.storagePath stringByAppendingPathComponent:@"state.json"];
}

- (NSDictionary *)loadStateDict {
    NSString *filePath = [self stateFilePath];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    if (![fileManager fileExistsAtPath:filePath]) {
        return @{};
    }
    
    @try {
        NSData *data = [NSData dataWithContentsOfFile:filePath];
        if (!data) {
            return @{};
        }
        
        NSError *error = nil;
        NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:data
                                                            options:0
                                                              error:&error];
        if (error) {
            [AWPLogger error:[NSString stringWithFormat:@"Failed to parse state JSON: %@",
                            error.localizedDescription]];
            return @{};
        }
        
        return dict ? dict : @{};
    }
    @catch (NSException *exception) {
        [AWPLogger error:[NSString stringWithFormat:@"Exception loading state: %@", exception.reason]];
        return @{};
    }
}

- (BOOL)saveStateDict:(NSDictionary *)dict {
    NSString *filePath = [self stateFilePath];
    
    @try {
        NSError *error = nil;
        NSData *data = [NSJSONSerialization dataWithJSONObject:dict
                                                      options:NSJSONWritingPrettyPrinted
                                                        error:&error];
        if (error) {
            [AWPLogger error:[NSString stringWithFormat:@"Failed to serialize state JSON: %@",
                            error.localizedDescription]];
            return NO;
        }
        
        BOOL success = [data writeToFile:filePath atomically:YES];
        if (!success) {
            [AWPLogger error:@"Failed to write state file"];
        }
        
        return success;
    }
    @catch (NSException *exception) {
        [AWPLogger error:[NSString stringWithFormat:@"Exception saving state: %@", exception.reason]];
        return NO;
    }
}

- (BOOL)saveState:(NSString *)value forKey:(NSString *)key {
    if (!key || key.length == 0) {
        [AWPLogger error:@"Cannot save state: key is empty"];
        return NO;
    }
    
    NSMutableDictionary *state = [[self loadStateDict] mutableCopy];
    state[key] = value ? value : @"";
    
    return [self saveStateDict:state];
}

- (NSString *)loadStateForKey:(NSString *)key {
    if (!key || key.length == 0) {
        [AWPLogger error:@"Cannot load state: key is empty"];
        return nil;
    }
    
    NSDictionary *state = [self loadStateDict];
    return state[key];
}

- (BOOL)clearState {
    NSString *filePath = [self stateFilePath];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    if ([fileManager fileExistsAtPath:filePath]) {
        NSError *error = nil;
        BOOL success = [fileManager removeItemAtPath:filePath error:&error];
        
        if (!success) {
            [AWPLogger error:[NSString stringWithFormat:@"Failed to clear state: %@",
                            error.localizedDescription]];
            return NO;
        }
    }
    
    [AWPLogger log:@"State cleared successfully"];
    return YES;
}

@end

