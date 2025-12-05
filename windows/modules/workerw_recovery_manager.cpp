#include "workerw_recovery_manager.h"
#include "../anywp_engine_plugin.h"
#include "../utils/logger.h"
#include "../utils/desktop_wallpaper_helper.h"
#include <windows.h>

namespace anywp_engine {

WorkerWRecoveryManager::WorkerWRecoveryManager() {
  Logger::Instance().Info("WorkerWRecoveryManager", "Module initialized");
}

WorkerWRecoveryManager::~WorkerWRecoveryManager() {
  Logger::Instance().Info("WorkerWRecoveryManager", 
    "Module destroyed (reparent attempts: " + std::to_string(reparent_attempt_count_.load()) + 
    ", successful: " + std::to_string(successful_reparent_count_.load()) + 
    ", recreate requests: " + std::to_string(recreate_request_count_.load()) + ")");
}

void WorkerWRecoveryManager::Initialize() {
  Logger::Instance().Info("WorkerWRecoveryManager", "Module configured");
}

WorkerWRecoveryManager::RecoveryStrategy WorkerWRecoveryManager::RecoverWorkerW() {
  if (is_recovering_.exchange(true)) {
    Logger::Instance().Warning("WorkerWRecoveryManager", 
      "Recovery already in progress, skipping duplicate request");
    return RecoveryStrategy::NONE;
  }

  try {
    Logger::Instance().Info("WorkerWRecoveryManager", "Starting WorkerW recovery");

    // 尝试策略1：重新父子关系
    if (RecoverWorkerW_Reparent()) {
      Logger::Instance().Info("WorkerWRecoveryManager", 
        "Recovery completed using REPARENT strategy");
      is_recovering_.store(false);
      return RecoveryStrategy::REPARENT;
    }

    // 策略1 失败，需要请求完全重建
    Logger::Instance().Warning("WorkerWRecoveryManager", 
      "REPARENT strategy failed, requesting full reinitialization");
    
    if (recreate_request_callback_) {
      recreate_request_callback_("WorkerW recovery failed, need full reinit");
      recreate_request_count_++;
    }

    is_recovering_.store(false);
    return RecoveryStrategy::FULL_REINIT;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WorkerWRecoveryManager", 
      std::string("Exception in RecoverWorkerW: ") + e.what());
    is_recovering_.store(false);
    return RecoveryStrategy::NONE;
  }
}

bool WorkerWRecoveryManager::RecoverWorkerW_Reparent() {
  reparent_attempt_count_++;
  
  Logger::Instance().Info("WorkerWRecoveryManager", 
    "Attempting WorkerW reparent recovery");

  try {
    // 查找或创建 WorkerW
    HWND worker_w = FindOrCreateWorkerW();
    if (!worker_w) {
      Logger::Instance().Error("WorkerWRecoveryManager", 
        "Failed to find or create WorkerW");
      return false;
    }

    Logger::Instance().Info("WorkerWRecoveryManager", 
      "Found/created WorkerW: " + std::to_string(reinterpret_cast<uintptr_t>(worker_w)));

    // 验证 Progman 层次结构
    if (!VerifyProgmanHierarchy(worker_w)) {
      Logger::Instance().Warning("WorkerWRecoveryManager", 
        "WorkerW hierarchy verification failed");
      return false;
    }

    // 通过回调重新父子关系所有实例
    if (reparent_callback_) {
      // Note: 回调需要处理所有实例的重新父子关系
      if (reparent_callback_(nullptr)) {
        successful_reparent_count_++;
        Logger::Instance().Info("WorkerWRecoveryManager", 
          "Successfully reparented all instances");
        return true;
      }
    } else {
      Logger::Instance().Error("WorkerWRecoveryManager", 
        "No reparent callback registered");
    }

    return false;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WorkerWRecoveryManager", 
      std::string("Exception in RecoverWorkerW_Reparent: ") + e.what());
    return false;
  }
}

WorkerWRecoveryManager::RecoveryStrategy WorkerWRecoveryManager::CheckWorkerWStatus(
    const WallpaperInstance* instance) {
  
  if (!instance) {
    return RecoveryStrategy::NONE;
  }

  // 检查 WorkerW 句柄
  if (!instance->worker_w_hwnd || !IsWindow(instance->worker_w_hwnd)) {
    Logger::Instance().Warning("WorkerWRecoveryManager", 
      "WorkerW window is invalid for monitor " + std::to_string(instance->monitor_index));
    return RecoveryStrategy::RECREATE_WORKERW;
  }

  // 检查 WebView 宿主窗口
  if (!instance->webview_host_hwnd || !IsWindow(instance->webview_host_hwnd)) {
    Logger::Instance().Warning("WorkerWRecoveryManager", 
      "WebView host window is invalid for monitor " + std::to_string(instance->monitor_index));
    return RecoveryStrategy::FULL_REINIT;
  }

  // 检查父子关系
  HWND parent = GetParent(instance->webview_host_hwnd);
  if (parent != instance->worker_w_hwnd) {
    Logger::Instance().Warning("WorkerWRecoveryManager", 
      "Parent-child relationship broken for monitor " + std::to_string(instance->monitor_index));
    return RecoveryStrategy::REPARENT;
  }

  // 验证 WorkerW 的有效性
  if (!ValidateWorkerW(instance->worker_w_hwnd)) {
    Logger::Instance().Warning("WorkerWRecoveryManager", 
      "WorkerW validation failed for monitor " + std::to_string(instance->monitor_index));
    return RecoveryStrategy::RECREATE_WORKERW;
  }

  return RecoveryStrategy::NONE;
}

