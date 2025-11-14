#ifndef ANYWP_ENGINE_CONFIG_MANAGER_H_
#define ANYWP_ENGINE_CONFIG_MANAGER_H_

#include <string>
#include <map>
#include <mutex>
#include <any>
#include <memory>
#include <functional>

namespace anywp_engine {

/**
 * @brief Configuration value with type safety
 */
class ConfigValue {
public:
  ConfigValue() = default;
  
  template<typename T>
  explicit ConfigValue(const T& value) : value_(value) {}
  
  template<typename T>
  T Get(const T& default_value = T()) const {
    try {
      return std::any_cast<T>(value_);
    } catch (const std::bad_any_cast&) {
      return default_value;
    }
  }
  
  template<typename T>
  bool TryGet(T& out_value) const {
    try {
      out_value = std::any_cast<T>(value_);
      return true;
    } catch (const std::bad_any_cast&) {
      return false;
    }
  }
  
  bool IsEmpty() const { return !value_.has_value(); }
  
private:
  std::any value_;
};

/**
 * @brief ConfigManager - Centralized configuration management system
 * 
 * Features:
 * - Type-safe configuration values
 * - Configuration validation
 * - Configuration change notifications
 * - Configuration persistence (save/load)
 * - Configuration profiles (dev, prod, test)
 * - Default values
 * - Environment variable overrides
 * 
 * Usage:
 *   // Set configuration
 *   ConfigManager::Instance().Set("webview.cache_enabled", true);
 *   ConfigManager::Instance().Set("webview.max_cache_size_mb", 100);
 *   ConfigManager::Instance().Set("log.level", "INFO");
 *   
 *   // Get configuration
 *   bool cache_enabled = ConfigManager::Instance().Get<bool>("webview.cache_enabled", false);
 *   int max_cache = ConfigManager::Instance().Get<int>("webview.max_cache_size_mb", 50);
 *   std::string log_level = ConfigManager::Instance().Get<std::string>("log.level", "INFO");
 *   
 *   // Subscribe to changes
 *   ConfigManager::Instance().Subscribe("log.level", [](const ConfigValue& value) {
 *     std::string level = value.Get<std::string>("INFO");
 *     Logger::Instance().SetMinLevel(ParseLogLevel(level));
 *   });
 *   
 *   // Save/Load
 *   ConfigManager::Instance().SaveToFile("config.json");
 *   ConfigManager::Instance().LoadFromFile("config.json");
 * 
 * Configuration Keys (Recommended):
 * - "webview.cache_enabled" (bool) - Enable WebView cache
 * - "webview.max_cache_size_mb" (int) - Maximum cache size in MB
 * - "webview.hardware_acceleration" (bool) - Enable hardware acceleration
 * - "log.level" (string) - Logging level (DEBUG, INFO, WARNING, ERROR)
 * - "log.file_enabled" (bool) - Enable file logging
 * - "log.file_path" (string) - Log file path
 * - "log.max_file_size_mb" (int) - Maximum log file size
 * - "log.rotation_enabled" (bool) - Enable log rotation
 * - "performance.cpu_profiling" (bool) - Enable CPU profiling
 * - "performance.memory_profiling" (bool) - Enable memory profiling
 * - "performance.benchmark_enabled" (bool) - Enable performance benchmarking
 * - "power.optimization_enabled" (bool) - Enable power optimization
 * - "power.auto_pause_when_hidden" (bool) - Auto-pause wallpaper when hidden
 * - "security.https_only" (bool) - Only allow HTTPS URLs
 * - "security.url_whitelist" (string[]) - Allowed URL patterns
 * - "security.permission_policy" (string) - Permission policy (default/restrictive/permissive)
 * 
 * Thread-safe: Yes
 */
class ConfigManager {
public:
  static ConfigManager& Instance();
  
  /**
   * Set configuration value
   * 
   * @param key Configuration key (use dot notation for hierarchy)
   * @param value Configuration value
   * 
   * Thread-safe: Yes
   */
  template<typename T>
  void Set(const std::string& key, const T& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_[key] = ConfigValue(value);
    NotifySubscribers(key);
  }
  
