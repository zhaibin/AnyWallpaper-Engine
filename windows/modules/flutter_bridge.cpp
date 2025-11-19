#include "flutter_bridge.h"
#include "../anywp_engine_plugin.h"
#include "../utils/logger.h"
#include "../utils/input_validator.h"
#include "custom_scheme_handler.h"  // v2.1.10+ Custom scheme support
#include <iostream>
#include <set>

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

// Helper function to convert EncodableValue to readable string
static std::string EncodableValueToString(const flutter::EncodableValue& value, int depth = 0) {
  if (depth > 3) return "...";  // Prevent infinite recursion
  
  if (std::holds_alternative<std::monostate>(value)) {
    return "null";
  } else if (auto* b = std::get_if<bool>(&value)) {
    return *b ? "true" : "false";
  } else if (auto* i = std::get_if<int32_t>(&value)) {
    return std::to_string(*i);
  } else if (auto* l = std::get_if<int64_t>(&value)) {
    return std::to_string(*l);
  } else if (auto* d = std::get_if<double>(&value)) {
    return std::to_string(*d);
  } else if (auto* s = std::get_if<std::string>(&value)) {
    return "\"" + *s + "\"";
  } else if (auto* list = std::get_if<flutter::EncodableList>(&value)) {
    std::string result = "[";
    for (size_t idx = 0; idx < list->size(); ++idx) {
      if (idx > 0) result += ", ";
      if (idx >= 5) {
        result += "... (" + std::to_string(list->size()) + " items)";
        break;
      }
      result += EncodableValueToString((*list)[idx], depth + 1);
    }
    result += "]";
    return result;
  } else if (auto* map = std::get_if<flutter::EncodableMap>(&value)) {
    std::string result = "{";
    size_t count = 0;
    for (const auto& pair : *map) {
      if (count > 0) result += ", ";
      if (count >= 5) {
        result += "... (" + std::to_string(map->size()) + " keys)";
        break;
      }
      result += EncodableValueToString(pair.first, depth + 1) + ": " + 
                EncodableValueToString(pair.second, depth + 1);
      ++count;
    }
    result += "}";
    return result;
  }
  return "<unknown type>";
}

