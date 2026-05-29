#include "workerw_health_monitor.h"
#include "../utils/logger.h"
#include <iostream>

namespace anywp_engine {

WorkerWHealthMonitor::WorkerWHealthMonitor()
    : workerw_hwnd_(nullptr),
      should_stop_(false),
      is_monitoring_(false),
      health_status_(HealthStatus::kUnknown),
      consecutive_failures_(0),
      check_interval_ms_(3000),
      last_check_time_(std::chrono::steady_clock::now()),
      last_recovery_time_(std::chrono::steady_clock::now()),
      last_explorer_pid_(0),
      check_counter_(0) {
  Logger::Instance().Info("WorkerWHealthMonitor", "Health monitor initialized");
  
  // v2.3.1+ Enhanced: Initialize Explorer PID
  HWND hShell = FindWindowW(L"Shell_TrayWnd", nullptr);
  if (hShell) {
    GetWindowThreadProcessId(hShell, &last_explorer_pid_);
    if (last_explorer_pid_ > 0) {
      Logger::Instance().Info("WorkerWHealthMonitor", 
        "Initial Explorer PID: " + std::to_string(last_explorer_pid_));
    }
  }
}

WorkerWHealthMonitor::~WorkerWHealthMonitor() {
  StopMonitoring();
  Logger::Instance().Info("WorkerWHealthMonitor", "Health monitor destroyed");
}

bool WorkerWHealthMonitor::StartMonitoring(HWND workerw, int check_interval_ms) {
  if (!workerw || !IsWindow(workerw)) {
    Logger::Instance().Error("WorkerWHealthMonitor", 
      "Cannot start monitoring: invalid WorkerW handle");
    return false;
  }
  
  if (is_monitoring_.load()) {
    Logger::Instance().Warning("WorkerWHealthMonitor", 
      "Already monitoring, stopping previous monitor first");
    StopMonitoring();
  }
  
  std::lock_guard<std::mutex> lock(mutex_);
  
  workerw_hwnd_ = workerw;
  check_interval_ms_ = (check_interval_ms > 0) ? check_interval_ms : 3000;
  should_stop_.store(false);
  consecutive_failures_.store(0);
  health_status_.store(HealthStatus::kHealthy);
  
  Logger::Instance().Info("WorkerWHealthMonitor", 
    "Starting WorkerW health monitoring (interval: " + 
    std::to_string(check_interval_ms_) + "ms)");
  
  // 启动监控线程
  try {
    monitor_thread_ = std::thread(&WorkerWHealthMonitor::MonitorThreadProc, this);
    is_monitoring_.store(true);
    
    Logger::Instance().Info("WorkerWHealthMonitor", 
      "Health monitor started successfully");
    return true;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("WorkerWHealthMonitor", 
      "Failed to start monitor thread: " + std::string(e.what()));
    is_monitoring_.store(false);
    return false;
  }
}

void WorkerWHealthMonitor::StopMonitoring() {
  if (!is_monitoring_.load()) {
    return;
  }
  
  Logger::Instance().Info("WorkerWHealthMonitor", "Stopping health monitor...");
  
  should_stop_.store(true);
  
  if (monitor_thread_.joinable()) {
    monitor_thread_.join();
  }
  
  is_monitoring_.store(false);
  health_status_.store(HealthStatus::kUnknown);
  
  Logger::Instance().Info("WorkerWHealthMonitor", "Health monitor stopped");
}

void WorkerWHealthMonitor::UpdateWorkerW(HWND workerw) {
  if (!workerw || !IsWindow(workerw)) {
    Logger::Instance().Warning("WorkerWHealthMonitor", 
      "UpdateWorkerW called with invalid handle");
    return;
  }
  
  std::lock_guard<std::mutex> lock(mutex_);
  
  workerw_hwnd_ = workerw;
  consecutive_failures_.store(0);
  health_status_.store(HealthStatus::kHealthy);
  
  Logger::Instance().Info("WorkerWHealthMonitor", 
    "WorkerW handle updated, health status reset to HEALTHY");
}

void WorkerWHealthMonitor::SetRecoveryCallback(RecoveryCallback callback) {
  std::lock_guard<std::mutex> lock(callback_mutex_);
  recovery_callback_ = callback;
  
  if (callback) {
    Logger::Instance().Info("WorkerWHealthMonitor", "Recovery callback registered");
  } else {
    Logger::Instance().Info("WorkerWHealthMonitor", "Recovery callback cleared");
  }
}

WorkerWHealthMonitor::HealthStatus WorkerWHealthMonitor::GetHealthStatus() const {
  return health_status_.load();
}

bool WorkerWHealthMonitor::CheckHealth() {
  last_check_time_ = std::chrono::steady_clock::now();
  
  bool is_healthy = IsWorkerWValid() && IsWorkerWStillWallpaperLayer();
  
  RecordCheckResult(is_healthy);
  
  return is_healthy;
}

int WorkerWHealthMonitor::GetConsecutiveFailures() const {
  return consecutive_failures_.load();
}

std::chrono::steady_clock::time_point WorkerWHealthMonitor::GetLastCheckTime() const {
  return last_check_time_;
}

bool WorkerWHealthMonitor::IsMonitoring() const {
  return is_monitoring_.load();
}

void WorkerWHealthMonitor::SetForceRefreshInterval(int interval) {
  if (interval > 0) {
    force_refresh_interval_ = interval;
    Logger::Instance().Info("WorkerWHealthMonitor", 
      "Force refresh interval set to " + std::to_string(interval) + " checks");
  } else {
    Logger::Instance().Warning("WorkerWHealthMonitor", 
      "Invalid force refresh interval, keeping default: " + 
      std::to_string(force_refresh_interval_));
  }
}

// ========== Private Methods ==========

void WorkerWHealthMonitor::MonitorThreadProc() {
  Logger::Instance().Info("WorkerWHealthMonitor", "Monitor thread started");
  
  while (!should_stop_.load()) {
    try {
      check_counter_++;
      
      // v2.3.1+ Enhanced: Check Explorer restart (Lively-style)
      HWND hShell = FindWindowW(L"Shell_TrayWnd", nullptr);
      DWORD current_explorer_pid = 0;
      if (hShell) {
        GetWindowThreadProcessId(hShell, &current_explorer_pid);
      }
      
      if (current_explorer_pid > 0 && current_explorer_pid != last_explorer_pid_) {
        Logger::Instance().Warning("WorkerWHealthMonitor", 
          "Explorer restart detected! PID changed: " + 
          std::to_string(last_explorer_pid_) + " -> " + 
          std::to_string(current_explorer_pid));
        
        last_explorer_pid_ = current_explorer_pid;
        
        // Explorer 重启意味着桌面完全重建，立即触发恢复
        Logger::Instance().Error("WorkerWHealthMonitor", 
          "Triggering recovery due to Explorer restart");
        health_status_.store(HealthStatus::kUnhealthy);
        TriggerRecovery();
        
        // 重置检查计数器
        check_counter_ = 0;
        continue;
      }
      
      // v2.3.1+ Enhanced: Periodic forced refresh (Lively-style)
      // Every N checks, force a refresh to ensure stability
      if (check_counter_ >= force_refresh_interval_) {
        Logger::Instance().Info("WorkerWHealthMonitor", 
          "Periodic forced refresh (every " + std::to_string(force_refresh_interval_) + " checks)");
        
        // Force refresh by triggering recovery if needed
        // This helps maintain stability over long periods
        bool is_healthy = CheckHealth();
        if (!is_healthy) {
          Logger::Instance().Warning("WorkerWHealthMonitor", 
            "Periodic check failed, triggering recovery");
          health_status_.store(HealthStatus::kUnhealthy);
          TriggerRecovery();
        }
        
        check_counter_ = 0;
        continue;
      }
      
      // 执行常规健康检查
      bool is_healthy = CheckHealth();
      
      if (!is_healthy) {
        int failures = consecutive_failures_.load();
        
        Logger::Instance().Warning("WorkerWHealthMonitor", 
          "WorkerW health check failed (consecutive failures: " + 
          std::to_string(failures) + ")");
        
        // 失败 2 次后触发恢复（避免误判）
        if (failures >= 2) {
          Logger::Instance().Error("WorkerWHealthMonitor", 
            "WorkerW unhealthy, triggering recovery...");
          
          health_status_.store(HealthStatus::kUnhealthy);
          TriggerRecovery();
        }
      } else {
        // 健康检查通过
        if (health_status_.load() == HealthStatus::kUnhealthy) {
          Logger::Instance().Info("WorkerWHealthMonitor", 
            "WorkerW recovered and healthy again");
        }
        health_status_.store(HealthStatus::kHealthy);
      }
      
    } catch (const std::exception& e) {
      Logger::Instance().Error("WorkerWHealthMonitor", 
        "Exception in monitor thread: " + std::string(e.what()));
    } catch (...) {
      Logger::Instance().Error("WorkerWHealthMonitor", 
        "Unknown exception in monitor thread");
    }
    
    // 等待下一次检查
    for (int i = 0; i < check_interval_ms_ / 100 && !should_stop_.load(); i++) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  
  Logger::Instance().Info("WorkerWHealthMonitor", "Monitor thread exiting");
}

bool WorkerWHealthMonitor::IsWorkerWValid() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // 基础验证：窗口句柄是否有效
  if (!workerw_hwnd_ || !IsWindow(workerw_hwnd_)) {
    Logger::Instance().Warning("WorkerWHealthMonitor", 
      "WorkerW handle is invalid or window destroyed");
    return false;
  }
  
  // 验证窗口类名（应该是 WorkerW 或 Progman）
  wchar_t class_name[256] = {0};
  if (GetClassNameW(workerw_hwnd_, class_name, 256) > 0) {
    std::wstring class_name_str(class_name);
    
    // WorkerW 或 Progman 都是有效的壁纸父窗口
    if (class_name_str != L"WorkerW" && class_name_str != L"Progman") {
      Logger::Instance().Warning("WorkerWHealthMonitor", 
        "WorkerW has unexpected class name");
      return false;
    }
  } else {
    Logger::Instance().Warning("WorkerWHealthMonitor", 
      "Failed to get WorkerW class name");
    return false;
  }
  
  return true;
}

bool WorkerWHealthMonitor::IsWorkerWStillWallpaperLayer() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // v2.6.7 FIX: 壁纸层 WorkerW 本身不包含 SHELLDLL_DefView！
  // SHELLDLL_DefView 位于图标层 WorkerW（另一个窗口），不在我们监控的壁纸层中
  // 正确的检查逻辑：
  // 1. 验证 Progman 存在（桌面结构基础）
  // 2. 验证我们的 WorkerW 仍然是 WorkerW 类型（已在 IsWorkerWValid 中检查）
  // 3. 验证桌面结构完整（SHELLDLL_DefView 存在于某处）
  