bool WorkerWRecoveryManager::ValidateWorkerW(HWND worker_w) {
  if (!worker_w || !IsWindow(worker_w)) {
    return false;
  }

  // 检查窗口类名
  wchar_t class_name[256];
  if (GetClassName(worker_w, class_name, 256) == 0) {
    return false;
  }

  if (wcscmp(class_name, L"WorkerW") != 0) {
    Logger::Instance().Warning("WorkerWRecoveryManager", 
      "Window is not a WorkerW");
    return false;
  }

  // 验证 Progman 层次结构
  return VerifyProgmanHierarchy(worker_w);
}

void WorkerWRecoveryManager::SetReparentCallback(ReparentCallback callback) {
  reparent_callback_ = callback;
  Logger::Instance().Info("WorkerWRecoveryManager", "Reparent callback registered");
}

void WorkerWRecoveryManager::SetRecreateRequestCallback(RecreateRequestCallback callback) {
  recreate_request_callback_ = callback;
  Logger::Instance().Info("WorkerWRecoveryManager", "Recreate request callback registered");
}

void WorkerWRecoveryManager::SetWallpaperInstances(std::map<HWND, WallpaperInstance>* instances) {
  // 可选：保存实例指针用于高级验证（当前实现中未使用）
  Logger::Instance().Debug("WorkerWRecoveryManager", "Wallpaper instances reference set");
}

void WorkerWRecoveryManager::SetWindowManager(class WindowManager* window_manager) {
  // 可选：保存窗口管理器指针用于高级诊断（当前实现中未使用）
  Logger::Instance().Debug("WorkerWRecoveryManager", "Window manager reference set");
}

size_t WorkerWRecoveryManager::GetReparentAttemptCount() const {
  return reparent_attempt_count_.load();
}

size_t WorkerWRecoveryManager::GetSuccessfulReparentCount() const {
  return successful_reparent_count_.load();
}

size_t WorkerWRecoveryManager::GetRecreateRequestCount() const {
  return recreate_request_count_.load();
}

// ========== Private Methods ==========

bool WorkerWRecoveryManager::ReparentSingleInstance(WallpaperInstance* instance) {
  if (!instance || !instance->webview_host_hwnd || !instance->worker_w_hwnd) {
    return false;
  }

  try {
    HWND new_parent = instance->worker_w_hwnd;
    
    // 重新设置父窗口
    HWND old_parent = SetParent(instance->webview_host_hwnd, new_parent);
    if (!old_parent) {
      Logger::Instance().Error("WorkerWRecoveryManager", 
        "Failed to reparent window for monitor " + std::to_string(instance->monitor_index));
      return false;
    }

    Logger::Instance().Info("WorkerWRecoveryManager", 
      "Successfully reparented monitor " + std::to_string(instance->monitor_index));
    return true;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WorkerWRecoveryManager", 
      std::string("Exception in ReparentSingleInstance: ") + e.what());
    return false;
  }
}

HWND WorkerWRecoveryManager::FindOrCreateWorkerW() {
  // 使用 DesktopWallpaperHelper 查找 WorkerW
  HWND progman = FindWindow(L"Progman", nullptr);
  if (!progman) {
    Logger::Instance().Error("WorkerWRecoveryManager", "Cannot find Progman window");
    return nullptr;
  }

  DesktopWallpaperHelper& helper = DesktopWallpaperHelper::Instance();
  if (helper.FindWorkerW()) {
    HWND worker_w = helper.GetWallpaperParent();
    if (worker_w && IsWindow(worker_w)) {
      Logger::Instance().Info("WorkerWRecoveryManager", "Found existing WorkerW");
      return worker_w;
    }
  }

  // 尝试触发 WorkerW 创建
  Logger::Instance().Info("WorkerWRecoveryManager", "Triggering WorkerW creation");
  if (helper.TriggerWorkerWCreation()) {
    HWND worker_w = helper.GetWallpaperParent();
    if (worker_w && IsWindow(worker_w)) {
      Logger::Instance().Info("WorkerWRecoveryManager", "WorkerW created successfully");
      return worker_w;
    }
  }

  Logger::Instance().Error("WorkerWRecoveryManager", "Failed to find/create WorkerW");
  return nullptr;
}

bool WorkerWRecoveryManager::VerifyProgmanHierarchy(HWND worker_w) {
  HWND progman = FindWindow(L"Progman", nullptr);
  if (!progman) {
    Logger::Instance().Error("WorkerWRecoveryManager", "Cannot find Progman");
    return false;
  }

  // 检查 WorkerW 的父窗口是否是桌面
  HWND parent = GetParent(worker_w);
  HWND desktop = GetDesktopWindow();
  
  if (parent != desktop) {
    Logger::Instance().Warning("WorkerWRecoveryManager", 
      "WorkerW parent is not desktop");
    return false;
  }

  // 检查 Z-Order（WorkerW 应该在 Progman 之后）
  // Note: 这里可以添加更详细的 Z-Order 验证

  return true;
}

}  // namespace anywp_engine

