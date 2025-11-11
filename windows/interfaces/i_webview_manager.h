#ifndef ANYWP_ENGINE_I_WEBVIEW_MANAGER_H_
#define ANYWP_ENGINE_I_WEBVIEW_MANAGER_H_

#include <windows.h>
#include <string>
#include <functional>
#include <wrl.h>
#include <WebView2.h>

namespace anywp_engine {

/**
 * @brief IWebViewManager - Abstract interface for WebView2 management
 * 
 * Purpose: Decouples WebView2 operations from concrete implementation,
 *          enabling testing with mock objects and easier maintenance.
 * 
 * Thread-safe: Implementation-dependent
 */
class IWebViewManager {
public:
  virtual ~IWebViewManager() = default;
  
  /**
   * Create a WebView2 instance
   * 
   * @param hwnd Parent window handle
   * @param url Initial URL to load
   * @param on_created Callback when WebView is created
   * @return true if creation initiated successfully
   */
  virtual bool CreateWebView(
    HWND hwnd,
    const std::string& url,
    std::function<void(HWND, Microsoft::WRL::ComPtr<ICoreWebView2>, 
                       Microsoft::WRL::ComPtr<ICoreWebView2Controller>)> on_created
  ) = 0;
  
  /**
   * Navigate to URL
   * 
   * @param webview WebView instance
   * @param url Target URL
   * @return true if navigation started
   */
  virtual bool Navigate(Microsoft::WRL::ComPtr<ICoreWebView2> webview, 
                       const std::string& url) = 0;
  
  /**
   * Execute JavaScript in WebView
   * 
   * @param webview WebView instance
   * @param script JavaScript code
   * @return true if execution started
   */
  virtual bool ExecuteScript(Microsoft::WRL::ComPtr<ICoreWebView2> webview,
                            const std::wstring& script) = 0;
  
  /**
   * Get shared WebView2 environment
   * 
   * @return Shared environment instance
   */
  virtual Microsoft::WRL::ComPtr<ICoreWebView2Environment> GetEnvironment() = 0;
  
  /**
   * Check if WebView is initialized
   * 
   * @return true if initialized
   */
  virtual bool IsInitialized() const = 0;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_I_WEBVIEW_MANAGER_H_

