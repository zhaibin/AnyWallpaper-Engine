#include "cpu_profiler.h"
#include "logger.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <tlhelp32.h>

#pragma comment(lib, "kernel32.lib")

namespace anywp_engine {

CPUProfiler& CPUProfiler::Instance() {
  static CPUProfiler instance;
  return instance;
}

CPUProfiler::CPUProfiler()
    : last_cpu_time_(0),
      last_system_time_(0),
      last_check_time_(0),
      cpu_sample_index_(0),
      target_fps_(60),
      last_frame_time_(0) {
  for (int i = 0; i < 10; i++) {
    recent_cpu_usage_[i] = 0.0;
  }
}

CPUProfiler::~CPUProfiler() {
  LogCPUStats("Shutdown");
}

CPUStats CPUProfiler::GetCurrentStats() {
  std::lock_guard<std::mutex> lock(mutex_);
  return QueryCPUUsage();
}

CPUStats CPUProfiler::QueryCPUUsage() {
  CPUStats stats;
  
  try {
    HANDLE hProcess = GetCurrentProcess();
    
    // Get CPU times
    FILETIME creation_time, exit_time, kernel_time, user_time;
    if (GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time)) {
      ULONGLONG current_cpu = 
          ((ULONGLONG)kernel_time.dwHighDateTime << 32 | kernel_time.dwLowDateTime) +
          ((ULONGLONG)user_time.dwHighDateTime << 32 | user_time.dwLowDateTime);
      
      FILETIME system_idle, system_kernel, system_user;
      if (GetSystemTimes(&system_idle, &system_kernel, &system_user)) {
        ULONGLONG current_system =
            ((ULONGLONG)system_kernel.dwHighDateTime << 32 | system_kernel.dwLowDateTime) +
            ((ULONGLONG)system_user.dwHighDateTime << 32 | system_user.dwLowDateTime);
        
        DWORD current_time = GetTickCount();
        
        if (last_cpu_time_ > 0 && last_system_time_ > 0) {
          ULONGLONG cpu_diff = current_cpu - last_cpu_time_;
          ULONGLONG system_diff = current_system - last_system_time_;
          
          if (system_diff > 0) {
            stats.process_cpu_percent = (cpu_diff * 100.0) / system_diff;
          }
        }
        
        last_cpu_time_ = current_cpu;
        last_system_time_ = current_system;
        last_check_time_ = current_time;
      }
    }
    
    // Get thread count
    DWORD thread_count = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
      THREADENTRY32 te;
      te.dwSize = sizeof(te);
      if (Thread32First(snapshot, &te)) {
        do {
          if (te.th32OwnerProcessID == GetCurrentProcessId()) {
            thread_count++;
          }
        } while (Thread32Next(snapshot, &te));
      }
      CloseHandle(snapshot);
    }
    stats.thread_count = thread_count;
    
    // Get handle count
    GetProcessHandleCount(hProcess, &stats.handle_count);
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("CPUProfiler", 
      std::string("Failed to query CPU: ") + e.what());
  }
  
  return stats;
}

void CPUProfiler::LogCPUStats(const std::string& context) {
  try {
    auto stats = GetCurrentStats();
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "[" << context << "] CPU Usage: "
        << stats.process_cpu_percent << "%, "
        << "Threads: " << stats.thread_count << ", "
        << "Handles: " << stats.handle_count;
    
    Logger::Instance().Info("CPUProfiler", oss.str());
  } catch (const std::exception& e) {
    Logger::Instance().Error("CPUProfiler", 
      std::string("Failed to log CPU stats: ") + e.what());
  }
}

void CPUProfiler::StartTiming(const std::string& operation) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    active_timings_[operation].start_time = 
        std::chrono::high_resolution_clock::now();
  } catch (const std::exception& e) {
    Logger::Instance().Error("CPUProfiler", 
      std::string("Failed to start timing: ") + e.what());
  }
}

void CPUProfiler::EndTiming(const std::string& operation) {
  auto end_time = std::chrono::high_resolution_clock::now();
  
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    auto it = active_timings_.find(operation);
    if (it == active_timings_.end()) {
      return;
    }
    
    auto duration = std::chrono::duration<double, std::milli>(
        end_time - it->second.start_time).count();
    
    // Update timing stats
    auto& stats = timing_stats_[operation];
    stats.name = operation;
    stats.call_count++;
    stats.total_ms += duration;
    stats.average_ms = stats.total_ms / stats.call_count;
    
    if (stats.call_count == 1) {
      stats.min_ms = duration;
      stats.max_ms = duration;
    } else {
      stats.min_ms = (std::min)(stats.min_ms, duration);
      stats.max_ms = (std::max)(stats.max_ms, duration);
    }
    
    // Keep recent durations
    it->second.durations.push_back(duration);
    if (it->second.durations.size() > 100) {
      it->second.durations.erase(it->second.durations.begin());
    }
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("CPUProfiler", 
      std::string("Failed to end timing: ") + e.what());
  }
}

