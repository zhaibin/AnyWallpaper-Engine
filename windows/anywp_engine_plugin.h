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

// Forward declarations of modular classes
#include "utils/url_validator.h"
#include "modules/power_manager.h"  // v1.4.0+ Refactoring: PowerManager module
#include "modules/monitor_manager.h"  // v1.4.0+ Refactoring: MonitorManager module
#include "modules/mouse_hook_manager.h"  // v1.4.0+ Refactoring: MouseHookManager module
#include "modules/iframe_detector.h"  // v1.4.0+ Refactoring: IframeDetector module
#include "modules/sdk_bridge.h"  // v1.4.0+ Refactoring: SDKBridge module
#include "modules/webview_manager.h"  // v1.4.1+ Refactoring: WebViewManager module
#include "modules/flutter_bridge.h"  // v2.0.0+ Phase2: FlutterBridge module
#include "modules/display_change_coordinator.h"  // v2.0.0+ Phase2: DisplayChangeCoordinator module
#include "modules/instance_manager.h"  // v2.0.0+ Phase2: InstanceManager module
#include "modules/window_manager.h"  // v2.0.0+ Phase2: WindowManager module
#include "modules/initialization_coordinator.h"  // v2.0+ Refactoring: InitializationCoordinator module
#include "modules/webview_configurator.h"  // v2.0+ Refactoring: WebViewConfigurator module

namespace anywp_engine {

// v1.4.0+ Refactoring: Use MonitorInfo from MonitorManager module
using MonitorInfo = anywp_engine::MonitorInfo;

// v1.4.0+ Refactoring: Use IframeInfo from IframeDetector module
using IframeInfo = anywp_engine::IframeInfo;

// Wallpaper instance for a specific monitor
struct WallpaperInstance {
  int monitor_index;
  bool enable_mouse_transparent;  // v2.0.1+ Bug Fix: Store per-monitor transparency setting
  HWND webview_host_hwnd;
  HWND worker_w_hwnd;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> webview_controller;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview;
  std::vector<IframeInfo> iframes;
};

// P0-1: Resource Tracker for memory leak detection (MOVED TO utils/resource_tracker.h)
// Forward declaration - full definition in utils/resource_tracker.h
class ResourceTracker;

// P0-3: URL Validator for security (MOVED TO utils/url_validator.h)
// Forward declaration - full definition in utils/url_validator.h
class URLValidator;

class AnyWPEnginePlugin : public flutter::Plugin {
  // v2.0.0+ Phase2: Allow FlutterBridge to access private members
  friend class FlutterBridge;

 public:
  // Power saving state enum (moved to public for FlutterBridge access)
  enum class PowerState {
    ACTIVE,          // 正常活跃状态
    IDLE,            // 用户无活动（但屏幕亮着）
    SCREEN_OFF,      // 屏幕已关闭
    LOCKED,          // 系统已锁屏
    FULLSCREEN_APP,  // 有全屏应用在前台
    PAUSED           // 手动暂停
  };

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

  static std::string GetPluginVersion();

  // Multi-monitor support
  std::vector<MonitorInfo> GetMonitors();
  bool InitializeWallpaperOnMonitor(const std::string& url, bool enable_mouse_transparent, int monitor_index);
  bool StopWallpaperOnMonitor(int monitor_index);
  bool NavigateToUrlOnMonitor(const std::string& url, int monitor_index);
  
  // Phase B: Common initialization logic (helper for both InitializeWallpaper and InitializeWallpaperOnMonitor)
  bool InitializeWallpaperCommon(const std::string& url, bool enable_mouse_transparent, 
                                  const MonitorInfo* monitor, HWND& out_hwnd, HWND& out_worker_w);

  // Legacy single-monitor methods (uses primary monitor)
  bool InitializeWallpaper(const std::string& url, bool enable_mouse_transparent);
  bool StopWallpaper();
  bool NavigateToUrl(const std::string& url);

  HWND CreateWebViewHostWindow(bool enable_mouse_transparent, const MonitorInfo* monitor = nullptr, HWND parent_window = nullptr);
  void SetupWebView2(HWND hwnd, const std::string& url, int monitor_index = -1);
  
  // v1.4.1+ Phase A: New method using WebViewManager module
  void SetupWebView2WithManager(HWND hwnd, const std::string& url, int monitor_index = -1);
  
  // Multi-monitor helpers
  WallpaperInstance* GetInstanceForMonitor(int monitor_index);
  static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
  
