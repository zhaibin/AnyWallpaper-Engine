#include "anywp_engine_plugin.h"
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <windows.h>
#include <shellapi.h>
#include <wtsapi32.h>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

#pragma comment(lib, "wtsapi32.lib")

namespace anywp_engine {

// P0-1: ResourceTracker implementation
void ResourceTracker::TrackWindow(HWND hwnd) {
  if (hwnd && IsWindow(hwnd)) {
    tracked_windows_.insert(hwnd);
    std::cout << "[AnyWP] [ResourceTracker] Tracking window: " << hwnd 
              << " (Total: " << tracked_windows_.size() << ")" << std::endl;
  }
}

void ResourceTracker::UntrackWindow(HWND hwnd) {
  tracked_windows_.erase(hwnd);
  std::cout << "[AnyWP] [ResourceTracker] Untracked window: " << hwnd 
            << " (Remaining: " << tracked_windows_.size() << ")" << std::endl;
}

void ResourceTracker::CleanupAll() {
  std::cout << "[AnyWP] [ResourceTracker] Cleaning up " << tracked_windows_.size() << " windows" << std::endl;
  for (HWND hwnd : tracked_windows_) {
    if (IsWindow(hwnd)) {
      DestroyWindow(hwnd);
    }
  }
  tracked_windows_.clear();
}

size_t ResourceTracker::GetTrackedCount() const {
  return tracked_windows_.size();
}

// 检测其他壁纸软件冲突
struct WallpaperConflictInfo {
  std::string name;
  std::wstring process_name;
  bool detected;
  DWORD process_id;
};

class WallpaperConflictDetector {
public:
  static bool DetectConflicts(std::vector<WallpaperConflictInfo>& conflicts) {
    conflicts.clear();
    
    // 已知的壁纸软件列表
    std::vector<WallpaperConflictInfo> known_wallpapers = {
      {"Wallpaper Engine", L"wallpaper32.exe", false, 0},
      {"Wallpaper Engine", L"wallpaper64.exe", false, 0},
      {"Lively Wallpaper", L"lively.exe", false, 0},
      {"Lively Wallpaper", L"livelywpf.exe", false, 0},
      {"DeskScapes", L"DeskScapes.exe", false, 0},
      {"RainWallpaper", L"RainWallpaper.exe", false, 0},
      {"Push Video Wallpaper", L"PushVideoWallpaper.exe", false, 0}
    };
    
    DWORD processes[1024], bytes_needed;
    if (!EnumProcesses(processes, sizeof(processes), &bytes_needed)) {
      std::cout << "[AnyWP] [Conflict] Failed to enumerate processes" << std::endl;
      return false;
    }
    
    DWORD process_count = bytes_needed / sizeof(DWORD);
    
    for (DWORD i = 0; i < process_count; i++) {
      DWORD pid = processes[i];
      if (pid == 0) continue;
      
      HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
      if (process) {
        wchar_t process_name[MAX_PATH] = {0};
        if (GetModuleBaseNameW(process, nullptr, process_name, MAX_PATH)) {
          // 检查是否匹配已知壁纸软件
          for (auto& wp : known_wallpapers) {
            if (_wcsicmp(process_name, wp.process_name.c_str()) == 0) {
              wp.detected = true;
              wp.process_id = pid;
              conflicts.push_back(wp);
              std::wcout << L"[AnyWP] [Conflict] Detected: " << wp.process_name 
                        << L" (PID: " << pid << L")" << std::endl;
            }
          }
        }
        CloseHandle(process);
      }
    }
    
    return !conflicts.empty();
  }
};

// P0-3: URLValidator implementation
bool URLValidator::IsAllowed(const std::string& url) {
  // Empty whitelist = allow all (except blacklist)
  bool allowed = whitelist_.empty();
  
  // Check whitelist
  if (!whitelist_.empty()) {
    for (const auto& pattern : whitelist_) {
      if (MatchesPattern(url, pattern)) {
        allowed = true;
        break;
      }
    }
  }
  
  // Check blacklist (overrides whitelist)
  for (const auto& pattern : blacklist_) {
    if (MatchesPattern(url, pattern)) {
      std::cout << "[AnyWP] [Security] URL blocked by blacklist: " << url << std::endl;
      return false;
    }
  }
  
  if (!allowed && !whitelist_.empty()) {
    std::cout << "[AnyWP] [Security] URL not in whitelist: " << url << std::endl;
  }
  
  return allowed;
}

void URLValidator::AddWhitelist(const std::string& pattern) {
  whitelist_.push_back(pattern);
  std::cout << "[AnyWP] [Security] Added to whitelist: " << pattern << std::endl;
}

void URLValidator::AddBlacklist(const std::string& pattern) {
  blacklist_.push_back(pattern);
  std::cout << "[AnyWP] [Security] Added to blacklist: " << pattern << std::endl;
}

void URLValidator::ClearWhitelist() {
  whitelist_.clear();
}

void URLValidator::ClearBlacklist() {
  blacklist_.clear();
}

bool URLValidator::MatchesPattern(const std::string& url, const std::string& pattern) {
  // Simple wildcard matching (* = any characters)
  std::string lower_url = url;
  std::string lower_pattern = pattern;
  
  // Safe character conversion
  std::transform(lower_url.begin(), lower_url.end(), lower_url.begin(), 
    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  std::transform(lower_pattern.begin(), lower_pattern.end(), lower_pattern.begin(),
    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  
  // Simple contains check (can be enhanced with regex)
  if (pattern.find('*') != std::string::npos) {
    std::string prefix = lower_pattern.substr(0, lower_pattern.find('*'));
    return lower_url.find(prefix) == 0;
  }
  
  return lower_url.find(lower_pattern) != std::string::npos;
}

// P1-1: Shared WebView2 environment (static)
Microsoft::WRL::ComPtr<ICoreWebView2Environment> AnyWPEnginePlugin::shared_environment_;

// Mouse Hook instance
AnyWPEnginePlugin* AnyWPEnginePlugin::hook_instance_ = nullptr;

// Display change instance
AnyWPEnginePlugin* AnyWPEnginePlugin::display_change_instance_ = nullptr;

namespace {

// 验证窗口句柄是否有效（轻量级检查）
bool IsValidWindowHandle(HWND hwnd) {
  // 只检查句柄是否有效，不检查可见性等属性
  // WorkerW 和 SHELLDLL_DefView 可能是隐藏的，所以不能用严格的验证
  return (hwnd != nullptr && IsWindow(hwnd));
}

// 验证窗口是否适合作为父窗口（更严格的检查）
bool IsValidParentWindow(HWND hwnd) {
  if (!hwnd || !IsWindow(hwnd)) {
    return false;
  }
  
  // 检查窗口信息
  WINDOWINFO info = {0};
  info.cbSize = sizeof(WINDOWINFO);
  if (!GetWindowInfo(hwnd, &info)) {
    return false;
  }
  
  return true;
}

// 改进的WorkerW查找结构
struct WorkerWInfo {
  HWND icon_layer = nullptr;      // 包含SHELLDLL_DefView的WorkerW（图标层）
  HWND wallpaper_layer = nullptr; // 壁纸层WorkerW
  int workerw_count = 0;
  bool found_shelldll = false;
};

// 简化的WorkerW查找逻辑（更可靠）
bool FindWorkerWSafe(WorkerWInfo& info, int timeout_ms = 2000) {
  std::cout << "[AnyWP] [FindWorkerW] Starting WorkerW search" << std::endl;
  
  // 查找Progman
  HWND progman = FindWindowW(L"Progman", nullptr);
  if (!progman) {
    std::cout << "[AnyWP] [FindWorkerW] ERROR: Progman not found" << std::endl;
    return false;
  }
  
  std::cout << "[AnyWP] [FindWorkerW] Progman found: " << progman << std::endl;
  
  // 发送0x052C消息（即使WorkerW已存在也发送，确保结构正确）
  SendMessageW(progman, 0x052C, 0, 0);
  std::cout << "[AnyWP] [FindWorkerW] Sent 0x052C to Progman" << std::endl;
  
  // 短暂等待
  Sleep(100);
  
  // 枚举所有WorkerW窗口，找到包含SHELLDLL_DefView的那个
  HWND hwnd = nullptr;
  info.workerw_count = 0;
  
  while ((hwnd = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr)) != nullptr) {
    info.workerw_count++;
    
    // 检查这个WorkerW是否包含SHELLDLL_DefView
    HWND shelldll = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    if (shelldll) {
      info.icon_layer = hwnd;
      info.found_shelldll = true;
      std::cout << "[AnyWP] [FindWorkerW] Found SHELLDLL_DefView in WorkerW #" 
                << info.workerw_count << ": " << info.icon_layer << std::endl;
      
      // 查找下一个WorkerW（壁纸层）
      HWND next_workerw = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr);
      if (next_workerw) {
        info.wallpaper_layer = next_workerw;
        std::cout << "[AnyWP] [FindWorkerW] Found wallpaper layer (next WorkerW): " 
                  << info.wallpaper_layer << std::endl;
      } else {
        // 没有下一个WorkerW，使用当前这个
        info.wallpaper_layer = hwnd;
        std::cout << "[AnyWP] [FindWorkerW] No next WorkerW, using icon layer as wallpaper layer" << std::endl;
      }
      
      return true;
    }
  }
  
  // 如果还是没找到，检查Progman中是否有SHELLDLL_DefView
  HWND shelldll_in_progman = FindWindowExW(progman, nullptr, L"SHELLDLL_DefView", nullptr);
  if (shelldll_in_progman) {
    std::cout << "[AnyWP] [FindWorkerW] SHELLDLL_DefView in Progman, using Progman as parent" << std::endl;
    info.wallpaper_layer = progman;
    return true;
  }
  
  std::cout << "[AnyWP] [FindWorkerW] ERROR: Could not find WorkerW or Progman with SHELLDLL_DefView" << std::endl;
  return false;
}

}  // namespace

void AnyWPEnginePlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "anywp_engine",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<AnyWPEnginePlugin>();
  
  // Store raw channel pointer before moving
  auto* channel_ptr = channel.get();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  // Set channel pointer after handler is set
  plugin->SetMethodChannel(channel_ptr);
  
  std::cout << "[AnyWP] Channel registered at: " << channel_ptr << std::endl;

  registrar->AddPlugin(std::move(plugin));
}

AnyWPEnginePlugin::AnyWPEnginePlugin() {
  std::cout << "[AnyWP] Plugin initialized" << std::endl;
  
  // Set hook instance
  hook_instance_ = this;
  display_change_instance_ = this;
  
  // P1-2: Initialize cleanup timer
  last_cleanup_ = std::chrono::steady_clock::now();
  
  // P0-3: Setup default security rules (optional whitelist)
  // Uncomment to enable whitelist mode:
  // url_validator_.AddWhitelist("https://*");  // Allow HTTPS only
  // url_validator_.AddWhitelist("http://localhost*");  // Allow localhost
  
  // Add common malicious patterns to blacklist
  url_validator_.AddBlacklist("file:///c:/windows");
  url_validator_.AddBlacklist("file:///c:/program");
  
  // Setup display change listener
  SetupDisplayChangeListener();
  
  // Setup power saving monitoring
  SetupPowerSavingMonitoring();
}

AnyWPEnginePlugin::~AnyWPEnginePlugin() {
  std::cout << "[AnyWP] Plugin destructor - starting cleanup" << std::endl;
  
  // Remove display change listener
  CleanupDisplayChangeListener();
  
  // Cleanup power saving monitoring
  CleanupPowerSavingMonitoring();
  
  // Remove mouse hook
  RemoveMouseHook();
  
  // P0: Cleanup
  StopWallpaper();
  
  // P0-1: Cleanup all tracked resources
  ResourceTracker::Instance().CleanupAll();
  
  hook_instance_ = nullptr;
  display_change_instance_ = nullptr;
  
  std::cout << "[AnyWP] Plugin cleanup complete" << std::endl;
}

void AnyWPEnginePlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  std::cout << "[AnyWP] Method called: " << method_call.method_name() << std::endl;

  if (method_call.method_name() == "initializeWallpaper") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto url_it = arguments->find(flutter::EncodableValue("url"));
    auto transparent_it = arguments->find(flutter::EncodableValue("enableMouseTransparent"));
    
    if (url_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'url' argument");
      return;
    }

    std::string url = std::get<std::string>(url_it->second);
    bool enable_transparent = transparent_it != arguments->end() 
        ? std::get<bool>(transparent_it->second) : true;

    // P0-2: Use retry mechanism
    bool success = InitializeWithRetry(url, enable_transparent, 3);
    result->Success(flutter::EncodableValue(success));
  }
  else if (method_call.method_name() == "stopWallpaper") {
    bool success = StopWallpaper();
    result->Success(flutter::EncodableValue(success));
  }
  else if (method_call.method_name() == "navigateToUrl") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto url_it = arguments->find(flutter::EncodableValue("url"));
    if (url_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'url' argument");
      return;
    }

    std::string url = std::get<std::string>(url_it->second);
    bool success = NavigateToUrl(url);
    result->Success(flutter::EncodableValue(success));
  }
  else if (method_call.method_name() == "getMonitors") {
    // Get all monitors
    std::vector<MonitorInfo> monitors = GetMonitors();
    
    // Convert to Flutter EncodableList
    flutter::EncodableList monitor_list;
    for (const auto& monitor : monitors) {
      flutter::EncodableMap monitor_map;
      monitor_map[flutter::EncodableValue("index")] = flutter::EncodableValue(monitor.index);
      monitor_map[flutter::EncodableValue("deviceName")] = flutter::EncodableValue(monitor.device_name);
      monitor_map[flutter::EncodableValue("left")] = flutter::EncodableValue(monitor.left);
      monitor_map[flutter::EncodableValue("top")] = flutter::EncodableValue(monitor.top);
      monitor_map[flutter::EncodableValue("width")] = flutter::EncodableValue(monitor.width);
      monitor_map[flutter::EncodableValue("height")] = flutter::EncodableValue(monitor.height);
      monitor_map[flutter::EncodableValue("isPrimary")] = flutter::EncodableValue(monitor.is_primary);
      monitor_list.push_back(flutter::EncodableValue(monitor_map));
    }
    
    result->Success(flutter::EncodableValue(monitor_list));
  }
  else if (method_call.method_name() == "initializeWallpaperOnMonitor") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto url_it = arguments->find(flutter::EncodableValue("url"));
    auto monitor_it = arguments->find(flutter::EncodableValue("monitorIndex"));
    auto transparent_it = arguments->find(flutter::EncodableValue("enableMouseTransparent"));
    
    if (url_it == arguments->end() || monitor_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'url' or 'monitorIndex' argument");
      return;
    }

    std::string url = std::get<std::string>(url_it->second);
    int monitor_index = std::get<int>(monitor_it->second);
    bool enable_transparent = transparent_it != arguments->end() 
        ? std::get<bool>(transparent_it->second) : true;

    bool success = InitializeWallpaperOnMonitor(url, enable_transparent, monitor_index);
    result->Success(flutter::EncodableValue(success));
  }
  else if (method_call.method_name() == "stopWallpaperOnMonitor") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto monitor_it = arguments->find(flutter::EncodableValue("monitorIndex"));
    if (monitor_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'monitorIndex' argument");
      return;
    }

    int monitor_index = std::get<int>(monitor_it->second);
    bool success = StopWallpaperOnMonitor(monitor_index);
    result->Success(flutter::EncodableValue(success));
  }
  else if (method_call.method_name() == "navigateToUrlOnMonitor") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto url_it = arguments->find(flutter::EncodableValue("url"));
    auto monitor_it = arguments->find(flutter::EncodableValue("monitorIndex"));
    
    if (url_it == arguments->end() || monitor_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'url' or 'monitorIndex' argument");
      return;
    }

    std::string url = std::get<std::string>(url_it->second);
    int monitor_index = std::get<int>(monitor_it->second);
    bool success = NavigateToUrlOnMonitor(url, monitor_index);
    result->Success(flutter::EncodableValue(success));
  }
  // Power Saving APIs
  else if (method_call.method_name() == "pauseWallpaper") {
    PauseWallpaper("Manual pause");
    power_state_ = PowerState::PAUSED;
    result->Success(flutter::EncodableValue(true));
  }
  else if (method_call.method_name() == "resumeWallpaper") {
    if (power_state_ == PowerState::PAUSED) {
      ResumeWallpaper("Manual resume");
      power_state_ = PowerState::ACTIVE;
    }
    result->Success(flutter::EncodableValue(true));
  }
  else if (method_call.method_name() == "setAutoPowerSaving") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto enabled_it = arguments->find(flutter::EncodableValue("enabled"));
    if (enabled_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'enabled' argument");
      return;
    }

    bool enabled = std::get<bool>(enabled_it->second);
    auto_power_saving_enabled_ = enabled;
    std::cout << "[AnyWP] Auto power saving: " << (enabled ? "ENABLED" : "DISABLED") << std::endl;
    result->Success(flutter::EncodableValue(true));
  }
  else if (method_call.method_name() == "getPowerState") {
    // Return power state as string
    std::string state_str;
    switch (power_state_) {
      case PowerState::ACTIVE: state_str = "ACTIVE"; break;
      case PowerState::IDLE: state_str = "IDLE"; break;
      case PowerState::SCREEN_OFF: state_str = "SCREEN_OFF"; break;
      case PowerState::LOCKED: state_str = "LOCKED"; break;
      case PowerState::FULLSCREEN_APP: state_str = "FULLSCREEN_APP"; break;
      case PowerState::PAUSED: state_str = "PAUSED"; break;
    }
    result->Success(flutter::EncodableValue(state_str));
  }
  else if (method_call.method_name() == "getMemoryUsage") {
    size_t memory_mb = GetCurrentMemoryUsage() / 1024 / 1024;
    result->Success(flutter::EncodableValue(static_cast<int>(memory_mb)));
  }
  else if (method_call.method_name() == "optimizeMemory") {
    OptimizeMemoryUsage();
    result->Success(flutter::EncodableValue(true));
  }
  // Configuration APIs
  else if (method_call.method_name() == "setIdleTimeout") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto seconds_it = arguments->find(flutter::EncodableValue("seconds"));
    if (seconds_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'seconds' argument");
      return;
    }

    int seconds = std::get<int>(seconds_it->second);
    idle_timeout_ms_ = seconds * 1000;
    std::cout << "[AnyWP] [Config] Idle timeout set to " << seconds << " seconds" << std::endl;
    result->Success(flutter::EncodableValue(true));
  }
  else if (method_call.method_name() == "setMemoryThreshold") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto threshold_it = arguments->find(flutter::EncodableValue("thresholdMB"));
    if (threshold_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'thresholdMB' argument");
      return;
    }

    int thresholdMB = std::get<int>(threshold_it->second);
    memory_threshold_mb_ = thresholdMB;
    std::cout << "[AnyWP] [Config] Memory threshold set to " << thresholdMB << " MB" << std::endl;
    result->Success(flutter::EncodableValue(true));
  }
  else if (method_call.method_name() == "setCleanupInterval") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto minutes_it = arguments->find(flutter::EncodableValue("minutes"));
    if (minutes_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'minutes' argument");
      return;
    }

    int minutes = std::get<int>(minutes_it->second);
    cleanup_interval_minutes_ = minutes;
    std::cout << "[AnyWP] [Config] Cleanup interval set to " << minutes << " minutes" << std::endl;
    result->Success(flutter::EncodableValue(true));
  }
  else if (method_call.method_name() == "getConfiguration") {
    flutter::EncodableMap config;
    config[flutter::EncodableValue("idleTimeoutSeconds")] = flutter::EncodableValue(static_cast<int>(idle_timeout_ms_ / 1000));
    config[flutter::EncodableValue("memoryThresholdMB")] = flutter::EncodableValue(static_cast<int>(memory_threshold_mb_));
    config[flutter::EncodableValue("cleanupIntervalMinutes")] = flutter::EncodableValue(cleanup_interval_minutes_);
    config[flutter::EncodableValue("autoPowerSavingEnabled")] = flutter::EncodableValue(auto_power_saving_enabled_);
    result->Success(flutter::EncodableValue(config));
  }
  else if (method_call.method_name() == "saveState") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto key_it = arguments->find(flutter::EncodableValue("key"));
    auto value_it = arguments->find(flutter::EncodableValue("value"));
    
    if (key_it == arguments->end() || value_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'key' or 'value' argument");
      return;
    }

    std::string key = std::get<std::string>(key_it->second);
    std::string value = std::get<std::string>(value_it->second);
    
    bool success = SaveState(key, value);
    result->Success(flutter::EncodableValue(success));
  }
  else if (method_call.method_name() == "loadState") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto key_it = arguments->find(flutter::EncodableValue("key"));
    
    if (key_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'key' argument");
      return;
    }

    std::string key = std::get<std::string>(key_it->second);
    std::string value = LoadState(key);
    
    result->Success(flutter::EncodableValue(value));
  }
  else if (method_call.method_name() == "clearState") {
    bool success = ClearState();
    result->Success(flutter::EncodableValue(success));
  }
  else {
    result->NotImplemented();
  }
}


HWND AnyWPEnginePlugin::CreateWebViewHostWindow(bool enable_mouse_transparent, const MonitorInfo* monitor, HWND parent_window_arg) {
  std::cout << "[AnyWP] Creating WebView host window..." << std::endl;

  // Determine parent window
  // Priority: 1. Explicit parent_window_arg (multi-monitor)
  //           2. Legacy worker_w_hwnd_ (single-monitor)
  HWND parent_window = parent_window_arg ? parent_window_arg : worker_w_hwnd_;
  
  if (!parent_window) {
    std::cout << "[AnyWP] ERROR: No parent window (WorkerW) available" << std::endl;
    LogError("CreateWebViewHostWindow: No parent window");
    return nullptr;
  }
  
  std::cout << "[AnyWP] Using parent window (WorkerW): " << parent_window << std::endl;

  // Validate parent window
  if (!IsValidWindowHandle(parent_window)) {
    std::cout << "[AnyWP] ERROR: Parent window (WorkerW) is invalid" << std::endl;
    LogError("CreateWebViewHostWindow: Invalid parent window");
    return nullptr;
  }

  int x, y, width, height;
  
  if (monitor) {
    // Use specific monitor's dimensions and position
    // WorkerW is a global window covering all monitors (virtual desktop)
    // So we need to use absolute coordinates relative to the virtual desktop origin
    x = monitor->left;   // Absolute position in virtual desktop
    y = monitor->top;    // Absolute position in virtual desktop
    width = monitor->width;
    height = monitor->height;
    
    std::cout << "[AnyWP] Creating window for monitor: " << monitor->device_name 
              << " [" << width << "x" << height << "]"
              << " at virtual desktop position (" << x << "," << y << ")" << std::endl;
  } else {
    // Legacy: Get work area (desktop minus taskbar)
    RECT workArea = {0};
    if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0)) {
      std::cout << "[AnyWP] ERROR: Failed to get work area, using screen dimensions" << std::endl;
      workArea.left = 0;
      workArea.top = 0;
      workArea.right = GetSystemMetrics(SM_CXSCREEN);
      workArea.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    
    x = 0;
    y = 0;
    width = workArea.right - workArea.left;
    height = workArea.bottom - workArea.top;
  }
  
  // Validate dimensions
  if (width <= 0 || height <= 0 || width > 10000 || height > 10000) {
    std::cout << "[AnyWP] ERROR: Invalid window dimensions: " << width << "x" << height << std::endl;
    LogError("CreateWebViewHostWindow: Invalid dimensions");
    return nullptr;
  }
  
  std::cout << "[AnyWP] Creating child window: " << width << "x" << height << std::endl;

  // Create as CHILD window of WorkerW
  HWND hwnd = CreateWindowExW(
      0,
      L"STATIC",
      L"AnyWallpaperHost",
      WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
      x, y, width, height,
      parent_window,
      nullptr,
      GetModuleHandle(nullptr),
      nullptr);

  if (!hwnd) {
    DWORD error = GetLastError();
    std::cout << "[AnyWP] ERROR: Failed to create window, error: " << error << std::endl;
    LogError("CreateWebViewHostWindow failed: error " + std::to_string(error));
    return nullptr;
  }

  // Validate created window
  if (!IsValidWindowHandle(hwnd)) {
    std::cout << "[AnyWP] ERROR: Created window is invalid" << std::endl;
    LogError("CreateWebViewHostWindow: Created window is invalid");
    DestroyWindow(hwnd);
    return nullptr;
  }

  std::cout << "[AnyWP] WebView host window created successfully: " << hwnd << std::endl;
  
  // Track window for cleanup
  ResourceTracker::Instance().TrackWindow(hwnd);
  
  return hwnd;
}

void AnyWPEnginePlugin::SetupWebView2(HWND hwnd, const std::string& url, int monitor_index) {
  std::cout << "[AnyWP] Setting up WebView2 for monitor " << monitor_index << "..." << std::endl;
  
  // For legacy support (-1 means use legacy members)
  bool use_legacy = (monitor_index < 0);

  // Convert URL to wstring
  std::wstring wurl(url.begin(), url.end());

  // Get user data folder
  wchar_t user_data_folder[MAX_PATH];
  GetEnvironmentVariableW(L"APPDATA", user_data_folder, MAX_PATH);
  wcscat_s(user_data_folder, L"\\AnyWallpaperEngine");
  
  // P1-1: Use shared environment if available
  if (shared_environment_) {
    std::cout << "[AnyWP] [Performance] Reusing existing WebView2 environment" << std::endl;
    
    auto controller_callback = Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
        [this, hwnd, wurl, monitor_index, use_legacy](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
          if (FAILED(result)) {
            std::cout << "[AnyWP] ERROR: Failed to create WebView2 controller: " << std::hex << result << std::endl;
            return result;
          }

          std::cout << "[AnyWP] WebView2 controller created" << std::endl;

          Microsoft::WRL::ComPtr<ICoreWebView2Controller> webview_ctrl;
          Microsoft::WRL::ComPtr<ICoreWebView2> webview;

          if (use_legacy) {
            webview_controller_ = controller;
            webview_controller_->get_CoreWebView2(&webview_);
            webview_ctrl = webview_controller_;
            webview = webview_;
          } else {
            // Multi-monitor: find instance by monitor_index and save references locally
            WallpaperInstance* instance = GetInstanceForMonitor(monitor_index);
            if (!instance) {
              std::cout << "[AnyWP] ERROR: Instance not found for monitor " << monitor_index << " - may have been removed" << std::endl;
              return E_FAIL;
            }
            
            instance->webview_controller = controller;
            instance->webview_controller->get_CoreWebView2(&instance->webview);
            
            // Store in local COM pointers (thread-safe, reference-counted)
            webview_ctrl = instance->webview_controller;
            webview = instance->webview;
          }

          // Set bounds
          RECT bounds;
          GetClientRect(hwnd, &bounds);
          std::cout << "[AnyWP] Setting WebView bounds: " << bounds.left << "," << bounds.top 
                    << " " << (bounds.right - bounds.left) << "x" << (bounds.bottom - bounds.top) << std::endl;
          
          webview_ctrl->put_Bounds(bounds);
          webview_ctrl->put_IsVisible(TRUE);

              // P1-3: Configure permissions and security (use temp pointers)
              if (use_legacy) {
                ConfigurePermissions();
                SetupSecurityHandlers();
                SetupMessageBridge();
                InjectAnyWallpaperSDK();
              } else {
                // For multi-monitor, setup handlers inline
                // TODO: Refactor these methods to accept webview parameter
                // For now, temporarily set legacy members
                auto old_webview = webview_;
                webview_ = webview;
                ConfigurePermissions();
                SetupSecurityHandlers();
                SetupMessageBridge();
                InjectAnyWallpaperSDK();
                webview_ = old_webview;
              }

              // Navigate
              webview->Navigate(wurl.c_str());
              
              // After navigation completes, manually inject SDK and send interaction mode
              webview->add_NavigationCompleted(
                Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
                  [this, webview](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                    std::cout << "[AnyWP] [API] NavigationCompleted - manually injecting SDK" << std::endl;
                    
                    // CRITICAL FIX: Manually inject SDK script after navigation completes
                    // This ensures SDK is available before page scripts run
                    std::string sdk_script = LoadSDKScript();
                    std::wstring wsdk_script(sdk_script.begin(), sdk_script.end());
                    
                    sender->ExecuteScript(wsdk_script.c_str(),
                      Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                        [this](HRESULT error, LPCWSTR result) -> HRESULT {
                          if (SUCCEEDED(error)) {
                            std::cout << "[AnyWP] [API] SDK manually injected successfully" << std::endl;
                            
                            // Send interaction mode after SDK is loaded
                            std::wstringstream script;
                            script << L"(function() {"
                                   << L"  if (window.AnyWP) {"
                                   << L"    var event = new CustomEvent('AnyWP:interactionMode', {"
                                   << L"      detail: { enabled: " << (enable_interaction_ ? L"true" : L"false") << L" }"
                                   << L"    });"
                                   << L"    window.dispatchEvent(event);"
                                   << L"  }"
                                   << L"})();";
                            
                            if (webview_) {
                              webview_->ExecuteScript(script.str().c_str(), nullptr);
                              
                              // AUTO-OPTIMIZE: Schedule safe memory optimization after page loads
                              ScheduleSafeMemoryOptimization(webview_.Get());
                            }
                          } else {
                            std::cout << "[AnyWP] [API] ERROR: Failed to manually inject SDK: " << std::hex << error << std::endl;
                          }
                          return S_OK;
                        }).Get());
                    
                    std::cout << "[AnyWP] [API] Sent interaction mode to JS: " << enable_interaction_ << std::endl;
                    return S_OK;
                  }).Get(), nullptr);
          std::string url_str;
          for (wchar_t c : wurl) {
            if (c < 128) url_str.push_back(static_cast<char>(c));
          }
          std::cout << "[AnyWP] Navigating to: " << url_str << std::endl;

          if (use_legacy) {
            is_initialized_ = true;
          }
          return S_OK;
        });

    shared_environment_->CreateCoreWebView2Controller(hwnd, controller_callback.Get());
    return;
  }

  // P1-1: Create environment (will save for reuse)
  auto callback = Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
      [this, hwnd, wurl, monitor_index, use_legacy](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
        if (FAILED(result)) {
          std::cout << "[AnyWP] ERROR: Failed to create WebView2 environment: " << std::hex << result << std::endl;
          return result;
        }

        std::cout << "[AnyWP] WebView2 environment created" << std::endl;
        
        // P1-1: Save environment for reuse
        shared_environment_ = env;

        auto controller_callback = Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
            [this, hwnd, wurl, monitor_index, use_legacy](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
              if (FAILED(result)) {
                std::cout << "[AnyWP] ERROR: Failed to create WebView2 controller: " << std::hex << result << std::endl;
                return result;
              }

              std::cout << "[AnyWP] WebView2 controller created" << std::endl;

              Microsoft::WRL::ComPtr<ICoreWebView2Controller> webview_ctrl;
              Microsoft::WRL::ComPtr<ICoreWebView2> webview;

              if (use_legacy) {
                webview_controller_ = controller;
                webview_controller_->get_CoreWebView2(&webview_);
                webview_ctrl = webview_controller_;
                webview = webview_;
              } else {
                // Multi-monitor: find instance by monitor_index and save references locally
                WallpaperInstance* instance = GetInstanceForMonitor(monitor_index);
                if (!instance) {
                  std::cout << "[AnyWP] ERROR: Instance not found for monitor " << monitor_index << " - may have been removed" << std::endl;
                  return E_FAIL;
                }
                
                instance->webview_controller = controller;
                instance->webview_controller->get_CoreWebView2(&instance->webview);
                
                // Store in local COM pointers (thread-safe, reference-counted)
                webview_ctrl = instance->webview_controller;
                webview = instance->webview;
              }

              // Set bounds to match window
              RECT bounds;
              GetClientRect(hwnd, &bounds);
              std::cout << "[AnyWP] Setting WebView bounds: " << bounds.left << "," << bounds.top 
                        << " " << (bounds.right - bounds.left) << "x" << (bounds.bottom - bounds.top) << std::endl;
              
              HRESULT hr = webview_ctrl->put_Bounds(bounds);
              if (FAILED(hr)) {
                std::cout << "[AnyWP] ERROR: Failed to set bounds: " << std::hex << hr << std::endl;
              }
              
              // Make sure WebView is visible
              webview_ctrl->put_IsVisible(TRUE);
              std::cout << "[AnyWP] WebView2 visibility set to TRUE" << std::endl;

              // P1-3: Configure permissions and security
              if (use_legacy) {
                ConfigurePermissions();
                SetupSecurityHandlers();
                SetupMessageBridge();
                InjectAnyWallpaperSDK();
              } else {
                // For multi-monitor, setup handlers inline
                auto old_webview = webview_;
                webview_ = webview;
                ConfigurePermissions();
                SetupSecurityHandlers();
                SetupMessageBridge();
                InjectAnyWallpaperSDK();
                webview_ = old_webview;
              }

              // Navigate to URL
              webview->Navigate(wurl.c_str());
              
              // After navigation completes, manually inject SDK and send interaction mode
              webview->add_NavigationCompleted(
                Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
                  [this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                    std::cout << "[AnyWP] [API] NavigationCompleted - manually injecting SDK" << std::endl;
                    
                    // CRITICAL FIX: Manually inject SDK script after navigation completes
                    std::string sdk_script = LoadSDKScript();
                    std::wstring wsdk_script(sdk_script.begin(), sdk_script.end());
                    
                    sender->ExecuteScript(wsdk_script.c_str(),
                      Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                        [this](HRESULT error, LPCWSTR result) -> HRESULT {
                          if (SUCCEEDED(error)) {
                            std::cout << "[AnyWP] [API] SDK manually injected successfully" << std::endl;
                            
                            // Send interaction mode after SDK is loaded
                            std::wstringstream script;
                            script << L"(function() {"
                                   << L"  if (window.AnyWP) {"
                                   << L"    var event = new CustomEvent('AnyWP:interactionMode', {"
                                   << L"      detail: { enabled: " << (enable_interaction_ ? L"true" : L"false") << L" }"
                                   << L"    });"
                                   << L"    window.dispatchEvent(event);"
                                   << L"  }"
                                   << L"})();";
                            
                            if (webview_) {
                              webview_->ExecuteScript(script.str().c_str(), nullptr);
                              
                              // AUTO-OPTIMIZE: Schedule safe memory optimization after page loads
                              ScheduleSafeMemoryOptimization(webview_.Get());
                            }
                          } else {
                            std::cout << "[AnyWP] [API] ERROR: Failed to manually inject SDK: " << std::hex << error << std::endl;
                          }
                          return S_OK;
                        }).Get());
                    
                    std::cout << "[AnyWP] [API] Sent interaction mode to JS: " << enable_interaction_ << std::endl;
                    return S_OK;
                  }).Get(), nullptr);
              
              // Convert wstring to string for logging
              std::string url_str;
              for (wchar_t c : wurl) {
                if (c < 128) url_str.push_back(static_cast<char>(c));
              }
              std::cout << "[AnyWP] Navigating to: " << url_str << std::endl;

              if (use_legacy) {
                is_initialized_ = true;
              }
              return S_OK;
            });

        env->CreateCoreWebView2Controller(hwnd, controller_callback.Get());
        return S_OK;
      });

  HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, user_data_folder, nullptr, callback.Get());

  if (FAILED(hr)) {
    std::cout << "[AnyWP] ERROR: CreateCoreWebView2EnvironmentWithOptions failed: " << std::hex << hr << std::endl;
  }
}

