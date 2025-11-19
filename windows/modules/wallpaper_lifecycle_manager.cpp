#include "wallpaper_lifecycle_manager.h"
#include "../utils/logger.h"
#include "../modules/memory_optimizer.h"
#include <windows.h>
#include <sstream>

namespace anywp_engine {

WallpaperLifecycleManager::WallpaperLifecycleManager() {
  Logger::Instance().Info("WallpaperLifecycleManager", "Module initialized");
}

WallpaperLifecycleManager::~WallpaperLifecycleManager() {
  Logger::Instance().Info("WallpaperLifecycleManager", 
    "Module destroyed (pauses: " + std::to_string(pause_count_) + 
    ", resumes: " + std::to_string(resume_count_) + 
    ", validation failures: " + std::to_string(validation_failure_count_) + ")");
}

void WallpaperLifecycleManager::Initialize(MemoryOptimizer* memory_optimizer) {
  memory_optimizer_ = memory_optimizer;
  current_state_ = WallpaperState::ACTIVE;
  is_paused_.store(false);
  
  Logger::Instance().Info("WallpaperLifecycleManager", "Module configured");
}

bool WallpaperLifecycleManager::PauseWallpaper(const std::string& reason) {
  if (is_paused_.load()) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Already paused, ignoring duplicate pause request (" + reason + ")");
    return true;
  }

  try {
    Logger::Instance().Info("WallpaperLifecycleManager", 
      "Pausing wallpaper (reason: " + reason + ")");
    
    last_pause_reason_ = reason;
    pause_count_++;

    // 暂停 WebView 内容
    PauseWebViewContent();

    // 更新状态
    ChangeState(WallpaperState::PAUSED, reason);
    is_paused_.store(true);

    // 触发内存优化
    // TODO: 实现内存优化调用（Phase 3+）
    // if (memory_optimizer_) {
    //   memory_optimizer_->OptimizeMemoryUsage();
    // }

    Logger::Instance().Info("WallpaperLifecycleManager", "Wallpaper paused successfully");
    return true;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WallpaperLifecycleManager", 
      std::string("Exception in PauseWallpaper: ") + e.what());
    return false;
  }
}

bool WallpaperLifecycleManager::ResumeWallpaper(const std::string& reason, bool force_reinit) {
  if (!is_paused_.load() && !force_reinit) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Not paused, ignoring resume request (" + reason + ")");
    return true;
  }

  try {
    Logger::Instance().Info("WallpaperLifecycleManager", 
      "Resuming wallpaper (reason: " + reason + 
      ", force_reinit: " + (force_reinit ? "true" : "false") + ")");
    
    last_resume_reason_ = reason;
    resume_count_++;

    // 更新状态
    ChangeState(WallpaperState::RESUMING, reason);

    // 验证窗口状态
    if (!ValidateWallpaperWindows()) {
      Logger::Instance().Warning("WallpaperLifecycleManager", 
        "Window validation failed, may need reinitialization");
      validation_failure_count_++;
      
      // 如果有配置恢复回调，尝试恢复
      if (config_restore_callback_) {
        Logger::Instance().Info("WallpaperLifecycleManager", 
          "Attempting configuration restore...");
        // TODO: 需要从外部获取 URL
      }
    }

    // 恢复 WebView 内容
    ResumeWebViewContent();

    // 更新状态
    ChangeState(WallpaperState::ACTIVE, reason);
    is_paused_.store(false);

    Logger::Instance().Info("WallpaperLifecycleManager", "Wallpaper resumed successfully");
    return true;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WallpaperLifecycleManager", 
      std::string("Exception in ResumeWallpaper: ") + e.what());
    is_paused_.store(false);  // 确保状态正确
    return false;
  }
}

bool WallpaperLifecycleManager::ValidateWallpaperWindows() {
  if (!wallpaper_instances_) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Wallpaper instances not set, cannot validate");
    return false;
  }

  if (window_validation_callback_) {
    return window_validation_callback_();
  }

  // 默认验证逻辑
  bool all_valid = true;
  for (const auto& instance : *wallpaper_instances_) {
    if (!ValidateSingleInstance(instance)) {
      all_valid = false;
    }
  }

  if (!all_valid) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Some wallpaper windows are invalid");
  }

  return all_valid;
}

bool WallpaperLifecycleManager::RestoreWallpaperConfiguration(
    const std::string& url, 
    const std::string& log_tag) {
  
  Logger::Instance().Info("WallpaperLifecycleManager", 
    "[" + log_tag + "] Restoring configuration with URL: " + url);

  if (config_restore_callback_) {
    try {
      return config_restore_callback_(url, log_tag);
    } catch (const std::exception& e) {
      Logger::Instance().Error("WallpaperLifecycleManager", 
        std::string("Exception in config restore callback: ") + e.what());
      return false;
    }
  }

  Logger::Instance().Warning("WallpaperLifecycleManager", 
    "No configuration restore callback registered");
  return false;
}

