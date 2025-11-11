#include "flutter_bridge.h"
#include "../anywp_engine_plugin.h"
#include "../utils/logger.h"
#include "../utils/input_validator.h"
#include <iostream>

namespace anywp_engine {

// Convenience alias for PowerState enum
using PowerState = AnyWPEnginePlugin::PowerState;

FlutterBridge::FlutterBridge(AnyWPEnginePlugin* plugin)
    : plugin_(plugin) {
  Logger::Instance().Info("FlutterBridge", "Initializing FlutterBridge");
  RegisterAllHandlers();
}

FlutterBridge::~FlutterBridge() {
  Logger::Instance().Info("FlutterBridge", "FlutterBridge destroyed");
}

// ========================================
// Main Dispatcher
// ========================================

void FlutterBridge::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  const std::string& method_name = method_call.method_name();
  Logger::Instance().Info("FlutterBridge", "Method called: " + method_name);

  // Find and invoke handler
  auto it = handlers_.find(method_name);
  if (it != handlers_.end()) {
    const auto* args = std::get_if<flutter::EncodableMap>(method_call.arguments());
    
    // Call handler directly - each handler is responsible for error handling
    // and must call either result->Success() or result->Error()
    it->second(args, std::move(result));
  } else {
    Logger::Instance().Warn("FlutterBridge",
      "Unknown method: " + method_name);
    result->NotImplemented();
  }
}

// ========================================
// Handler Registration
// ========================================

void FlutterBridge::RegisterHandler(const std::string& method_name, MethodHandler handler) {
  handlers_[method_name] = handler;
  Logger::Instance().Debug("FlutterBridge", "Registered handler: " + method_name);
}

void FlutterBridge::RegisterAllHandlers() {
  // Wallpaper control
  RegisterHandler("initializeWallpaper",
      [this](auto* args, auto result) { HandleInitializeWallpaper(args, std::move(result)); });
  RegisterHandler("stopWallpaper",
      [this](auto* args, auto result) { HandleStopWallpaper(args, std::move(result)); });
  RegisterHandler("navigateToUrl",
      [this](auto* args, auto result) { HandleNavigateToUrl(args, std::move(result)); });

  // Multi-monitor
  RegisterHandler("getMonitors",
      [this](auto* args, auto result) { HandleGetMonitors(args, std::move(result)); });
  RegisterHandler("initializeWallpaperOnMonitor",
      [this](auto* args, auto result) { HandleInitializeWallpaperOnMonitor(args, std::move(result)); });
  RegisterHandler("stopWallpaperOnMonitor",
      [this](auto* args, auto result) { HandleStopWallpaperOnMonitor(args, std::move(result)); });
  RegisterHandler("navigateToUrlOnMonitor",
      [this](auto* args, auto result) { HandleNavigateToUrlOnMonitor(args, std::move(result)); });

  // Power management
  RegisterHandler("pauseWallpaper",
      [this](auto* args, auto result) { HandlePauseWallpaper(args, std::move(result)); });
  RegisterHandler("resumeWallpaper",
      [this](auto* args, auto result) { HandleResumeWallpaper(args, std::move(result)); });
  RegisterHandler("setAutoPowerSaving",
      [this](auto* args, auto result) { HandleSetAutoPowerSaving(args, std::move(result)); });
  RegisterHandler("getPowerState",
      [this](auto* args, auto result) { HandleGetPowerState(args, std::move(result)); });

  // Memory management
  RegisterHandler("getMemoryUsage",
      [this](auto* args, auto result) { HandleGetMemoryUsage(args, std::move(result)); });
  RegisterHandler("optimizeMemory",
      [this](auto* args, auto result) { HandleOptimizeMemory(args, std::move(result)); });

  // Configuration
  RegisterHandler("setIdleTimeout",
      [this](auto* args, auto result) { HandleSetIdleTimeout(args, std::move(result)); });
  RegisterHandler("setMemoryThreshold",
      [this](auto* args, auto result) { HandleSetMemoryThreshold(args, std::move(result)); });
  RegisterHandler("setCleanupInterval",
      [this](auto* args, auto result) { HandleSetCleanupInterval(args, std::move(result)); });
  RegisterHandler("getConfiguration",
      [this](auto* args, auto result) { HandleGetConfiguration(args, std::move(result)); });

  // State persistence
  RegisterHandler("saveState",
      [this](auto* args, auto result) { HandleSaveState(args, std::move(result)); });
  RegisterHandler("loadState",
      [this](auto* args, auto result) { HandleLoadState(args, std::move(result)); });
  RegisterHandler("clearState",
      [this](auto* args, auto result) { HandleClearState(args, std::move(result)); });
  RegisterHandler("setApplicationName",
      [this](auto* args, auto result) { HandleSetApplicationName(args, std::move(result)); });
  RegisterHandler("getStoragePath",
      [this](auto* args, auto result) { HandleGetStoragePath(args, std::move(result)); });

  // Utility
  RegisterHandler("getVersion",
      [this](auto* args, auto result) { HandleGetVersion(args, std::move(result)); });

  Logger::Instance().Info("FlutterBridge",
    "Registered " + std::to_string(handlers_.size()) + " method handlers");
}

