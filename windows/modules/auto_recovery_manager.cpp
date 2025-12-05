#include "auto_recovery_manager.h"
#include "../utils/logger.h"

namespace anywp_engine {

AutoRecoveryManager::AutoRecoveryManager() {
  Logger::Instance().Info("AutoRecoveryManager", "Module initialized");
}

AutoRecoveryManager::~AutoRecoveryManager() {
  Logger::Instance().Info("AutoRecoveryManager", 
    "Module destroyed (saved configs: " + std::to_string(saved_configs_.size()) + 
    ", attempts: " + std::to_string(recovery_attempt_count_.load()) + 
    ", successful: " + std::to_string(successful_recovery_count_.load()) + ")");
}

void AutoRecoveryManager::SetEnabled(bool enabled) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  
  bool was_enabled = enabled_.load();
  enabled_.store(enabled);
  
  Logger::Instance().Info("AutoRecoveryManager", 
    std::string("Auto recovery ") + (enabled ? "ENABLED" : "DISABLED"));
  
  // If disabling, clear saved configurations and reset state
  if (!enabled && was_enabled) {
    size_t config_count = saved_configs_.size();
    saved_configs_.clear();
    is_recovering_.store(false);
    
    Logger::Instance().Info("AutoRecoveryManager", 
      "Cleared " + std::to_string(config_count) + " saved wallpaper configuration(s)");
  }
  
  // If enabling, reset recovery state for clean start
  if (enabled && !was_enabled) {
    is_recovering_.store(false);
    
    Logger::Instance().Info("AutoRecoveryManager", 
      "Reset recovery state. Configurations will be saved when wallpapers are initialized.");
  }
}

bool AutoRecoveryManager::IsEnabled() const {
  return enabled_.load();
}

void AutoRecoveryManager::SaveWallpaperConfig(
    int monitor_index, 
    const std::string& url, 
    bool enable_mouse_transparent) {
  
  std::lock_guard<std::mutex> lock(config_mutex_);
  
  WallpaperConfig config(url, monitor_index, enable_mouse_transparent);
  saved_configs_[monitor_index] = config;
  
  Logger::Instance().Info("AutoRecoveryManager", 
    "Saved config for monitor " + std::to_string(monitor_index) + 
    ": " + url + " (transparent: " + (enable_mouse_transparent ? "true" : "false") + ")");
}

void AutoRecoveryManager::RemoveWallpaperConfig(int monitor_index) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  
  auto it = saved_configs_.find(monitor_index);
  if (it != saved_configs_.end()) {
    Logger::Instance().Info("AutoRecoveryManager", 
      "Removed config for monitor " + std::to_string(monitor_index));
    saved_configs_.erase(it);
  } else {
    Logger::Instance().Warning("AutoRecoveryManager", 
      "No config found for monitor " + std::to_string(monitor_index));
  }
}

void AutoRecoveryManager::ClearAllConfigs() {
  std::lock_guard<std::mutex> lock(config_mutex_);
  
  size_t count = saved_configs_.size();
  saved_configs_.clear();
  
  Logger::Instance().Info("AutoRecoveryManager", 
    "Cleared all configs (" + std::to_string(count) + " configs removed)");
}

const AutoRecoveryManager::WallpaperConfig* AutoRecoveryManager::GetConfig(int monitor_index) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  
  auto it = saved_configs_.find(monitor_index);
  if (it != saved_configs_.end()) {
    return &(it->second);
  }
  
  return nullptr;
}

std::map<int, AutoRecoveryManager::WallpaperConfig> AutoRecoveryManager::GetAllConfigs() const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  return saved_configs_;
}

// v2.5.0+ Thread Safety Fix: 获取需要恢复的配置，由主插件在主线程执行
std::map<int, AutoRecoveryManager::WallpaperConfig> AutoRecoveryManager::GetConfigsForRecovery() {
  if (!enabled_.load()) {
    Logger::Instance().Debug("AutoRecoveryManager", 
      "Auto recovery is disabled");
    return {};
  }

  if (is_recovering_.exchange(true)) {
    Logger::Instance().Warning("AutoRecoveryManager", 
      "Auto recovery already in progress, skipping duplicate request");
    return {};
  }

  try {
    recovery_attempt_count_++;
    
    std::map<int, WallpaperConfig> configs_copy;
    {
      std::lock_guard<std::mutex> lock(config_mutex_);
      configs_copy = saved_configs_;
    }

    if (configs_copy.empty()) {
      Logger::Instance().Debug("AutoRecoveryManager", 
        "No saved configs to recover");
      is_recovering_.store(false);
      return {};
    }

    Logger::Instance().Info("AutoRecoveryManager", 
      "Prepared " + std::to_string(configs_copy.size()) + " config(s) for recovery");

    // 返回配置，由调用者在主线程执行恢复
    // is_recovering_ 标志将在主插件完成恢复后重置
    return configs_copy;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("AutoRecoveryManager", 
      std::string("Exception in GetConfigsForRecovery: ") + e.what());
    is_recovering_.store(false);
    return {};
  }
}