void WallpaperLifecycleManager::NotifyWebContentVisibility(bool visible) {
  Logger::Instance().Debug("WallpaperLifecycleManager", 
    "Notifying web content visibility: " + std::string(visible ? "visible" : "hidden"));

  std::wostringstream wss;
  wss << L"(function() {"
      << L"  if (typeof window.AnyWP !== 'undefined' && window.AnyWP.onVisibilityChange) {"
      << L"    window.AnyWP.onVisibilityChange(" << (visible ? L"true" : L"false") << L");"
      << L"  }"
      << L"  document.dispatchEvent(new CustomEvent('anywp:visibility', { detail: { visible: " 
      << (visible ? L"true" : L"false") << L" } }));"
      << L"})();";
  
  std::wstring visibility_script = wss.str();
  ExecuteScriptToAllInstances(visibility_script);
}

void WallpaperLifecycleManager::ExecuteScriptToAllInstances(const std::wstring& script) {
  if (!wallpaper_instances_) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Wallpaper instances not set, cannot execute script");
    return;
  }

  size_t executed_count = 0;
  for (const auto& instance : *wallpaper_instances_) {
    if (instance.webview) {
      try {
        instance.webview->ExecuteScript(script.c_str(), nullptr);
        executed_count++;
      } catch (const std::exception& e) {
        Logger::Instance().Error("WallpaperLifecycleManager", 
          std::string("Exception executing script to instance: ") + e.what());
      }
    }
  }

  Logger::Instance().Debug("WallpaperLifecycleManager", 
    "Script executed to " + std::to_string(executed_count) + " instances");
}

WallpaperLifecycleManager::WallpaperState WallpaperLifecycleManager::GetCurrentState() const {
  return current_state_;
}

bool WallpaperLifecycleManager::IsPaused() const {
  return is_paused_.load();
}

void WallpaperLifecycleManager::SetStateChangeCallback(StateChangeCallback callback) {
  state_change_callback_ = callback;
}

void WallpaperLifecycleManager::SetWindowValidationCallback(WindowValidationCallback callback) {
  window_validation_callback_ = callback;
}

void WallpaperLifecycleManager::SetConfigurationRestoreCallback(ConfigurationRestoreCallback callback) {
  config_restore_callback_ = callback;
}

void WallpaperLifecycleManager::SetWallpaperInstances(std::vector<WallpaperInstance>* instances) {
  wallpaper_instances_ = instances;
}

// ========== Private Methods ==========

void WallpaperLifecycleManager::ChangeState(WallpaperState new_state, const std::string& reason) {
  WallpaperState old_state = current_state_;
  current_state_ = new_state;

  Logger::Instance().Info("WallpaperLifecycleManager", 
    "State changed: " + std::to_string(static_cast<int>(old_state)) + 
    " -> " + std::to_string(static_cast<int>(new_state)) + 
    " (reason: " + reason + ")");

  if (state_change_callback_) {
    try {
      state_change_callback_(old_state, new_state, reason);
    } catch (const std::exception& e) {
      Logger::Instance().Error("WallpaperLifecycleManager", 
        std::string("Exception in state change callback: ") + e.what());
    }
  }
}

void WallpaperLifecycleManager::PauseWebViewContent() {
  if (!wallpaper_instances_) {
    return;
  }

  for (const auto& instance : *wallpaper_instances_) {
    if (instance.webview) {
      try {
        // 暂停渲染和 JavaScript 执行
        // Note: WebView2 没有直接的暂停 API，使用隐藏窗口的方式
        if (instance.webview_host_hwnd && IsWindow(instance.webview_host_hwnd)) {
          ShowWindow(instance.webview_host_hwnd, SW_HIDE);
        }
      } catch (const std::exception& e) {
        Logger::Instance().Error("WallpaperLifecycleManager", 
          std::string("Exception pausing instance: ") + e.what());
      }
    }
  }
}

void WallpaperLifecycleManager::ResumeWebViewContent() {
  if (!wallpaper_instances_) {
    return;
  }

  for (const auto& instance : *wallpaper_instances_) {
    if (instance.webview) {
      try {
        // 恢复显示
        if (instance.webview_host_hwnd && IsWindow(instance.webview_host_hwnd)) {
          ShowWindow(instance.webview_host_hwnd, SW_SHOW);
        }
      } catch (const std::exception& e) {
        Logger::Instance().Error("WallpaperLifecycleManager", 
          std::string("Exception resuming instance: ") + e.what());
      }
    }
  }
}

bool WallpaperLifecycleManager::ValidateSingleInstance(const WallpaperInstance& instance) {
  // 检查窗口句柄
  if (!instance.webview_host_hwnd || !IsWindow(instance.webview_host_hwnd)) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Invalid webview_host_hwnd for monitor " + std::to_string(instance.monitor_index));
    return false;
  }

  // 检查 WorkerW
  if (!instance.worker_w_hwnd || !IsWindow(instance.worker_w_hwnd)) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Invalid worker_w_hwnd for monitor " + std::to_string(instance.monitor_index));
    return false;
  }

  // 检查 WebView2 对象
  if (!instance.webview || !instance.webview_controller) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "WebView2 objects not initialized for monitor " + std::to_string(instance.monitor_index));
    return false;
  }

  return true;
}

}  // namespace anywp_engine