// ========================================
// Wallpaper Control Handlers
// ========================================

void FlutterBridge::HandleInitializeWallpaper(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  std::string url;
  if (!GetStringArgument(args, "url", url, result)) {
    return;  // Error already sent
  }

  // Validate URL
  if (!InputValidator::IsValidUrl(url)) {
    result->Error("INVALID_URL", "URL is not valid or contains dangerous protocol");
    return;
  }

  bool enable_transparent = GetBoolArgument(args, "enableMouseTransparent", false);  // 默认为 false（交互模式）
  
  std::cout << "[FlutterBridge] initializeWallpaper: enableMouseTransparent = " 
            << (enable_transparent ? "true" : "false") << std::endl;

  // Call plugin method
  bool success = plugin_->InitializeWithRetry(url, enable_transparent, 3);
  result->Success(flutter::EncodableValue(success));
}

void FlutterBridge::HandleStopWallpaper(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  bool success = plugin_->StopWallpaper();
  result->Success(flutter::EncodableValue(success));
}

void FlutterBridge::HandleNavigateToUrl(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  std::string url;
  if (!GetStringArgument(args, "url", url, result)) {
    return;
  }

  // Validate URL
  if (!InputValidator::IsValidUrl(url)) {
    result->Error("INVALID_URL", "URL is not valid or contains dangerous protocol");
    return;
  }

  bool success = plugin_->NavigateToUrl(url);
  result->Success(flutter::EncodableValue(success));
}

// ========================================
// Multi-Monitor Handlers
// ========================================

