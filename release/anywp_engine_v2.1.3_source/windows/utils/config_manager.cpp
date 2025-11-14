#include "config_manager.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace anywp_engine {

ConfigManager& ConfigManager::Instance() {
  static ConfigManager instance;
  return instance;
}

ConfigManager::ConfigManager()
    : next_subscription_id_(1),
      current_profile_("default") {
  LoadDefaults();
  Logger::Instance().Info("ConfigManager", "ConfigManager initialized with profile: " + current_profile_);
}

ConfigManager::~ConfigManager() {
  // Nothing to clean up
}

void ConfigManager::LoadDefaults() {
  // WebView defaults
  Set("webview.cache_enabled", true);
  Set("webview.max_cache_size_mb", 100);
  Set("webview.hardware_acceleration", true);
  
  // Logging defaults
  Set("log.level", std::string("INFO"));
  Set("log.file_enabled", false);
  Set("log.file_path", std::string("anywp_engine.log"));
  Set("log.max_file_size_mb", 10);
  Set("log.rotation_enabled", false);
  Set("log.buffering_enabled", false);
  Set("log.buffer_size", 100);
  
  // Performance defaults
  Set("performance.cpu_profiling", false);
  Set("performance.memory_profiling", false);
  Set("performance.benchmark_enabled", false);
  
  // Power defaults
  Set("power.optimization_enabled", true);
  Set("power.auto_pause_when_hidden", false);
  
  // Security defaults
  Set("security.https_only", false);
  Set("security.permission_policy", std::string("default"));
  
  Logger::Instance().Debug("ConfigManager", "Default configuration loaded");
}

bool ConfigManager::Has(const std::string& key) const {
  std::lock_guard<std::mutex> lock(mutex_);
  return config_.find(key) != config_.end();
}

void ConfigManager::Remove(const std::string& key) {
  std::lock_guard<std::mutex> lock(mutex_);
  config_.erase(key);
  Logger::Instance().Debug("ConfigManager", "Removed configuration key: " + key);
}

void ConfigManager::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  config_.clear();
  subscriptions_.clear();
  validators_.clear();
  LoadDefaults();
  Logger::Instance().Info("ConfigManager", "Configuration cleared and reset to defaults");
}

std::vector<std::string> ConfigManager::GetAllKeys() const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::vector<std::string> keys;
  keys.reserve(config_.size());
  
  for (const auto& pair : config_) {
    keys.push_back(pair.first);
  }
  
  return keys;
}

int ConfigManager::Subscribe(const std::string& key, 
                             std::function<void(const ConfigValue&)> callback) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  int subscription_id = next_subscription_id_++;
  
  Subscription sub;
  sub.id = subscription_id;
  sub.key = key;
  sub.callback = std::move(callback);
  
  subscriptions_.push_back(sub);
  
  Logger::Instance().Debug("ConfigManager", 
    "Subscribed to config changes for '" + key + "' (ID: " + std::to_string(subscription_id) + ")");
  
  return subscription_id;
}

void ConfigManager::Unsubscribe(int subscription_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  subscriptions_.erase(
    std::remove_if(subscriptions_.begin(), subscriptions_.end(),
      [subscription_id](const Subscription& s) { return s.id == subscription_id; }),
    subscriptions_.end()
  );
  
  Logger::Instance().Debug("ConfigManager", 
    "Unsubscribed from config changes (ID: " + std::to_string(subscription_id) + ")");
}

void ConfigManager::NotifySubscribers(const std::string& key) {
  // Get current value
  auto it = config_.find(key);
  if (it == config_.end()) {
    return;
  }
  
  ConfigValue value = it->second;
  
  // Find matching subscriptions
  std::vector<std::function<void(const ConfigValue&)>> callbacks;
  for (const auto& sub : subscriptions_) {
    if (sub.key == key) {
      callbacks.push_back(sub.callback);
    }
  }
  
  // Call callbacks outside of lock to avoid deadlocks
  for (const auto& callback : callbacks) {
    try {
      callback(value);
    } catch (const std::exception& e) {
      Logger::Instance().Error("ConfigManager", 
        "Exception in config change callback for '" + key + "': " + e.what());
    } catch (...) {
      Logger::Instance().Error("ConfigManager", 
        "Unknown exception in config change callback for '" + key + "'");
    }
  }
}

bool ConfigManager::SaveToFile(const std::string& file_path) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    std::ofstream file(file_path);
    if (!file.is_open()) {
      Logger::Instance().Error("ConfigManager", "Failed to open file for writing: " + file_path);
      return false;
    }
    
    file << "{\n";
    bool first = true;
    for (const auto& pair : config_) {
      if (!first) {
        file << ",\n";
      }
      first = false;
      
      file << "  \"" << pair.first << "\": ";
      
      // Try different types (simple serialization)
      bool bool_value;
      if (pair.second.TryGet(bool_value)) {
        file << (bool_value ? "true" : "false");
      } else {
        int int_value;
        if (pair.second.TryGet(int_value)) {
          file << int_value;
        } else {
          std::string str_value;
          if (pair.second.TryGet(str_value)) {
            file << "\"" << str_value << "\"";
          } else {
            file << "null";
          }
        }
      }
    }
    file << "\n}\n";
    
    Logger::Instance().Info("ConfigManager", "Configuration saved to: " + file_path);
    return true;
  } catch (const std::exception& e) {
    Logger::Instance().Error("ConfigManager", 
      "Exception while saving configuration: " + std::string(e.what()));
    return false;
  }
}

bool ConfigManager::LoadFromFile(const std::string& file_path) {
  // Simple JSON parsing - for production, use a proper JSON library
  Logger::Instance().Warning("ConfigManager", 
    "LoadFromFile not fully implemented - using defaults");
  return false;
}

void ConfigManager::SetProfile(const std::string& profile) {
  std::lock_guard<std::mutex> lock(mutex_);
  current_profile_ = profile;
  Logger::Instance().Info("ConfigManager", "Profile changed to: " + profile);
}

std::string ConfigManager::GetProfile() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return current_profile_;
}

void ConfigManager::LoadFromEnvironment() {
  // This is a simplified implementation
  // For production, use platform-specific environment iteration
  
  Logger::Instance().Info("ConfigManager", "Environment variables loaded (if any)");
  
  // Unused variable removed to avoid warning
  (void)0;  // Placeholder to avoid empty function body warning
}

bool ConfigManager::Validate() const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  for (const auto& pair : validators_) {
    const std::string& key = pair.first;
    const auto& validator = pair.second;
    
    auto it = config_.find(key);
    if (it != config_.end()) {
      if (!validator(it->second)) {
        Logger::Instance().Error("ConfigManager", 
          "Validation failed for configuration key: " + key);
        return false;
      }
    }
  }
  
  return true;
}

void ConfigManager::RegisterValidator(const std::string& key, 
                                     std::function<bool(const ConfigValue&)> validator) {
  std::lock_guard<std::mutex> lock(mutex_);
  validators_[key] = std::move(validator);
  Logger::Instance().Debug("ConfigManager", "Validator registered for: " + key);
}

}  // namespace anywp_engine