  HWND progman = FindWindowW(L"Progman", nullptr);
  if (!progman) {
    Logger::Instance().Warning("WorkerWHealthMonitor", 
      "Progman window not found");
    return false;
  }
  
  // 递归查找 SHELLDLL_DefView（在整个桌面结构中，不仅仅是我们的 WorkerW）
  auto FindSHELLDLL = [](HWND parent) -> HWND {
    HWND child = nullptr;
    while ((child = FindWindowExW(parent, child, nullptr, nullptr)) != nullptr) {
      wchar_t class_name[256] = {0};
      if (GetClassNameW(child, class_name, 256) > 0) {
        if (_wcsicmp(class_name, L"SHELLDLL_DefView") == 0) {
          return child;
        }
      }
      
      // 递归查找子窗口
      HWND nested = FindWindowExW(child, nullptr, L"SHELLDLL_DefView", nullptr);
      if (nested) {
        return nested;
      }
    }
    return nullptr;
  };
  
  // v2.6.7 FIX: 在整个桌面结构中查找 SHELLDLL_DefView
  // 先检查 Progman（Windows 11 常见）
  HWND shelldll = FindSHELLDLL(progman);
  
  // 如果 Progman 中没有，遍历所有 WorkerW 窗口查找
  if (!shelldll) {
    HWND workerw = nullptr;
    while ((workerw = FindWindowExW(nullptr, workerw, L"WorkerW", nullptr)) != nullptr) {
      shelldll = FindSHELLDLL(workerw);
      if (shelldll) {
        break;
      }
    }
  }
  