void FlutterBridge::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  const std::string& method_name = method_call.method_name();
  
  // Polling methods use DEBUG level to reduce noise
  static const std::set<std::string> polling_methods = {
    "getPendingPowerStateChanges",
    "getPendingMessages",
    "getMonitors"
  };
  
  bool is_polling = polling_methods.count(method_name) > 0;
  
  if (is_polling) {
    Logger::Instance().Debug("FlutterBridge", "Method called: " + method_name);
  } else {
    Logger::Instance().Info("FlutterBridge", "Method called: " + method_name);
    
    // v2.4.1+ Log method arguments for non-polling methods
    const auto* args = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (args && !args->empty()) {
      std::string args_str = "Arguments: ";
      size_t count = 0;
      for (const auto& pair : *args) {
        if (count > 0) args_str += ", ";
        if (count >= 10) {
          args_str += "... (" + std::to_string(args->size()) + " parameters total)";
          break;
        }
        const auto* key = std::get_if<std::string>(&pair.first);
        if (key) {
          args_str += *key + "=" + EncodableValueToString(pair.second);
        }
        ++count;
      }
      Logger::Instance().Info("FlutterBridge", args_str);
    } else if (!args) {
      Logger::Instance().Debug("FlutterBridge", "No arguments (null)");
    } else {
      Logger::Instance().Debug("FlutterBridge", "No arguments (empty map)");
    }
  }

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
  
  // Web SDK Version (v2.1.10+)
  RegisterHandler("getSDKVersion",
      [this](auto* args, auto result) { HandleGetSDKVersion(args, std::move(result)); });

  // Message communication
  RegisterHandler("sendMessage",
      [this](auto* args, auto result) { HandleSendMessage(args, std::move(result)); });
  
  // v2.1.0+ Bidirectional Communication: Get pending messages from JavaScript
  RegisterHandler("getPendingMessages",
      [this](auto* args, auto result) { HandleGetPendingMessages(args, std::move(result)); });
  
  // v2.1.1+ Fix: Get pending power state changes (polling-based to avoid thread safety issues)
  RegisterHandler("getPendingPowerStateChanges",
      [this](auto* args, auto result) { HandleGetPendingPowerStateChanges(args, std::move(result)); });
  
  // v2.3.2+ Auto recovery
  RegisterHandler("enableAutoRecovery",
      [this](auto* args, auto result) { HandleEnableAutoRecovery(args, std::move(result)); });
  RegisterHandler("isAutoRecoveryEnabled",
      [this](auto* args, auto result) { HandleIsAutoRecoveryEnabled(args, std::move(result)); });
  
  // v2.4.0+ Manual save wallpaper configuration
  RegisterHandler("saveWallpaperConfiguration",
      [this](auto* args, auto result) { HandleSaveWallpaperConfiguration(args, std::move(result)); });
  
  // v2.1.10+ Custom scheme: File encryption/decryption
  RegisterHandler("encryptFile",
      [this](auto* args, auto result) { HandleEncryptFile(args, std::move(result)); });
  RegisterHandler("decryptFile",
      [this](auto* args, auto result) { HandleDecryptFile(args, std::move(result)); });

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

  bool enable_transparent = GetBoolArgument(args, "enableMouseTransparent", false);  // Default: false (interactive mode)
  
  Logger::Instance().Debug("FlutterBridge", 
    std::string("initializeWallpaper: enableMouseTransparent = ") + (enable_transparent ? "true" : "false"));

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

  bool enable_transparent = GetBoolArgument(args, "enableMouseTransparent", false);  // Default: false (interactive mode)
  bool auto_save = GetBoolArgument(args, "autoSave", true);  // v2.4.0+: Default: true (auto-save)
  
  Logger::Instance().Debug("FlutterBridge", 
    "initializeWallpaperOnMonitor: enableMouseTransparent = " + 
    std::string(enable_transparent ? "true" : "false") + 
    ", autoSave = " + std::string(auto_save ? "true" : "false") +
    ", monitor = " + std::to_string(monitor_index));

  bool success = plugin_->InitializeWallpaperOnMonitor(url, enable_transparent, monitor_index, auto_save);
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

void FlutterBridge::HandleGetSDKVersion(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  std::string sdk_version = plugin_->GetSDKVersion();
  result->Success(flutter::EncodableValue(sdk_version));
}

// ========================================
// Message Communication Handlers
// ========================================