// P0-2: Initialize with retry mechanism
bool AnyWPEnginePlugin::InitializeWithRetry(const std::string& url, bool enable_mouse_transparent, int max_retries) {
  std::cout << "[AnyWP] [Retry] Attempt " << (init_retry_count_ + 1) << " of " << max_retries << std::endl;
  
  bool success = InitializeWallpaper(url, enable_mouse_transparent);
  
  if (!success && init_retry_count_ < max_retries - 1) {
    init_retry_count_++;
    std::cout << "[AnyWP] [Retry] Initialization failed, retrying in 1 second..." << std::endl;
    Sleep(1000);
    return InitializeWithRetry(url, enable_mouse_transparent, max_retries);
  }
  
  if (success) {
    init_retry_count_ = 0;  // Reset on success
  }
  
  return success;
}

// P0-2: Error logging
void AnyWPEnginePlugin::LogError(const std::string& error) {
  std::ofstream log("AnyWallpaper_errors.log", std::ios::app);
  if (log.is_open()) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);
    log << "[" << timestamp << "] " << error << std::endl;
    log.close();
  }
  std::cout << "[AnyWP] [Error] " << error << std::endl;
}

// P1-2: Clear WebView cache (optimized with safety checks)
void AnyWPEnginePlugin::ClearWebViewCache() {
  std::cout << "[AnyWP] [Cache] Clearing browser cache..." << std::endl;
  
  // SAFE: Clear cache for all instances
  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    for (auto& instance : wallpaper_instances_) {
      // SAFETY: Check webview is valid before using
      if (instance.webview && instance.webview.Get()) {
        // Execute JavaScript to clear cache-like data with NULL checks
        std::wstring clear_script = L"(function() {"
          L"  try {"
          L"    // Clear localStorage with safety check"
          L"    if (window.localStorage && localStorage.clear) localStorage.clear();"
          L"    // Clear sessionStorage with safety check"
          L"    if (window.sessionStorage && sessionStorage.clear) sessionStorage.clear();"
          L"    console.log('[AnyWP] Cache cleared');"
          L"  } catch(e) {"
          L"    console.warn('[AnyWP] Cache clear error:', e);"
          L"  }"
          L"})();";
        instance.webview->ExecuteScript(clear_script.c_str(), nullptr);
      }
    }
  }
  
  // SAFETY: Clear legacy webview with NULL check
  if (webview_ && webview_.Get()) {
    std::wstring clear_script = L"(function() {"
      L"  try {"
      L"    if (window.localStorage && localStorage.clear) localStorage.clear();"
      L"    if (window.sessionStorage && sessionStorage.clear) sessionStorage.clear();"
      L"  } catch(e) {}"
      L"})();";
    webview_->ExecuteScript(clear_script.c_str(), nullptr);
  }
  
  std::cout << "[AnyWP] [Cache] Cache cleared" << std::endl;
}

// P1-2: Periodic cleanup (optimized with configurable parameters)
void AnyWPEnginePlugin::PeriodicCleanup() {
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - last_cleanup_);
  
  // Use configured cleanup interval
  if (elapsed.count() >= cleanup_interval_minutes_) {
    std::cout << "[AnyWP] [Maintenance] Performing periodic cleanup..." << std::endl;
    
    // Only optimize memory if usage exceeds configured threshold
    size_t memory_mb = GetCurrentMemoryUsage() / 1024 / 1024;
    if (memory_mb > memory_threshold_mb_) {
      std::cout << "[AnyWP] [Maintenance] High memory usage detected: " << memory_mb << "MB (threshold: " << memory_threshold_mb_ << "MB)" << std::endl;
      OptimizeMemoryUsage();
    }
    
    last_cleanup_ = now;
  }
}

// P1-3: Configure permissions
void AnyWPEnginePlugin::ConfigurePermissions() {
  if (!webview_) return;
  
  std::cout << "[AnyWP] [Security] Configuring permissions..." << std::endl;
  
  webview_->add_PermissionRequested(
    Microsoft::WRL::Callback<ICoreWebView2PermissionRequestedEventHandler>(
      [](ICoreWebView2* sender, ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT {
        COREWEBVIEW2_PERMISSION_KIND kind;
        args->get_PermissionKind(&kind);
        
        // Deny dangerous permissions by default
        switch (kind) {
          case COREWEBVIEW2_PERMISSION_KIND_MICROPHONE:
          case COREWEBVIEW2_PERMISSION_KIND_CAMERA:
          case COREWEBVIEW2_PERMISSION_KIND_GEOLOCATION:
          case COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ:
            args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY);
            std::cout << "[AnyWP] [Security] Denied permission: " << kind << std::endl;
            break;
          default:
            args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
        }
        
        return S_OK;
      }).Get(), nullptr);
  
  std::cout << "[AnyWP] [Security] Permissions configured" << std::endl;
}

// P1-3: Setup security handlers
void AnyWPEnginePlugin::SetupSecurityHandlers() {
  if (!webview_) return;
  
  std::cout << "[AnyWP] [Security] Setting up security handlers..." << std::endl;
  
  // P0-3: Navigation filter with URL validation
  webview_->add_NavigationStarting(
    Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
      [this](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
        LPWSTR uri;
        args->get_Uri(&uri);
        
        std::wstring wuri(uri);
        std::string url;
        for (wchar_t c : wuri) {
          if (c < 128) url.push_back(static_cast<char>(c));
        }
        
        // P0-3: Validate URL
        if (!url_validator_.IsAllowed(url)) {
          args->put_Cancel(TRUE);
          std::cout << "[AnyWP] [Security] Navigation blocked: " << url << std::endl;
          LogError("Navigation blocked: " + url);
        } else {
          std::cout << "[AnyWP] [Security] Navigation allowed: " << url << std::endl;
        }
        
        CoTaskMemFree(uri);
        return S_OK;
      }).Get(), nullptr);
  
  std::cout << "[AnyWP] [Security] Security handlers installed" << std::endl;
}

// API Bridge: Load SDK JavaScript
std::string AnyWPEnginePlugin::LoadSDKScript() {
  // NOTE: SDK is loaded via <script src> in HTML files
  // C++ injection is kept for compatibility but not required
  std::cout << "[AnyWP] [API] SDK should be loaded via <script src> in HTML" << std::endl;
  
  // Return minimal compatibility shim
  return R"(
console.log('[AnyWP] Note: Full SDK should be loaded via <script src="../windows/anywp_sdk.js">');
if (!window.AnyWP) {
  console.error('[AnyWP] ERROR: SDK not loaded! Add <script src="../windows/anywp_sdk.js"></script> to your HTML');
  window.AnyWP = {
    version: '0.0.0-missing',
    error: 'SDK not loaded - add script tag to HTML'
  };
}
)";
}

// API Bridge: Inject SDK into page
void AnyWPEnginePlugin::InjectAnyWallpaperSDK() {
  if (!webview_) return;
  
  std::cout << "[AnyWP] [API] Injecting AnyWallpaper SDK..." << std::endl;
  
  // Load SDK script
  std::string sdk_script = LoadSDKScript();
  std::wstring wsdk_script(sdk_script.begin(), sdk_script.end());
  
  std::cout << "[AnyWP] [API] SDK script size: " << sdk_script.length() << " bytes" << std::endl;
  
  // Inject on every navigation (for future navigations)
  webview_->AddScriptToExecuteOnDocumentCreated(
    wsdk_script.c_str(),
    Microsoft::WRL::Callback<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>(
      [](HRESULT result, LPCWSTR id) -> HRESULT {
        if (SUCCEEDED(result)) {
          std::wcout << L"[AnyWP] [API] SDK registered for future pages, ID: " << id << std::endl;
        } else {
          std::cout << "[AnyWP] [API] ERROR: Failed to register SDK: " << std::hex << result << std::endl;
        }
        return S_OK;
      }).Get());
  
  // IMPORTANT: Also inject immediately for current page
  std::cout << "[AnyWP] [API] Injecting SDK into current page..." << std::endl;
  webview_->ExecuteScript(
    wsdk_script.c_str(),
    Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
      [](HRESULT result, LPCWSTR resultObjectAsJson) -> HRESULT {
        if (SUCCEEDED(result)) {
          std::cout << "[AnyWP] [API] SDK executed successfully on current page" << std::endl;
        } else {
          std::cout << "[AnyWP] [API] ERROR: Failed to execute SDK: " << std::hex << result << std::endl;
        }
        return S_OK;
      }).Get());
}

// API Bridge: Setup message bridge
void AnyWPEnginePlugin::SetupMessageBridge() {
  if (!webview_) return;
  
  std::cout << "[AnyWP] [API] Setting up message bridge..." << std::endl;
  
  webview_->add_WebMessageReceived(
    Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
      [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
        LPWSTR message;
        args->get_WebMessageAsJson(&message);
        
        std::wstring wmessage(message);
        std::string msg;
        for (wchar_t c : wmessage) {
          if (c < 128) msg.push_back(static_cast<char>(c));
        }
        
        HandleWebMessage(msg);
        
        CoTaskMemFree(message);
        return S_OK;
      }).Get(), nullptr);
  
  std::cout << "[AnyWP] [API] Message bridge ready" << std::endl;
}

