#include "memory_optimizer.h"
#include "../utils/logger.h"
#include "../utils/error_handler.h"
#include <psapi.h>
#include <sstream>
#include <iomanip>

namespace anywp_engine {

MemoryOptimizer::MemoryOptimizer() {
  Logger::Instance().Info("MemoryOptimizer", "Module created");
}

MemoryOptimizer::~MemoryOptimizer() {
  Shutdown();
}

bool MemoryOptimizer::Initialize() {
  Logger::Instance().Info("MemoryOptimizer", "Initializing module...");
  
  // Reset statistics
  stats_ = OptimizationStats{};
  
  // Initialize memory history
  history_index_ = 0;
  history_count_ = 0;
  for (size_t i = 0; i < kMaxHistorySize; i++) {
    memory_history_[i] = 0;
  }
  
  // Record initial memory sample
  RecordMemorySample();
  
  Logger::Instance().Info("MemoryOptimizer", "Module initialized successfully");
  return true;
}

void MemoryOptimizer::Shutdown() {
  // Prevent double shutdown
  if (shutdown_requested_) {
    return;
  }
  
  Logger::Instance().Info("MemoryOptimizer", "Shutting down module...");
  
  shutdown_requested_ = true;
  
  StopAutoOptimization();
  StopPeriodicTrimming();
  CancelScheduledCacheClear();
  
  // Clear any scheduled webview reference to prevent use-after-free
  {
    std::lock_guard<std::mutex> lock(scheduled_mutex_);
    scheduled_webview_.Reset();
  }
  
  Logger::Instance().Info("MemoryOptimizer", "Module shutdown complete");
}

void MemoryOptimizer::ClearWebViewCache(Microsoft::WRL::ComPtr<ICoreWebView2> webview) {
  // Safety check: null webview
  if (!webview) {
    return;
  }
  
  // Safety check: shutdown in progress
  if (shutdown_requested_) {
    Logger::Instance().Warning("MemoryOptimizer", "ClearWebViewCache called during shutdown, skipping");
    return;
  }
  
  TRY_CATCH_CONTINUE("MemoryOptimizer", "ClearWebViewCache", {
    ClearWebViewCacheInternal(webview);
    
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.cache_clears++;
  });
}

void MemoryOptimizer::ClearWebViewCacheInternal(Microsoft::WRL::ComPtr<ICoreWebView2> webview) {
  if (!webview) {
    return;
  }
  
  Microsoft::WRL::ComPtr<ICoreWebView2_2> webview2;
  HRESULT hr = webview.As(&webview2);
  if (FAILED(hr) || !webview2) {
    if (config_.log_operations) {
      Logger::Instance().Warning("MemoryOptimizer", "WebView2_2 interface not available");
    }
    return;
  }
  
  // Clear browsing data - use ExecuteScript as fallback
  // Note: ClearBrowsingData may not be available in all WebView2 SDK versions
  std::wstring script = L"(function() { \
    try { \
      if (window.caches) { \
        caches.keys().then(function(names) { \
          names.forEach(function(name) { caches.delete(name); }); \
        }); \
      } \
      if (window.localStorage) { window.localStorage.clear(); } \
      if (window.sessionStorage) { window.sessionStorage.clear(); } \
    } catch(e) { console.error('Cache clear failed:', e); } \
  })()";
  
  webview->ExecuteScript(script.c_str(), nullptr);
  
  if (config_.log_operations) {
    Logger::Instance().Info("MemoryOptimizer", "WebView cache cleared via script");
  }
}

void MemoryOptimizer::ScheduleSafeCacheClear(Microsoft::WRL::ComPtr<ICoreWebView2> webview, int delay_ms) {
  // Simplified implementation - immediate execution
  if (delay_ms <= 0) {
    ClearWebViewCache(webview);
  }
}

void MemoryOptimizer::CancelScheduledCacheClear() {
  // Simplified - no-op
}

bool MemoryOptimizer::TrimWorkingSet() {
  return TrimWorkingSetInternal();
}

bool MemoryOptimizer::TrimWorkingSetInternal() {
  TRY_CATCH_REPORT("MemoryOptimizer", "TrimWorkingSet", {
    HANDLE process = GetCurrentProcess();
    if (SetProcessWorkingSetSize(process, (SIZE_T)-1, (SIZE_T)-1)) {
      std::lock_guard<std::mutex> lock(stats_mutex_);
      stats_.working_set_trims++;
      
      if (config_.log_operations) {
        Logger::Instance().Info("MemoryOptimizer", "Working set trimmed successfully");
      }
      return true;
    }
    return false;
  });
}

void MemoryOptimizer::SchedulePeriodicTrimming(int interval_ms) {
  // Simplified - not implemented
}

void MemoryOptimizer::StopPeriodicTrimming() {
  periodic_trimming_running_ = false;
}

MemoryOptimizer::MemoryStats MemoryOptimizer::GetMemoryStats() const {
  MemoryStats stats;
  
  // Get system memory info
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(MEMORYSTATUSEX);
  if (GlobalMemoryStatusEx(&memInfo)) {
    stats.total_memory_mb = memInfo.ullTotalPhys / (1024 * 1024);
    stats.available_memory_mb = memInfo.ullAvailPhys / (1024 * 1024);
    stats.used_memory_mb = stats.total_memory_mb - stats.available_memory_mb;
    stats.usage_percent = memInfo.dwMemoryLoad;
  }
  
  // Get process memory info
  PROCESS_MEMORY_COUNTERS pmc;
  if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
    stats.process_memory_mb = pmc.WorkingSetSize / (1024 * 1024);
    stats.process_peak_mb = pmc.PeakWorkingSetSize / (1024 * 1024);
  }
  
