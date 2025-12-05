#import "Logger.h"

@implementation AWPLogger

+ (void)log:(NSString *)message {
    NSLog(@"[AnyWP] %@", message);
}

+ (void)error:(NSString *)message {
    NSLog(@"[AnyWP ERROR] %@", message);
}

+ (void)warn:(NSString *)message {
    NSLog(@"[AnyWP WARN] %@", message);
}

+ (void)debug:(NSString *)message {
#ifdef DEBUG
    NSLog(@"[AnyWP DEBUG] %@", message);
#endif
}

@end

