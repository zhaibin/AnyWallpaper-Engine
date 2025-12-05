#include "wallpaper_lifecycle_manager.h"
#include "../anywp_engine_plugin.h"
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
  // Guard: Avoid duplicate pause
  if (is_paused_.exchange(true)) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Already paused, ignoring duplicate pause request (" + reason + ")");
    return true;
  }

  try {
    Logger::Instance().Info("WallpaperLifecycleManager", 
      "Pausing wallpaper (reason: " + reason + ")");
    
    last_pause_reason_ = reason;
    pause_count_++;

    // 更新状态
    ChangeState(WallpaperState::PAUSED, reason);

    // Execute pause scripts to freeze animations
    // Note: This will freeze the last frame without completely hiding the wallpaper
    std::wostringstream pause_script;
    pause_script << L"(function() {"
                 << L"  if (typeof window.AnyWP !== 'undefined') {"
                 << L"    if (window.AnyWP.onPause) {"
                 << L"      window.AnyWP.onPause();"
                 << L"    }"
                 << L"    if (typeof window.AnyWP._notifyVisibilityChange === 'function') {"
                 << L"      window.AnyWP._notifyVisibilityChange(false);"
                 << L"    }"
                 << L"  }"
                 << L"  document.dispatchEvent(new CustomEvent('anywp:pause'));"
                 << L"  if (typeof requestAnimationFrame === 'function') {"
                 << L"    window.__anywp_cancelAllAnimations = true;"
                 << L"  }"
                 << L"})();";
    
    ExecuteScriptToAllInstances(pause_script.str());

    // Light memory trim (Windows API)
    SetProcessWorkingSetSize(GetCurrentProcess(), static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1));

    // Note: MemoryOptimizer integration is available but disabled to avoid
    // aggressive memory operations during pause. Light trim above is sufficient.
    // if (memory_optimizer_) {
    //   memory_optimizer_->OptimizeMemoryUsage();
    // }

    Logger::Instance().Info("WallpaperLifecycleManager", "Wallpaper paused successfully - last frame frozen");
    return true;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WallpaperLifecycleManager", 
      std::string("Exception in PauseWallpaper: ") + e.what());
    is_paused_.store(false);  // 回滚状态
    return false;
  }
}