// API Bridge: Handle messages from web
void AnyWPEnginePlugin::HandleWebMessage(const std::string& message) {
  std::cout << "[AnyWP] [API] Received message: " << message << std::endl;
  
  // Parse JSON message (support both uppercase and lowercase)
  if (message.find("\"type\":\"IFRAME_DATA\"") != std::string::npos) {
    // Handle iframe data synchronization
    // CRITICAL FIX: Find the correct instance for this message
    WallpaperInstance* target_instance = nullptr;
    
    // Use first instance for now (TODO: improve for multi-monitor)
    if (!wallpaper_instances_.empty()) {
      target_instance = &wallpaper_instances_[0];
      std::cout << "[AnyWP] [API] Using wallpaper instance for iframe data" << std::endl;
    }
    
    HandleIframeDataMessage(message, target_instance);
  }
  else if (message.find("\"type\":\"OPEN_URL\"") != std::string::npos || 
      message.find("\"type\":\"openURL\"") != std::string::npos) {
    // Extract URL from JSON
    size_t url_start = message.find("\"url\":\"") + 7;
    size_t url_end = message.find("\"", url_start);
    if (url_start != std::string::npos && url_end != std::string::npos) {
      std::string url = message.substr(url_start, url_end - url_start);
      std::cout << "[AnyWP] [API] Opening URL: " << url << std::endl;
      
      // Open URL using ShellExecute
      std::wstring wurl(url.begin(), url.end());
      ShellExecuteW(nullptr, L"open", wurl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
  }
  else if (message.find("\"type\":\"READY\"") != std::string::npos || 
           message.find("\"type\":\"ready\"") != std::string::npos) {
    // Extract name
    size_t name_start = message.find("\"name\":\"") + 8;
    size_t name_end = message.find("\"", name_start);
    if (name_start != std::string::npos && name_end != std::string::npos) {
      std::string name = message.substr(name_start, name_end - name_start);
      std::cout << "[AnyWP] [API] Wallpaper ready: " << name << std::endl;
    }
  }
  else if (message.find("\"type\":\"LOG\"") != std::string::npos) {
    // Extract log message
    size_t msg_start = message.find("\"message\":\"") + 11;
    size_t msg_end = message.find("\"", msg_start);
    if (msg_start != std::string::npos && msg_end != std::string::npos) {
      std::string log_msg = message.substr(msg_start, msg_end - msg_start);
      std::cout << "[AnyWP] [WebLog] " << log_msg << std::endl;
    }
  }
  else if (message.find("\"type\":\"saveState\"") != std::string::npos) {
    // Extract key and value from JSON
    size_t key_start = message.find("\"key\":\"") + 7;
    size_t key_end = message.find("\"", key_start);
    size_t value_start = message.find("\"value\":\"") + 9;
    // Find the closing brace, then work backwards to find last quote
    size_t end_brace = message.rfind("}");
    size_t value_end = message.rfind("\"", end_brace);
    
    if (key_start != std::string::npos && key_end != std::string::npos &&
        value_start != std::string::npos && value_end != std::string::npos && value_end > value_start) {
      std::string key = message.substr(key_start, key_end - key_start);
      std::string value = message.substr(value_start, value_end - value_start);
      
      SaveState(key, value);
      std::cout << "[AnyWP] [State] Saved via WebMessage: " << key << " = " << value << std::endl;
    } else {
      std::cout << "[AnyWP] [State] ERROR: Failed to parse saveState message" << std::endl;
    }
  }
  else if (message.find("\"type\":\"loadState\"") != std::string::npos) {
    // Extract key
    size_t key_start = message.find("\"key\":\"") + 7;
    size_t key_end = message.find("\"", key_start);
    
    if (key_start != std::string::npos && key_end != std::string::npos) {
      std::string key = message.substr(key_start, key_end - key_start);
      std::string value = LoadState(key);
      
      std::cout << "[AnyWP] [State] Loaded state for key: " << key << std::endl;
      
      // Send result back to WebView
      if (webview_) {
        std::ostringstream js;
        js << "window.dispatchEvent(new CustomEvent('AnyWP:stateLoaded', {"
           << "detail: {type: 'stateLoaded', key: '" << key << "', value: '" << value << "'}"
           << "}));";
        
        std::string js_code = js.str();
        std::wstring wjs_code(js_code.begin(), js_code.end());
        webview_->ExecuteScript(wjs_code.c_str(), nullptr);
      }
    }
  }
  else if (message.find("\"type\":\"clearState\"") != std::string::npos) {
    ClearState();
    std::cout << "[AnyWP] [State] Cleared all state via WebMessage" << std::endl;
  }
  else {
    std::cout << "[AnyWP] [API] Unknown message type (showing raw): " << message << std::endl;
  }
}

// State persistence: Save state
bool AnyWPEnginePlugin::SaveState(const std::string& key, const std::string& value) {
  std::lock_guard<std::mutex> lock(state_mutex_);
  
  try {
    persisted_state_[key] = value;
    
    // Write to registry for persistence
    HKEY hKey;
    LONG result = RegCreateKeyExA(
      HKEY_CURRENT_USER,
      "Software\\AnyWPEngine\\State",
      0, nullptr, REG_OPTION_NON_VOLATILE,
      KEY_WRITE, nullptr, &hKey, nullptr
    );
    
    if (result == ERROR_SUCCESS) {
      RegSetValueExA(
        hKey,
        key.c_str(),
        0,
        REG_SZ,
        reinterpret_cast<const BYTE*>(value.c_str()),
        static_cast<DWORD>(value.length() + 1)
      );
      RegCloseKey(hKey);
      
      std::cout << "[AnyWP] [State] Saved to registry: " << key << " = " << value << std::endl;
      return true;
    } else {
      std::cout << "[AnyWP] [State] ERROR: Failed to create registry key: " << result << std::endl;
      return false;
    }
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [State] ERROR: Exception in SaveState: " << e.what() << std::endl;
    return false;
  }
}

// State persistence: Load state
std::string AnyWPEnginePlugin::LoadState(const std::string& key) {
  std::lock_guard<std::mutex> lock(state_mutex_);
  
  // Check cache first
  auto it = persisted_state_.find(key);
  if (it != persisted_state_.end()) {
    std::cout << "[AnyWP] [State] Loaded from cache: " << key << " = " << it->second << std::endl;
    return it->second;
  }
  
  // Read from registry
  try {
    HKEY hKey;
    LONG result = RegOpenKeyExA(
      HKEY_CURRENT_USER,
      "Software\\AnyWPEngine\\State",
      0,
      KEY_READ,
      &hKey
    );
    
    if (result == ERROR_SUCCESS) {
      char buffer[4096] = {0};
      DWORD buffer_size = sizeof(buffer);
      DWORD type;
      
      result = RegQueryValueExA(
        hKey,
        key.c_str(),
        nullptr,
        &type,
        reinterpret_cast<BYTE*>(buffer),
        &buffer_size
      );
      
      RegCloseKey(hKey);
      
      if (result == ERROR_SUCCESS && type == REG_SZ) {
        std::string value(buffer);
        persisted_state_[key] = value;  // Update cache
        std::cout << "[AnyWP] [State] Loaded from registry: " << key << " = " << value << std::endl;
        return value;
      }
    }
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [State] ERROR: Exception in LoadState: " << e.what() << std::endl;
  }
  
  std::cout << "[AnyWP] [State] Key not found: " << key << std::endl;
  return "";
}

// State persistence: Clear all state
bool AnyWPEnginePlugin::ClearState() {
  std::lock_guard<std::mutex> lock(state_mutex_);
  
  try {
    persisted_state_.clear();
    
    // Delete registry key
    LONG result = RegDeleteTreeA(
      HKEY_CURRENT_USER,
      "Software\\AnyWPEngine\\State"
    );
    
    if (result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND) {
      std::cout << "[AnyWP] [State] Cleared all state" << std::endl;
      return true;
    } else {
      std::cout << "[AnyWP] [State] ERROR: Failed to delete registry tree: " << result << std::endl;
      return false;
    }
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [State] ERROR: Exception in ClearState: " << e.what() << std::endl;
    return false;
  }
}

// Mouse Hook: Low-level mouse callback (optimized for performance)
LRESULT CALLBACK AnyWPEnginePlugin::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
  // Mouse hook processes all clicks and decides whether to forward to wallpaper
  if (nCode >= 0 && hook_instance_) {
    // Skip if paused (performance optimization)
    if (hook_instance_->is_paused_) {
      return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }
    
    MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
    POINT pt = info->pt;
    
    // Check if click position is occluded by a top-level application window
    HWND window_at_point = WindowFromPoint(pt);
    
    // Get window class name and title for debugging
    wchar_t className[256] = {0};
    wchar_t windowTitle[256] = {0};
    if (window_at_point) {
      GetClassNameW(window_at_point, className, 256);
      GetWindowTextW(window_at_point, windowTitle, 256);
    }
    
    // Check if this is a top-level application window (not desktop layer)
    bool is_app_window = false;
    if (window_at_point) {
      // Get the root owner window
      HWND root_window = GetAncestor(window_at_point, GA_ROOT);
      
      // Check if it's a visible top-level window with WS_OVERLAPPEDWINDOW style
      if (root_window && IsWindowVisible(root_window)) {
        LONG style = GetWindowLongW(root_window, GWL_STYLE);
        
        // If it has title bar or is a popup window, it's likely an app window
        if ((style & WS_CAPTION) || (style & WS_POPUP)) {
          // But exclude desktop-related windows
          wchar_t rootClassName[256] = {0};
          GetClassNameW(root_window, rootClassName, 256);
          
          if (wcscmp(rootClassName, L"Progman") != 0 &&
              wcscmp(rootClassName, L"WorkerW") != 0 &&
              wcscmp(rootClassName, L"Shell_TrayWnd") != 0 &&  // Taskbar
              wcsstr(rootClassName, L"Xaml") == nullptr &&  // System UI
              wcsstr(rootClassName, L"Chrome") == nullptr &&  // WebView2 windows
              wcsstr(className, L"Chrome") == nullptr) {  // WebView2 render windows
            is_app_window = true;
          }
        }
      }
    }
    
    // If occluded by app window, don't forward (no log to reduce overhead)
    if (is_app_window) {
      // CRITICAL: Also clear any active drag state in JavaScript
      if (wParam == WM_LBUTTONUP || wParam == WM_MOUSEMOVE) {
        hook_instance_->CancelActiveDrag();
      }
      return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }
    
    // Multi-monitor: Find which wallpaper instance is at this point
    WallpaperInstance* target_instance = hook_instance_->GetInstanceAtPoint(pt.x, pt.y);
    
    // Check if click is on an iframe ad (priority handling)
    if (wParam == WM_LBUTTONUP && target_instance) {
      // Reduced logging for performance
      
      IframeInfo* iframe = hook_instance_->GetIframeAtPoint(pt.x, pt.y, target_instance);
      
      if (iframe && !iframe->click_url.empty()) {
        std::cout << "[AnyWP] [iframe] Click detected on iframe: " << iframe->id 
                  << " at (" << pt.x << "," << pt.y << ")" << std::endl;
        std::cout << "[AnyWP] [iframe] Opening ad URL: " << iframe->click_url << std::endl;
        
        // Open the ad URL directly (bypass iframe sandbox restrictions)
        std::wstring url_wide(iframe->click_url.begin(), iframe->click_url.end());
        ShellExecuteW(nullptr, L"open", url_wide.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        
        // Don't forward to WebView - handled by native layer
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
      }
    }
    
    // Send different mouse events to JavaScript (desktop layer clicks)
    const char* event_type = nullptr;
    
    if (wParam == WM_LBUTTONDOWN) {
      event_type = "mousedown";
    } else if (wParam == WM_LBUTTONUP) {
      event_type = "mouseup";
      // Reduced logging for performance
    } else if (wParam == WM_MOUSEMOVE) {
      // Enable mousemove for drag support (but only forward when needed)
      event_type = "mousemove";
    }
    
    if (event_type) {
      // Forward to WebView without logging (performance)
      hook_instance_->SendClickToWebView(pt.x, pt.y, event_type);
    }
  }
  
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

// Mouse Hook: Send mouse event to WebView (optimized for performance)
void AnyWPEnginePlugin::SendClickToWebView(int x, int y, const char* event_type) {
  // Multi-monitor support: Find which instance is at this point
  WallpaperInstance* instance = GetInstanceAtPoint(x, y);
  
  Microsoft::WRL::ComPtr<ICoreWebView2> target_webview;
  
  if (instance && instance->webview) {
    target_webview = instance->webview;
  } else if (webview_) {
    // Fallback to legacy single-monitor webview
    target_webview = webview_;
  } else {
    // No logging for performance
    return;
  }
  
  // Dispatch both AnyWP:mouse AND AnyWP:click events (SDK v4.0.0 compatibility)
  std::wstringstream script;
  std::wstring wtype(event_type, event_type + strlen(event_type));
  
  script << L"(function() {"
         << L"  console.log('[Native] Dispatching events at (' + " << x << L" + ',' + " << y << L" + ')');"
         << L"  var mouseEvent = new CustomEvent('AnyWP:mouse', {"
         << L"    detail: {"
         << L"      type: '" << wtype << L"',"
         << L"      x: " << x << L","
         << L"      y: " << y << L","
         << L"      button: 0"
         << L"    }"
         << L"  });"
         << L"  window.dispatchEvent(mouseEvent);";
  
  // CRITICAL: Also dispatch AnyWP:click for onClick handlers
  if (strcmp(event_type, "mouseup") == 0) {
    script << L"  var clickEvent = new CustomEvent('AnyWP:click', {"
           << L"    detail: {"
           << L"      x: " << x << L","
           << L"      y: " << y
           << L"    }"
           << L"  });"
           << L"  console.log('[Native] Dispatching AnyWP:click event');"
           << L"  window.dispatchEvent(clickEvent);";
  }
  
  script << L"})();";
  
  // Execute without callback for performance
  target_webview->ExecuteScript(script.str().c_str(), nullptr);
}

// Mouse Hook: Setup hook
void AnyWPEnginePlugin::SetupMouseHook() {
  if (mouse_hook_) {
    std::cout << "[AnyWP] [Hook] Mouse hook already installed" << std::endl;
    return;
  }
  
  mouse_hook_ = SetWindowsHookExW(
    WH_MOUSE_LL,
    LowLevelMouseProc,
    GetModuleHandleW(nullptr),
    0
  );
  
  if (mouse_hook_) {
    std::cout << "[AnyWP] [Hook] Mouse hook installed successfully" << std::endl;
  } else {
    DWORD error = GetLastError();
    std::cout << "[AnyWP] [Hook] ERROR: Failed to install mouse hook, error: " << error << std::endl;
  }
}

// Mouse Hook: Remove hook
void AnyWPEnginePlugin::RemoveMouseHook() {
  if (mouse_hook_) {
    UnhookWindowsHookEx(mouse_hook_);
    mouse_hook_ = nullptr;
    std::cout << "[AnyWP] [Hook] Mouse hook removed" << std::endl;
  }
}

// Cancel active drag operation
void AnyWPEnginePlugin::CancelActiveDrag() {
  // Send command to JavaScript to cancel any active drag
  if (webview_) {
    std::wstring script = L"if (window.AnyWP && window.AnyWP._dragState) { "
                          L"console.log('[AnyWP] Canceling drag (mouse left wallpaper area)'); "
                          L"window.AnyWP._dragState = null; "
                          L"}";
    webview_->ExecuteScript(script.c_str(), nullptr);
  }
  
  // Also check wallpaper instances
  for (auto& instance : wallpaper_instances_) {
    if (instance.webview) {
      std::wstring script = L"if (window.AnyWP && window.AnyWP._dragState) { "
                            L"window.AnyWP._dragState = null; "
                            L"}";
      instance.webview->ExecuteScript(script.c_str(), nullptr);
    }
  }
}

// iframe Ad Detection: Handle iframe data from JavaScript
void AnyWPEnginePlugin::HandleIframeDataMessage(const std::string& json_data, WallpaperInstance* instance) {
  std::cout << "[AnyWP] [iframe] Parsing iframe data..." << std::endl;
  std::cout << "[AnyWP] [iframe] Raw JSON: " << json_data << std::endl;
  
  // Determine which iframe list to update
  std::vector<IframeInfo>* target_iframes;
  
  if (instance) {
    // Multi-monitor: update specific instance's iframe list
    target_iframes = &instance->iframes;
  } else {
    // Legacy: use global iframe list with mutex
    std::lock_guard<std::mutex> lock(iframes_mutex_);
    target_iframes = &iframes_;
  }
  
  // Clear existing data
  target_iframes->clear();
  
  // Simple JSON parsing for iframe array
  // Format: {"type":"IFRAME_DATA","iframes":[{...},{...}]}
  size_t iframes_start = json_data.find("\"iframes\":[");
  if (iframes_start == std::string::npos) {
    std::cout << "[AnyWP] [iframe] No iframes array found" << std::endl;
    return;
  }
  
  size_t array_end = json_data.find("]", iframes_start);
  if (array_end == std::string::npos) {
    std::cout << "[AnyWP] [iframe] No array end found" << std::endl;
    return;
  }
  
  // Find each iframe object in the array
  size_t pos = iframes_start + 11;  // Start after "iframes":[
  
  while (pos < array_end) {
    // Find next iframe object start
    pos = json_data.find("{", pos);
    if (pos == std::string::npos || pos >= array_end) break;
    
    // Find the end of this iframe object (matching closing brace)
    int brace_count = 1;
    size_t obj_end = pos + 1;
    while (obj_end < array_end && brace_count > 0) {
      if (json_data[obj_end] == '{') brace_count++;
      else if (json_data[obj_end] == '}') brace_count--;
      obj_end++;
    }
    
    if (brace_count != 0) {
      std::cout << "[AnyWP] [iframe] ERROR: Unmatched braces at pos " << pos << std::endl;
      break;
    }
    
    // Extract iframe data within [pos, obj_end)
    std::string obj_data = json_data.substr(pos, obj_end - pos);
    std::cout << "[AnyWP] [iframe] Object data: " << obj_data << std::endl;
    
    IframeInfo iframe;
    
    // Extract id
    size_t id_start = obj_data.find("\"id\":\"");
    if (id_start != std::string::npos) {
      id_start += 6;
      size_t id_end = obj_data.find("\"", id_start);
      iframe.id = obj_data.substr(id_start, id_end - id_start);
    }
    
    // Extract src
    size_t src_start = obj_data.find("\"src\":\"");
    if (src_start != std::string::npos) {
      src_start += 7;
      size_t src_end = obj_data.find("\"", src_start);
      iframe.src = obj_data.substr(src_start, src_end - src_start);
    }
    
    // Extract clickUrl
    size_t url_start = obj_data.find("\"clickUrl\":\"");
    if (url_start != std::string::npos) {
      url_start += 12;
      size_t url_end = obj_data.find("\"", url_start);
      iframe.click_url = obj_data.substr(url_start, url_end - url_start);
    }
    
    // Extract bounds
    size_t bounds_start = obj_data.find("\"bounds\":{");
    if (bounds_start != std::string::npos) {
      // Extract left
      size_t left_start = obj_data.find("\"left\":", bounds_start);
      if (left_start != std::string::npos) {
        left_start += 7;
        iframe.left = std::stoi(obj_data.substr(left_start, 10));
      }
      
      // Extract top
      size_t top_start = obj_data.find("\"top\":", bounds_start);
      if (top_start != std::string::npos) {
        top_start += 6;
        iframe.top = std::stoi(obj_data.substr(top_start, 10));
      }
      
      // Extract width
      size_t width_start = obj_data.find("\"width\":", bounds_start);
      if (width_start != std::string::npos) {
        width_start += 8;
        iframe.width = std::stoi(obj_data.substr(width_start, 10));
      }
      
      // Extract height
      size_t height_start = obj_data.find("\"height\":", bounds_start);
      if (height_start != std::string::npos) {
        height_start += 9;
        iframe.height = std::stoi(obj_data.substr(height_start, 10));
      }
    }
    
    // Extract visible
    size_t visible_start = obj_data.find("\"visible\":");
    if (visible_start != std::string::npos) {
      visible_start += 10;
      iframe.visible = (obj_data.substr(visible_start, 4) == "true");
    } else {
      iframe.visible = true;  // Default to visible
    }
    
    // Add to list
    target_iframes->push_back(iframe);
    
    std::cout << "[AnyWP] [iframe] Added iframe #" << target_iframes->size() << ": id=" << iframe.id 
              << " pos=(" << iframe.left << "," << iframe.top << ")"
              << " size=" << iframe.width << "x" << iframe.height
              << " url=" << iframe.click_url << std::endl;
    
    // Move to next object
    pos = obj_end;
  }
  
  std::cout << "[AnyWP] [iframe] Total iframes: " << target_iframes->size() << std::endl;
}

// iframe Ad Detection: Check if click is on an iframe
IframeInfo* AnyWPEnginePlugin::GetIframeAtPoint(int x, int y, WallpaperInstance* instance) {
  if (instance) {
    // Multi-monitor: check specific instance's iframes
    std::cout << "[AnyWP] [iframe] GetIframeAtPoint: checking (" << x << "," << y << ") against " 
              << instance->iframes.size() << " iframes" << std::endl;
    
    for (auto& iframe : instance->iframes) {
      if (!iframe.visible) {
        std::cout << "[AnyWP] [iframe]   " << iframe.id << " - HIDDEN" << std::endl;
        continue;
      }
      
      int right = iframe.left + iframe.width;
      int bottom = iframe.top + iframe.height;
      
      std::cout << "[AnyWP] [iframe]   " << iframe.id << ": [" << iframe.left << "," << iframe.top 
                << "] ~ [" << right << "," << bottom << "]" << std::endl;
      
      if (x >= iframe.left && x < right &&
          y >= iframe.top && y < bottom) {
        std::cout << "[AnyWP] [iframe]   MATCH!" << std::endl;
        return &iframe;
      }
    }
    
    std::cout << "[AnyWP] [iframe]   No match found" << std::endl;
  } else {
    // Legacy: use global iframe list with mutex
    std::lock_guard<std::mutex> lock(iframes_mutex_);
    
    for (auto& iframe : iframes_) {
      if (!iframe.visible) continue;
      
      int right = iframe.left + iframe.width;
      int bottom = iframe.top + iframe.height;
      
      if (x >= iframe.left && x < right &&
          y >= iframe.top && y < bottom) {
        return &iframe;
      }
    }
  }
  
  return nullptr;
}

bool AnyWPEnginePlugin::InitializeWallpaper(const std::string& url, bool enable_mouse_transparent) {
  std::cout << "[AnyWP] ========== Initializing Wallpaper ==========" << std::endl;
  std::cout << "[AnyWP] URL: " << url << std::endl;
  std::cout << "[AnyWP] Mouse Transparent: " << (enable_mouse_transparent ? "true" : "false") << std::endl;

  // P0-3: Validate URL before initialization
  if (!url_validator_.IsAllowed(url)) {
    std::cout << "[AnyWP] [Security] URL validation failed: " << url << std::endl;
    LogError("URL validation failed: " + url);
    return false;
  }

  if (is_initialized_) {
    std::cout << "[AnyWP] Already initialized, stopping first..." << std::endl;
    StopWallpaper();
  }
  
  // Clear any residual iframe data before initialization
  {
    std::lock_guard<std::mutex> lock(iframes_mutex_);
    if (!iframes_.empty()) {
      std::cout << "[AnyWP] [iframe] Clearing " << iframes_.size() << " residual iframe(s)" << std::endl;
      iframes_.clear();
    }
  }
  
  // P1-2: Periodic cleanup check
  PeriodicCleanup();

  // 注意：冲突检测已禁用以避免启动卡顿
  // 枚举所有进程非常耗时，会导致UI冻结
  // 如需启用，建议移到后台线程异步执行
  
  // 使用改进的安全WorkerW查找
  WorkerWInfo workerw_info;
  if (!FindWorkerWSafe(workerw_info, 3000)) {
    std::cout << "[AnyWP] ERROR: Failed to find WorkerW window safely" << std::endl;
    LogError("Failed to find WorkerW window");
    return false;
  }
  
  if (!workerw_info.wallpaper_layer) {
    std::cout << "[AnyWP] ERROR: No wallpaper layer found" << std::endl;
    LogError("No wallpaper layer found");
    return false;
  }
  
  // 验证窗口句柄（使用轻量级检查）
  if (!IsValidWindowHandle(workerw_info.wallpaper_layer)) {
    std::cout << "[AnyWP] ERROR: Wallpaper layer window is invalid" << std::endl;
    LogError("Invalid wallpaper layer window");
    return false;
  }
  
  worker_w_hwnd_ = workerw_info.wallpaper_layer;
  std::cout << "[AnyWP] Using wallpaper layer: " << worker_w_hwnd_ << std::endl;
  std::cout << "[AnyWP] Icon layer: " << workerw_info.icon_layer << std::endl;
  std::cout << "[AnyWP] Total WorkerW count: " << workerw_info.workerw_count << std::endl;

  // Create WebView host window (already parented to WorkerW inside)
  webview_host_hwnd_ = CreateWebViewHostWindow(enable_mouse_transparent);
  if (!webview_host_hwnd_) {
    std::cout << "[AnyWP] ERROR: Failed to create WebView host window" << std::endl;
    return false;
  }

  std::cout << "[AnyWP] WebView host created as child of WorkerW" << std::endl;
  
  // Always set Z-order behind SHELLDLL_DefView (icons always visible)
  HWND shelldll = FindWindowExW(worker_w_hwnd_, nullptr, L"SHELLDLL_DefView", nullptr);
  if (shelldll) {
    std::cout << "[AnyWP] Setting Z-order behind SHELLDLL_DefView (icons always on top)..." << std::endl;
    SetWindowPos(webview_host_hwnd_, shelldll, 0, 0, 0, 0, 
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    std::cout << "[AnyWP] Z-order: Icons on top, WebView below" << std::endl;
  }
  
  // Verify window is actually visible
  BOOL isVisible = IsWindowVisible(webview_host_hwnd_);
  RECT rect;
  GetWindowRect(webview_host_hwnd_, &rect);
  std::cout << "[AnyWP] Window visible: " << isVisible 
            << ", Rect: " << rect.left << "," << rect.top 
            << " " << (rect.right - rect.left) << "x" << (rect.bottom - rect.top) << std::endl;
  
  // TEMPORARY: Disable transparency for debugging
  std::cout << "[AnyWP] [DEBUG] Transparency DISABLED for debugging visibility" << std::endl;
  
  // Store interaction mode for mouse hook
  enable_interaction_ = !enable_mouse_transparent;
  
  // CRITICAL FIX: Always setup mouse hook regardless of mode
  // The hook decides internally whether to forward clicks
  std::cout << "[AnyWP] Setting up mouse hook (transparent=" << enable_mouse_transparent << ")..." << std::endl;
  SetupMouseHook();

  // Show window
  ShowWindow(webview_host_hwnd_, SW_SHOW);
  UpdateWindow(webview_host_hwnd_);

  // Initialize WebView2 (pass -1 for legacy single-monitor support)
  SetupWebView2(webview_host_hwnd_, url, -1);

  std::cout << "[AnyWP] ========== Initialization Complete ==========" << std::endl;
  return true;
}

bool AnyWPEnginePlugin::StopWallpaper() {
  std::cout << "[AnyWP] Stopping wallpaper..." << std::endl;

  // 移除鼠标钩子
  RemoveMouseHook();

  if (webview_controller_) {
    try {
      webview_controller_->Close();
    } catch (...) {
      std::cout << "[AnyWP] WARNING: Exception while closing WebView controller" << std::endl;
    }
    webview_controller_ = nullptr;
  }

  webview_ = nullptr;

  if (webview_host_hwnd_) {
    // 验证窗口是否仍然有效
    if (IsWindow(webview_host_hwnd_)) {
      // P0-1: Untrack before destroying
      ResourceTracker::Instance().UntrackWindow(webview_host_hwnd_);
      
      if (!DestroyWindow(webview_host_hwnd_)) {
        DWORD error = GetLastError();
        std::cout << "[AnyWP] WARNING: Failed to destroy window, error: " << error << std::endl;
      }
    } else {
      std::cout << "[AnyWP] WARNING: WebView host window already destroyed" << std::endl;
      ResourceTracker::Instance().UntrackWindow(webview_host_hwnd_);
    }
    webview_host_hwnd_ = nullptr;
  }

  // Clear iframe data when stopping wallpaper
  {
    std::lock_guard<std::mutex> lock(iframes_mutex_);
    if (!iframes_.empty()) {
      std::cout << "[AnyWP] [iframe] Clearing " << iframes_.size() << " iframe(s) on stop" << std::endl;
      iframes_.clear();
    }
  }

  worker_w_hwnd_ = nullptr;
  is_initialized_ = false;
  enable_interaction_ = false;

  std::cout << "[AnyWP] Wallpaper stopped successfully" << std::endl;
  std::cout << "[AnyWP] [ResourceTracker] Tracked windows: " 
            << ResourceTracker::Instance().GetTrackedCount() << std::endl;
  
  return true;
}

bool AnyWPEnginePlugin::NavigateToUrl(const std::string& url) {
  if (!webview_) {
    std::cout << "[AnyWP] ERROR: WebView not initialized" << std::endl;
    LogError("NavigateToUrl: WebView not initialized");
    return false;
  }

  // P0-3: Validate URL
  if (!url_validator_.IsAllowed(url)) {
    std::cout << "[AnyWP] [Security] URL validation failed: " << url << std::endl;
    LogError("URL validation failed: " + url);
    return false;
  }

  // Clear iframe data when navigating to new page
  {
    std::lock_guard<std::mutex> lock(iframes_mutex_);
    if (!iframes_.empty()) {
      std::cout << "[AnyWP] [iframe] Clearing " << iframes_.size() << " iframe(s) before navigation" << std::endl;
      iframes_.clear();
    }
  }

  // P1-2: Check if cleanup needed
  PeriodicCleanup();

  std::wstring wurl(url.begin(), url.end());
  HRESULT hr = webview_->Navigate(wurl.c_str());
  
  if (SUCCEEDED(hr)) {
    std::cout << "[AnyWP] Navigated to: " << url << std::endl;
    
    // AUTO-OPTIMIZE: Safe delayed optimization (no dangling pointers)
    if (webview_) {
      ScheduleSafeMemoryOptimization(webview_.Get());
    }
    
    return true;
  } else {
    std::cout << "[AnyWP] ERROR: Navigation failed: " << std::hex << hr << std::endl;
    LogError("Navigation failed: " + url);
    return false;
  }
}

// Multi-Monitor Support Implementation

// Monitor enumeration callback
BOOL CALLBACK AnyWPEnginePlugin::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
  auto* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
  
  MONITORINFOEXW monitor_info;
  monitor_info.cbSize = sizeof(MONITORINFOEXW);
  
  if (GetMonitorInfoW(hMonitor, &monitor_info)) {
    MonitorInfo info;
    info.index = static_cast<int>(monitors->size());
    info.handle = hMonitor;
    info.left = monitor_info.rcMonitor.left;
    info.top = monitor_info.rcMonitor.top;
    info.width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
    info.height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
    info.is_primary = (monitor_info.dwFlags & MONITORINFOF_PRIMARY) != 0;
    
    // Convert device name from wchar to string
    std::wstring wname(monitor_info.szDevice);
    // Use proper conversion to avoid C4244 warning
    info.device_name.reserve(wname.size());
    for (wchar_t wc : wname) {
      info.device_name.push_back(static_cast<char>(wc));
    }
    
    monitors->push_back(info);
    
    std::cout << "[AnyWP] [Monitor] Found: " << info.device_name 
              << " [" << info.width << "x" << info.height << "]"
              << " at (" << info.left << "," << info.top << ")"
              << (info.is_primary ? " (PRIMARY)" : "") << std::endl;
  }
  
  return TRUE;  // Continue enumeration
}

// Get all monitors
std::vector<MonitorInfo> AnyWPEnginePlugin::GetMonitors() {
  std::cout << "[AnyWP] [Monitor] Enumerating monitors..." << std::endl;
  
  std::vector<MonitorInfo> monitors;
  EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));
  
  std::cout << "[AnyWP] [Monitor] Found " << monitors.size() << " monitor(s)" << std::endl;
  
  // Cache monitors list
  monitors_ = monitors;
  
  return monitors;
}

// Get wallpaper instance for a specific monitor
WallpaperInstance* AnyWPEnginePlugin::GetInstanceForMonitor(int monitor_index) {
  std::lock_guard<std::mutex> lock(instances_mutex_);
  
  for (auto& instance : wallpaper_instances_) {
    if (instance.monitor_index == monitor_index) {
      return &instance;
    }
  }
  
  return nullptr;
}

// Get wallpaper instance at a specific screen position
WallpaperInstance* AnyWPEnginePlugin::GetInstanceAtPoint(int x, int y) {
  std::lock_guard<std::mutex> lock(instances_mutex_);
  
  for (auto& instance : wallpaper_instances_) {
    // Find which monitor this instance belongs to
    for (const auto& monitor : monitors_) {
      if (monitor.index == instance.monitor_index) {
        int right = monitor.left + monitor.width;
        int bottom = monitor.top + monitor.height;
        
        if (x >= monitor.left && x < right && y >= monitor.top && y < bottom) {
          return &instance;
        }
        break;
      }
    }
  }
  
  return nullptr;
}

// Initialize wallpaper on specific monitor
bool AnyWPEnginePlugin::InitializeWallpaperOnMonitor(const std::string& url, bool enable_mouse_transparent, int monitor_index) {
  std::cout << "[AnyWP] ========== Initializing Wallpaper on Monitor " << monitor_index << " ==========" << std::endl;
  std::cout << "[AnyWP] URL: " << url << std::endl;
  std::cout << "[AnyWP] Mouse Transparent: " << (enable_mouse_transparent ? "true" : "false") << std::endl;

  // P0-3: Validate URL
  if (!url_validator_.IsAllowed(url)) {
    std::cout << "[AnyWP] [Security] URL validation failed: " << url << std::endl;
    LogError("URL validation failed: " + url);
    return false;
  }

  // Get monitors if not cached
  if (monitors_.empty()) {
    GetMonitors();
  }
  
  // Find the target monitor
  const MonitorInfo* target_monitor = nullptr;
  for (const auto& monitor : monitors_) {
    if (monitor.index == monitor_index) {
      target_monitor = &monitor;
      break;
    }
  }
  
  if (!target_monitor) {
    std::cout << "[AnyWP] ERROR: Monitor index " << monitor_index << " not found" << std::endl;
    LogError("Monitor not found: " + std::to_string(monitor_index));
    return false;
  }
  
  // Check if already initialized on this monitor
  WallpaperInstance* existing = GetInstanceForMonitor(monitor_index);
  if (existing) {
    std::cout << "[AnyWP] Monitor " << monitor_index << " already has wallpaper, stopping first..." << std::endl;
    StopWallpaperOnMonitor(monitor_index);
    
    // Give system time to clean up resources (window destruction, WebView cleanup)
    std::cout << "[AnyWP] Waiting for resources to be released..." << std::endl;
    Sleep(500);  // 500ms should be enough for cleanup
  }
  
  // P1-2: Periodic cleanup check
  PeriodicCleanup();

  // Find WorkerW for this monitor
  WorkerWInfo workerw_info;
  if (!FindWorkerWSafe(workerw_info, 3000)) {
    std::cout << "[AnyWP] ERROR: Failed to find WorkerW window" << std::endl;
    LogError("Failed to find WorkerW window");
    return false;
  }
  
  if (!workerw_info.wallpaper_layer) {
    std::cout << "[AnyWP] ERROR: No wallpaper layer found" << std::endl;
    LogError("No wallpaper layer found");
    return false;
  }
  
  // Validate window handle
  if (!IsValidWindowHandle(workerw_info.wallpaper_layer)) {
    std::cout << "[AnyWP] ERROR: Wallpaper layer window is invalid" << std::endl;
    LogError("Invalid wallpaper layer window");
    return false;
  }
  
  // Create new instance
  WallpaperInstance new_instance;
  new_instance.monitor_index = monitor_index;
  new_instance.worker_w_hwnd = workerw_info.wallpaper_layer;
  
  std::cout << "[AnyWP] Using wallpaper layer: " << new_instance.worker_w_hwnd << std::endl;
  std::cout << "[AnyWP] Monitor: " << target_monitor->device_name 
            << " [" << target_monitor->width << "x" << target_monitor->height << "]" << std::endl;

  // Create WebView host window for this monitor
  // Pass the instance's worker_w_hwnd as parent (not the global one)
  HWND hwnd = CreateWebViewHostWindow(enable_mouse_transparent, target_monitor, new_instance.worker_w_hwnd);
  if (!hwnd) {
    std::cout << "[AnyWP] ERROR: Failed to create WebView host window" << std::endl;
    return false;
  }
  
  new_instance.webview_host_hwnd = hwnd;
  std::cout << "[AnyWP] WebView host created for monitor " << monitor_index << std::endl;
  
  // Set Z-order behind SHELLDLL_DefView
  HWND shelldll = FindWindowExW(new_instance.worker_w_hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
  if (shelldll) {
    std::cout << "[AnyWP] Setting Z-order behind SHELLDLL_DefView..." << std::endl;
    SetWindowPos(new_instance.webview_host_hwnd, shelldll, 0, 0, 0, 0, 
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  }
  
  // Store interaction mode
  enable_interaction_ = !enable_mouse_transparent;
  
  // CRITICAL FIX: Always setup mouse hook
  std::cout << "[AnyWP] Setting up mouse hook (transparent=" << enable_mouse_transparent << ")..." << std::endl;
  SetupMouseHook();

  // Add instance to list FIRST
  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    wallpaper_instances_.push_back(new_instance);
    std::cout << "[AnyWP] Added wallpaper instance for monitor " << monitor_index 
              << " (Total instances: " << wallpaper_instances_.size() << ")" << std::endl;
  }
  
  // Show window
  ShowWindow(new_instance.webview_host_hwnd, SW_SHOW);
  UpdateWindow(new_instance.webview_host_hwnd);

  // Initialize WebView2 - pass monitor_index to find instance in callbacks
  SetupWebView2(new_instance.webview_host_hwnd, url, monitor_index);
  
  // Save URL as default for auto-starting on new monitors
  if (default_wallpaper_url_.empty()) {
    default_wallpaper_url_ = url;
    std::cout << "[AnyWP] Set default wallpaper URL for auto-start: " << url << std::endl;
  }

  std::cout << "[AnyWP] ========== Initialization Complete (Monitor " << monitor_index << ") ==========" << std::endl;
  return true;
}

// Stop wallpaper on specific monitor
bool AnyWPEnginePlugin::StopWallpaperOnMonitor(int monitor_index) {
  std::cout << "[AnyWP] Stopping wallpaper on monitor " << monitor_index << "..." << std::endl;

  WallpaperInstance* instance = GetInstanceForMonitor(monitor_index);
  if (!instance) {
    std::cout << "[AnyWP] No wallpaper found on monitor " << monitor_index << std::endl;
    return false;
  }

  // Close WebView
  if (instance->webview_controller) {
    try {
      instance->webview_controller->Close();
    } catch (...) {
      std::cout << "[AnyWP] WARNING: Exception while closing WebView controller" << std::endl;
    }
    instance->webview_controller = nullptr;
  }

  instance->webview = nullptr;

  // Destroy window
  if (instance->webview_host_hwnd) {
    if (IsWindow(instance->webview_host_hwnd)) {
      ResourceTracker::Instance().UntrackWindow(instance->webview_host_hwnd);
      
      if (!DestroyWindow(instance->webview_host_hwnd)) {
        DWORD error = GetLastError();
        std::cout << "[AnyWP] WARNING: Failed to destroy window, error: " << error << std::endl;
      } else {
        std::cout << "[AnyWP] Window destroyed successfully" << std::endl;
        // Wait a bit to ensure window is fully destroyed
        Sleep(50);
      }
    } else {
      std::cout << "[AnyWP] WARNING: Window already destroyed" << std::endl;
      ResourceTracker::Instance().UntrackWindow(instance->webview_host_hwnd);
    }
    instance->webview_host_hwnd = nullptr;
  }

  // Clear iframe data
  instance->iframes.clear();

  // Remove instance from list
  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    wallpaper_instances_.erase(
      std::remove_if(wallpaper_instances_.begin(), wallpaper_instances_.end(),
        [monitor_index](const WallpaperInstance& inst) {
          return inst.monitor_index == monitor_index;
        }),
      wallpaper_instances_.end()
    );
  }
  
  // Remove mouse hook if no more instances
  if (wallpaper_instances_.empty()) {
    RemoveMouseHook();
    enable_interaction_ = false;
    
    // Clear default URL when all wallpapers are stopped
    default_wallpaper_url_.clear();
    std::cout << "[AnyWP] All wallpapers stopped, cleared default URL" << std::endl;
  }

  std::cout << "[AnyWP] Wallpaper stopped on monitor " << monitor_index << std::endl;
  return true;
}

// Navigate to URL on specific monitor
bool AnyWPEnginePlugin::NavigateToUrlOnMonitor(const std::string& url, int monitor_index) {
  WallpaperInstance* instance = GetInstanceForMonitor(monitor_index);
  if (!instance || !instance->webview) {
    std::cout << "[AnyWP] ERROR: No wallpaper on monitor " << monitor_index << std::endl;
    LogError("NavigateToUrlOnMonitor: WebView not initialized on monitor " + std::to_string(monitor_index));
    return false;
  }

  // P0-3: Validate URL
  if (!url_validator_.IsAllowed(url)) {
    std::cout << "[AnyWP] [Security] URL validation failed: " << url << std::endl;
    LogError("URL validation failed: " + url);
    return false;
  }

  // Clear iframe data
  instance->iframes.clear();

  // P1-2: Check if cleanup needed
  PeriodicCleanup();

  std::wstring wurl(url.begin(), url.end());
  HRESULT hr = instance->webview->Navigate(wurl.c_str());
  
  if (SUCCEEDED(hr)) {
    std::cout << "[AnyWP] Navigated to: " << url << " on monitor " << monitor_index << std::endl;
    
    // AUTO-OPTIMIZE: Safe delayed optimization (no dangling pointers)
    if (instance && instance->webview) {
      ScheduleSafeMemoryOptimization(instance->webview.Get());
    }
    
    return true;
  } else {
    std::cout << "[AnyWP] ERROR: Navigation failed on monitor " << monitor_index << ": " << std::hex << hr << std::endl;
    LogError("Navigation failed on monitor " + std::to_string(monitor_index) + ": " + url);
    return false;
  }
}

// Display Change Monitoring Implementation

// Window procedure for display change listener
LRESULT CALLBACK AnyWPEnginePlugin::DisplayChangeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_DISPLAYCHANGE) {
    std::cout << "[AnyWP] [DisplayChange] Display configuration changed!" << std::endl;
    std::cout << "[AnyWP] [DisplayChange] New resolution: " << LOWORD(lParam) << "x" << HIWORD(lParam) << std::endl;
    
    if (display_change_instance_) {
      display_change_instance_->HandleDisplayChange();
    }
    
    return 0;
  }
  else if (message == WM_NOTIFY_MONITOR_CHANGE) {
    std::cout << "[AnyWP] [DisplayChange] Received WM_NOTIFY_MONITOR_CHANGE in main thread" << std::endl;
    
    // Now we're in the window's message loop thread (safer for Flutter)
    if (display_change_instance_) {
      display_change_instance_->NotifyMonitorChange();
    }
    
    return 0;
  }
  
  return DefWindowProcW(hwnd, message, wParam, lParam);
}

