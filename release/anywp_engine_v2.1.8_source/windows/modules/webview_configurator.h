#ifndef ANYWP_ENGINE_WEBVIEW_CONFIGURATOR_H_
#define ANYWP_ENGINE_WEBVIEW_CONFIGURATOR_H_

#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <string>
#include <functional>

namespace anywp_engine {

/**
 * @brief Configures WebView2 settings, handlers, and SDK injection (v3.0+)
 * 
 * Responsibilities:
 * - Permission configuration
 * - Security handler setup
 * - Navigation handler setup
 * - Console message capturing
 * - SDK injection coordination
 * 
 * This module extracts ~150 lines of WebView configuration logic from the main plugin file.
 */
class WebViewConfigurator {
 public:
  // Configuration options
  struct ConfigOptions {
    bool enable_console_logging;
    bool enable_permission_filtering;
    bool enable_security_handlers;
    bool inject_sdk;
  };
  
  WebViewConfigurator();
  ~WebViewConfigurator();
  
  /**
   * @brief Configure WebView2 with full security and features
   * 
   * @param webview WebView2 instance to configure
   * @param options Configuration options
   * @return true if configuration successful
   */
  bool ConfigureWebView(
      Microsoft::WRL::ComPtr<ICoreWebView2> webview,
      const ConfigOptions& options);
  
  /**
   * @brief Setup permission request handlers
   * 
   * Denies dangerous permissions (microphone, camera, geolocation, clipboard)
   * 
   * @param webview WebView2 instance
   * @return true if successful
   */
  bool SetupPermissions(Microsoft::WRL::ComPtr<ICoreWebView2> webview);
  
  /**
   * @brief Setup security handlers (navigation filtering)
   * 
   * @param webview WebView2 instance
   * @param url_validator URL validator callback
   * @return true if successful
   */
  bool SetupSecurityHandlers(
      Microsoft::WRL::ComPtr<ICoreWebView2> webview,
      std::function<bool(const std::string&)> url_validator);
  
  /**
   * @brief Setup navigation handlers (completion, errors)
   * 
   * @param webview WebView2 instance
   * @param on_navigation_completed Callback for navigation completion
   * @return true if successful
   */
  bool SetupNavigationHandlers(
      Microsoft::WRL::ComPtr<ICoreWebView2> webview,
      std::function<void(bool success)> on_navigation_completed);
  
  /**
   * @brief Setup console message listener
   * 
   * Captures JavaScript console.log/warn/error messages
   * 
   * @param webview WebView2 instance
   * @param filter_pattern Optional filter pattern (e.g., "AnyWP")
   * @return true if successful
   */
  bool SetupConsoleListener(
      Microsoft::WRL::ComPtr<ICoreWebView2> webview,
      const std::string& filter_pattern = "");
  
  /**
   * @brief Inject SDK script into WebView
   * 
   * @param webview WebView2 instance
   * @param sdk_script SDK JavaScript code
   * @return true if injection successful
   */
  bool InjectSDK(
      Microsoft::WRL::ComPtr<ICoreWebView2> webview,
      const std::string& sdk_script);
  
  /**
   * @brief Execute script in WebView
   * 
   * @param webview WebView2 instance
   * @param script JavaScript code to execute
   * @return true if execution successful
   */
  bool ExecuteScript(
      Microsoft::WRL::ComPtr<ICoreWebView2> webview,
      const std::wstring& script);
  
 private:
  // Helper methods
  void LogPermissionDenied(COREWEBVIEW2_PERMISSION_KIND kind);
  void LogNavigationBlocked(const std::string& url);
  void LogConsoleMessage(const std::string& message);
  
  // Configuration state
  ConfigOptions current_options_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_WEBVIEW_CONFIGURATOR_H_

