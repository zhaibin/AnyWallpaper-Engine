#include "cache_manager.h"
#include "../utils/logger.h"

namespace anywp_engine {

CacheManager::CacheManager() {
  Logger::Instance().Info("CacheManager", "Module initialized");
}

CacheManager::~CacheManager() {
  Logger::Instance().Info("CacheManager", 
    "Module destroyed (cleanups performed: " + std::to_string(cleanup_count_.load()) + ")");
}

void CacheManager::Initialize(int cleanup_interval_minutes) {
  cleanup_interval_minutes_ = cleanup_interval_minutes;
  last_cleanup_ = std::chrono::steady_clock::now();
  Logger::Instance().Info("CacheManager", 
    "Cleanup interval: " + std::to_string(cleanup_interval_minutes) + " minutes");
}

bool CacheManager::ClearWebViewCache() {
  try {
    // TODO: 实现缓存清理逻辑
    Logger::Instance().Info("CacheManager", "Cache cleared (stub)");
    cleanup_count_++;
    last_cleanup_ = std::chrono::steady_clock::now();
    return true;
  } catch (const std::exception& e) {
    Logger::Instance().Error("CacheManager", 
      std::string("Exception in ClearWebViewCache: ") + e.what());
    return false;
  }
}

bool CacheManager::PeriodicCleanup() {
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - last_cleanup_).count();

  if (elapsed >= cleanup_interval_minutes_) {
    Logger::Instance().Info("CacheManager", "Periodic cleanup triggered");
    return ClearWebViewCache();
  }

  return false;
}

std::chrono::steady_clock::time_point CacheManager::GetLastCleanupTime() const {
  return last_cleanup_;
}

size_t CacheManager::GetCleanupCount() const {
  return cleanup_count_.load();
}

}  // namespace anywp_engine