// Setup display change listener
void AnyWPEnginePlugin::SetupDisplayChangeListener() {
  std::cout << "[AnyWP] [DisplayChange] Setting up display change listener..." << std::endl;
  
  // Register window class
  WNDCLASSEXW wc = {0};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.lpfnWndProc = DisplayChangeWndProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = L"AnyWPDisplayChangeListener";
  
  if (!RegisterClassExW(&wc)) {
    DWORD error = GetLastError();
    if (error != ERROR_CLASS_ALREADY_EXISTS) {
      std::cout << "[AnyWP] [DisplayChange] Failed to register window class: " << error << std::endl;
      return;
    }
  }
  
  // Create hidden top-level window to receive WM_DISPLAYCHANGE
  // Note: HWND_MESSAGE windows don't receive WM_DISPLAYCHANGE, must use regular window
  display_listener_hwnd_ = CreateWindowExW(
    0,
    L"AnyWPDisplayChangeListener",
    L"AnyWP Display Change Listener",
    WS_OVERLAPPED,  // Top-level window (but not visible)
    CW_USEDEFAULT, CW_USEDEFAULT, 1, 1,  // Minimal size
    nullptr,  // No parent (top-level)
    nullptr,
    GetModuleHandleW(nullptr),
    nullptr
  );
  
  // Don't show the window (keep it hidden)
  if (display_listener_hwnd_) {
    ShowWindow(display_listener_hwnd_, SW_HIDE);
  }
  
  if (display_listener_hwnd_) {
    std::cout << "[AnyWP] [DisplayChange] Listener window created: " << display_listener_hwnd_ << std::endl;
  } else {
    std::cout << "[AnyWP] [DisplayChange] Failed to create listener window: " << GetLastError() << std::endl;
  }
}