  if (!shelldll) {
    Logger::Instance().Warning("WorkerWHealthMonitor", 
      "SHELLDLL_DefView not found in desktop structure");
    return false;
  }
  
  // 桌面结构完整，我们的 WorkerW 有效（已在 IsWorkerWValid 中验证）
  return true;
}

void WorkerWHealthMonitor::TriggerRecovery() {
  // 防止频繁恢复
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - last_recovery_time_);
  
  if (elapsed.count() < min_recovery_interval_ms_) {
    Logger::Instance().Info("WorkerWHealthMonitor", 
      "Recovery throttled (last recovery was " + 
      std::to_string(elapsed.count()) + "ms ago)");
    return;
  }
  
  last_recovery_time_ = now;
  health_status_.store(HealthStatus::kRecovering);
  
  // 调用恢复回调
  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    if (recovery_callback_) {
      Logger::Instance().Info("WorkerWHealthMonitor", 
        "Invoking recovery callback...");
      
      try {
        recovery_callback_();
        Logger::Instance().Info("WorkerWHealthMonitor", 
          "Recovery callback completed");
      } catch (const std::exception& e) {
        Logger::Instance().Error("WorkerWHealthMonitor", 
          "Exception in recovery callback: " + std::string(e.what()));
      } catch (...) {
        Logger::Instance().Error("WorkerWHealthMonitor", 
          "Unknown exception in recovery callback");
      }
    } else {
      Logger::Instance().Warning("WorkerWHealthMonitor", 
        "No recovery callback registered");
    }
  }
}

void WorkerWHealthMonitor::RecordCheckResult(bool is_healthy) {
  if (is_healthy) {
    // 健康：重置失败计数
    consecutive_failures_.store(0);
  } else {
    // 不健康：增加失败计数
    int failures = consecutive_failures_.fetch_add(1) + 1;
    
    Logger::Instance().Warning("WorkerWHealthMonitor", 
      "Health check failed (consecutive: " + std::to_string(failures) + ")");
  }
}

}  // namespace anywp_engine
