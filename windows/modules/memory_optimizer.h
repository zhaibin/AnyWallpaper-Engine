#ifndef ANYWP_ENGINE_MEMORY_OPTIMIZER_H_
#define ANYWP_ENGINE_MEMORY_OPTIMIZER_H_

#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <string>
#include <functional>
#include <atomic>
#include <mutex>

namespace anywp_engine {

/**
 * @brief MemoryOptimizer - Unified memory optimization module (v2.1.0+)
 * 
 * Centralizes all memory optimization logic:
 * - WebView2 cache cleanup
 * - Working Set trimming  
 * - Memory profiling
 * - Scheduled optimization
 * 
 * Benefits:
 * - Single source of truth for memory operations
 * - Configurable optimization strategies
 * - Performance tracking
 * - Thread-safe operations
 */
class MemoryOptimizer {
public:
  MemoryOptimizer();
  ~MemoryOptimizer();

  // Initialization
  bool Initialize();
  void Shutdown();

  /**
   * @brief WebView2 cache optimization
   */
  
  // Clear WebView2 cache for specific instance
  void ClearWebViewCache(Microsoft::WRL::ComPtr<ICoreWebView2> webview);
  
  // Schedule safe cache clearing (uses delay to avoid crashes)
  void ScheduleSafeCacheClear(Microsoft::WRL::ComPtr<ICoreWebView2> webview, int delay_ms = 5000);
  
  // Cancel scheduled cache clearing
  void CancelScheduledCacheClear();

  /**
   * @brief Working Set optimization
   */
  
  // Trim current process working set
  bool TrimWorkingSet();
  
  // Schedule periodic working set trimming
  void SchedulePeriodicTrimming(int interval_ms = 300000);  // Default: 5 minutes
  
  // Stop periodic trimming
  void StopPeriodicTrimming();

  /**
   * @brief Memory profiling and monitoring
   */
  
  struct MemoryStats {
    size_t total_memory_mb{0};
    size_t used_memory_mb{0};
    size_t available_memory_mb{0};
    size_t process_memory_mb{0};
    size_t process_peak_mb{0};
    double usage_percent{0.0};
  };
  
  // Get current memory statistics
  MemoryStats GetMemoryStats() const;
  
  // Check if memory usage exceeds threshold
  bool IsMemoryExceeded(size_t threshold_mb = 300) const;
  
  // Get memory usage trend (increasing/decreasing/stable)
  enum class MemoryTrend { INCREASING, DECREASING, STABLE, UNKNOWN };
  MemoryTrend GetMemoryTrend() const;

  /**
   * @brief Automatic optimization
   */
  
  struct OptimizationConfig {
    bool auto_optimize{true};           // Enable automatic optimization
    size_t threshold_mb{250};           // Trigger optimization when exceeds this
    int check_interval_ms{60000};       // Check every 60 seconds
    bool clear_cache{true};             // Clear WebView cache
    bool trim_working_set{true};        // Trim working set
    bool log_operations{true};          // Log optimization operations
  };
  
  // Configure automatic optimization
  void SetOptimizationConfig(const OptimizationConfig& config);
  
  // Get current configuration
  OptimizationConfig GetOptimizationConfig() const;
  
  // Start automatic optimization monitoring
  void StartAutoOptimization();
  
  // Stop automatic optimization
  void StopAutoOptimization();

  /**
   * @brief Manual optimization
   */
  
  // Perform full memory optimization
  void OptimizeMemory(Microsoft::WRL::ComPtr<ICoreWebView2> webview = nullptr);
  
  // Aggressive optimization (clear everything)
  void AggressiveOptimization(Microsoft::WRL::ComPtr<ICoreWebView2> webview = nullptr);

  /**
   * @brief Statistics and reporting
   */
  
  struct OptimizationStats {
    uint64_t total_optimizations{0};
    uint64_t cache_clears{0};
    uint64_t working_set_trims{0};
    size_t memory_freed_mb{0};
    uint64_t last_optimization_time{0};
  };
  
  // Get optimization statistics
  OptimizationStats GetOptimizationStats() const;
  
  // Reset statistics
  void ResetStats();
  
  // Generate optimization report
  std::string GenerateReport() const;

private:
  // Internal optimization methods
  void ClearWebViewCacheInternal(Microsoft::WRL::ComPtr<ICoreWebView2> webview);
  bool TrimWorkingSetInternal();
  void CheckAndOptimize();
  void RecordMemorySample();
  
  // Threading
  void AutoOptimizationThread();
  void PeriodicTrimmingThread();
  
  // Configuration
  OptimizationConfig config_;
  std::mutex config_mutex_;
  
  // Statistics
  OptimizationStats stats_;
  std::mutex stats_mutex_;
  
  // Memory history (for trend analysis)
  static constexpr size_t kMaxHistorySize = 10;
  size_t memory_history_[kMaxHistorySize];
  size_t history_index_{0};
  size_t history_count_{0};
  
  // Threading
  std::atomic<bool> auto_optimization_running_{false};
  std::atomic<bool> periodic_trimming_running_{false};
  std::atomic<bool> shutdown_requested_{false};
  
  // Scheduled tasks
  HANDLE scheduled_cache_clear_timer_{nullptr};
  Microsoft::WRL::ComPtr<ICoreWebView2> scheduled_webview_;
  std::mutex scheduled_mutex_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_MEMORY_OPTIMIZER_H_