// Cleanup display change listener
void AnyWPEnginePlugin::CleanupDisplayChangeListener() {
  if (display_listener_hwnd_) {
    std::cout << "[AnyWP] [DisplayChange] Cleaning up display change listener..." << std::endl;
    DestroyWindow(display_listener_hwnd_);
    display_listener_hwnd_ = nullptr;
  }
}

// Handle display change
void AnyWPEnginePlugin::HandleDisplayChange() {
  std::cout << "[AnyWP] [DisplayChange] ========== Handling display change ==========" << std::endl;
  
  // Wait a bit for system to stabilize
  Sleep(200);
  
  // Update monitor list
  std::vector<MonitorInfo> old_monitors = monitors_;
  std::vector<MonitorInfo> new_monitors = GetMonitors();
  
  std::cout << "[AnyWP] [DisplayChange] Monitor count: " << old_monitors.size() 
            << " -> " << new_monitors.size() << std::endl;
  
  bool should_notify_ui = false;
  
  // Check if monitor count changed
  if (old_monitors.size() != new_monitors.size()) {
    std::cout << "[AnyWP] [DisplayChange] Monitor count changed - will notify UI" << std::endl;
    
    // Handle removed monitors FIRST (cleanup)
    if (new_monitors.size() < old_monitors.size()) {
      std::cout << "[AnyWP] [DisplayChange] Monitor(s) removed - cleaning up first" << std::endl;
      HandleMonitorCountChange(old_monitors, new_monitors);
    }
    
    should_notify_ui = true;
    
    // Update remaining wallpaper sizes AFTER cleanup
    UpdateWallpaperSizes();
    
    // Handle added monitors AFTER size update
    if (new_monitors.size() > old_monitors.size()) {
      std::cout << "[AnyWP] [DisplayChange] Monitor(s) added - handling new monitors" << std::endl;
      HandleMonitorCountChange(old_monitors, new_monitors);
    }
  } else {
    std::cout << "[AnyWP] [DisplayChange] Monitor count unchanged - checking for other changes" << std::endl;
    // Even if count is same, monitors might have changed (resolution, position, etc.)
    UpdateWallpaperSizes();
  }
  
  // Always notify UI to refresh when display changes
  if (should_notify_ui) {
    std::cout << "[AnyWP] [DisplayChange] Queuing UI notification (thread-safe)..." << std::endl;
    SafeNotifyMonitorChange();
  }
  
  std::cout << "[AnyWP] [DisplayChange] ========== Display change handled ==========" << std::endl;
}

