#define NOMINMAX  // Prevent Windows.h from defining min/max macros
#include "performance_benchmark.h"
#include "logger.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <limits>

namespace anywp_engine {

PerformanceBenchmark& PerformanceBenchmark::Instance() {
  static PerformanceBenchmark instance;
  return instance;
}

void PerformanceBenchmark::StartTimer(const std::string& operation_name) {
  if (!enabled_) return;

  std::lock_guard<std::mutex> lock(mutex_);
  
  auto& timer = active_timers_[operation_name];
  if (timer.is_running) {
    Logger::Instance().Warn("PerformanceBenchmark", 
      "Timer already running for: " + operation_name);
    return;
  }

  timer.start_time = std::chrono::high_resolution_clock::now();
  timer.is_running = true;
}

void PerformanceBenchmark::StopTimer(const std::string& operation_name) {
  if (!enabled_) return;

  auto end_time = std::chrono::high_resolution_clock::now();

  std::lock_guard<std::mutex> lock(mutex_);

  auto timer_it = active_timers_.find(operation_name);
  if (timer_it == active_timers_.end() || !timer_it->second.is_running) {
    Logger::Instance().Warn("PerformanceBenchmark", 
      "Timer not running for: " + operation_name);
    return;
  }

  auto& timer = timer_it->second;
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - timer.start_time);
  double duration_ms = duration.count() / 1000.0;

  timer.is_running = false;

  // Update metrics
  auto& metrics = metrics_[operation_name];
  metrics.operation_name = operation_name;
  UpdateMetrics(metrics, duration_ms);
}

void PerformanceBenchmark::RecordMeasurement(const std::string& operation_name, 
                                              double duration_ms) {
  if (!enabled_) return;

  std::lock_guard<std::mutex> lock(mutex_);

  auto& metrics = metrics_[operation_name];
  metrics.operation_name = operation_name;
  UpdateMetrics(metrics, duration_ms);
}

void PerformanceBenchmark::UpdateMetrics(BenchmarkMetrics& metrics, 
                                          double duration_ms) {
  metrics.call_count++;
  metrics.total_time_ms += duration_ms;
  metrics.last_time_ms = duration_ms;

  if (metrics.call_count == 1) {
    metrics.min_time_ms = duration_ms;
    metrics.max_time_ms = duration_ms;
  } else {
    metrics.min_time_ms = std::min(metrics.min_time_ms, duration_ms);
    metrics.max_time_ms = std::max(metrics.max_time_ms, duration_ms);
  }

  metrics.avg_time_ms = metrics.total_time_ms / metrics.call_count;
}

PerformanceBenchmark::BenchmarkMetrics 
PerformanceBenchmark::GetMetrics(const std::string& operation_name) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = metrics_.find(operation_name);
  if (it != metrics_.end()) {
    return it->second;
  }

  return BenchmarkMetrics{operation_name, 0, 0.0, 0.0, 0.0, 0.0, 0.0};
}

std::vector<PerformanceBenchmark::BenchmarkMetrics> 
PerformanceBenchmark::GetAllMetrics() const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<BenchmarkMetrics> result;
  result.reserve(metrics_.size());

  for (const auto& pair : metrics_) {
    result.push_back(pair.second);
  }

  // Sort by total time (descending)
  std::sort(result.begin(), result.end(),
      [](const BenchmarkMetrics& a, const BenchmarkMetrics& b) {
        return a.total_time_ms > b.total_time_ms;
      });

  return result;
}

std::string PerformanceBenchmark::GetSummaryReport() const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::ostringstream report;
  report << std::fixed << std::setprecision(3);

  report << "\n========== Performance Benchmark Summary ==========\n";
  report << "Total Operations: " << metrics_.size() << "\n";
  report << "Benchmark Enabled: " << (enabled_ ? "Yes" : "No") << "\n";
  report << "\n";

  if (metrics_.empty()) {
    report << "No metrics recorded.\n";
    report << "===================================================\n";
    return report.str();
  }

  // Table header
  report << std::left;
  report << std::setw(40) << "Operation"
         << std::setw(10) << "Calls"
         << std::setw(12) << "Avg (ms)"
         << std::setw(12) << "Min (ms)"
         << std::setw(12) << "Max (ms)"
         << std::setw(12) << "Total (ms)"
         << "\n";
  report << std::string(98, '-') << "\n";

  // Collect and sort metrics
  std::vector<BenchmarkMetrics> sorted_metrics;
  for (const auto& pair : metrics_) {
    sorted_metrics.push_back(pair.second);
  }
  std::sort(sorted_metrics.begin(), sorted_metrics.end(),
      [](const BenchmarkMetrics& a, const BenchmarkMetrics& b) {
        return a.total_time_ms > b.total_time_ms;
      });

  // Table rows
  for (const auto& metrics : sorted_metrics) {
    report << std::setw(40) << metrics.operation_name
           << std::setw(10) << metrics.call_count
           << std::setw(12) << metrics.avg_time_ms
           << std::setw(12) << metrics.min_time_ms
           << std::setw(12) << metrics.max_time_ms
           << std::setw(12) << metrics.total_time_ms
           << "\n";
  }

  report << "===================================================\n";

  return report.str();
}

void PerformanceBenchmark::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  metrics_.clear();
  active_timers_.clear();
}

void PerformanceBenchmark::ClearOperation(const std::string& operation_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  metrics_.erase(operation_name);
  active_timers_.erase(operation_name);
}

void PerformanceBenchmark::SetEnabled(bool enabled) {
  std::lock_guard<std::mutex> lock(mutex_);
  enabled_ = enabled;
  
  if (!enabled) {
    active_timers_.clear();
  }
}

bool PerformanceBenchmark::IsEnabled() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return enabled_;
}

}  // namespace anywp_engine

