#include "memory_profiler.h"
#include "logger.h"
#include <psapi.h>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "psapi.lib")

namespace anywp_engine {

MemoryProfiler& MemoryProfiler::Instance() {
  static MemoryProfiler instance;
  return instance;
}

MemoryProfiler::MemoryProfiler()
    : memory_threshold_(300 * 1024 * 1024),  // 300 MB default
      peak_tracked_memory_(0),
      last_gc_time_(0) {
}

MemoryProfiler::~MemoryProfiler() {
  // Log final memory stats
  LogMemoryStats("Shutdown");
}

MemoryStats MemoryProfiler::GetCurrentStats() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  MemoryStats stats = QuerySystemMemory();
  
  // Add tracked info
  stats.active_webviews = static_cast<int>(webviews_.size());
  stats.active_windows = static_cast<int>(windows_.size());
  stats.tracked_allocations = allocations_.size();
  stats.tracked_bytes = GetTrackedMemory();
  
  return stats;
}

MemoryStats MemoryProfiler::QuerySystemMemory() const {
  MemoryStats stats;
  
  try {
    // Get process memory info
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), 
                            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), 
                            sizeof(pmc))) {
      stats.working_set_size = pmc.WorkingSetSize;
      stats.peak_working_set = pmc.PeakWorkingSetSize;
      stats.private_usage = pmc.PrivateUsage;
    }
    
    // Get system memory info
    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(mem_status);
    if (GlobalMemoryStatusEx(&mem_status)) {
      stats.available_physical = static_cast<size_t>(mem_status.ullAvailPhys);
      stats.virtual_size = static_cast<size_t>(mem_status.ullTotalVirtual - mem_status.ullAvailVirtual);
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("MemoryProfiler", 
      std::string("Failed to query memory: ") + e.what());
  }
  
  return stats;
}

void MemoryProfiler::LogMemoryStats(const std::string& context) {
  try {
    auto stats = GetCurrentStats();
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "[" << context << "] Memory Usage: "
        << (stats.working_set_size / (1024.0 * 1024.0)) << " MB (working set), "
        << (stats.private_usage / (1024.0 * 1024.0)) << " MB (private), "
        << "Peak: " << (stats.peak_working_set / (1024.0 * 1024.0)) << " MB, "
        << "WebViews: " << stats.active_webviews << ", "
        << "Windows: " << stats.active_windows << ", "
        << "Tracked: " << (stats.tracked_bytes / 1024.0) << " KB";
    
    Logger::Instance().Info("MemoryProfiler", oss.str());
  } catch (const std::exception& e) {
    Logger::Instance().Error("MemoryProfiler", 
      std::string("Failed to log memory stats: ") + e.what());
  }
}

void MemoryProfiler::TrackAllocation(void* ptr, size_t size, const std::string& tag) {
  if (!ptr || size == 0) return;
  
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    AllocationInfo info;
    info.size = size;
    info.tag = tag;
    info.timestamp = GetTickCount();
    
    allocations_[ptr] = info;
    
    size_t total = GetTrackedMemory();
    if (total > peak_tracked_memory_) {
      peak_tracked_memory_ = total;
    }
    
    CheckMemoryThreshold();
  } catch (const std::exception& e) {
    Logger::Instance().Error("MemoryProfiler", 
      std::string("Failed to track allocation: ") + e.what());
  }
}

void MemoryProfiler::TrackDeallocation(void* ptr) {
  if (!ptr) return;
  
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    allocations_.erase(ptr);
  } catch (const std::exception& e) {
    Logger::Instance().Error("MemoryProfiler", 
      std::string("Failed to track deallocation: ") + e.what());
  }
}

size_t MemoryProfiler::GetTrackedMemory() const {
  size_t total = 0;
  for (const auto& pair : allocations_) {
    total += pair.second.size;
  }
  return total;
}

bool MemoryProfiler::IsMemoryPressureHigh() const {
  try {
    auto stats = QuerySystemMemory();
    
    // High pressure if:
    // 1. Using > threshold
    // 2. Available physical < 500 MB
    return stats.working_set_size > memory_threshold_ ||
           stats.available_physical < (500 * 1024 * 1024);
  } catch (...) {
    return false;
  }
}

void MemoryProfiler::HandleMemoryPressure() {
  try {
    Logger::Instance().Warning("MemoryProfiler", "Handling memory pressure");
    
    // Try to reduce memory usage
    ClearCaches();
    TrimWorkingSet();
    
    LogMemoryStats("After pressure handling");
  } catch (const std::exception& e) {
    Logger::Instance().Error("MemoryProfiler", 
      std::string("Failed to handle memory pressure: ") + e.what());
  }
}

void MemoryProfiler::SetMemoryThreshold(size_t bytes) {
  std::lock_guard<std::mutex> lock(mutex_);
  memory_threshold_ = bytes;
  Logger::Instance().Info("MemoryProfiler", 
    "Memory threshold set to " + std::to_string(bytes / (1024 * 1024)) + " MB");
}

