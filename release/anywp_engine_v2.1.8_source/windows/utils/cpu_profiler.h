#ifndef ANYWP_ENGINE_CPU_PROFILER_H_
#define ANYWP_ENGINE_CPU_PROFILER_H_

#include <windows.h>
#include <string>
#include <map>
#include <mutex>
#include <chrono>
#include <vector>

namespace anywp_engine {

/**
 * @brief CPU usage statistics
 */
struct CPUStats {
  double process_cpu_percent = 0.0;    // Process CPU usage %
  double system_cpu_percent = 0.0;     // System CPU usage %
  DWORD thread_count = 0;               // Active threads
  DWORD handle_count = 0;               // Open handles
};

/**
 * @brief Performance timing data
 */
struct TimingData {
  std::string name;
  double average_ms = 0.0;
  double min_ms = 0.0;
  double max_ms = 0.0;
  size_t call_count = 0;
  double total_ms = 0.0;
};

/**
 * @brief CPU profiler for monitoring and optimizing CPU usage
 */
class CPUProfiler {
 public:
  static CPUProfiler& Instance();
  
  // CPU statistics
  CPUStats GetCurrentStats();
  void LogCPUStats(const std::string& context);
  
  // Performance timing
  void StartTiming(const std::string& operation);
  void EndTiming(const std::string& operation);
  TimingData GetTimingData(const std::string& operation) const;
  
  // Hot path detection
  std::vector<TimingData> GetHotPaths(size_t top_n = 10);
  void ResetTimingData();
  
  // CPU optimization
  void OptimizeCPU();
  void SetThreadPriority(HANDLE thread, int priority);
  
  // Reporting
  std::string GenerateCPUReport();
  void DumpTimings();
  
  // Adaptive frame rate
  void SetTargetFPS(int fps);
  int GetRecommendedFPS() const;
  bool ShouldThrottleRendering() const;
  
 private:
  CPUProfiler();
  ~CPUProfiler();
  
  CPUProfiler(const CPUProfiler&) = delete;
  CPUProfiler& operator=(const CPUProfiler&) = delete;
  
  // Internal helpers
  CPUStats QueryCPUUsage();
  void UpdateCPUHistory();
  
  struct TimingEntry {
    std::chrono::high_resolution_clock::time_point start_time;
    std::vector<double> durations;  // Recent durations for averaging
  };
  
  mutable std::mutex mutex_;
  std::map<std::string, TimingEntry> active_timings_;
  std::map<std::string, TimingData> timing_stats_;
  
  // CPU monitoring
  ULONGLONG last_cpu_time_;
  ULONGLONG last_system_time_;
  DWORD last_check_time_;
  double recent_cpu_usage_[10];  // Last 10 samples
  int cpu_sample_index_;
  
  // Frame rate control
  int target_fps_;
  DWORD last_frame_time_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_CPU_PROFILER_H_

