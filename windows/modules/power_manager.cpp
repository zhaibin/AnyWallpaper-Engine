#include "power_manager.h"
#include "../utils/logger.h"
#include <psapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "psapi.lib")

namespace anywp_engine {

PowerManager* PowerManager::instance_ = nullptr;

PowerManager::PowerManager()
    : enabled_(false),
      current_state_(PowerState::ACTIVE),
      last_state_(PowerState::ACTIVE),
      idle_timeout_ms_(300000),  // 5 minutes
      memory_threshold_mb_(150),
      cleanup_interval_minutes_(15),
      listener_hwnd_(nullptr),
      stop_fullscreen_detection_(false),
      is_session_locked_(false),
      is_remote_session_(false) {
  instance_ = this;
}

PowerManager::~PowerManager() {
  Enable(false);
  instance_ = nullptr;
}

void PowerManager::Enable(bool enabled) {
  if (enabled_ == enabled) {
    return;
  }
  
  enabled_ = enabled;
  
  try {
    if (enabled) {
      Logger::Instance().Info("PowerManager", "Enabling power management");
      StartFullscreenDetection();
    } else {
      Logger::Instance().Info("PowerManager", "Disabling power management");
      StopFullscreenDetection();
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("PowerManager", "Exception in Enable: " + std::string(e.what()));
    enabled_ = !enabled;  // Rollback state
  } catch (...) {
    Logger::Instance().Error("PowerManager", "Unknown exception in Enable");
    enabled_ = !enabled;  // Rollback state
  }
}

bool PowerManager::IsEnabled() const {
  return enabled_;
}

PowerManager::PowerState PowerManager::GetCurrentState() const {
  return current_state_;
}

void PowerManager::SetSessionLocked(bool locked) {
  Logger::Instance().Info("PowerManager", "Session lock state changed: " + std::string(locked ? "true" : "false"));
  is_session_locked_.store(locked);
  UpdatePowerState();
}

void PowerManager::SetRemoteSession(bool remote) {
  Logger::Instance().Info("PowerManager", "Remote session state changed: " + std::string(remote ? "true" : "false"));
  is_remote_session_.store(remote);
  UpdatePowerState();
}

void PowerManager::UpdatePowerState() {
  PowerState new_state = PowerState::ACTIVE;
  
  try {
    // Check in priority order
    if (is_session_locked_.load()) {
      new_state = PowerState::LOCKED;
    } else if (IsFullscreenAppActive()) {
      new_state = PowerState::FULLSCREEN_APP;
    }
    // Note: Idle detection and screen off detection are planned for future versions.
    // Current implementation relies on session lock and fullscreen app detection.
    
    if (new_state != current_state_) {
      last_state_ = current_state_;
      current_state_ = new_state;
      
      Logger::Instance().Info("PowerManager", 
        "State changed: " + std::to_string(static_cast<int>(last_state_)) + " -> " + std::to_string(static_cast<int>(current_state_)));
      
      if (on_state_changed_) {
        try {
          on_state_changed_(last_state_, current_state_);
        } catch (const std::exception& e) {
          Logger::Instance().Error("PowerManager", "Exception in state change callback: " + std::string(e.what()));
        } catch (...) {
          Logger::Instance().Error("PowerManager", "Unknown exception in state change callback");
        }
      }
      
      // Auto pause/resume based on state transition
      if (new_state != PowerState::ACTIVE && last_state_ == PowerState::ACTIVE) {
        // Transitioning from ACTIVE to any paused state
        std::string reason = "PowerManager: ";
        switch (new_state) {
          case PowerState::LOCKED: reason += "screen_locked"; break;
          case PowerState::FULLSCREEN_APP: reason += "fullscreen_app"; break;
          case PowerState::IDLE: reason += "user_idle"; break;
          case PowerState::SCREEN_OFF: reason += "screen_off"; break;
          default: reason += "unknown"; break;
        }
        Pause(reason);
      } else if (new_state == PowerState::ACTIVE && last_state_ != PowerState::ACTIVE) {
        // Transitioning from any paused state to ACTIVE
        std::string reason = "PowerManager: ";
        switch (last_state_) {
          case PowerState::LOCKED: reason += "screen_unlocked"; break;
          case PowerState::FULLSCREEN_APP: reason += "fullscreen_closed"; break;
          case PowerState::IDLE: reason += "user_active"; break;
          case PowerState::SCREEN_OFF: reason += "screen_on"; break;
          default: reason += "unknown"; break;
        }
        Resume(reason);
      }
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("PowerManager", "Exception in UpdatePowerState: " + std::string(e.what()));
  } catch (...) {
    Logger::Instance().Error("PowerManager", "Unknown exception in UpdatePowerState");
  }
}

void PowerManager::Pause(const std::string& reason) {
  Logger::Instance().Info("PowerManager", "Pause requested: " + reason);
  
  if (on_pause_) {
    on_pause_(reason);
  }
}

void PowerManager::Resume(const std::string& reason, bool force_reinit) {
  Logger::Instance().Info("PowerManager", 
    "Resume requested: " + reason + " (force_reinit=" + std::string(force_reinit ? "true" : "false") + ")");
  
  if (on_resume_) {
    on_resume_(reason);
  }
}

void PowerManager::SetIdleTimeout(DWORD timeout_ms) {
  idle_timeout_ms_ = timeout_ms;
  Logger::Instance().Info("PowerManager", 
    "Idle timeout set to " + std::to_string(timeout_ms / 1000) + " seconds");
}

void PowerManager::SetMemoryThreshold(size_t mb) {
  memory_threshold_mb_ = mb;
  Logger::Instance().Info("PowerManager", 
    "Memory threshold set to " + std::to_string(mb) + " MB");
}

void PowerManager::SetCleanupInterval(int minutes) {
  cleanup_interval_minutes_ = minutes;
  Logger::Instance().Info("PowerManager", 
    "Cleanup interval set to " + std::to_string(minutes) + " minutes");
}

void PowerManager::SetOnStateChanged(StateChangeCallback callback) {
  on_state_changed_ = callback;
}

void PowerManager::SetOnPause(PauseCallback callback) {
  on_pause_ = callback;
}

void PowerManager::SetOnResume(ResumeCallback callback) {
  on_resume_ = callback;
}

bool PowerManager::ShouldWallpaperBeActive() const {
  // Wallpaper should NOT be active when:
  // - Session is locked
  // - Screen is off
  // - Fullscreen app is active
  
  if (is_session_locked_.load()) {
    return false;
  }
  
  if (current_state_ == PowerState::FULLSCREEN_APP) {
    return false;
  }
  
  if (current_state_ == PowerState::SCREEN_OFF) {
    return false;
  }
  
  return true;
}

size_t PowerManager::GetCurrentMemoryUsage() {
  PROCESS_MEMORY_COUNTERS_EX pmc = {0};
  pmc.cb = sizeof(pmc);
  
  if (GetProcessMemoryInfo(GetCurrentProcess(), 
                           reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), 
                           sizeof(pmc))) {
    // Return bytes (caller will convert to MB)
    // Validate reasonable value (<10GB)
    if (pmc.WorkingSetSize > 0 && pmc.WorkingSetSize < (10ULL * 1024 * 1024 * 1024)) {
      return pmc.WorkingSetSize;
    }
  }
  return 0;
}

void PowerManager::OptimizeMemoryUsage() {
  Logger::Instance().Info("PowerManager", "Optimizing memory usage...");
  
  // Trigger garbage collection (Windows will reclaim unused pages)
  SetProcessWorkingSetSize(GetCurrentProcess(), 
                          static_cast<SIZE_T>(-1), 
                          static_cast<SIZE_T>(-1));
  
  Logger::Instance().Info("PowerManager", "Memory optimization complete");
}

bool PowerManager::IsFullscreenAppActive() {
  // Get foreground window
  HWND foreground = GetForegroundWindow();
  if (!foreground) {
    return false;
  }
  
  // Get window rect
  RECT window_rect;
  if (!GetWindowRect(foreground, &window_rect)) {
    return false;
  }
  
  // Get monitor rect for the window
  HMONITOR monitor = MonitorFromWindow(foreground, MONITOR_DEFAULTTONEAREST);
  MONITORINFO monitor_info = { sizeof(MONITORINFO) };
  if (!GetMonitorInfo(monitor, &monitor_info)) {
    return false;
  }
  
  // Check if window covers entire monitor
  bool is_fullscreen = (
    window_rect.left <= monitor_info.rcMonitor.left &&
    window_rect.top <= monitor_info.rcMonitor.top &&
    window_rect.right >= monitor_info.rcMonitor.right &&
    window_rect.bottom >= monitor_info.rcMonitor.bottom
  );
  
  if (!is_fullscreen) {
    return false;
  }
  
  // Verify it's not desktop or shell window
  wchar_t class_name[256] = {0};
  GetClassNameW(foreground, class_name, 256);
  
  if (wcscmp(class_name, L"Progman") == 0 ||
      wcscmp(class_name, L"WorkerW") == 0 ||
      wcscmp(class_name, L"Shell_TrayWnd") == 0) {
    return false;
  }
  
  // Log fullscreen app info
  wchar_t window_title[256] = {0};
  GetWindowTextW(foreground, window_title, 256);
  
  // Convert wchar_t to std::string for logging
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, window_title, -1, nullptr, 0, nullptr, nullptr);
  std::string title_str(size_needed - 1, 0);
  WideCharToMultiByte(CP_UTF8, 0, window_title, -1, &title_str[0], size_needed, nullptr, nullptr);
  
  size_needed = WideCharToMultiByte(CP_UTF8, 0, class_name, -1, nullptr, 0, nullptr, nullptr);
  std::string class_str(size_needed - 1, 0);
  WideCharToMultiByte(CP_UTF8, 0, class_name, -1, &class_str[0], size_needed, nullptr, nullptr);
  
  Logger::Instance().Info("PowerManager", 
    "Fullscreen app detected: \"" + title_str + "\" (Class: " + class_str + ")");
  
  return true;
}

void PowerManager::StartFullscreenDetection() {
  if (fullscreen_detection_thread_.joinable()) {
    return;  // Already running
  }
  
  stop_fullscreen_detection_ = false;
  
  fullscreen_detection_thread_ = std::thread([this]() {
    Logger::Instance().Info("PowerManager", "Fullscreen detection thread started");
    
    while (!stop_fullscreen_detection_) {
      UpdatePowerState();
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    Logger::Instance().Info("PowerManager", "Fullscreen detection thread stopped");
  });
}

void PowerManager::StopFullscreenDetection() {
  if (!fullscreen_detection_thread_.joinable()) {
    return;
  }
  
  stop_fullscreen_detection_ = true;
  
  if (fullscreen_detection_thread_.joinable()) {
    fullscreen_detection_thread_.join();
  }
}

LRESULT CALLBACK PowerManager::PowerSavingWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  // Handle power-related messages
  switch (message) {
    case WM_POWERBROADCAST:
      if (instance_) {
        // Handle power state changes
        switch (wParam) {
          case PBT_APMSUSPEND:
            Logger::Instance().Info("PowerManager", "System suspending");
            break;
          case PBT_APMRESUMESUSPEND:
            Logger::Instance().Info("PowerManager", "System resuming from suspend");
            break;
        }
      }
      break;
  }
  
  return DefWindowProc(hwnd, message, wParam, lParam);
}

// v1.4.1+ Phase E: Script execution helpers
void PowerManager::ExecutePauseScripts(ScriptExecutor executor) {
  Logger::Instance().Info("PowerManager", "Executing pause scripts...");
  
  // 1. Freeze animations and pause media
  std::wstring freeze_script = LR"(
    (function() {
      try {
        // Create or update freeze overlay
        var freezeStyle = document.getElementById('__anywp_freeze_style');
        if (!freezeStyle) {
          freezeStyle = document.createElement('style');
          freezeStyle.id = '__anywp_freeze_style';
          freezeStyle.textContent = 
            '* { ' +
            '  animation-play-state: paused !important; ' +
            '  transition: none !important; ' +
            '  animation: none !important; ' +
            '} ' +
            '*::before, *::after { ' +
            '  animation-play-state: paused !important; ' +
            '  transition: none !important; ' +
            '  animation: none !important; ' +
            '}';
          (document.head || document.documentElement).appendChild(freezeStyle);
        }
        
        // Pause all videos
        document.querySelectorAll('video').forEach(function(v) {
          if (!v.paused) {
            v.__anyWP_wasPlaying = true;
            v.pause();
          }
        });
        
        // Pause all audio
        document.querySelectorAll('audio').forEach(function(a) {
          if (!a.paused) {
            a.__anyWP_wasPlaying = true;
            a.pause();
          }
        });
        
        // Stop requestAnimationFrame loops
        if (!window.__anyWP_rafPaused) {
          window.__anyWP_rafPaused = true;
          window.__anyWP_originalRAF = window.requestAnimationFrame;
          window.requestAnimationFrame = function() { return 0; };
        }
        
        // Trigger visibility change event for test page counters
        if (window.AnyWP && typeof window.AnyWP._notifyVisibilityChange === 'function') {
          window.AnyWP._notifyVisibilityChange(false);
        }
        
        return 'PAUSED';
      } catch(e) {
        return 'ERROR: ' + e.message;
      }
    })();
  )";
  