void FlutterBridge::HandleSendMessage(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }

  // Step 1: Get message parameter
  std::string message_json;
  if (!GetStringArgument(args, "message", message_json, result)) {
    return;  // Error already sent
  }

  // Step 2: Get target monitor index (optional, -1 means all monitors)
  int monitor_index = -1;
  auto monitor_it = args->find(flutter::EncodableValue("monitorIndex"));
  if (monitor_it != args->end() && !monitor_it->second.IsNull()) {
    try {
      monitor_index = std::get<int>(monitor_it->second);
    } catch (const std::bad_variant_access&) {
      // Ignore type error, use default value -1
      Logger::Instance().Warn("FlutterBridge", "monitorIndex is not an integer, using -1 (all monitors)");
    }
  }

  std::string monitor_desc = (monitor_index == -1) ? "all monitors" : "monitor " + std::to_string(monitor_index);
  Logger::Instance().Info("FlutterBridge", "Sending WebMessage");
  Logger::Instance().Info("FlutterBridge", "  Target: " + monitor_desc);
  Logger::Instance().Debug("FlutterBridge", "  Message JSON: " + message_json);

  // Step 3: Get wallpaper instances
  std::vector<WallpaperInstance*> target_instances;
  
  if (monitor_index >= 0) {
    // Send to specific monitor
    auto* instance = plugin_->GetInstanceForMonitor(monitor_index);
    if (instance) {
      target_instances.push_back(instance);
    } else {
      result->Error("INSTANCE_NOT_FOUND", 
                   "No wallpaper instance for monitor " + std::to_string(monitor_index));
      return;
    }
  } else {
    // Send to all monitors
    std::lock_guard<std::mutex> lock(plugin_->instances_mutex_);
    for (auto& instance : plugin_->wallpaper_instances_) {
      target_instances.push_back(&instance);
    }
  }

  if (target_instances.empty()) {
    result->Error("NO_INSTANCES", "No active wallpaper instances");
    return;
  }

  // Step 4: Send message to JavaScript
  bool all_success = true;
  int sent_count = 0;

  Logger::Instance().Info("FlutterBridge", 
    "Target instances count: " + std::to_string(target_instances.size()));

  for (auto* instance : target_instances) {
    Logger::Instance().Info("FlutterBridge", 
      "Checking instance: " + std::to_string((long long)instance));
    
    if (!instance) {
      Logger::Instance().Error("FlutterBridge", "Instance is null!");
      all_success = false;
      continue;
    }
    
    Logger::Instance().Info("FlutterBridge", 
      "Instance webview pointer: " + std::to_string((long long)instance->webview.Get()));
    
    if (!instance->webview) {
      Logger::Instance().Error("FlutterBridge", 
        "Instance webview is null! Monitor: " + std::to_string(instance->monitor_index));
      all_success = false;
      continue;
    }

    // Build JavaScript code: trigger CustomEvent
    // Use JSON.parse() to safely parse the message string
    // First, escape special characters in the JSON string
    std::string escaped_json;
    escaped_json.reserve(message_json.length() * 2);
    for (char c : message_json) {
      switch (c) {
        case '\\': escaped_json += "\\\\"; break;
        case '\"': escaped_json += "\\\""; break;
        case '\n': escaped_json += "\\n"; break;
        case '\r': escaped_json += "\\r"; break;
        case '\t': escaped_json += "\\t"; break;
        default: escaped_json += c; break;
      }
    }
    
    // Convert escaped UTF-8 string to wide string
    int wide_size = MultiByteToWideChar(CP_UTF8, 0, escaped_json.c_str(), -1, nullptr, 0);
    if (wide_size == 0) {
      Logger::Instance().Error("FlutterBridge", "Failed to convert message to wide string");
      all_success = false;
      continue;
    }
    
    std::wstring message_wide(wide_size - 1, 0);
    int result_size = MultiByteToWideChar(CP_UTF8, 0, escaped_json.c_str(), -1, &message_wide[0], wide_size);
    if (result_size == 0) {
      Logger::Instance().Error("FlutterBridge", "Failed to convert message to wide string (step 2)");
      all_success = false;
      continue;
    }
    
    std::wstring script = L"(function() {\n"
                          L"  try {\n"
                          L"    const messageStr = \"" + message_wide + L"\";\n"
                          L"    const message = JSON.parse(messageStr);\n"
                          L"    const event = new CustomEvent('AnyWP:message', {\n"
                          L"      detail: message,\n"
                          L"      bubbles: true\n"
                          L"    });\n"
                          L"    window.dispatchEvent(event);\n"
                          L"    console.log('[AnyWP Engine] Message dispatched:', message);\n"
                          L"  } catch(e) {\n"
                          L"    console.error('[AnyWP Engine] Failed to dispatch message:', e);\n"
                          L"    console.error('[AnyWP Engine] Message string:', \"" + message_wide + L"\");\n"
                          L"  }\n"
                          L"})();\n";

    // Execute script
    HRESULT hr = instance->webview->ExecuteScript(
        script.c_str(),
        Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
            [](HRESULT error_code, LPCWSTR result_object_as_json) -> HRESULT {
              if (FAILED(error_code)) {
                Logger::Instance().Error("FlutterBridge", "ExecuteScript failed");
              }
              return S_OK;
            }
        ).Get()
    );

    if (SUCCEEDED(hr)) {
      sent_count++;
    } else {
      all_success = false;
      Logger::Instance().Error("FlutterBridge", "Failed to send message to instance");
    }
  }

  // Step 5: Return result
  if (all_success && sent_count > 0) {
    result->Success(flutter::EncodableValue(true));
    Logger::Instance().Info("FlutterBridge",
      "WebMessage sent successfully (" + std::to_string(sent_count) + " instance(s))");
  } else {
    result->Error("SEND_FAILED", 
                 "Failed to send message to some instances (" + 
                 std::to_string(sent_count) + "/" + 
                 std::to_string(target_instances.size()) + " succeeded)");
  }
}