void MemoryProfiler::RegisterWebView(void* webview) {
  if (!webview) return;
  
  std::lock_guard<std::mutex> lock(mutex_);
  webviews_[webview] = GetTickCount();
  
  Logger::Instance().Debug("MemoryProfiler", 
    "Registered WebView, total: " + std::to_string(webviews_.size()));
}

void MemoryProfiler::UnregisterWebView(void* webview) {
  if (!webview) return;
  
  std::lock_guard<std::mutex> lock(mutex_);
  webviews_.erase(webview);
  
  Logger::Instance().Debug("MemoryProfiler", 
    "Unregistered WebView, total: " + std::to_string(webviews_.size()));
}

void MemoryProfiler::RegisterWindow(HWND hwnd) {
  if (!hwnd) return;
  
  std::lock_guard<std::mutex> lock(mutex_);
  windows_[hwnd] = GetTickCount();
  
  Logger::Instance().Debug("MemoryProfiler", 
    "Registered Window, total: " + std::to_string(windows_.size()));
}

void MemoryProfiler::UnregisterWindow(HWND hwnd) {
  if (!hwnd) return;
  
  std::lock_guard<std::mutex> lock(mutex_);
  windows_.erase(hwnd);
  
  Logger::Instance().Debug("MemoryProfiler", 
    "Unregistered Window, total: " + std::to_string(windows_.size()));
}

void MemoryProfiler::TrimWorkingSet() {
  try {
    // Ask OS to trim working set
    if (SetProcessWorkingSetSize(GetCurrentProcess(), 
                                 static_cast<SIZE_T>(-1), 
                                 static_cast<SIZE_T>(-1))) {
      Logger::Instance().Info("MemoryProfiler", "Working set trimmed");
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("MemoryProfiler", 
      std::string("Failed to trim working set: ") + e.what());
  }
}

void MemoryProfiler::ClearCaches() {
  // Placeholder for cache clearing
  // Can be implemented to clear internal caches
  Logger::Instance().Info("MemoryProfiler", "Caches cleared");
}

void MemoryProfiler::OptimizeMemory() {
  try {
    LogMemoryStats("Before optimization");
    
    // Trim working set
    TrimWorkingSet();
    
    // Clear caches
    ClearCaches();
    
    // Update last GC time
    last_gc_time_ = GetTickCount();
    
    LogMemoryStats("After optimization");
  } catch (const std::exception& e) {
    Logger::Instance().Error("MemoryProfiler", 
      std::string("Failed to optimize memory: ") + e.what());
  }
}

std::string MemoryProfiler::GenerateMemoryReport() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2);
  
  auto stats = QuerySystemMemory();
  
  oss << "=== Memory Report ===\n";
  oss << "Working Set: " << (stats.working_set_size / (1024.0 * 1024.0)) << " MB\n";
  oss << "Peak Working Set: " << (stats.peak_working_set / (1024.0 * 1024.0)) << " MB\n";
  oss << "Private Usage: " << (stats.private_usage / (1024.0 * 1024.0)) << " MB\n";
  oss << "Virtual Size: " << (stats.virtual_size / (1024.0 * 1024.0)) << " MB\n";
  oss << "Available Physical: " << (stats.available_physical / (1024.0 * 1024.0)) << " MB\n";
  oss << "\nTracked Resources:\n";
  oss << "  WebViews: " << webviews_.size() << "\n";
  oss << "  Windows: " << windows_.size() << "\n";
  oss << "  Allocations: " << allocations_.size() << " (" 
      << (GetTrackedMemory() / 1024.0) << " KB)\n";
  oss << "  Peak Tracked: " << (peak_tracked_memory_ / 1024.0) << " KB\n";
  oss << "\nThreshold: " << (memory_threshold_ / (1024.0 * 1024.0)) << " MB\n";
  oss << "Memory Pressure: " << (IsMemoryPressureHigh() ? "HIGH" : "Normal") << "\n";
  
  return oss.str();
}

void MemoryProfiler::DumpAllocations() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  Logger::Instance().Info("MemoryProfiler", 
    "=== Allocation Dump (" + std::to_string(allocations_.size()) + " allocations) ===");
  
  for (const auto& pair : allocations_) {
    std::ostringstream oss;
    oss << "  " << pair.first << ": " 
        << (pair.second.size / 1024.0) << " KB ["
        << pair.second.tag << "] age: "
        << (GetTickCount() - pair.second.timestamp) << " ms";
    Logger::Instance().Debug("MemoryProfiler", oss.str());
  }
}

void MemoryProfiler::CheckMemoryThreshold() {
  try {
    auto stats = QuerySystemMemory();
    
    if (stats.working_set_size > memory_threshold_) {
      // Auto-optimize if threshold exceeded and enough time passed
      DWORD now = GetTickCount();
      if (last_gc_time_ == 0 || (now - last_gc_time_) > 60000) {  // 1 minute cooldown
        Logger::Instance().Warning("MemoryProfiler", 
          "Memory threshold exceeded, auto-optimizing");
        OptimizeMemory();
      }
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("MemoryProfiler", 
      std::string("Failed to check memory threshold: ") + e.what());
  }
}

}  // namespace anywp_engine