  /**
   * Get configuration value
   * 
   * @param key Configuration key
   * @param default_value Default value if key doesn't exist
   * @return Configuration value or default
   * 
   * Thread-safe: Yes
   */
  template<typename T>
  T Get(const std::string& key, const T& default_value = T()) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = config_.find(key);
    if (it != config_.end()) {
      return it->second.Get<T>(default_value);
    }
    return default_value;
  }
  
  /**
   * Check if configuration key exists
   * 
   * @param key Configuration key
   * @return true if key exists
   * 
   * Thread-safe: Yes
   */
  bool Has(const std::string& key) const;
  
  /**
   * Remove configuration key
   * 
   * @param key Configuration key
   * 
   * Thread-safe: Yes
   */
  void Remove(const std::string& key);
  
  /**
   * Clear all configuration
   * 
   * Thread-safe: Yes
   */
  void Clear();
  
  /**
   * Get all configuration keys
   * 
   * @return Vector of all keys
   * 
   * Thread-safe: Yes
   */
  std::vector<std::string> GetAllKeys() const;
  
  /**
   * Subscribe to configuration changes
   * 
   * @param key Configuration key to watch
   * @param callback Function to call when value changes
   * @return Subscription ID (use to unsubscribe)
   * 
   * Thread-safe: Yes
   */
  int Subscribe(const std::string& key, std::function<void(const ConfigValue&)> callback);
  
  /**
   * Unsubscribe from configuration changes
   * 
   * @param subscription_id Subscription ID from Subscribe()
   * 
   * Thread-safe: Yes
   */
  void Unsubscribe(int subscription_id);
  
  /**
   * Save configuration to JSON file
   * 
   * @param file_path Path to JSON file
   * @return true if successful
   * 
   * Thread-safe: Yes
   */
  bool SaveToFile(const std::string& file_path) const;
  
  /**
   * Load configuration from JSON file
   * 
   * @param file_path Path to JSON file
   * @return true if successful
   * 
   * Thread-safe: Yes
   */
  bool LoadFromFile(const std::string& file_path);
  
  /**
   * Set configuration profile
   * 
   * @param profile Profile name (e.g., "dev", "prod", "test")
   * 
   * Thread-safe: Yes
   */
  void SetProfile(const std::string& profile);
  
  /**
   * Get current profile
   * 
   * @return Current profile name
   * 
   * Thread-safe: Yes
   */
  std::string GetProfile() const;
  
  /**
   * Load environment variables as configuration
   * Looks for variables with prefix "ANYWP_"
   * Example: ANYWP_LOG_LEVEL=DEBUG sets "log.level" to "DEBUG"
   * 
   * Thread-safe: Yes
   */
  void LoadFromEnvironment();
  
  /**
   * Validate configuration
   * 
   * @return true if all configuration is valid
   * 
   * Thread-safe: Yes
   */
  bool Validate() const;
  
  /**
   * Register configuration validator
   * 
   * @param key Configuration key
   * @param validator Validation function (return true if valid)
   * 
   * Thread-safe: Yes
   */
  void RegisterValidator(const std::string& key, 
                         std::function<bool(const ConfigValue&)> validator);

private:
  ConfigManager();
  ~ConfigManager();
  
  ConfigManager(const ConfigManager&) = delete;
  ConfigManager& operator=(const ConfigManager&) = delete;
  
  void NotifySubscribers(const std::string& key);
  void LoadDefaults();
  
  struct Subscription {
    int id;
    std::string key;
    std::function<void(const ConfigValue&)> callback;
  };
  
  std::map<std::string, ConfigValue> config_;
  std::map<std::string, std::function<bool(const ConfigValue&)>> validators_;
  std::vector<Subscription> subscriptions_;
  int next_subscription_id_;
  std::string current_profile_;
  
  mutable std::mutex mutex_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_CONFIG_MANAGER_H_