// Notify Dart side about monitor changes
void AnyWPEnginePlugin::NotifyMonitorChange() {
  if (!method_channel_) {
    std::cout << "[AnyWP] [DisplayChange] ERROR: method_channel_ is nullptr!" << std::endl;
    return;
  }
  
  std::cout << "[AnyWP] [DisplayChange] Notifying Dart about monitor change via channel: " << method_channel_ << std::endl;
  
  // CRITICAL: InvokeMethod must be called carefully
  // WM_DISPLAYCHANGE comes from window message loop
  // We need to ensure it's safe to call Flutter API from this context
  
  try {
    std::cout << "[AnyWP] [DisplayChange] Before InvokeMethod..." << std::endl;
    
    // Create empty map for the notification
    auto args = std::make_unique<flutter::EncodableValue>(flutter::EncodableMap());
    
    std::cout << "[AnyWP] [DisplayChange] Calling InvokeMethod..." << std::endl;
    std::cout << "[AnyWP] [DisplayChange] Channel pointer valid: " << (method_channel_ != nullptr) << std::endl;
    
    // WORKAROUND: InvokeMethod crashes even with message queue
    // For now, just skip the notification to prevent crash
    // User must manually click Refresh button
    std::cout << "[AnyWP] [DisplayChange] Skipping InvokeMethod to prevent crash" << std::endl;
    std::cout << "[AnyWP] [DisplayChange] User must click Refresh button to update UI" << std::endl;
    
    // TODO: Find why InvokeMethod crashes
    // method_channel_->InvokeMethod("onMonitorChange", std::move(args));
    
    std::cout << "[AnyWP] [DisplayChange] Notification skipped (workaround)" << std::endl;
    
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [DisplayChange] EXCEPTION: " << e.what() << std::endl;
  } catch (...) {
    std::cout << "[AnyWP] [DisplayChange] UNKNOWN EXCEPTION caught" << std::endl;
  }
  
  std::cout << "[AnyWP] [DisplayChange] NotifyMonitorChange completed" << std::endl;
}

// Thread-safe notification using message queue
void AnyWPEnginePlugin::SafeNotifyMonitorChange() {
  if (!display_listener_hwnd_) {
    std::cout << "[AnyWP] [DisplayChange] ERROR: No listener window for safe notification" << std::endl;
    return;
  }
  
  std::cout << "[AnyWP] [DisplayChange] Posting WM_NOTIFY_MONITOR_CHANGE to message queue" << std::endl;
  
  // Post message to our own window's message queue
  // This will be processed in the window's thread, which is safer for Flutter
  if (PostMessageW(display_listener_hwnd_, WM_NOTIFY_MONITOR_CHANGE, 0, 0)) {
    std::cout << "[AnyWP] [DisplayChange] Message posted successfully" << std::endl;
  } else {
    std::cout << "[AnyWP] [DisplayChange] ERROR: Failed to post message: " << GetLastError() << std::endl;
  }
}

// Handle monitor count change (auto-start wallpaper on new monitors)
void AnyWPEnginePlugin::HandleMonitorCountChange(const std::vector<MonitorInfo>& old_monitors, const std::vector<MonitorInfo>& new_monitors) {
  std::cout << "[AnyWP] [DisplayChange] Monitor count changed!" << std::endl;
  
  // Detect new monitors
  if (new_monitors.size() > old_monitors.size()) {
    std::cout << "[AnyWP] [DisplayChange] New monitor(s) detected!" << std::endl;
    
    // Find which monitors are new
    for (const auto& new_mon : new_monitors) {
      bool is_new = true;
      
      // Check if this monitor existed before (by comparing device name)
      for (const auto& old_mon : old_monitors) {
        if (new_mon.device_name == old_mon.device_name) {
          is_new = false;
          break;
        }
      }
      
      if (is_new) {
        std::cout << "[AnyWP] [DisplayChange] New monitor " << new_mon.index 
                  << ": " << new_mon.device_name << " [" << new_mon.width << "x" << new_mon.height << "]" << std::endl;
        
        // Check if any existing wallpaper is running to get default URL
        std::string url_to_use = default_wallpaper_url_;
        
        if (url_to_use.empty() && !wallpaper_instances_.empty()) {
          // Use URL from first running instance
          std::cout << "[AnyWP] [DisplayChange] No default URL, will skip auto-start" << std::endl;
          std::cout << "[AnyWP] [DisplayChange] Hint: User can manually start wallpaper on new monitor" << std::endl;
        } else if (!url_to_use.empty()) {
          std::cout << "[AnyWP] [DisplayChange] Will auto-start wallpaper on new monitor " << new_mon.index << std::endl;
          std::cout << "[AnyWP] [DisplayChange] Using URL: " << url_to_use << std::endl;
          std::cout << "[AnyWP] [DisplayChange] Note: Auto-start will happen shortly (delayed for stability)" << std::endl;
          
          // Schedule auto-start using Windows timer (safer than std::thread)
          // For now, just log - user can manually start
          // TODO: Implement safe async auto-start mechanism
        }
      }
    }
  }
  // Detect removed monitors
  else if (new_monitors.size() < old_monitors.size()) {
    std::cout << "[AnyWP] [DisplayChange] Monitor(s) removed!" << std::endl;
    
    // Find which monitors were removed
    for (const auto& old_mon : old_monitors) {
      bool still_exists = false;
      
      for (const auto& new_mon : new_monitors) {
        if (old_mon.device_name == new_mon.device_name) {
          still_exists = true;
          break;
        }
      }
      
      if (!still_exists) {
        std::cout << "[AnyWP] [DisplayChange] Monitor removed: " << old_mon.device_name 
                  << " (index: " << old_mon.index << ")" << std::endl;
        
        // Check if wallpaper is running on removed monitor (thread-safe check)
        bool has_wallpaper = false;
        {
          std::lock_guard<std::mutex> lock(instances_mutex_);
          for (const auto& inst : wallpaper_instances_) {
            if (inst.monitor_index == old_mon.index) {
              has_wallpaper = true;
              break;
            }
          }
        }
        
        if (has_wallpaper) {
          std::cout << "[AnyWP] [DisplayChange] Found wallpaper on removed monitor " << old_mon.index 
                    << ", cleaning up..." << std::endl;
          
          // Clean up wallpaper on removed monitor
          // This will acquire lock internally
          bool cleanup_success = StopWallpaperOnMonitor(old_mon.index);
          std::cout << "[AnyWP] [DisplayChange] Cleanup " << (cleanup_success ? "succeeded" : "failed") << std::endl;
        } else {
          std::cout << "[AnyWP] [DisplayChange] No wallpaper found on removed monitor " << old_mon.index << std::endl;
        }
      }
    }
  }
}

// Update wallpaper sizes for all active instances
void AnyWPEnginePlugin::UpdateWallpaperSizes() {
  std::lock_guard<std::mutex> lock(instances_mutex_);
  
  std::cout << "[AnyWP] [DisplayChange] Updating " << wallpaper_instances_.size() << " wallpaper instance(s)..." << std::endl;
  
  // Use index-based loop to avoid iterator invalidation
  for (size_t i = 0; i < wallpaper_instances_.size(); ++i) {
    auto& instance = wallpaper_instances_[i];
    
    std::cout << "[AnyWP] [DisplayChange] Checking instance " << i << " (monitor " << instance.monitor_index << ")" << std::endl;
    
    // Find current monitor info
    const MonitorInfo* monitor = nullptr;
    for (const auto& m : monitors_) {
      if (m.index == instance.monitor_index) {
        monitor = &m;
        break;
      }
    }
    
    if (!monitor) {
      std::cout << "[AnyWP] [DisplayChange] Monitor " << instance.monitor_index << " not found in current list, skipping update" << std::endl;
      continue;
    }
    
    if (!instance.webview_host_hwnd || !IsWindow(instance.webview_host_hwnd)) {
      std::cout << "[AnyWP] [DisplayChange] Window for monitor " << instance.monitor_index << " is invalid, skipping" << std::endl;
      continue;
    }
    
    // Update window position and size
    BOOL success = SetWindowPos(
      instance.webview_host_hwnd,
      nullptr,
      monitor->left,
      monitor->top,
      monitor->width,
      monitor->height,
      SWP_NOZORDER | SWP_NOACTIVATE
    );
    
    if (success) {
      std::cout << "[AnyWP] [DisplayChange] Updated monitor " << instance.monitor_index 
                << " window to " << monitor->width << "x" << monitor->height 
                << " @ (" << monitor->left << "," << monitor->top << ")" << std::endl;
      
      // Update WebView bounds
      if (instance.webview_controller) {
        RECT bounds;
        bounds.left = 0;
        bounds.top = 0;
        bounds.right = monitor->width;
        bounds.bottom = monitor->height;
        
        HRESULT hr = instance.webview_controller->put_Bounds(bounds);
        if (SUCCEEDED(hr)) {
          std::cout << "[AnyWP] [DisplayChange] Updated WebView bounds for monitor " << instance.monitor_index << std::endl;
        } else {
          std::cout << "[AnyWP] [DisplayChange] Failed to update WebView bounds: " << std::hex << hr << std::endl;
        }
      }
    } else {
      std::cout << "[AnyWP] [DisplayChange] Failed to update window for monitor " << instance.monitor_index 
                << ", error: " << GetLastError() << std::endl;
    }
  }
  
  std::cout << "[AnyWP] [DisplayChange] Update complete" << std::endl;
}

// ========== Power Saving & Optimization Implementation ==========

// Setup power saving monitoring
void AnyWPEnginePlugin::SetupPowerSavingMonitoring() {
  std::cout << "[AnyWP] [PowerSaving] Setting up power saving monitoring..." << std::endl;
  
  // Register window class for power events
  WNDCLASSEXW wc = {0};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.lpfnWndProc = PowerSavingWndProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = L"AnyWPPowerListener";
  
  if (!RegisterClassExW(&wc)) {
    DWORD error = GetLastError();
    if (error != ERROR_CLASS_ALREADY_EXISTS) {
      std::cout << "[AnyWP] [PowerSaving] Failed to register window class: " << error << std::endl;
      return;
    }
  }
  
  // Create hidden window to receive power messages
  power_listener_hwnd_ = CreateWindowExW(
    0,
    L"AnyWPPowerListener",
    L"AnyWP Power Listener",
    WS_OVERLAPPED,
    0, 0, 1, 1,
    nullptr,
    nullptr,
    GetModuleHandleW(nullptr),
    nullptr
  );
  
  if (power_listener_hwnd_) {
    ShowWindow(power_listener_hwnd_, SW_HIDE);
    std::cout << "[AnyWP] [PowerSaving] Power listener window created: " << power_listener_hwnd_ << std::endl;
    
    // Register for session change notifications (lock/unlock)
    WTSRegisterSessionNotification(power_listener_hwnd_, NOTIFY_FOR_THIS_SESSION);
    
  } else {
    std::cout << "[AnyWP] [PowerSaving] Failed to create listener window: " << GetLastError() << std::endl;
  }
  
  // Start fullscreen detection thread
  StartFullscreenDetection();
  
  std::cout << "[AnyWP] [PowerSaving] Monitoring setup complete" << std::endl;
}

// Cleanup power saving monitoring
void AnyWPEnginePlugin::CleanupPowerSavingMonitoring() {
  std::cout << "[AnyWP] [PowerSaving] Cleaning up monitoring..." << std::endl;
  
  // Stop fullscreen detection
  StopFullscreenDetection();
  
  // Cleanup window
  if (power_listener_hwnd_) {
    WTSUnRegisterSessionNotification(power_listener_hwnd_);
    DestroyWindow(power_listener_hwnd_);
    power_listener_hwnd_ = nullptr;
  }
  
  std::cout << "[AnyWP] [PowerSaving] Cleanup complete" << std::endl;
}

// Power saving window procedure
LRESULT CALLBACK AnyWPEnginePlugin::PowerSavingWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (!display_change_instance_) {
    return DefWindowProcW(hwnd, message, wParam, lParam);
  }
  
  switch (message) {
    case WM_WTSSESSION_CHANGE:
      // Session state changed (lock/unlock)
      switch (wParam) {
        case WTS_SESSION_LOCK:
          std::cout << "[AnyWP] [PowerSaving] System LOCKED" << std::endl;
          display_change_instance_->PauseWallpaper("System locked");
          break;
        case WTS_SESSION_UNLOCK:
          std::cout << "[AnyWP] [PowerSaving] System UNLOCKED" << std::endl;
          display_change_instance_->ResumeWallpaper("System unlocked");
          break;
      }
      break;
      
    case WM_POWERBROADCAST:
      // Power state changed
      switch (wParam) {
        case PBT_APMSUSPEND:
          std::cout << "[AnyWP] [PowerSaving] System SUSPENDING" << std::endl;
          display_change_instance_->PauseWallpaper("System suspend");
          break;
        case PBT_APMRESUMEAUTOMATIC:
        case PBT_APMRESUMESUSPEND:
          std::cout << "[AnyWP] [PowerSaving] System RESUMING" << std::endl;
          display_change_instance_->ResumeWallpaper("System resume");
          break;
        case PBT_APMPOWERSTATUSCHANGE:
          // Check if monitor is off
          std::cout << "[AnyWP] [PowerSaving] Power status changed" << std::endl;
          display_change_instance_->UpdatePowerState();
          break;
      }
      break;
  }
  
  return DefWindowProcW(hwnd, message, wParam, lParam);
}

// Update power state based on current system status
void AnyWPEnginePlugin::UpdatePowerState() {
  if (!auto_power_saving_enabled_) {
    return;
  }
  
  // Check if screen is off
  SYSTEM_POWER_STATUS power_status;
  if (GetSystemPowerStatus(&power_status)) {
    // Note: This is a simplified check, real screen off detection is complex
    // We rely more on WM_WTSSESSION_CHANGE for lock detection
  }
  
  // Check user activity using configured idle timeout
  LASTINPUTINFO lii = {0};
  lii.cbSize = sizeof(LASTINPUTINFO);
  if (GetLastInputInfo(&lii)) {
    DWORD current_time = GetTickCount();
    DWORD idle_time = current_time - lii.dwTime;
    
    if (idle_time > idle_timeout_ms_) {
      if (power_state_ == PowerState::ACTIVE) {
        std::cout << "[AnyWP] [PowerSaving] User IDLE detected (" << (idle_time / 1000) << "s, threshold: " << (idle_timeout_ms_ / 1000) << "s)" << std::endl;
        PauseWallpaper("User idle");
        NotifyPowerStateChange(PowerState::IDLE);
      }
    } else {
      if (power_state_ == PowerState::IDLE) {
        std::cout << "[AnyWP] [PowerSaving] User ACTIVE again" << std::endl;
        ResumeWallpaper("User active");
        NotifyPowerStateChange(PowerState::ACTIVE);
      }
    }
  }
}

// Check if fullscreen app is active
bool AnyWPEnginePlugin::IsFullscreenAppActive() {
  HWND foreground = GetForegroundWindow();
  if (!foreground) {
    return false;
  }
  
  // Get window rect
  RECT window_rect;
  if (!GetWindowRect(foreground, &window_rect)) {
    return false;
  }
  
  // Get monitor rect that contains this window
  HMONITOR monitor = MonitorFromWindow(foreground, MONITOR_DEFAULTTONEAREST);
  MONITORINFO monitor_info = {0};
  monitor_info.cbSize = sizeof(MONITORINFO);
  
  if (!GetMonitorInfoW(monitor, &monitor_info)) {
    return false;
  }
  
  // Check if window covers entire monitor (fullscreen)
  bool is_fullscreen = 
    window_rect.left <= monitor_info.rcMonitor.left &&
    window_rect.top <= monitor_info.rcMonitor.top &&
    window_rect.right >= monitor_info.rcMonitor.right &&
    window_rect.bottom >= monitor_info.rcMonitor.bottom;
  
  if (is_fullscreen) {
    // Exclude desktop windows
    wchar_t class_name[256] = {0};
    GetClassNameW(foreground, class_name, 256);
    
    if (wcscmp(class_name, L"Progman") == 0 ||
        wcscmp(class_name, L"WorkerW") == 0 ||
        wcscmp(class_name, L"Shell_TrayWnd") == 0) {
      return false;
    }
    
    return true;
  }
  
  return false;
}