void FlutterBridge::HandleGetPendingMessages(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  try {
    // Get pending messages from plugin
    std::vector<std::string> messages = plugin_->GetPendingMessages();
    
    // Convert to Flutter list
    flutter::EncodableList message_list;
    for (const auto& message : messages) {
      message_list.push_back(flutter::EncodableValue(message));
    }
    
    result->Success(flutter::EncodableValue(message_list));
  } catch (const std::exception& e) {
    Logger::Instance().Error("FlutterBridge", 
      std::string("Exception in GetPendingMessages: ") + e.what());
    result->Error("GET_MESSAGES_FAILED", e.what());
  }
}

// v2.1.1+ Fix: Get pending power state changes (polling-based to avoid thread safety issues)
void FlutterBridge::HandleGetPendingPowerStateChanges(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  try {
    // Get pending power state changes from plugin
    std::vector<std::pair<std::string, std::string>> changes = 
        plugin_->GetPendingPowerStateChanges();
    
    // Convert to Flutter list of maps
    flutter::EncodableList change_list;
    for (const auto& change : changes) {
      flutter::EncodableMap change_map;
      change_map[flutter::EncodableValue("oldState")] = 
          flutter::EncodableValue(change.first);
      change_map[flutter::EncodableValue("newState")] = 
          flutter::EncodableValue(change.second);
      change_list.push_back(flutter::EncodableValue(change_map));
    }
    
    result->Success(flutter::EncodableValue(change_list));
  } catch (const std::exception& e) {
    Logger::Instance().Error("FlutterBridge", 
      std::string("Exception in GetPendingPowerStateChanges: ") + e.what());
    result->Error("GET_POWER_STATE_CHANGES_FAILED", e.what());
  }
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

// ========================================
// v2.1.10+ Custom Scheme Handlers
// ========================================

void FlutterBridge::HandleEncryptFile(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }
  
  // Get source path
  std::string source_path_utf8;
  if (!GetStringArgument(args, "sourcePath", source_path_utf8, result)) {
    return;  // Error already sent
  }
  
  // Get dest path
  std::string dest_path_utf8;
  if (!GetStringArgument(args, "destPath", dest_path_utf8, result)) {
    return;  // Error already sent
  }
  
  // Convert UTF-8 to UTF-16
  std::wstring source_path = std::wstring(source_path_utf8.begin(), source_path_utf8.end());
  std::wstring dest_path = std::wstring(dest_path_utf8.begin(), dest_path_utf8.end());
  
  Logger::Instance().Info("FlutterBridge", 
    "Encrypting file: " + source_path_utf8 + " -> " + dest_path_utf8);
  
  // Call CustomSchemeHandler to encrypt
  HRESULT hr = CustomSchemeHandler::EncryptFile(source_path, dest_path);
  
  if (SUCCEEDED(hr)) {
    Logger::Instance().Info("FlutterBridge", "File encrypted successfully");
    result->Success(flutter::EncodableValue(true));
  } else {
    std::string error_msg = "Encryption failed. HRESULT: " + std::to_string(hr);
    Logger::Instance().Error("FlutterBridge", error_msg);
    result->Error("ENCRYPT_FAILED", error_msg);
  }
}