  return stats;
}

bool MemoryOptimizer::IsMemoryExceeded(size_t threshold_mb) const {
  auto stats = GetMemoryStats();
  return stats.process_memory_mb > threshold_mb;
}

MemoryOptimizer::MemoryTrend MemoryOptimizer::GetMemoryTrend() const {
  if (history_count_ < 3) {
    return MemoryTrend::UNKNOWN;
  }
  
  // Simple trend: compare last 3 samples
  size_t idx1 = (history_index_ + kMaxHistorySize - 1) % kMaxHistorySize;
  size_t idx2 = (history_index_ + kMaxHistorySize - 2) % kMaxHistorySize;
  size_t idx3 = (history_index_ + kMaxHistorySize - 3) % kMaxHistorySize;
  
  size_t recent = memory_history_[idx1];
  size_t middle = memory_history_[idx2];
  size_t older = memory_history_[idx3];
  
  if (recent > middle && middle > older) {
    return MemoryTrend::INCREASING;
  } else if (recent < middle && middle < older) {
    return MemoryTrend::DECREASING;
  }
  
  return MemoryTrend::STABLE;
}

void MemoryOptimizer::SetOptimizationConfig(const OptimizationConfig& config) {
  config_mutex_.lock();
  config_ = config;
  config_mutex_.unlock();
  Logger::Instance().Info("MemoryOptimizer", "Configuration updated");
}

MemoryOptimizer::OptimizationConfig MemoryOptimizer::GetOptimizationConfig() const {
  const_cast<std::mutex&>(config_mutex_).lock();
  OptimizationConfig result = config_;
  const_cast<std::mutex&>(config_mutex_).unlock();
  return result;
}

void MemoryOptimizer::StartAutoOptimization() {
  auto_optimization_running_ = true;
  Logger::Instance().Info("MemoryOptimizer", "Auto optimization started");
}

void MemoryOptimizer::StopAutoOptimization() {
  auto_optimization_running_ = false;
  Logger::Instance().Info("MemoryOptimizer", "Auto optimization stopped");
}

