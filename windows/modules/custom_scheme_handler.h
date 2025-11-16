#pragma once
#include <string>
#include <Windows.h>
#include <wil/com.h>
#include <WebView2.h>

namespace anywp_engine {

/// 自定义协议处理器
/// 
/// 实现 anywp:// 自定义协议，用于加密缓存文件的流式解密和传输。
/// 
/// 支持的协议格式：
///   - anywp://image/{fileId}       - 普通图片
///   - anywp://image/{fileId}/thumb - 缩略图
///   - anywp://video/{fileId}       - 视频文件
/// 
/// 加密算法：XOR 混淆（前64字节）
/// - XOR Key: 0x5A
/// - 混淆范围: 前64字节
class CustomSchemeHandler {
public:
  /// 初始化并注册自定义协议
  /// 
  /// @param webview WebView2 实例
  /// @return 成功返回 S_OK
  static HRESULT Initialize(ICoreWebView2* webview);
  
  /// 清理资源
  static void Uninitialize();
  
private:
  /// 处理 anywp:// 协议请求
  /// 
  /// @param webview WebView2 实例
  /// @param args 请求参数
  /// @return 成功返回 S_OK
  static HRESULT HandleRequest(
    ICoreWebView2* webview,
    ICoreWebView2WebResourceRequestedEventArgs* args
  );
  
  /// 解密文件到内存流
  /// 
  /// @param filePath 加密文件路径
  /// @param ppStream 输出流指针
  /// @return 成功返回 S_OK
  static HRESULT DecryptToStream(
    const std::wstring& filePath,
    IStream** ppStream
  );
  
  /// 解密字节（XOR 混淆）
  /// 
  /// @param data 数据指针
  /// @param length 数据长度
  static void DeobfuscateBytes(BYTE* data, DWORD length);
  
  /// 获取缓存文件路径
  /// 
  /// @param resourceType 资源类型（image/video）
  /// @param fileId 文件ID
  /// @return 完整文件路径
  static std::wstring GetCacheFilePath(
    const std::wstring& resourceType,
    const std::wstring& fileId
  );
  
  /// 创建错误响应
  /// 
  /// @param env WebView2 环境
  /// @param args 请求参数
  /// @param statusCode HTTP 状态码
  /// @param message 错误消息
  /// @return 成功返回 S_OK
  static HRESULT CreateErrorResponse(
    ICoreWebView2Environment* env,
    ICoreWebView2WebResourceRequestedEventArgs* args,
    int statusCode,
    const std::wstring& message
  );
  
  /// 解析 anywp:// URL
  /// 
  /// @param url 完整URL（如 anywp://image/abc123）
  /// @param resourceType 输出资源类型
  /// @param fileId 输出文件ID
  /// @return 解析成功返回 true
  static bool ParseUrl(
    const std::wstring& url,
    std::wstring& resourceType,
    std::wstring& fileId
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