void FlutterBridge::HandleDecryptFile(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }
  
  // Get encrypted path
  std::string encrypted_path_utf8;
  if (!GetStringArgument(args, "encryptedPath", encrypted_path_utf8, result)) {
    return;  // Error already sent
  }
  
  // Get dest path
  std::string dest_path_utf8;
  if (!GetStringArgument(args, "destPath", dest_path_utf8, result)) {
    return;  // Error already sent
  }
  
  // Convert UTF-8 to UTF-16
  std::wstring encrypted_path = std::wstring(encrypted_path_utf8.begin(), encrypted_path_utf8.end());
  std::wstring dest_path = std::wstring(dest_path_utf8.begin(), dest_path_utf8.end());
  
  Logger::Instance().Info("FlutterBridge", 
    "Decrypting file: " + encrypted_path_utf8 + " -> " + dest_path_utf8);
  
  // Call CustomSchemeHandler to decrypt
  HRESULT hr = CustomSchemeHandler::DecryptFile(encrypted_path, dest_path);
  
  if (SUCCEEDED(hr)) {
    Logger::Instance().Info("FlutterBridge", "File decrypted successfully");
    result->Success(flutter::EncodableValue(true));
  } else {
    std::string error_msg = "Decryption failed. HRESULT: " + std::to_string(hr);
    Logger::Instance().Error("FlutterBridge", error_msg);
    result->Error("DECRYPT_FAILED", error_msg);
  }
}

// ========================================
// v2.3.2+ Auto Recovery Handlers
// ========================================

void FlutterBridge::HandleEnableAutoRecovery(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }
  
  // Get enabled flag
  bool enabled = GetBoolArgument(args, "enabled", false);
  
  Logger::Instance().Info("FlutterBridge", 
    std::string("Setting auto recovery: ") + (enabled ? "enabled" : "disabled"));
  
  try {
    plugin_->SetAutoRecoveryEnabled(enabled);
    result->Success(flutter::EncodableValue(true));
  } catch (const std::exception& e) {
    std::string error_msg = "Failed to set auto recovery: " + std::string(e.what());
    Logger::Instance().Error("FlutterBridge", error_msg);
    result->Error("AUTO_RECOVERY_ERROR", error_msg);
  }
}

void FlutterBridge::HandleIsAutoRecoveryEnabled(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  try {
    bool enabled = plugin_->IsAutoRecoveryEnabled();
    Logger::Instance().Debug("FlutterBridge", 
      std::string("Auto recovery status: ") + (enabled ? "enabled" : "disabled"));
    result->Success(flutter::EncodableValue(enabled));
  } catch (const std::exception& e) {
    std::string error_msg = "Failed to get auto recovery status: " + std::string(e.what());
    Logger::Instance().Error("FlutterBridge", error_msg);
    result->Error("AUTO_RECOVERY_ERROR", error_msg);
  }
}

void FlutterBridge::HandleSaveWallpaperConfiguration(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  if (!args) {
    result->Error("INVALID_ARGS", "Arguments must be a map");
    return;
  }
  
  // Get monitor index (-1 = all monitors, default if not specified)
  int monitor_index = -1;
  auto it = args->find(flutter::EncodableValue("monitorIndex"));
  if (it != args->end()) {
    try {
      monitor_index = std::get<int>(it->second);
    } catch (...) {
      // Invalid type, use default -1
      Logger::Instance().Debug("FlutterBridge", 
        "monitorIndex not an integer, using default -1");
    }
  }
  
  Logger::Instance().Info("FlutterBridge", 
    "Manual save wallpaper configuration requested for monitor: " + 
    (monitor_index == -1 ? "all" : std::to_string(monitor_index)));
  
  try {
    bool success = plugin_->SaveWallpaperConfigurationManually(monitor_index);
    
    if (success) {
      Logger::Instance().Info("FlutterBridge", 
        "Wallpaper configuration saved successfully");
    } else {
      Logger::Instance().Warn("FlutterBridge", 
        "Failed to save wallpaper configuration (auto recovery may be disabled)");
    }
    
    result->Success(flutter::EncodableValue(success));
  } catch (const std::exception& e) {
    std::string error_msg = "Error saving wallpaper configuration: " + std::string(e.what());
    Logger::Instance().Error("FlutterBridge", error_msg);
    result->Error("SAVE_CONFIG_ERROR", error_msg);
  }
}

}  // namespace anywp_engine