  // Display change monitoring
  void SetupDisplayChangeListener();
  void CleanupDisplayChangeListener();
  void HandleDisplayChange();
  // v2.0.0+ Phase2: UpdateWallpaperSizes and HandleMonitorCountChange moved to DisplayChangeCoordinator
  static LRESULT CALLBACK DisplayChangeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  
  // P0-2: Exception recovery
  bool InitializeWithRetry(const std::string& url, bool enable_mouse_transparent, int max_retries = 3);
  void LogError(const std::string& error);
  
  // P1-2: Cache management
  void ClearWebViewCache();
  void PeriodicCleanup();
  
  // P1-3: Permission control
  // v1.4.1+ Phase C: Accept webview parameter to avoid temporary swap
  void ConfigurePermissions(ICoreWebView2* webview = nullptr);
  void SetupSecurityHandlers(ICoreWebView2* webview = nullptr);
  
  // State persistence: Save/load wallpaper state
  bool SaveState(const std::string& key, const std::string& value);
  std::string LoadState(const std::string& key);
  bool ClearState();
  void SetApplicationName(const std::string& name);  // Set app identifier for storage isolation
  std::string GetStoragePath();  // Get application-specific storage path

  // Interactive mode control (v2.0.1+)
  bool SetInteractive(bool interactive);  // Set interactive mode for all instances
  bool SetInteractiveOnMonitor(bool interactive, int monitor_index);  // Set for specific monitor
  
  // API Bridge: JavaScript SDK injection and message handling
  // v1.4.1+ Phase C: Accept webview parameter to avoid temporary swap
  void InjectAnyWallpaperSDK(ICoreWebView2* webview = nullptr);
  void SetupMessageBridge(ICoreWebView2* webview = nullptr);
  void HandleWebMessage(const std::string& message);
  std::string LoadSDKScript();
  
  // HandleWebMessage helper methods (Phase B refactoring)
  void HandleIframeDataWebMessage(const std::string& message);
  void HandleOpenUrlWebMessage(const std::string& message);
  void HandleReadyWebMessage(const std::string& message);
  void HandleLogWebMessage(const std::string& message);
  void HandleConsoleLogWebMessage(const std::string& message);
  void HandleSaveStateWebMessage(const std::string& message);
  void HandleLoadStateWebMessage(const std::string& message);
  void HandleClearStateWebMessage(const std::string& message);
  
  // Mouse Hook: Capture desktop clicks and forward to WebView
  void SetupMouseHook();
  void RemoveMouseHook();
  static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
  void SendClickToWebView(int x, int y, const char* event_type = "mouseup");
  
  // iframe Ad Detection: Handle iframe click regions
  void HandleIframeDataMessage(const std::string& json_data, WallpaperInstance* instance);
  IframeInfo* GetIframeAtPoint(int x, int y, WallpaperInstance* instance);
  WallpaperInstance* GetInstanceAtPoint(int x, int y);
  bool IsOurWindow(HWND hwnd);

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
  
  // Persistent monitor configuration (survives session switches)
  std::vector<std::string> original_monitor_devices_;  // Device names (e.g. "\\.\DISPLAY1")
  
  // P0: Retry tracking
  int init_retry_count_ = 0;
  
  // P0-3: URL validation
  URLValidator url_validator_;
  
  // P1-2: Cache cleanup timing
  std::chrono::steady_clock::time_point last_cleanup_;
  
  // P1-1: Shared WebView2 environment
  static Microsoft::WRL::ComPtr<ICoreWebView2Environment> shared_environment_;
  
  // Mouse Hook
  // Mouse hook moved to MouseHookManager module (v1.4.0+)
  static AnyWPEnginePlugin* hook_instance_;
  bool enable_interaction_ = false;
  
  // iframe Ad Detection
  std::vector<IframeInfo> iframes_;
  std::mutex iframes_mutex_;
  
  // Display change monitoring
  // Display listener moved to MonitorManager module (v1.4.0+)
  static AnyWPEnginePlugin* display_change_instance_;
  std::string default_wallpaper_url_ = "";  // Default URL for auto-start on new monitors
  
  // State persistence (v2.0.1+ Refactoring: Moved to StatePersistence module)
  // Removed: persisted_state_, state_mutex_, application_name_
  
  // Method channel for callbacks to Dart
  flutter::MethodChannel<flutter::EncodableValue>* method_channel_ = nullptr;
  void NotifyMonitorChange();
  void SafeNotifyMonitorChange();  // Thread-safe version using message queue
  
  // Message forwarding to Flutter
  void NotifyFlutterMessage(const std::string& message);
  
  // Custom window message for safe thread communication
  static constexpr UINT WM_NOTIFY_MONITOR_CHANGE = WM_USER + 100;
  
