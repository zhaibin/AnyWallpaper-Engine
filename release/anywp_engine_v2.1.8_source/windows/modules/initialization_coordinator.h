#ifndef ANYWP_ENGINE_INITIALIZATION_COORDINATOR_H_
#define ANYWP_ENGINE_INITIALIZATION_COORDINATOR_H_

#include <windows.h>
#include <string>
#include <functional>
#include "monitor_manager.h"

namespace anywp_engine {

// Forward declarations
class WindowManager;
class WebViewManager;
class URLValidator;

/**
 * @brief Coordinates wallpaper initialization process (v3.0+)
 * 
 * Responsibilities:
 * - URL validation
 * - WorkerW window finding
 * - Host window creation
 * - WebView initialization coordination
 * - Error handling and retry logic
 * 
 * This module extracts ~300 lines of initialization logic from the main plugin file.
 */
class InitializationCoordinator {
 public:
  // Initialization configuration
  struct InitConfig {
    std::string url;
    bool enable_mouse_transparent;
    const MonitorInfo* monitor;  // nullptr for single-monitor mode
    int monitor_index;  // -1 for legacy single-monitor
  };
  
  // Initialization result
  struct InitResult {
    bool success;
    HWND host_window;
    HWND worker_w_window;
    std::string error_message;
  };
  
  InitializationCoordinator();
  ~InitializationCoordinator();
  
  // Initialize dependencies
  void SetWindowManager(WindowManager* window_manager);
  void SetWebViewManager(WebViewManager* webview_manager);
  void SetURLValidator(URLValidator* url_validator);
  
  /**
   * @brief Initialize wallpaper with given configuration
   * 
   * @param config Initialization configuration
   * @return InitResult Result containing success status and handles
   */
  InitResult Initialize(const InitConfig& config);
  
  /**
   * @brief Validate URL before initialization
   * 
   * @param url URL to validate
   * @return true if URL is allowed
   */
  bool ValidateURL(const std::string& url);
  
  /**
   * @brief Find WorkerW window for wallpaper embedding
   * 
   * @param out_worker_w Output parameter for WorkerW handle
   * @return true if WorkerW found successfully
   */
  bool FindWallpaperLayer(HWND& out_worker_w);
  
  /**
   * @brief Create host window for WebView
   * 
   * @param config Initialization configuration
   * @param worker_w WorkerW parent window
   * @param out_hwnd Output parameter for created window handle
   * @return true if window created successfully
   */
  bool CreateHostWindow(const InitConfig& config, HWND worker_w, HWND& out_hwnd);
  
  /**
   * @brief Configure mouse hook based on transparency setting
   * 
   * @param enable_mouse_transparent Transparency mode
   */
  void ConfigureMouseHook(bool enable_mouse_transparent);
  
  /**
   * @brief Log initialization error
   * 
   * @param error Error message
   */
  void LogError(const std::string& error);
  
 private:
  // Dependencies (not owned)
  WindowManager* window_manager_;
  WebViewManager* webview_manager_;
  URLValidator* url_validator_;
  
  // Initialization state
  int retry_count_;
  static constexpr int MAX_RETRIES = 3;
  
  // Helper methods
  bool ValidateWindowHandle(HWND hwnd);
  bool SetupWindowZOrder(HWND host_window, HWND worker_w);
  void LogInitializationStart(const InitConfig& config);
  void LogInitializationSuccess(const InitResult& result);
  void LogInitializationFailure(const InitResult& result);
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_INITIALIZATION_COORDINATOR_H_