bool WallpaperLifecycleManager::ResumeWallpaper(const std::string& reason, bool force_reinit) {
  // Guard: Avoid duplicate resume
  if (!is_paused_.exchange(false) && !force_reinit) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Already resumed, ignoring duplicate resume request (" + reason + ")");
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

    // CRITICAL FIX: Verify and restore window if necessary (for long-term lock/sleep)
    bool need_reinitialize = force_reinit;  // Force if requested
    
    // Skip validation if force reinit requested
    if (!force_reinit) {
      need_reinitialize = !ValidateWallpaperWindows();
    }
    
    // If window is lost, try to restore it
    if (need_reinitialize) {
      Logger::Instance().Warning("WallpaperLifecycleManager", 
        "Window validation failed or force reinit requested, need reinitialization");
      validation_failure_count_++;
      
      // 如果有配置恢复回调，尝试恢复
      if (config_restore_callback_) {
        Logger::Instance().Info("WallpaperLifecycleManager", 
          "Attempting configuration restore via callback...");
        // Pass empty URL to use default URL in the callback implementation
        if (config_restore_callback_("", "WallpaperLifecycleManager")) {
          is_paused_.store(false);
          ChangeState(WallpaperState::ACTIVE, "Configuration restored");
          return true;  // Restoration successful
        }
        // Restoration failed, continue to error return
      } else {
        Logger::Instance().Error("WallpaperLifecycleManager", 
          "No configuration restore callback available");
        is_paused_.store(false);
        return false;  // Cannot recover
      }
    }

    // Execute resume scripts to restart animations
    std::wostringstream resume_script;
    resume_script << L"(function() {"
                  << L"  if (typeof window.AnyWP !== 'undefined') {"
                  << L"    if (window.AnyWP.onResume) {"
                  << L"      window.AnyWP.onResume();"
                  << L"    }"
                  << L"    if (typeof window.AnyWP._notifyVisibilityChange === 'function') {"
                  << L"      window.AnyWP._notifyVisibilityChange(true);"
                  << L"    }"
                  << L"  }"
                  << L"  document.dispatchEvent(new CustomEvent('anywp:resume'));"
                  << L"  if (typeof requestAnimationFrame === 'function') {"
                  << L"    window.__anywp_cancelAllAnimations = false;"
                  << L"  }"
                  << L"})();";
    
    ExecuteScriptToAllInstances(resume_script.str());

    // 更新状态
    ChangeState(WallpaperState::ACTIVE, reason);
    is_paused_.store(false);

    Logger::Instance().Info("WallpaperLifecycleManager", "Wallpaper resumed successfully - animations restarted");
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

  Logger::Instance().Info("WallpaperLifecycleManager", "Verifying wallpaper window state...");

  // 默认验证逻辑 - Check multi-monitor mode
  if (wallpaper_instances_->empty()) {
    Logger::Instance().Warning("WallpaperLifecycleManager", "No wallpaper instances found");
    return false;
  }

  Logger::Instance().Info("WallpaperLifecycleManager", 
    "Multi-monitor mode detected (" + std::to_string(wallpaper_instances_->size()) + " instances)");

  bool all_valid = true;
  for (const auto& instance : *wallpaper_instances_) {
    Logger::Instance().Debug("WallpaperLifecycleManager", 
      "Checking monitor " + std::to_string(instance.monitor_index));
    
    if (!ValidateSingleInstance(instance)) {
      all_valid = false;
      Logger::Instance().Warning("WallpaperLifecycleManager", 
        "Monitor " + std::to_string(instance.monitor_index) + " validation failed");
    } else {
      Logger::Instance().Info("WallpaperLifecycleManager", 
        "[OK] Monitor " + std::to_string(instance.monitor_index) + " window valid");
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
  // Note: This method is deprecated
  // Pause is now handled via JavaScript scripts in PauseWallpaper()
  // Kept for backward compatibility
  Logger::Instance().Debug("WallpaperLifecycleManager", 
    "PauseWebViewContent called (deprecated, using script-based pause instead)");
}

void WallpaperLifecycleManager::ResumeWebViewContent() {
  // Note: This method is deprecated
  // Resume is now handled via JavaScript scripts in ResumeWallpaper()
  // Kept for backward compatibility
  Logger::Instance().Debug("WallpaperLifecycleManager", 
    "ResumeWebViewContent called (deprecated, using script-based resume instead)");
}

void WallpaperLifecycleManager::SetMemoryOptimizer(MemoryOptimizer* optimizer) {
  memory_optimizer_ = optimizer;
  Logger::Instance().Info("WallpaperLifecycleManager", 
    "Memory optimizer set");
}

bool WallpaperLifecycleManager::ValidateSingleInstance(const WallpaperInstance& instance) {
  // 检查窗口句柄
  if (!instance.webview_host_hwnd || !IsWindow(instance.webview_host_hwnd)) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Invalid webview_host_hwnd for monitor " + std::to_string(instance.monitor_index));
    return false;
  }

  // 检查窗口可见性
  if (!IsWindowVisible(instance.webview_host_hwnd)) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "WebView window not visible for monitor " + std::to_string(instance.monitor_index));
    return false;
  }

  // 检查 WorkerW
  if (!instance.worker_w_hwnd || !IsWindow(instance.worker_w_hwnd)) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Invalid worker_w_hwnd for monitor " + std::to_string(instance.monitor_index));
    return false;
  }

  // 验证父子关系
  HWND parent = GetParent(instance.webview_host_hwnd);
  if (parent != instance.worker_w_hwnd) {
    Logger::Instance().Warning("WallpaperLifecycleManager", 
      "Parent window relationship broken for monitor " + std::to_string(instance.monitor_index));
    Logger::Instance().Debug("WallpaperLifecycleManager", 
      "Expected parent: " + std::to_string(reinterpret_cast<uintptr_t>(instance.worker_w_hwnd)) + 
      ", Actual parent: " + std::to_string(reinterpret_cast<uintptr_t>(parent)));
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