  // ========== Power Saving & Optimization ==========
  
  // Power saving state (enum moved to public section)
  PowerState power_state_ = PowerState::ACTIVE;
  PowerState last_power_state_ = PowerState::ACTIVE;  // Track state changes
  bool auto_power_saving_enabled_ = true;  // 自动省电开关
  
  // Configurable parameters
  DWORD idle_timeout_ms_ = 5 * 60 * 1000;  // 5 minutes default
  size_t memory_threshold_mb_ = 150;        // 150MB default (aggressive)
  int cleanup_interval_minutes_ = 15;       // 15 minutes default (more frequent)
  
  // Power saving detection
  void SetupPowerSavingMonitoring();
  void CleanupPowerSavingMonitoring();
  void UpdatePowerState();
  bool IsFullscreenAppActive();
  void PauseWallpaper(const std::string& reason);
  void ResumeWallpaper(const std::string& reason, bool force_reinit = false);
  // v1.4.1+ Phase G: Helper methods for ResumeWallpaper
  bool ValidateWallpaperWindows();
  bool RestoreWallpaperConfiguration(const std::string& url);
  void NotifyPowerStateChange(PowerState newState);
  std::string PowerStateToString(PowerState state);
  void NotifyWebContentVisibility(bool visible);
  void ExecuteScriptToAllInstances(const std::wstring& script);  // Helper to execute script to all WebView instances
  
  // System message handling
  static LRESULT CALLBACK PowerSavingWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  HWND power_listener_hwnd_ = nullptr;
  
  // Fullscreen detection
  void StartFullscreenDetection();
  void StopFullscreenDetection();
  // Fullscreen detection moved to PowerManager module (v1.4.0+)
  
  // User activity monitoring
  DWORD last_user_input_time_ = 0;
  
  // Memory optimization
  void OptimizeMemoryUsage();
  void ConfigureWebView2Memory();
  size_t GetCurrentMemoryUsage();
  void ScheduleSafeMemoryOptimization(ICoreWebView2* webview);  // Safe delayed optimization
  
  // Performance optimization
  std::atomic<bool> is_paused_{false};
  
  // Session state tracking
  std::atomic<bool> is_session_locked_{false};     // Track lock state from WTS events
  std::atomic<bool> is_remote_session_{false};     // Track remote session state
  bool ShouldWallpaperBeActive();  // Check if user is in local desktop environment
  
  // ========== v1.4.0+ Refactoring: Module Integration ==========
  // PowerManager module for centralized power management
  std::unique_ptr<PowerManager> power_manager_;
  
  // MonitorManager module for display management
  std::unique_ptr<MonitorManager> monitor_manager_;
  
  // MouseHookManager module for mouse event handling
  std::unique_ptr<MouseHookManager> mouse_hook_manager_;
  std::unique_ptr<anywp_engine::IframeDetector> iframe_detector_;  // v1.4.0+
  std::unique_ptr<anywp_engine::SDKBridge> sdk_bridge_;  // v1.4.0+
  
  // WebViewManager module for WebView2 creation and management (v1.4.1+)
  std::unique_ptr<class WebViewManager> webview_manager_;
  
  // FlutterBridge module for Flutter method channel communication (v2.0.0+ Phase2)
  std::unique_ptr<class FlutterBridge> flutter_bridge_;
  
  // DisplayChangeCoordinator module for display change handling (v2.0.0+ Phase2)
  std::unique_ptr<class DisplayChangeCoordinator> display_change_coordinator_;
  
  // InstanceManager module for wallpaper instance management (v2.0.0+ Phase2)
  std::unique_ptr<class InstanceManager> instance_manager_;
  
  // WindowManager module for window creation and management (v2.0.0+ Phase2)
  std::unique_ptr<class WindowManager> window_manager_;
  
  // StatePersistence module for application-level state storage (v2.0.1+ Refactoring)
  std::unique_ptr<class StatePersistence> state_persistence_;
  
  // InitializationCoordinator module for wallpaper initialization (v2.0+ Refactoring)
  std::unique_ptr<class InitializationCoordinator> initialization_coordinator_;
  
  // WebViewConfigurator module for WebView2 configuration (v2.0+ Refactoring)
  std::unique_ptr<class WebViewConfigurator> webview_configurator_;
  
  // EventDispatcher module for high-performance mouse event routing (v2.1.0+ Refactoring)
  std::unique_ptr<class EventDispatcher> event_dispatcher_;
  
  // MemoryOptimizer module for unified memory management (v2.1.0+ Refactoring)
  std::unique_ptr<class MemoryOptimizer> memory_optimizer_;
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_PLUGIN_H_