TimingData CPUProfiler::GetTimingData(const std::string& operation) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = timing_stats_.find(operation);
  if (it != timing_stats_.end()) {
    return it->second;
  }
  
  return TimingData{operation, 0.0, 0.0, 0.0, 0, 0.0};
}

std::vector<TimingData> CPUProfiler::GetHotPaths(size_t top_n) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::vector<TimingData> results;
  for (const auto& pair : timing_stats_) {
    results.push_back(pair.second);
  }
  
  // Sort by total time
  std::sort(results.begin(), results.end(), 
    [](const TimingData& a, const TimingData& b) {
      return a.total_ms > b.total_ms;
    });
  
  if (results.size() > top_n) {
    results.resize(top_n);
  }
  
  return results;
}

void CPUProfiler::ResetTimingData() {
  std::lock_guard<std::mutex> lock(mutex_);
  active_timings_.clear();
  timing_stats_.clear();
  Logger::Instance().Info("CPUProfiler", "Timing data reset");
}

void CPUProfiler::OptimizeCPU() {
  try {
    LogCPUStats("Before CPU optimization");
    
    // Lower process priority if CPU usage is high
    auto stats = QueryCPUUsage();
    if (stats.process_cpu_percent > 80.0) {
      SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
      Logger::Instance().Info("CPUProfiler", "Lowered process priority");
    }
    
    LogCPUStats("After CPU optimization");
  } catch (const std::exception& e) {
    Logger::Instance().Error("CPUProfiler", 
      std::string("Failed to optimize CPU: ") + e.what());
  }
}

void CPUProfiler::SetThreadPriority(HANDLE thread, int priority) {
  try {
    if (::SetThreadPriority(thread, priority)) {
      Logger::Instance().Info("CPUProfiler", 
        "Thread priority set to " + std::to_string(priority));
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("CPUProfiler", 
      std::string("Failed to set thread priority: ") + e.what());
  }
}

std::string CPUProfiler::GenerateCPUReport() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2);
  
  auto stats = QueryCPUUsage();
  
  oss << "=== CPU Report ===\n";
  oss << "Process CPU: " << stats.process_cpu_percent << "%\n";
  oss << "Thread Count: " << stats.thread_count << "\n";
  oss << "Handle Count: " << stats.handle_count << "\n";
  oss << "\nPerformance Timings:\n";
  
  auto hot_paths = GetHotPaths(10);
  for (const auto& timing : hot_paths) {
    oss << "  " << timing.name << ":\n";
    oss << "    Calls: " << timing.call_count << "\n";
    oss << "    Avg: " << timing.average_ms << " ms\n";
    oss << "    Min/Max: " << timing.min_ms << " / " << timing.max_ms << " ms\n";
    oss << "    Total: " << timing.total_ms << " ms\n";
  }
  
  oss << "\nTarget FPS: " << target_fps_ << "\n";
  oss << "Recommended FPS: " << GetRecommendedFPS() << "\n";
  
  return oss.str();
}

void CPUProfiler::DumpTimings() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  Logger::Instance().Info("CPUProfiler", 
    "=== Timing Dump (" + std::to_string(timing_stats_.size()) + " operations) ===");
  
  for (const auto& pair : timing_stats_) {
    const auto& stats = pair.second;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "  " << stats.name << ": "
        << stats.average_ms << " ms avg ("
        << stats.call_count << " calls, "
        << stats.total_ms << " ms total)";
    Logger::Instance().Debug("CPUProfiler", oss.str());
  }
}

void CPUProfiler::SetTargetFPS(int fps) {
  std::lock_guard<std::mutex> lock(mutex_);
  target_fps_ = fps;
  Logger::Instance().Info("CPUProfiler", "Target FPS set to " + std::to_string(fps));
}

int CPUProfiler::GetRecommendedFPS() const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Calculate average CPU usage
  double avg_cpu = 0.0;
  int samples = 0;
  for (int i = 0; i < 10; i++) {
    if (recent_cpu_usage_[i] > 0.0) {
      avg_cpu += recent_cpu_usage_[i];
      samples++;
    }
  }
  
  if (samples > 0) {
    avg_cpu /= samples;
  }
  
  // Recommend FPS based on CPU usage
  if (avg_cpu < 30.0) {
    return 60;  // Low CPU, can handle 60 FPS
  } else if (avg_cpu < 60.0) {
    return 30;  // Medium CPU, throttle to 30 FPS
  } else {
    return 15;  // High CPU, throttle to 15 FPS
  }
}

bool CPUProfiler::ShouldThrottleRendering() const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  DWORD now = GetTickCount();
  DWORD frame_time = 1000 / target_fps_;
  
  return (now - last_frame_time_) < frame_time;
}

void CPUProfiler::UpdateCPUHistory() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto stats = QueryCPUUsage();
  recent_cpu_usage_[cpu_sample_index_] = stats.process_cpu_percent;
  cpu_sample_index_ = (cpu_sample_index_ + 1) % 10;
}

}  // namespace anywp_engine

