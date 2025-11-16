#pragma once
#include <string>
#include <Windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <wrl/client.h>

namespace anywp_engine {

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
 * - anywp://file?path=C:/my_wallpaper/cache/image_001.encrypted
 * - anywp://file?path=D:/projects/assets/video.dat
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
 */
class CustomSchemeHandler {
public:
  /**
   * @brief 初始化并注册自定义协议
   * @param webview WebView2 实例
   * @return 成功返回 S_OK
   */
  static HRESULT Initialize(ICoreWebView2* webview);
  
  /**
   * @brief 清理资源
   */
  static void Uninitialize();
  
  /**
   * @brief 加密文件（供 MethodChannel 调用）
   * @param sourcePath 源文件路径
   * @param destPath 目标加密文件路径
   * @return 成功返回 S_OK
   */
  static HRESULT EncryptFile(const std::wstring& sourcePath, const std::wstring& destPath);
  
  /**
   * @brief 解密文件（供 MethodChannel 调用）
   * @param encryptedPath 加密文件路径
   * @param destPath 目标解密文件路径
   * @return 成功返回 S_OK
   */
  static HRESULT DecryptFile(const std::wstring& encryptedPath, const std::wstring& destPath);
  
private:
  /**
   * @brief 处理 anywp:// 协议请求
   * @param webview WebView2 实例
   * @param args 请求参数
   * @return 成功返回 S_OK
   */
  static HRESULT HandleRequest(
    ICoreWebView2* webview,
    ICoreWebView2WebResourceRequestedEventArgs* args
  );
  
  /**
   * @brief 解密文件到内存流
   * @param filePath 加密文件路径
   * @param ppStream 输出流指针
   * @return 成功返回 S_OK
   */
  static HRESULT DecryptToStream(
    const std::wstring& filePath,
    IStream** ppStream
  );
  
  /**
   * @brief 解密字节（XOR 混淆）
   * @param data 数据指针
   * @param length 数据长度
   */
  static void DeobfuscateBytes(BYTE* data, DWORD length);
  
  /**
   * @brief 加密字节（XOR 混淆，与解密相同）
   * @param data 数据指针
   * @param length 数据长度
   */
  static void ObfuscateBytes(BYTE* data, DWORD length);
  
  /**
   * @brief 从 URL 提取文件路径
   * @param url 完整 URL (anywp://file?path=...)
   * @param filePath 输出文件路径
   * @return true 解析成功, false 失败
   */
  static bool ExtractFilePath(const std::wstring& url, std::wstring& filePath);
  
  /**
   * @brief 验证路径安全性（防止路径遍历攻击）
   * @param filePath 待验证的文件路径
   * @return true 安全, false 存在安全风险
   */
  static bool ValidatePathSecurity(const std::wstring& filePath);
  
  /**
   * @brief 创建错误响应
   * @param env WebView2 环境
   * @param args 请求参数
   * @param statusCode HTTP 状态码
   * @param message 错误消息
   * @return 成功返回 S_OK
   */
  static HRESULT CreateErrorResponse(
    ICoreWebView2Environment* env,
    ICoreWebView2WebResourceRequestedEventArgs* args,
    int statusCode,
    const std::wstring& message
  );
  
  // XOR 混淆常量（与 Dart 端保持一致）
  static constexpr BYTE XOR_KEY = 0x5A;
  static constexpr DWORD OBFUSCATION_BYTE_COUNT = 64;
  
  // 流式传输块大小
  static constexpr DWORD CHUNK_SIZE = 64 * 1024;  // 64KB
  
  // 事件注册 Token
  static EventRegistrationToken s_requestedToken_;
};

}  // namespace anywp_engine