// @deprecated 旧的直接恢复方法，保留用于向后兼容
size_t AutoRecoveryManager::ExecuteAutoRecovery() {
  Logger::Instance().Warning("AutoRecoveryManager", 
    "ExecuteAutoRecovery() is deprecated, use GetConfigsForRecovery() instead");
  
  if (!enabled_.load()) {
    Logger::Instance().Warning("AutoRecoveryManager", 
      "Auto recovery is disabled, skipping");
    return 0;
  }

  if (is_recovering_.exchange(true)) {
    Logger::Instance().Warning("AutoRecoveryManager", 
      "Auto recovery already in progress, skipping duplicate request");
    return 0;
  }

  try {
    recovery_attempt_count_++;
    
    std::map<int, WallpaperConfig> configs_copy;
    {
      std::lock_guard<std::mutex> lock(config_mutex_);
      configs_copy = saved_configs_;
    }

    if (configs_copy.empty()) {
      Logger::Instance().Info("AutoRecoveryManager", 
        "No saved configs to recover");
      is_recovering_.store(false);
      return 0;
    }

    Logger::Instance().Info("AutoRecoveryManager", 
      "Starting auto recovery for " + std::to_string(configs_copy.size()) + " configs");

    size_t success_count = 0;
    for (const auto& pair : configs_copy) {
      const WallpaperConfig& config = pair.second;
      
      Logger::Instance().Info("AutoRecoveryManager", 
        "Recovering config for monitor " + std::to_string(config.monitor_index) + 
        ": " + config.url);

      if (RecoverSingleConfig(config)) {
        success_count++;
        Logger::Instance().Info("AutoRecoveryManager", 
          "Successfully recovered monitor " + std::to_string(config.monitor_index));
      } else {
        Logger::Instance().Error("AutoRecoveryManager", 
          "Failed to recover monitor " + std::to_string(config.monitor_index));
      }
    }

    successful_recovery_count_ += success_count;

    Logger::Instance().Info("AutoRecoveryManager", 
      "Auto recovery completed: " + std::to_string(success_count) + "/" + 
      std::to_string(configs_copy.size()) + " succeeded");

    is_recovering_.store(false);
    return success_count;
  } catch (const std::exception& e) {
    Logger::Instance().Error("AutoRecoveryManager", 
      std::string("Exception in ExecuteAutoRecovery: ") + e.what());
    is_recovering_.store(false);
    return 0;
  }
}

void AutoRecoveryManager::CompleteRecovery() {
  is_recovering_.store(false);
  Logger::Instance().Debug("AutoRecoveryManager", "Recovery completed, flag reset");
}

void AutoRecoveryManager::SetRecoveryCallback(RecoveryCallback callback) {
  recovery_callback_ = callback;
  Logger::Instance().Info("AutoRecoveryManager", "Recovery callback registered");
}

size_t AutoRecoveryManager::GetSavedConfigCount() const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  return saved_configs_.size();
}

size_t AutoRecoveryManager::GetRecoveryAttemptCount() const {
  return recovery_attempt_count_.load();
}

size_t AutoRecoveryManager::GetSuccessfulRecoveryCount() const {
  return successful_recovery_count_.load();
}

// ========== Private Methods ==========

bool AutoRecoveryManager::RecoverSingleConfig(const WallpaperConfig& config) {
  if (!recovery_callback_) {
    Logger::Instance().Error("AutoRecoveryManager", 
      "No recovery callback registered, cannot recover");
    return false;
  }

  try {
    return recovery_callback_(config);
  } catch (const std::exception& e) {
    Logger::Instance().Error("AutoRecoveryManager", 
      std::string("Exception in recovery callback: ") + e.what());
    return false;
  }
}

}  // namespace anywp_engine