  executor(freeze_script);
  
  // 2. Notify SDK about pause
  std::wstring notify_pause = LR"(
    (function() {
      if (window.AnyWP && typeof window.AnyWP._notifyVisibilityChange === 'function') {
        console.log('[C++] Calling AnyWP._notifyVisibilityChange(false)');
        window.AnyWP._notifyVisibilityChange(false);
        return 'NOTIFY_PAUSE_OK';
      } else {
        console.log('[C++] AnyWP._notifyVisibilityChange not available');
        return 'NOTIFY_PAUSE_FAILED';
      }
    })();
  )";
  
  executor(notify_pause);
  Logger::Instance().Info("PowerManager", "Pause scripts executed");
}

void PowerManager::ExecuteResumeScripts(ScriptExecutor executor) {
  Logger::Instance().Info("PowerManager", "Executing resume scripts...");
  
  // 1. Unfreeze animations and resume media
  std::wstring unfreeze_script = LR"(
    (function() {
      try {
        // Remove freeze style
        var freezeStyle = document.getElementById('__anywp_freeze_style');
        if (freezeStyle) {
          freezeStyle.remove();
        }
        
        // Resume videos
        document.querySelectorAll('video').forEach(function(v) {
          if (v.__anyWP_wasPlaying) {
            v.play().catch(function() {});
            delete v.__anyWP_wasPlaying;
          }
        });
        
        // Resume audio
        document.querySelectorAll('audio').forEach(function(a) {
          if (a.__anyWP_wasPlaying) {
            a.play().catch(function() {});
            delete a.__anyWP_wasPlaying;
          }
        });
        
        // Restore requestAnimationFrame
        if (window.__anyWP_rafPaused && window.__anyWP_originalRAF) {
          window.requestAnimationFrame = window.__anyWP_originalRAF;
          delete window.__anyWP_originalRAF;
          window.__anyWP_rafPaused = false;
        }
        
        // Trigger visibility change event for test page counters
        if (window.AnyWP && typeof window.AnyWP._notifyVisibilityChange === 'function') {
          window.AnyWP._notifyVisibilityChange(true);
        }
        
        return 'RESUMED';
      } catch(e) {
        return 'ERROR: ' + e.message;
      }
    })();
  )";
  
  executor(unfreeze_script);
  
  // 2. Notify SDK about resume
  std::wstring notify_resume = LR"(
    (function() {
      if (window.AnyWP && typeof window.AnyWP._notifyVisibilityChange === 'function') {
        console.log('[C++] Calling AnyWP._notifyVisibilityChange(true)');
        window.AnyWP._notifyVisibilityChange(true);
        return 'NOTIFY_RESUME_OK';
      } else {
        console.log('[C++] AnyWP._notifyVisibilityChange not available');
        return 'NOTIFY_RESUME_FAILED';
      }
    })();
  )";
  
  executor(notify_resume);
  Logger::Instance().Info("PowerManager", "Resume scripts executed");
}

}  // namespace anywp_engine
