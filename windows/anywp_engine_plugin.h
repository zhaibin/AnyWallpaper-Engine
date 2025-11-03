#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_PLUGIN_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <memory>
#include <set>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <fstream>
#include <psapi.h>
#include <mutex>

namespace anywp_engine {

// Monitor information
struct MonitorInfo {
  int index;
  std::string device_name;
  int left;
  int top;
  int width;
  int height;
  bool is_primary;
  HMONITOR handle;
};

// iframe information for ad click detection
struct IframeInfo {
  std::string id;
  std::string src;
  std::string click_url;
  int left;
  int top;
  int width;
  int height;
  bool visible;
};

// Wallpaper instance for a specific monitor
struct WallpaperInstance {
  int monitor_index;
  HWND webview_host_hwnd;
  HWND worker_w_hwnd;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> webview_controller;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview;
  std::vector<IframeInfo> iframes;
};

// P0-1: Resource Tracker for memory leak detection
class ResourceTracker {
public:
  static ResourceTracker& Instance() {
    static ResourceTracker instance;
    return instance;
  }
  
  void TrackWindow(HWND hwnd);
  void UntrackWindow(HWND hwnd);
  void CleanupAll();
  size_t GetTrackedCount() const;

private:
  std::set<HWND> tracked_windows_;
};

// P0-3: URL Validator for security
class URLValidator {
public:
  bool IsAllowed(const std::string& url);
  void AddWhitelist(const std::string& pattern);
  void AddBlacklist(const std::string& pattern);
  void ClearWhitelist();
  void ClearBlacklist();

private:
  std::vector<std::string> whitelist_;
  std::vector<std::string> blacklist_;
  bool MatchesPattern(const std::string& url, const std::string& pattern);
};

class AnyWPEnginePlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  AnyWPEnginePlugin();
  virtual ~AnyWPEnginePlugin();

  AnyWPEnginePlugin(const AnyWPEnginePlugin&) = delete;
  AnyWPEnginePlugin& operator=(const AnyWPEnginePlugin&) = delete;
  
  // Set method channel for callbacks
  void SetMethodChannel(flutter::MethodChannel<flutter::EncodableValue>* channel) {
    method_channel_ = channel;
  }

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // Multi-monitor support
  std::vector<MonitorInfo> GetMonitors();
  bool InitializeWallpaperOnMonitor(const std::string& url, bool enable_mouse_transparent, int monitor_index);
  bool StopWallpaperOnMonitor(int monitor_index);
  bool NavigateToUrlOnMonitor(const std::string& url, int monitor_index);

  // Legacy single-monitor methods (uses primary monitor)
  bool InitializeWallpaper(const std::string& url, bool enable_mouse_transparent);
  bool StopWallpaper();
  bool NavigateToUrl(const std::string& url);

  HWND CreateWebViewHostWindow(bool enable_mouse_transparent, const MonitorInfo* monitor = nullptr, HWND parent_window = nullptr);
  void SetupWebView2(HWND hwnd, const std::string& url, int monitor_index = -1);
  
  // Multi-monitor helpers
  WallpaperInstance* GetInstanceForMonitor(int monitor_index);
  static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
  
  // Display change monitoring
  void SetupDisplayChangeListener();
  void CleanupDisplayChangeListener();
  void HandleDisplayChange();
  void UpdateWallpaperSizes();
  void HandleMonitorCountChange(const std::vector<MonitorInfo>& old_monitors, const std::vector<MonitorInfo>& new_monitors);
  static LRESULT CALLBACK DisplayChangeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  
  // P0-2: Exception recovery
  bool InitializeWithRetry(const std::string& url, bool enable_mouse_transparent, int max_retries = 3);
  void LogError(const std::string& error);
  
  // P1-2: Cache management
  void ClearWebViewCache();
  void PeriodicCleanup();
  
  // P1-3: Permission control
  void ConfigurePermissions();
  void SetupSecurityHandlers();
  
  // API Bridge: JavaScript SDK injection and message handling
  void InjectAnyWallpaperSDK();
  void SetupMessageBridge();
  void HandleWebMessage(const std::string& message);
  std::string LoadSDKScript();
  
  // Mouse Hook: Capture desktop clicks and forward to WebView
  void SetupMouseHook();
  void RemoveMouseHook();
  static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
  void SendClickToWebView(int x, int y, const char* event_type = "mouseup");
  
  // iframe Ad Detection: Handle iframe click regions
  void HandleIframeDataMessage(const std::string& json_data, WallpaperInstance* instance);
  IframeInfo* GetIframeAtPoint(int x, int y, WallpaperInstance* instance);
  WallpaperInstance* GetInstanceAtPoint(int x, int y);

  // Legacy single-monitor members (kept for compatibility)
  HWND webview_host_hwnd_ = nullptr;
  HWND worker_w_hwnd_ = nullptr;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> webview_controller_;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
  bool is_initialized_ = false;
  
  // Multi-monitor members
  std::vector<MonitorInfo> monitors_;
  std::vector<WallpaperInstance> wallpaper_instances_;
  std::mutex instances_mutex_;
  
  // P0: Retry tracking
  int init_retry_count_ = 0;
  
  // P0-3: URL validation
  URLValidator url_validator_;
  
  // P1-2: Cache cleanup timing
  std::chrono::steady_clock::time_point last_cleanup_;
  
  // P1-1: Shared WebView2 environment
  static Microsoft::WRL::ComPtr<ICoreWebView2Environment> shared_environment_;
  
  // Mouse Hook
  HHOOK mouse_hook_ = nullptr;
  static AnyWPEnginePlugin* hook_instance_;
  bool enable_interaction_ = false;
  
  // iframe Ad Detection
  std::vector<IframeInfo> iframes_;
  std::mutex iframes_mutex_;
  
  // Display change monitoring
  HWND display_listener_hwnd_ = nullptr;
  static AnyWPEnginePlugin* display_change_instance_;
  std::string default_wallpaper_url_ = "";  // Default URL for auto-start on new monitors
  
  // Method channel for callbacks to Dart
  flutter::MethodChannel<flutter::EncodableValue>* method_channel_ = nullptr;
  void NotifyMonitorChange();
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_PLUGIN_H_