void FlutterBridge::HandleGetMonitors(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  std::vector<MonitorInfo> monitors = plugin_->GetMonitors();
  
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

void FlutterBridge::HandleInitializeWallpaperOnMonitor(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  std::string url;
  int monitor_index = -1;  // Initialize to invalid value
  
  if (!GetStringArgument(args, "url", url, result) ||
      !GetIntArgument(args, "monitorIndex", monitor_index, result)) {
    return;
  }

  // Validate URL
  if (!InputValidator::IsValidUrl(url)) {
    result->Error("INVALID_URL", "URL is not valid or contains dangerous protocol");
    return;
  }

  // Validate monitor index
  if (!InputValidator::IsValidMonitorIndex(monitor_index, 10)) {  // Max 10 monitors
    result->Error("INVALID_INDEX", "Monitor index out of range");
    return;
  }

  bool enable_transparent = GetBoolArgument(args, "enableMouseTransparent", false);  // 默认为 false（交互模式）
  
  std::cout << "[FlutterBridge] initializeWallpaperOnMonitor: enableMouseTransparent = " 
            << (enable_transparent ? "true" : "false") 
            << ", monitor = " << monitor_index << std::endl;

  bool success = plugin_->InitializeWallpaperOnMonitor(url, enable_transparent, monitor_index);
  result->Success(flutter::EncodableValue(success));
}

void FlutterBridge::HandleStopWallpaperOnMonitor(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  int monitor_index;
  if (!GetIntArgument(args, "monitorIndex", monitor_index, result)) {
    return;
  }

  // Validate monitor index
  if (!InputValidator::IsValidMonitorIndex(monitor_index, 10)) {
    result->Error("INVALID_INDEX", "Monitor index out of range");
    return;
  }

  bool success = plugin_->StopWallpaperOnMonitor(monitor_index);
  result->Success(flutter::EncodableValue(success));
}

void FlutterBridge::HandleNavigateToUrlOnMonitor(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  std::string url;
  int monitor_index = -1;  // Initialize to invalid value
  
  if (!GetStringArgument(args, "url", url, result) ||
      !GetIntArgument(args, "monitorIndex", monitor_index, result)) {
    return;
  }

  // Validate URL
  if (!InputValidator::IsValidUrl(url)) {
    result->Error("INVALID_URL", "URL is not valid or contains dangerous protocol");
    return;
  }

  bool success = plugin_->NavigateToUrlOnMonitor(url, monitor_index);
  result->Success(flutter::EncodableValue(success));
}

// ========================================
// Power Management Handlers
// ========================================

void FlutterBridge::HandlePauseWallpaper(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  plugin_->PauseWallpaper("Manual pause from Flutter");
  result->Success();
}

void FlutterBridge::HandleResumeWallpaper(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  bool force_reinit = GetBoolArgument(args, "forceReinit", false);
  plugin_->ResumeWallpaper("Manual resume from Flutter", force_reinit);
  result->Success();
}

void FlutterBridge::HandleSetAutoPowerSaving(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  bool enabled;
  auto it = args->find(flutter::EncodableValue("enabled"));
  if (it == args->end()) {
    result->Error("INVALID_ARGS", "Missing 'enabled' argument");
    return;
  }

  enabled = std::get<bool>(it->second);
  
  if (enabled) {
    plugin_->SetupPowerSavingMonitoring();
  } else {
    plugin_->CleanupPowerSavingMonitoring();
  }
  
  result->Success();
}

void FlutterBridge::HandleGetPowerState(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  // Convert PowerState enum to string
  std::string state_str;
  switch (plugin_->power_state_) {
    case PowerState::ACTIVE: state_str = "ACTIVE"; break;
    case PowerState::IDLE: state_str = "IDLE"; break;
    case PowerState::SCREEN_OFF: state_str = "SCREEN_OFF"; break;
    case PowerState::LOCKED: state_str = "LOCKED"; break;
    case PowerState::FULLSCREEN_APP: state_str = "FULLSCREEN_APP"; break;
    case PowerState::PAUSED: state_str = "PAUSED"; break;
    default: state_str = "UNKNOWN"; break;
  }
  result->Success(flutter::EncodableValue(state_str));
}

// ========================================
// Memory Management Handlers
// ========================================

void FlutterBridge::HandleGetMemoryUsage(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  // Get memory usage in bytes and convert to MB
  size_t memory_bytes = plugin_->GetCurrentMemoryUsage();
  int memory_mb = static_cast<int>(memory_bytes / 1024 / 1024);
  result->Success(flutter::EncodableValue(memory_mb));
}

void FlutterBridge::HandleOptimizeMemory(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  plugin_->OptimizeMemoryUsage();
  result->Success();
}

// ========================================
// Configuration Handlers
// ========================================

void FlutterBridge::HandleSetIdleTimeout(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  int timeout_seconds;
  if (!GetIntArgument(args, "seconds", timeout_seconds, result)) {
    return;
  }

  if (timeout_seconds < 0) {
    result->Error("INVALID_VALUE", "Timeout must be non-negative");
    return;
  }

  // Directly set member variable (FlutterBridge is a friend class)
  plugin_->idle_timeout_ms_ = timeout_seconds * 1000;
  Logger::Instance().Info("FlutterBridge", "Idle timeout set to " + std::to_string(timeout_seconds) + " seconds");
  result->Success();
}

void FlutterBridge::HandleSetMemoryThreshold(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  int threshold_mb;
  if (!GetIntArgument(args, "megabytes", threshold_mb, result)) {
    return;
  }

  if (threshold_mb <= 0) {
    result->Error("INVALID_VALUE", "Threshold must be positive");
    return;
  }

  // Directly set member variable (FlutterBridge is a friend class)
  plugin_->memory_threshold_mb_ = threshold_mb;
  Logger::Instance().Info("FlutterBridge", "Memory threshold set to " + std::to_string(threshold_mb) + " MB");
  result->Success();
}

void FlutterBridge::HandleSetCleanupInterval(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  int interval_minutes;
  if (!GetIntArgument(args, "minutes", interval_minutes, result)) {
    return;
  }

  if (interval_minutes <= 0) {
    result->Error("INVALID_VALUE", "Interval must be positive");
    return;
  }

  // Directly set member variable (FlutterBridge is a friend class)
  plugin_->cleanup_interval_minutes_ = interval_minutes;
  Logger::Instance().Info("FlutterBridge", "Cleanup interval set to " + std::to_string(interval_minutes) + " minutes");
  result->Success();
}

void FlutterBridge::HandleGetConfiguration(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  flutter::EncodableMap config_map;
  config_map[flutter::EncodableValue("version")] = flutter::EncodableValue(plugin_->GetPluginVersion());
  config_map[flutter::EncodableValue("storagePath")] = flutter::EncodableValue(plugin_->GetStoragePath());
  
  // Convert PowerState to string
  std::string state_str;
  switch (plugin_->power_state_) {
    case PowerState::ACTIVE: state_str = "ACTIVE"; break;
    case PowerState::IDLE: state_str = "IDLE"; break;
    case PowerState::SCREEN_OFF: state_str = "SCREEN_OFF"; break;
    case PowerState::LOCKED: state_str = "LOCKED"; break;
    case PowerState::FULLSCREEN_APP: state_str = "FULLSCREEN_APP"; break;
    case PowerState::PAUSED: state_str = "PAUSED"; break;
    default: state_str = "UNKNOWN"; break;
  }
  config_map[flutter::EncodableValue("powerState")] = flutter::EncodableValue(state_str);
  
  result->Success(flutter::EncodableValue(config_map));
}

// ========================================
// State Persistence Handlers
// ========================================

void FlutterBridge::HandleSaveState(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  std::string key, value;
  if (!GetStringArgument(args, "key", key, result) ||
      !GetStringArgument(args, "value", value, result)) {
    return;
  }

  // Validate key and value
  if (InputValidator::IsEmptyOrWhitespace(key) || !InputValidator::IsLengthValid(key, 256)) {
    result->Error("INVALID_KEY", "Key must be non-empty and less than 256 characters");
    return;
  }
  if (!InputValidator::IsLengthValid(value, 1024 * 1024)) {  // Max 1MB
    result->Error("INVALID_VALUE", "Value exceeds maximum size (1MB)");
    return;
  }

  bool success = plugin_->SaveState(key, value);
  result->Success(flutter::EncodableValue(success));
}

void FlutterBridge::HandleLoadState(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  std::string key;
  if (!GetStringArgument(args, "key", key, result)) {
    return;
  }

  // Validate key
  if (InputValidator::IsEmptyOrWhitespace(key) || !InputValidator::IsLengthValid(key, 256)) {
    result->Error("INVALID_KEY", "Key must be non-empty and less than 256 characters");
    return;
  }

  std::string value = plugin_->LoadState(key);
  result->Success(flutter::EncodableValue(value));
}

void FlutterBridge::HandleClearState(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  bool success = plugin_->ClearState();
  result->Success(flutter::EncodableValue(success));
}

void FlutterBridge::HandleSetApplicationName(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  std::string app_name;
  if (!GetStringArgument(args, "name", app_name, result)) {
    return;
  }

  // Validate app name
  if (InputValidator::IsEmptyOrWhitespace(app_name)) {
    result->Error("INVALID_NAME", "Application name cannot be empty");
    return;
  }

  plugin_->SetApplicationName(app_name);
  result->Success();
}

void FlutterBridge::HandleGetStoragePath(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  std::string path = plugin_->GetStoragePath();
  result->Success(flutter::EncodableValue(path));
}

// ========================================
// Utility Handlers
// ========================================

void FlutterBridge::HandleGetVersion(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  std::string version = plugin_->GetPluginVersion();
  result->Success(flutter::EncodableValue(version));
}

// ========================================
// Helper Methods
// ========================================

bool FlutterBridge::ValidateArgument(
    const flutter::EncodableMap* args,
    const std::string& key,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>& result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return false;
  }

  auto it = args->find(flutter::EncodableValue(key));
  if (it == args->end()) {
    result->Error("INVALID_ARGS", "Missing '" + key + "' argument");
    return false;
  }

  return true;
}

