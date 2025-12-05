#ifndef FLUTTER_PLUGIN_SCRIPT_INJECTION_MANAGER_H_
#define FLUTTER_PLUGIN_SCRIPT_INJECTION_MANAGER_H_

#include <string>
#include <WebView2.h>
#include <wrl.h>

namespace anywp_engine {

/**
 * @brief 脚本注入管理器
 * 
 * 管理 SDK 和自定义脚本的注入：
 * - 加载和注入 AnyWP SDK
 * - 设置消息桥接
 * - 管理脚本缓存
 * 
 * @since v2.5.0
 */
class ScriptInjectionManager {
 public:
  ScriptInjectionManager();
  ~ScriptInjectionManager();

  // 禁止拷贝和赋值
  ScriptInjectionManager(const ScriptInjectionManager&) = delete;
  ScriptInjectionManager& operator=(const ScriptInjectionManager&) = delete;

  /**
   * @brief 初始化管理器
   * 
   * @param sdk_path SDK 文件路径（可选，默认使用内置路径）
   */
  void Initialize(const std::string& sdk_path = "");

  /**
   * @brief 注入 AnyWP SDK 到 WebView
   * 
   * @param webview WebView2 实例
   * @return true 如果成功注入
   */
  bool InjectSDK(ICoreWebView2* webview);

  /**
   * @brief 设置消息桥接
   * 
   * @param webview WebView2 实例
   * @return true 如果成功设置
   */
  bool SetupMessageBridge(ICoreWebView2* webview);

  /**
   * @brief 加载 SDK 脚本内容
   * 
   * @return SDK 脚本字符串
   */
  std::string LoadSDKScript();

  /**
   * @brief 清除脚本缓存
   */
  void ClearCache();

 private:
  std::string sdk_path_;
  std::string cached_sdk_script_;
  bool cache_valid_ = false;
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_SCRIPT_INJECTION_MANAGER_H_

