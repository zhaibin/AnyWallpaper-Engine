#include "wallpaper_configuration_manager.h"
#include "../utils/logger.h"

namespace anywp_engine {

WallpaperConfigurationManager::WallpaperConfigurationManager() {
  Logger::Instance().Info("WallpaperConfigurationManager", "Module initialized");
  LoadDefaults();
}

WallpaperConfigurationManager::~WallpaperConfigurationManager() {
  Logger::Instance().Info("WallpaperConfigurationManager", "Module destroyed");
}

void WallpaperConfigurationManager::SetIdleTimeout(DWORD timeout_ms) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  idle_timeout_ms_ = timeout_ms;
  Logger::Instance().Info("WallpaperConfigurationManager", 
    "Idle timeout set to " + std::to_string(timeout_ms) + "ms");
}

DWORD WallpaperConfigurationManager::GetIdleTimeout() const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  return idle_timeout_ms_;
}

void WallpaperConfigurationManager::SetMemoryThreshold(size_t threshold_mb) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  memory_threshold_mb_ = threshold_mb;
  Logger::Instance().Info("WallpaperConfigurationManager", 
    "Memory threshold set to " + std::to_string(threshold_mb) + "MB");
}

size_t WallpaperConfigurationManager::GetMemoryThreshold() const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  return memory_threshold_mb_;
}

void WallpaperConfigurationManager::SetCleanupInterval(int interval_minutes) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  cleanup_interval_minutes_ = interval_minutes;
  Logger::Instance().Info("WallpaperConfigurationManager", 
    "Cleanup interval set to " + std::to_string(interval_minutes) + " minutes");
}

int WallpaperConfigurationManager::GetCleanupInterval() const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  return cleanup_interval_minutes_;
}

void WallpaperConfigurationManager::SetAutoPowerSavingEnabled(bool enabled) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  auto_power_saving_enabled_ = enabled;
  Logger::Instance().Info("WallpaperConfigurationManager", 
    "Auto power saving " + std::string(enabled ? "enabled" : "disabled"));
}

bool WallpaperConfigurationManager::IsAutoPowerSavingEnabled() const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  return auto_power_saving_enabled_;
}

void WallpaperConfigurationManager::LoadDefaults() {
  std::lock_guard<std::mutex> lock(config_mutex_);
  idle_timeout_ms_ = 5 * 60 * 1000;
  memory_threshold_mb_ = 150;
  cleanup_interval_minutes_ = 15;
  auto_power_saving_enabled_ = true;
  Logger::Instance().Info("WallpaperConfigurationManager", "Loaded default configuration");
}

bool WallpaperConfigurationManager::LoadFromFile(const std::string& config_path) {
  // Note: Configuration persistence is optional. Current implementation uses
  // in-memory configuration with sensible defaults. File-based configuration
  // can be added here for advanced use cases.
  Logger::Instance().Info("WallpaperConfigurationManager", "LoadFromFile: using in-memory configuration");
  return false;
}

bool WallpaperConfigurationManager::SaveToFile(const std::string& config_path) {
  // Note: Configuration persistence is optional. Current implementation uses
  // in-memory configuration. File-based persistence can be added here if needed.
  Logger::Instance().Info("WallpaperConfigurationManager", "SaveToFile: persistence not enabled");
  return false;
}

bool WallpaperConfigurationManager::ValidateConfig() const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  
  if (idle_timeout_ms_ < 1000 || idle_timeout_ms_ > 3600000) {
    return false;
  }
  
  if (memory_threshold_mb_ < 50 || memory_threshold_mb_ > 1000) {
    return false;
  }
  
  if (cleanup_interval_minutes_ < 1 || cleanup_interval_minutes_ > 120) {
    return false;
  }
  
  return true;
}

}  // namespace anywp_engine

