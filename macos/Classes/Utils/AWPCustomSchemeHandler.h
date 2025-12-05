//
//  AWPCustomSchemeHandler.h
//  anywp_engine
//
//  Created on 2025-11-17.
//  Custom URL scheme handler for anywp:// protocol
//

#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * @brief 自定义协议处理器 - 处理 anywp:// 协议
 * 
 * 设计理念：
 * - 引擎只负责加解密算法，不管理文件路径
 * - 文件路径由开发者完全控制
 * - 通用化设计，服务更多开发者
 * 
 * 协议格式：anywp://file?path={完整文件路径}
 * 
 * 示例：
 * - anywp://file?path=/Users/user/my_wallpaper/cache/image_001.encrypted
 * - anywp://file?path=/Volumes/Data/projects/assets/video.dat
 * 
 * 安全特性：
 * - 路径安全验证（防止路径遍历攻击）
 * - 文件存在性检查
 * - 解密失败处理
 * - MIME 类型自动检测
 * 
 * 加密算法：XOR 混淆（前64字节）
 * - XOR Key: 0x5A
 * - 混淆范围: 前64字节
 * - 与 Windows 端保持一致
 */
@interface AWPCustomSchemeHandler : NSObject <WKURLSchemeHandler>

/**
 * @brief 初始化自定义协议处理器
 * @return 实例
 */
- (instancetype)init;

/**
 * @brief 加密文件（供 MethodChannel 调用）
 * @param sourcePath 源文件路径
 * @param destPath 目标加密文件路径
 * @param error 错误信息
 * @return 成功返回 YES
 */
+ (BOOL)encryptFile:(NSString *)sourcePath
          toDestination:(NSString *)destPath
                  error:(NSError **)error;

/**
 * @brief 解密文件（供 MethodChannel 调用）
 * @param encryptedPath 加密文件路径
 * @param destPath 目标解密文件路径
 * @param error 错误信息
 * @return 成功返回 YES
 */
+ (BOOL)decryptFile:(NSString *)encryptedPath
      toDestination:(NSString *)destPath
              error:(NSError **)error;

@end

NS_ASSUME_NONNULL_END