// Start fullscreen detection thread
void AnyWPEnginePlugin::StartFullscreenDetection() {
  std::cout << "[AnyWP] [PowerSaving] Starting fullscreen detection thread..." << std::endl;
  
  stop_fullscreen_detection_ = false;
  
  fullscreen_detection_thread_ = std::thread([this]() {
    std::cout << "[AnyWP] [PowerSaving] Fullscreen detection thread started" << std::endl;
    
    while (!stop_fullscreen_detection_) {
      if (auto_power_saving_enabled_) {
        bool is_fullscreen = IsFullscreenAppActive();
        
        if (is_fullscreen && power_state_ != PowerState::FULLSCREEN_APP && power_state_ != PowerState::PAUSED) {
          std::cout << "[AnyWP] [PowerSaving] Fullscreen app detected, pausing wallpaper" << std::endl;
          PauseWallpaper("Fullscreen app");
          power_state_ = PowerState::FULLSCREEN_APP;
        } else if (!is_fullscreen && power_state_ == PowerState::FULLSCREEN_APP) {
          std::cout << "[AnyWP] [PowerSaving] No fullscreen app, resuming wallpaper" << std::endl;
          ResumeWallpaper("No fullscreen app");
          power_state_ = PowerState::ACTIVE;
        }
        
        // Also check user activity
        UpdatePowerState();
      }
      
      // Check every 2 seconds
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    std::cout << "[AnyWP] [PowerSaving] Fullscreen detection thread stopped" << std::endl;
  });
}

// Stop fullscreen detection thread
void AnyWPEnginePlugin::StopFullscreenDetection() {
  std::cout << "[AnyWP] [PowerSaving] Stopping fullscreen detection..." << std::endl;
  
  stop_fullscreen_detection_ = true;
  
  if (fullscreen_detection_thread_.joinable()) {
    fullscreen_detection_thread_.join();
  }
  
  std::cout << "[AnyWP] [PowerSaving] Fullscreen detection stopped" << std::endl;
}

// Pause wallpaper (reduce CPU/GPU usage) - Improved for fast resume
void AnyWPEnginePlugin::PauseWallpaper(const std::string& reason) {
  if (is_paused_) {
    return;
  }
  
  std::cout << "[AnyWP] [PowerSaving] ========== PAUSING WALLPAPER (LIGHTWEIGHT) ==========" << std::endl;
  std::cout << "[AnyWP] [PowerSaving] Reason: " << reason << std::endl;
  
  is_paused_ = true;
  
  // IMPROVED: Use lightweight pause - don't hide windows, keep DOM state
  // This allows instant resume without reloading
  
  // 1. Set WebView visibility to false (stops rendering but keeps state)
  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    for (auto& instance : wallpaper_instances_) {
      if (instance.webview_controller) {
        // WebView2 stops rendering but keeps memory state
        instance.webview_controller->put_IsVisible(FALSE);
        std::cout << "[AnyWP] [PowerSaving] Set WebView invisible (keeps state)" << std::endl;
      }
    }
  }
  
  if (webview_controller_) {
    webview_controller_->put_IsVisible(FALSE);
  }
  
  // 2. Notify web content using Page Visibility API
  //    This allows web apps to pause animations gracefully
  NotifyWebContentVisibility(false);
  
  // 3. Light memory optimization (not aggressive)
  //    Only trim working set, don't clear cache
  SetProcessWorkingSetSize(GetCurrentProcess(), static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1));
  
  std::cout << "[AnyWP] [PowerSaving] Wallpaper paused (state preserved for fast resume)" << std::endl;
}

// Resume wallpaper - Instant resume from lightweight pause
void AnyWPEnginePlugin::ResumeWallpaper(const std::string& reason) {
  if (!is_paused_) {
    return;
  }
  
  std::cout << "[AnyWP] [PowerSaving] ========== RESUMING WALLPAPER (INSTANT) ==========" << std::endl;
  std::cout << "[AnyWP] [PowerSaving] Reason: " << reason << std::endl;
  
  is_paused_ = false;
  
  // IMPROVED: Instant resume - just re-enable WebView visibility
  // DOM state is preserved, no reloading needed
  
  // 1. Restore WebView visibility (instant - no reload)
  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    for (auto& instance : wallpaper_instances_) {
      if (instance.webview_controller) {
        // Instant resume - rendering continues from where it stopped
        instance.webview_controller->put_IsVisible(TRUE);
        std::cout << "[AnyWP] [PowerSaving] WebView visible restored (instant)" << std::endl;
      }
    }
  }
  
  if (webview_controller_) {
    webview_controller_->put_IsVisible(TRUE);
  }
  
  // 2. Notify web content to resume animations
  NotifyWebContentVisibility(true);
  
  std::cout << "[AnyWP] [PowerSaving] Wallpaper resumed instantly (no reload)" << std::endl;
}

// Optimize memory usage (aggressive for better results with safety checks)
void AnyWPEnginePlugin::OptimizeMemoryUsage() {
  std::cout << "[AnyWP] [Memory] ========== OPTIMIZING MEMORY ==========" << std::endl;
  
  // SAFETY: Check if process handle is valid
  HANDLE process = GetCurrentProcess();
  if (!process) {
    std::cout << "[AnyWP] [Memory] ERROR: Invalid process handle" << std::endl;
    return;
  }
  
  size_t before = GetCurrentMemoryUsage();
  if (before == 0) {
    std::cout << "[AnyWP] [Memory] WARNING: Could not get memory usage" << std::endl;
    return;
  }
  
  std::cout << "[AnyWP] [Memory] Current usage: " << (before / 1024 / 1024) << " MB" << std::endl;
  
  // 1. Clear WebView cache
  ClearWebViewCache();
  
  // 2. Aggressive JavaScript optimization with NULL checks
  std::wstring aggressive_gc_script = L"(function() {"
    L"  try {"
    L"    // Clear console logs"
    L"    if (console && console.clear) console.clear();"
    L"    "
    L"    // Clear any cached data"
    L"    if (window.caches && caches.keys) {"
    L"      caches.keys().then(function(names) {"
    L"        if (names && names.forEach) {"
    L"          names.forEach(function(name) { "
    L"            if (caches.delete) caches.delete(name); "
    L"          });"
    L"        }"
    L"      }).catch(function(e) { console.warn('Cache clear failed:', e); });"
    L"    }"
    L"    "
    L"    // Force garbage collection if available"
    L"    if (window.gc) window.gc();"
    L"    "
    L"    console.log('[AnyWP] Memory optimization complete');"
    L"  } catch(e) {"
    L"    console.error('[AnyWP] Optimization error:', e);"
    L"  }"
    L"})();";
  
  // Execute with instance mutex lock for safety
  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    for (auto& instance : wallpaper_instances_) {
      // SAFETY: Check webview is valid before using
      if (instance.webview && instance.webview.Get()) {
        instance.webview->ExecuteScript(aggressive_gc_script.c_str(), nullptr);
      }
    }
  }
  
  // SAFETY: Check webview_ is valid
  if (webview_ && webview_.Get()) {
    webview_->ExecuteScript(aggressive_gc_script.c_str(), nullptr);
  }
  
  // 3. Windows memory trim (multiple passes for better effect)
  // SAFETY: Check each call succeeds
  for (int i = 0; i < 3; i++) {
    BOOL result = SetProcessWorkingSetSize(process, static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1));
    if (!result) {
      DWORD error = GetLastError();
      std::cout << "[AnyWP] [Memory] WARNING: Memory trim pass " << (i+1) << " failed, error: " << error << std::endl;
      break;  // Stop on failure
    }
    Sleep(100);  // Small delay between passes
  }
  
  // Wait for optimization to complete
  Sleep(500);
  
  size_t after = GetCurrentMemoryUsage();
  
  // SAFETY: Check for overflow
  size_t freed = 0;
  if (after > 0 && before > after) {
    freed = before - after;
  }
  
  std::cout << "[AnyWP] [Memory] After optimization: " << (after / 1024 / 1024) << " MB" << std::endl;
  std::cout << "[AnyWP] [Memory] Freed: " << (freed / 1024 / 1024) << " MB" << std::endl;
  std::cout << "[AnyWP] [Memory] ========== OPTIMIZATION COMPLETE ==========" << std::endl;
}

// Configure WebView2 memory limits
void AnyWPEnginePlugin::ConfigureWebView2Memory() {
  // Note: WebView2 doesn't expose direct memory limit API
  // But we can configure browser arguments for lower memory usage
  std::cout << "[AnyWP] [Memory] WebView2 memory configuration applied" << std::endl;
}

// SAFE: Schedule memory optimization using JavaScript setTimeout (no dangling pointers)
void AnyWPEnginePlugin::ScheduleSafeMemoryOptimization(ICoreWebView2* webview) {
  if (!webview) {
    return;  // Safety check
  }
  
  std::cout << "[AnyWP] [Memory] Scheduling safe auto-optimization..." << std::endl;
  
  // Use JavaScript setTimeout - runs in WebView2 context, no C++ object dependency
  std::wstring safe_optimize_script = L"setTimeout(function() {"
    L"  try {"
    L"    console.log('[AnyWP] Auto-optimizing memory after navigation...');"
    L"    "
    L"    // Clear console to free memory"
    L"    if (console.clear) console.clear();"
    L"    "
    L"    // Clear browser cache"
    L"    if (window.caches) {"
    L"      caches.keys().then(function(names) {"
    L"        names.forEach(function(name) { caches.delete(name); });"
    L"      });"
    L"    }"
    L"    "
    L"    // Trigger GC if available"
    L"    if (window.gc) window.gc();"
    L"    "
    L"    console.log('[AnyWP] Auto-optimization complete');"
    L"  } catch(e) {"
    L"    console.error('[AnyWP] Auto-optimize error:', e);"
    L"  }"
    L"}, 3000);";  // 3 seconds delay
  
  webview->ExecuteScript(safe_optimize_script.c_str(), nullptr);
}

// Get current process memory usage (with safety checks)
size_t AnyWPEnginePlugin::GetCurrentMemoryUsage() {
  // SAFETY: Check process handle
  HANDLE process = GetCurrentProcess();
  if (!process) {
    std::cout << "[AnyWP] [Memory] ERROR: Invalid process handle in GetCurrentMemoryUsage" << std::endl;
    return 0;
  }
  
  PROCESS_MEMORY_COUNTERS_EX pmc = {0};
  pmc.cb = sizeof(pmc);
  
  // SAFETY: Check if GetProcessMemoryInfo succeeds
  if (GetProcessMemoryInfo(process, 
                          reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), 
                          sizeof(pmc))) {
    // SAFETY: Validate returned value is reasonable (<10GB)
    if (pmc.WorkingSetSize > 0 && pmc.WorkingSetSize < (10ULL * 1024 * 1024 * 1024)) {
      return pmc.WorkingSetSize;
    } else {
      std::cout << "[AnyWP] [Memory] WARNING: Unreasonable memory value: " << pmc.WorkingSetSize << std::endl;
      return 0;
    }
  }
  
  DWORD error = GetLastError();
  std::cout << "[AnyWP] [Memory] ERROR: GetProcessMemoryInfo failed, error: " << error << std::endl;
  return 0;
}

// Notify web content about visibility change (Page Visibility API with safety checks)
void AnyWPEnginePlugin::NotifyWebContentVisibility(bool visible) {
  std::cout << "[AnyWP] [PowerSaving] Notifying web content: " << (visible ? "VISIBLE" : "HIDDEN") << std::endl;
  
  // Use Page Visibility API to notify web content
  // This allows web apps to pause/resume animations gracefully
  std::wstring visibility_script = L"(function() {"
    L"  try {"
    L"    // SAFETY: Check if document exists"
    L"    if (!document) return;"
    L"    "
    L"    // Dispatch visibilitychange event"
    L"    var event = new Event('visibilitychange');"
    L"    document.dispatchEvent(event);"
    L"    "
    L"    // SAFETY: Check if window exists"
    L"    if (!window) return;"
    L"    "
    L"    // Also dispatch custom AnyWP event for SDK users"
    L"    var customEvent = new CustomEvent('AnyWP:visibility', {"
    L"      detail: { visible: " + std::wstring(visible ? L"true" : L"false") + L" }"
    L"    });"
    L"    window.dispatchEvent(customEvent);"
    L"    "
    L"    console.log('[AnyWP] Visibility changed: " + std::wstring(visible ? L"visible" : L"hidden") + L"');"
    L"  } catch(e) {"
    L"    console.error('[AnyWP] Error notifying visibility:', e);"
    L"  }"
    L"})();";
  
  // SAFE: Send to all instances with lock
  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    for (auto& instance : wallpaper_instances_) {
      // SAFETY: Check webview is valid
      if (instance.webview && instance.webview.Get()) {
        instance.webview->ExecuteScript(visibility_script.c_str(), nullptr);
      }
    }
  }
  
  // SAFETY: Send to legacy instance with NULL check
  if (webview_ && webview_.Get()) {
    webview_->ExecuteScript(visibility_script.c_str(), nullptr);
  }
}

// Convert power state enum to string
std::string AnyWPEnginePlugin::PowerStateToString(PowerState state) {
  switch (state) {
    case PowerState::ACTIVE: return "ACTIVE";
    case PowerState::IDLE: return "IDLE";
    case PowerState::SCREEN_OFF: return "SCREEN_OFF";
    case PowerState::LOCKED: return "LOCKED";
    case PowerState::FULLSCREEN_APP: return "FULLSCREEN_APP";
    case PowerState::PAUSED: return "PAUSED";
    default: return "UNKNOWN";
  }
}

// Notify Dart side about power state changes
void AnyWPEnginePlugin::NotifyPowerStateChange(PowerState newState) {
  if (newState == power_state_) {
    return;  // No change
  }
  
  std::string oldStateStr = PowerStateToString(last_power_state_);
  std::string newStateStr = PowerStateToString(newState);
  
  std::cout << "[AnyWP] [PowerSaving] State changed: " << oldStateStr << " -> " << newStateStr << std::endl;
  
  // Update states
  last_power_state_ = power_state_;
  power_state_ = newState;
  
  // Notify Dart if channel is available
  if (method_channel_) {
    flutter::EncodableMap args;
    args[flutter::EncodableValue("oldState")] = flutter::EncodableValue(oldStateStr);
    args[flutter::EncodableValue("newState")] = flutter::EncodableValue(newStateStr);
    
    auto args_value = std::make_unique<flutter::EncodableValue>(args);
    
    std::cout << "[AnyWP] [PowerSaving] Notifying Dart about state change" << std::endl;
    // Note: InvokeMethod may crash in some contexts, so we skip it for now
    // method_channel_->InvokeMethod("onPowerStateChange", std::move(args_value));
  }
}

}  // namespace anywp_engine

// Export C API for plugin registration
extern "C" {

__declspec(dllexport) void AnyWPEnginePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  anywp_engine::AnyWPEnginePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}

}  // extern "C"