void MemoryOptimizer::OptimizeMemory(Microsoft::WRL::ComPtr<ICoreWebView2> webview) {
  // Safety check: prevent optimization during shutdown
  if (shutdown_requested_) {
    Logger::Instance().Warning("MemoryOptimizer", "OptimizeMemory called during shutdown, skipping");
    return;
  }
  
  Logger::Instance().Info("MemoryOptimizer", "Starting memory optimization...");
  
  size_t memory_before = 0;
  size_t memory_after = 0;
  
  try {
    memory_before = GetMemoryStats().process_memory_mb;
    
    // Safety: Check config under lock
    bool should_clear_cache = false;
    bool should_trim = false;
    {
      std::lock_guard<std::mutex> lock(config_mutex_);
      should_clear_cache = config_.clear_cache;
      should_trim = config_.trim_working_set;
    }
    
    if (should_clear_cache && webview) {
      ClearWebViewCache(webview);
    }
    
    if (should_trim) {
      TrimWorkingSet();
    }
    
    memory_after = GetMemoryStats().process_memory_mb;
  } catch (const std::exception& e) {
    Logger::Instance().Error("MemoryOptimizer", 
      std::string("Exception during OptimizeMemory: ") + e.what());
    return;
  } catch (...) {
    Logger::Instance().Error("MemoryOptimizer", "Unknown exception during OptimizeMemory");
    return;
  }
  
  size_t freed = (memory_before > memory_after) ? (memory_before - memory_after) : 0;
  
  {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.total_optimizations++;
    stats_.memory_freed_mb += freed;
    stats_.last_optimization_time = GetTickCount64();
  }
  
  if (config_.log_operations) {
    std::stringstream ss;
    ss << "Memory optimization complete. Before: " << memory_before 
       << " MB, After: " << memory_after << " MB, Freed: " << freed << " MB";
    Logger::Instance().Info("MemoryOptimizer", ss.str());
  }
}

void MemoryOptimizer::AggressiveOptimization(Microsoft::WRL::ComPtr<ICoreWebView2> webview) {
  Logger::Instance().Info("MemoryOptimizer", "Starting aggressive optimization...");
  
  if (webview) {
    ClearWebViewCache(webview);
  }
  
  TrimWorkingSet();
  
  // Additional aggressive steps
  SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);
  
  Logger::Instance().Info("MemoryOptimizer", "Aggressive optimization complete");
}

MemoryOptimizer::OptimizationStats MemoryOptimizer::GetOptimizationStats() const {
  const_cast<std::mutex&>(stats_mutex_).lock();
  OptimizationStats result = stats_;
  const_cast<std::mutex&>(stats_mutex_).unlock();
  return result;
}

void MemoryOptimizer::ResetStats() {
  stats_mutex_.lock();
  stats_ = OptimizationStats{};
  stats_mutex_.unlock();
  Logger::Instance().Info("MemoryOptimizer", "Statistics reset");
}

std::string MemoryOptimizer::GenerateReport() const {
  auto stats = GetOptimizationStats();
  auto mem_stats = GetMemoryStats();
  auto trend = GetMemoryTrend();
  
  std::stringstream ss;
  ss << "\n=== MemoryOptimizer Report ===\n";
  ss << "Current Memory: " << mem_stats.process_memory_mb << " MB (Peak: " << mem_stats.process_peak_mb << " MB)\n";
  ss << "System Usage: " << std::fixed << std::setprecision(1) << mem_stats.usage_percent << "%\n";
  ss << "Trend: " << (trend == MemoryTrend::INCREASING ? "INCREASING" :
                       trend == MemoryTrend::DECREASING ? "DECREASING" :
                       trend == MemoryTrend::STABLE ? "STABLE" : "UNKNOWN") << "\n";
  ss << "\nOptimizations: " << stats.total_optimizations << "\n";
  ss << "Cache Clears: " << stats.cache_clears << "\n";
  ss << "Working Set Trims: " << stats.working_set_trims << "\n";
  ss << "Memory Freed: " << stats.memory_freed_mb << " MB\n";
  ss << "============================\n";
  
  return ss.str();
}

void MemoryOptimizer::RecordMemorySample() {
  auto stats = GetMemoryStats();
  memory_history_[history_index_] = stats.process_memory_mb;
  history_index_ = (history_index_ + 1) % kMaxHistorySize;
  if (history_count_ < kMaxHistorySize) {
    history_count_++;
  }
}

void MemoryOptimizer::CheckAndOptimize() {
  if (IsMemoryExceeded(config_.threshold_mb)) {
    Logger::Instance().Warning("MemoryOptimizer", "Memory threshold exceeded, triggering optimization");
    OptimizeMemory();
  }
  
  RecordMemorySample();
}

}  // namespace anywp_engine

