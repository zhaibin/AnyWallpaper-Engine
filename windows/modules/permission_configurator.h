#ifndef FLUTTER_PLUGIN_PERMISSION_CONFIGURATOR_H_
#define FLUTTER_PLUGIN_PERMISSION_CONFIGURATOR_H_

#include <WebView2.h>
#include <wrl.h>

namespace anywp_engine {

/**
 * @brief 权限配置器
 * 
 * 统一配置 WebView2 的权限和安全设置：
 * - 权限请求处理
 * - 安全策略配置
 * - CSP 设置
 * 
 * @since v2.5.0
 */
class PermissionConfigurator {
 public:
  PermissionConfigurator();
  ~PermissionConfigurator();

  // 禁止拷贝和赋值
  PermissionConfigurator(const PermissionConfigurator&) = delete;
  PermissionConfigurator& operator=(const PermissionConfigurator&) = delete;

  /**
   * @brief 配置 WebView2 权限
   * 
   * @param webview WebView2 实例
   * @return true 如果成功配置
   */
  bool ConfigurePermissions(ICoreWebView2* webview);

  /**
   * @brief 设置安全处理器
   * 
   * @param webview WebView2 实例
   * @return true 如果成功设置
   */
  bool SetupSecurityHandlers(ICoreWebView2* webview);

 private:
  // 权限处理回调
  HRESULT OnPermissionRequested(
    ICoreWebView2* sender,
    ICoreWebView2PermissionRequestedEventArgs* args);
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_PERMISSION_CONFIGURATOR_H_

