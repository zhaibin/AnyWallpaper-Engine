#import "Logger.h"

@implementation AWPLogger

static AWPLogLevel _currentLogLevel = AWPLogLevelInfo; // Default to Info in Release

+ (void)initialize {
    if (self == [AWPLogger class]) {
#ifdef DEBUG
        _currentLogLevel = AWPLogLevelDebug; // Show all logs in Debug builds
#else
        _currentLogLevel = AWPLogLevelInfo;  // Hide debug logs in Release builds
#endif
    }
}

+ (void)setLogLevel:(AWPLogLevel)level {
    _currentLogLevel = level;
}

+ (AWPLogLevel)logLevel {
    return _currentLogLevel;
}

+ (void)debug:(NSString *)message {
    if (_currentLogLevel <= AWPLogLevelDebug) {
        NSLog(@"[AnyWP DEBUG] %@", message);
    }
}

+ (void)log:(NSString *)message {
    if (_currentLogLevel <= AWPLogLevelInfo) {
        NSLog(@"[AnyWP] %@", message);
    }
}

+ (void)warn:(NSString *)message {
    if (_currentLogLevel <= AWPLogLevelWarn) {
        NSLog(@"[AnyWP WARN] %@", message);
    }
}

+ (void)error:(NSString *)message {
    if (_currentLogLevel <= AWPLogLevelError) {
        NSLog(@"[AnyWP ERROR] %@", message);
    }
}

@end

