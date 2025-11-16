#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_WEBVIEW_MANAGER_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_WEBVIEW_MANAGER_H_

#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <string>
#include <memory>
#include <functional>
#include <chrono>

namespace anywp_engine {

// WebView creation callback
using WebViewReadyCallback = std::function<void(
    Microsoft::WRL::ComPtr<ICoreWebView2Controller>,
    Microsoft::WRL::ComPtr<ICoreWebView2>)>;

// WebView2 Manager - Handles WebView2 creation, initialization and navigation
class WebViewManager {
public:
  WebViewManager();
  ~WebViewManager();

  // Disable copy
  WebViewManager(const WebViewManager&) = delete;
  WebViewManager& operator=(const WebViewManager&) = delete;

  // Initialize shared WebView2 environment (call once at startup)
  static void InitializeEnvironment(const std::wstring& user_data_folder);
  
  // Create WebView2 for a window
  void CreateWebView(
      HWND parent_hwnd,
      const std::string& url,
      WebViewReadyCallback callback);

  // Navigate to URL
  bool Navigate(ICoreWebView2* webview, const std::string& url);

  // Inject JavaScript SDK
  bool InjectSDK(ICoreWebView2* webview, const std::string& sdk_script);

  // Execute JavaScript
  void ExecuteScript(
      ICoreWebView2* webview,
      const std::wstring& script,
      std::function<void(HRESULT, const std::wstring&)> callback = nullptr);

  // Setup navigation handlers
  void SetupNavigationHandlers(ICoreWebView2* webview);

  // Setup permission handlers
  void ConfigurePermissions(ICoreWebView2* webview);

  // Setup security handlers
  void SetupSecurityHandlers(ICoreWebView2* webview);

  // Memory optimization
  void OptimizeMemory(ICoreWebView2* webview);
  void ScheduleSafeMemoryOptimization(ICoreWebView2* webview);

  // Cache management
  void ClearCache(ICoreWebView2* webview);

  // Get shared environment
  static Microsoft::WRL::ComPtr<ICoreWebView2Environment> GetEnvironment();

private:
  // Shared WebView2 environment
  static Microsoft::WRL::ComPtr<ICoreWebView2Environment> shared_environment_;

  // Last cleanup time for throttling
  std::chrono::steady_clock::time_point last_cleanup_;

  // Helper: Load SDK script from file
  std::string LoadSDKScriptFromFile();
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_WEBVIEW_MANAGER_H_

