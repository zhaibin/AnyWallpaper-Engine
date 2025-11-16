#ifndef ANYWP_ENGINE_MEMORY_PROFILER_H_
#define ANYWP_ENGINE_MEMORY_PROFILER_H_

#include <windows.h>
#include <string>
#include <map>
#include <mutex>
#include <memory>

namespace anywp_engine {

/**
 * @brief Memory usage statistics
 */
struct MemoryStats {
  size_t working_set_size = 0;      // Current physical memory usage
  size_t peak_working_set = 0;      // Peak physical memory usage
  size_t private_usage = 0;         // Private memory usage
  size_t virtual_size = 0;          // Virtual memory size
  size_t available_physical = 0;    // Available physical memory
  
  // COM object tracking
  int active_webviews = 0;
  int active_windows = 0;
  
  // Custom allocations tracking
  size_t tracked_allocations = 0;
  size_t tracked_bytes = 0;
};

/**
 * @brief Memory allocation info for tracking
 */
struct AllocationInfo {
  size_t size;
  std::string tag;
  DWORD timestamp;
};

/**
 * @brief Memory profiler for monitoring and optimizing memory usage
 */
class MemoryProfiler {
 public:
  static MemoryProfiler& Instance();
  
  // Memory statistics
  MemoryStats GetCurrentStats();
  void LogMemoryStats(const std::string& context);
  
  // Allocation tracking
  void TrackAllocation(void* ptr, size_t size, const std::string& tag);
  void TrackDeallocation(void* ptr);
  size_t GetTrackedMemory() const;
  
  // Memory pressure handling
  bool IsMemoryPressureHigh() const;
  void HandleMemoryPressure();
  void SetMemoryThreshold(size_t bytes);
  
  // COM object tracking
  void RegisterWebView(void* webview);
  void UnregisterWebView(void* webview);
  void RegisterWindow(HWND hwnd);
  void UnregisterWindow(HWND hwnd);
  
  // Memory optimization
  void TrimWorkingSet();
  void ClearCaches();
  void OptimizeMemory();
  
  // Reporting
  std::string GenerateMemoryReport();
  void DumpAllocations();
  
 private:
  MemoryProfiler();
  ~MemoryProfiler();
  
  MemoryProfiler(const MemoryProfiler&) = delete;
  MemoryProfiler& operator=(const MemoryProfiler&) = delete;
  
  // Internal helpers
  MemoryStats QuerySystemMemory() const;
  void CheckMemoryThreshold();
  
  mutable std::mutex mutex_;
  std::map<void*, AllocationInfo> allocations_;
  std::map<void*, DWORD> webviews_;
  std::map<HWND, DWORD> windows_;
  
  size_t memory_threshold_;  // Bytes
  size_t peak_tracked_memory_;
  DWORD last_gc_time_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_MEMORY_PROFILER_H_

