#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_PERFORMANCE_BENCHMARK_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_PERFORMANCE_BENCHMARK_H_

#include <windows.h>
#include <string>
#include <chrono>
#include <map>
#include <vector>
#include <mutex>

namespace anywp_engine {

// Performance benchmark utility for measuring operation execution time
// Thread-safe singleton implementation
class PerformanceBenchmark {
public:
  // Benchmark metrics for a specific operation
  struct BenchmarkMetrics {
    std::string operation_name;
    size_t call_count = 0;
    double total_time_ms = 0.0;
    double min_time_ms = 0.0;
    double max_time_ms = 0.0;
    double avg_time_ms = 0.0;
    double last_time_ms = 0.0;
  };

  // Singleton instance
  static PerformanceBenchmark& Instance();

  // Delete copy constructor and assignment operator
  PerformanceBenchmark(const PerformanceBenchmark&) = delete;
  PerformanceBenchmark& operator=(const PerformanceBenchmark&) = delete;

  // ========================================
  // Benchmark Operations
  // ========================================

  // Start timing an operation
  void StartTimer(const std::string& operation_name);

  // Stop timing and record the duration
  void StopTimer(const std::string& operation_name);

  // Record a single measurement (if timing is done externally)
  void RecordMeasurement(const std::string& operation_name, double duration_ms);

  // ========================================
  // Statistics
  // ========================================

  // Get metrics for a specific operation
  BenchmarkMetrics GetMetrics(const std::string& operation_name) const;

  // Get all recorded metrics
  std::vector<BenchmarkMetrics> GetAllMetrics() const;

  // Get summary report as string
  std::string GetSummaryReport() const;

  // ========================================
  // Management
  // ========================================

  // Clear all metrics
  void Clear();

  // Clear metrics for a specific operation
  void ClearOperation(const std::string& operation_name);

  // Enable/disable benchmarking
  void SetEnabled(bool enabled);
  bool IsEnabled() const;

private:
  PerformanceBenchmark() = default;
  ~PerformanceBenchmark() = default;

  // Internal data structures
  struct TimerData {
    std::chrono::high_resolution_clock::time_point start_time;
    bool is_running = false;
  };

  mutable std::mutex mutex_;
  std::map<std::string, BenchmarkMetrics> metrics_;
  std::map<std::string, TimerData> active_timers_;
  bool enabled_ = true;

  // Helper: Update metrics after recording a measurement
  void UpdateMetrics(BenchmarkMetrics& metrics, double duration_ms);
};

// RAII helper class for automatic timing
class ScopedTimer {
public:
  explicit ScopedTimer(const std::string& operation_name)
      : operation_name_(operation_name) {
    PerformanceBenchmark::Instance().StartTimer(operation_name_);
  }

  ~ScopedTimer() {
    PerformanceBenchmark::Instance().StopTimer(operation_name_);
  }

  // Delete copy constructor and assignment operator
  ScopedTimer(const ScopedTimer&) = delete;
  ScopedTimer& operator=(const ScopedTimer&) = delete;

private:
  std::string operation_name_;
};

// Convenience macro for timing a scope
#define BENCHMARK_SCOPE(name) \
  anywp_engine::ScopedTimer __benchmark_timer__(name)

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_PERFORMANCE_BENCHMARK_H_

