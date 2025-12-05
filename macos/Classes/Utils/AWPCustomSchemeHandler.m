//
//  AWPCustomSchemeHandler.m
//  anywp_engine
//
//  Created on 2025-11-17.
//  Custom URL scheme handler for anywp:// protocol
//

#import "AWPCustomSchemeHandler.h"
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

// XOR 混淆常量（与 Windows 端和 Dart 端保持一致）
static const uint8_t XOR_KEY = 0x5A;
static const NSUInteger OBFUSCATION_BYTE_COUNT = 64;

@implementation AWPCustomSchemeHandler

- (instancetype)init {
    self = [super init];
    if (self) {
        NSLog(@"[AnyWP] AWPCustomSchemeHandler initialized");
    }
    return self;
}

#pragma mark - WKURLSchemeHandler Protocol

- (void)webView:(WKWebView *)webView startURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask {
    @autoreleasepool {
        NSURL *url = urlSchemeTask.request.URL;
        NSLog(@"[AnyWP] Processing anywp:// request: %@", url.absoluteString);
        
        // 1. 提取文件路径
        NSString *filePath = [self extractFilePathFromURL:url];
        if (!filePath) {
            NSLog(@"[AnyWP ERROR] Invalid URL format: %@", url.absoluteString);
            [self sendErrorResponse:urlSchemeTask 
                         statusCode:400 
                            message:@"Invalid URL format: expected anywp://file?path=..."];
            return;
        }
        
        // 2. 验证路径安全性
        if (![self validatePathSecurity:filePath]) {
            NSLog(@"[AnyWP ERROR] Path security validation failed: %@", filePath);
            [self sendErrorResponse:urlSchemeTask 
                         statusCode:403 
                            message:@"Forbidden: Invalid file path"];
            return;
        }
        
        NSLog(@"[AnyWP] Decrypting file: %@", filePath);
        
        // 3. 检查文件是否存在
        if (![[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
            NSLog(@"[AnyWP ERROR] File not found: %@", filePath);
            [self sendErrorResponse:urlSchemeTask 
                         statusCode:404 
                            message:@"File not found"];
            return;
        }
        
        // 4. 检测 MIME 类型
        NSString *mimeType = [self detectMimeTypeForFile:filePath];
        if (!mimeType) {
            mimeType = @"application/octet-stream";
        }
        
        // 5. 解密文件到内存
        NSError *error = nil;
        NSData *decryptedData = [self decryptFileToData:filePath error:&error];
        
        if (!decryptedData || error) {
            NSLog(@"[AnyWP ERROR] Decryption failed for: %@ Error: %@", filePath, error.localizedDescription);
            [self sendErrorResponse:urlSchemeTask 
                         statusCode:500 
                            message:@"Decryption failed"];
            return;
        }
        
        // 6. 创建响应
        NSHTTPURLResponse *response = [[NSHTTPURLResponse alloc] 
            initWithURL:url
             statusCode:200
            HTTPVersion:@"HTTP/1.1"
           headerFields:@{
               @"Content-Type": mimeType,
               @"Content-Length": [NSString stringWithFormat:@"%lu", (unsigned long)decryptedData.length],
               @"Cache-Control": @"max-age=31536000",
               @"Access-Control-Allow-Origin": @"*"
           }];
        
        // 7. 发送响应
        [urlSchemeTask didReceiveResponse:response];
        [urlSchemeTask didReceiveData:decryptedData];
        [urlSchemeTask didFinish];
        
        NSLog(@"[AnyWP] Successfully handled request - MimeType: %@ Size: %lu bytes", 
              mimeType, (unsigned long)decryptedData.length);
    }
}

- (void)webView:(WKWebView *)webView stopURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask {
    NSLog(@"[AnyWP] URL scheme task stopped");
}

#pragma mark - Path Extraction and Validation

- (NSString *)extractFilePathFromURL:(NSURL *)url {
    NSString *urlString = url.absoluteString;
    
    // 验证协议前缀 "anywp://file?path="
    NSString *prefix = @"anywp://file?path=";
    if (![urlString hasPrefix:prefix]) {
        return nil;
    }
    
    // 提取路径部分
    NSString *encodedPath = [urlString substringFromIndex:prefix.length];
    
    // URL 解码
    NSString *filePath = [encodedPath stringByRemovingPercentEncoding];
    
    // 基本验证
    if (!filePath || filePath.length < 3) {
        return nil;
    }
    
    return filePath;
}

- (BOOL)validatePathSecurity:(NSString *)filePath {
    // 1. 检查路径遍历攻击模式
    if ([filePath containsString:@".."]) {
        return NO;
    }
    
    // 2. 检查非法字符（macOS 文件系统）
    NSCharacterSet *illegalChars = [NSCharacterSet characterSetWithCharactersInString:@"<>|\""];
    if ([filePath rangeOfCharacterFromSet:illegalChars].location != NSNotFound) {
        return NO;
    }
    
    // 3. 验证是否为绝对路径（macOS: 必须以 / 开头）
    if (![filePath hasPrefix:@"/"]) {
        return NO;  // 必须使用绝对路径
    }
    
    return YES;
}

#pragma mark - MIME Type Detection

- (NSString *)detectMimeTypeForFile:(NSString *)filePath {
    if (@available(macOS 11.0, *)) {
        NSString *extension = [filePath pathExtension];
        UTType *type = [UTType typeWithFilenameExtension:extension];
        
        if (type && type.preferredMIMEType) {
            return type.preferredMIMEType;
        }
    }
    
    // Fallback: 简单的扩展名映射
    NSDictionary *mimeTypes = @{
        @"jpg": @"image/jpeg",
        @"jpeg": @"image/jpeg",
        @"png": @"image/png",
        @"gif": @"image/gif",
        @"webp": @"image/webp",
        @"mp4": @"video/mp4",
        @"webm": @"video/webm",
        @"mov": @"video/quicktime",
        @"avi": @"video/x-msvideo",
        @"mp3": @"audio/mpeg",
        @"wav": @"audio/wav",
        @"ogg": @"audio/ogg",
        @"html": @"text/html",
        @"css": @"text/css",
        @"js": @"application/javascript",
        @"json": @"application/json",
        @"txt": @"text/plain"
    };
    
    NSString *extension = [[filePath pathExtension] lowercaseString];
    return mimeTypes[extension];
}

#pragma mark - Encryption/Decryption Implementation

- (NSData *)decryptFileToData:(NSString *)filePath error:(NSError **)error {
    // 读取文件
    NSData *encryptedData = [NSData dataWithContentsOfFile:filePath options:0 error:error];
    if (!encryptedData) {
        return nil;
    }
    
    // 创建可变数据副本
    NSMutableData *decryptedData = [encryptedData mutableCopy];
    
    // 解密前 64 字节（或更少）
    NSUInteger length = MIN(decryptedData.length, OBFUSCATION_BYTE_COUNT);
    uint8_t *bytes = (uint8_t *)decryptedData.mutableBytes;
    
    for (NSUInteger i = 0; i < length; i++) {
        bytes[i] ^= XOR_KEY;
    }
    
    return [decryptedData copy];
}

+ (BOOL)encryptFile:(NSString *)sourcePath 
      toDestination:(NSString *)destPath 
              error:(NSError **)error {
    @autoreleasepool {
        // 1. 读取源文件
        NSData *sourceData = [NSData dataWithContentsOfFile:sourcePath options:0 error:error];
        if (!sourceData) {
            NSLog(@"[AnyWP ERROR] Failed to read source file: %@", sourcePath);
            return NO;
        }
        
        // 2. 创建可变数据副本
        NSMutableData *encryptedData = [sourceData mutableCopy];
        
        // 3. 加密前 64 字节（或更少）
        NSUInteger length = MIN(encryptedData.length, OBFUSCATION_BYTE_COUNT);
        uint8_t *bytes = (uint8_t *)encryptedData.mutableBytes;
        
        for (NSUInteger i = 0; i < length; i++) {
            bytes[i] ^= XOR_KEY;
        }
        
        // 4. 写入目标文件
        BOOL success = [encryptedData writeToFile:destPath options:NSDataWritingAtomic error:error];
        
        if (success) {
            NSLog(@"[AnyWP] Successfully encrypted file: %@ -> %@", sourcePath, destPath);
        } else {
            NSLog(@"[AnyWP ERROR] Failed to write encrypted file: %@", destPath);
        }
        
        return success;
    }
}

+ (BOOL)decryptFile:(NSString *)encryptedPath 
      toDestination:(NSString *)destPath 
              error:(NSError **)error {
    // 解密逻辑与加密相同（XOR 对称）
    return [self encryptFile:encryptedPath toDestination:destPath error:error];
}

#pragma mark - Error Response

- (void)sendErrorResponse:(id<WKURLSchemeTask>)urlSchemeTask
               statusCode:(NSInteger)statusCode
                  message:(NSString *)message {
    @autoreleasepool {
        NSURL *url = urlSchemeTask.request.URL;
        
        // 创建错误消息
        NSString *errorMsg = [NSString stringWithFormat:@"Error: %@", message];
        NSData *errorData = [errorMsg dataUsingEncoding:NSUTF8StringEncoding];
        
        // 确定状态文本
        NSString *reasonPhrase = @"Error";
        switch (statusCode) {
            case 400: reasonPhrase = @"Bad Request"; break;
            case 403: reasonPhrase = @"Forbidden"; break;
            case 404: reasonPhrase = @"Not Found"; break;
            case 500: reasonPhrase = @"Internal Server Error"; break;
        }
        
        // 创建错误响应
        NSHTTPURLResponse *response = [[NSHTTPURLResponse alloc]
            initWithURL:url
             statusCode:statusCode
            HTTPVersion:@"HTTP/1.1"
           headerFields:@{
               @"Content-Type": @"text/plain; charset=utf-8",
               @"Content-Length": [NSString stringWithFormat:@"%lu", (unsigned long)errorData.length]
           }];
        
        // 发送响应
        [urlSchemeTask didReceiveResponse:response];
        [urlSchemeTask didReceiveData:errorData];
        [urlSchemeTask didFinish];
        
        NSLog(@"[AnyWP WARN] Error response sent - Code: %ld Message: %@", 
              (long)statusCode, message);
    }
}

@end

