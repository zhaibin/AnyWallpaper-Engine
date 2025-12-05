#ifndef FLUTTER_PLUGIN_CACHE_MANAGER_H_
#define FLUTTER_PLUGIN_CACHE_MANAGER_H_

#include <string>
#include <chrono>
#include <atomic>

namespace anywp_engine {

/**
 * @brief 缓存管理器
 * 
 * 管理 WebView2 缓存和定期清理：
 * - 缓存清理
 * - 定期维护
 * - 缓存大小监控
 * 
 * @since v2.5.0
 */
class CacheManager {
 public:
  CacheManager();
  ~CacheManager();

  // 禁止拷贝和赋值
  CacheManager(const CacheManager&) = delete;
  CacheManager& operator=(const CacheManager&) = delete;

  /**
   * @brief 初始化管理器
   * 
   * @param cleanup_interval_minutes 清理间隔（分钟）
   */
  void Initialize(int cleanup_interval_minutes = 15);

  /**
   * @brief 清除 WebView2 缓存
   * 
   * @return true 如果成功清除
   */
  bool ClearWebViewCache();

  /**
   * @brief 执行定期清理
   * 
   * 检查是否需要清理，如果需要则执行
   * 
   * @return true 如果执行了清理
   */
  bool PeriodicCleanup();

  /**
   * @brief 获取上次清理时间
   */
  std::chrono::steady_clock::time_point GetLastCleanupTime() const;

  /**
   * @brief 获取清理统计
   */
  size_t GetCleanupCount() const;

 private:
  int cleanup_interval_minutes_;
  std::chrono::steady_clock::time_point last_cleanup_;
  std::atomic<size_t> cleanup_count_{0};
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_CACHE_MANAGER_H_