bool FlutterBridge::GetStringArgument(
    const flutter::EncodableMap* args,
    const std::string& key,
    std::string& out_value,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>& result) {
  
  if (!ValidateArgument(args, key, result)) {
    return false;
  }

  auto it = args->find(flutter::EncodableValue(key));
  try {
    out_value = std::get<std::string>(it->second);
    return true;
  } catch (const std::bad_variant_access&) {
    result->Error("TYPE_ERROR", "Argument '" + key + "' must be a string");
    return false;
  }
}

bool FlutterBridge::GetIntArgument(
    const flutter::EncodableMap* args,
    const std::string& key,
    int& out_value,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>& result) {
  
  if (!ValidateArgument(args, key, result)) {
    return false;
  }

  auto it = args->find(flutter::EncodableValue(key));
  try {
    out_value = std::get<int>(it->second);
    return true;
  } catch (const std::bad_variant_access&) {
    result->Error("TYPE_ERROR", "Argument '" + key + "' must be an integer");
    return false;
  }
}

bool FlutterBridge::GetBoolArgument(
    const flutter::EncodableMap* args,
    const std::string& key,
    bool default_value) {
  
  if (!args) {
    return default_value;
  }

  auto it = args->find(flutter::EncodableValue(key));
  if (it == args->end()) {
    return default_value;
  }

  try {
    return std::get<bool>(it->second);
  } catch (const std::bad_variant_access&) {
    return default_value;
  }
}

}  // namespace anywp_engine

