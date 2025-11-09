#include "power_manager.h"
#include <iostream>
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
      std::cout << "[AnyWP] [PowerManager] Enabling power management" << std::endl;
      StartFullscreenDetection();
    } else {
      std::cout << "[AnyWP] [PowerManager] Disabling power management" << std::endl;
      StopFullscreenDetection();
    }
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [PowerManager] ERROR: Exception in Enable: " << e.what() << std::endl;
    enabled_ = !enabled;  // Rollback state
  } catch (...) {
    std::cout << "[AnyWP] [PowerManager] ERROR: Unknown exception in Enable" << std::endl;
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
  std::cout << "[AnyWP] [PowerManager] Session lock state changed: " << locked << std::endl;
  is_session_locked_.store(locked);
  UpdatePowerState();
}

void PowerManager::SetRemoteSession(bool remote) {
  std::cout << "[AnyWP] [PowerManager] Remote session state changed: " << remote << std::endl;
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
    // TODO: Add idle detection
    // TODO: Add screen off detection
    
    if (new_state != current_state_) {
      last_state_ = current_state_;
      current_state_ = new_state;
      
      std::cout << "[AnyWP] [PowerManager] State changed: "
                << static_cast<int>(last_state_) << " -> "
                << static_cast<int>(current_state_) << std::endl;
      
      if (on_state_changed_) {
        try {
          on_state_changed_(last_state_, current_state_);
        } catch (const std::exception& e) {
          std::cout << "[AnyWP] [PowerManager] ERROR: Exception in state change callback: " 
                    << e.what() << std::endl;
        } catch (...) {
          std::cout << "[AnyWP] [PowerManager] ERROR: Unknown exception in state change callback" << std::endl;
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
    std::cout << "[AnyWP] [PowerManager] ERROR: Exception in UpdatePowerState: " 
              << e.what() << std::endl;
  } catch (...) {
    std::cout << "[AnyWP] [PowerManager] ERROR: Unknown exception in UpdatePowerState" << std::endl;
  }
}

void PowerManager::Pause(const std::string& reason) {
  std::cout << "[AnyWP] [PowerManager] Pause requested: " << reason << std::endl;
  
  if (on_pause_) {
    on_pause_(reason);
  }
}

void PowerManager::Resume(const std::string& reason, bool force_reinit) {
  std::cout << "[AnyWP] [PowerManager] Resume requested: " << reason 
            << " (force_reinit=" << force_reinit << ")" << std::endl;
  
  if (on_resume_) {
    on_resume_(reason);
  }
}

void PowerManager::SetIdleTimeout(DWORD timeout_ms) {
  idle_timeout_ms_ = timeout_ms;
  std::cout << "[AnyWP] [PowerManager] Idle timeout set to " 
            << (timeout_ms / 1000) << " seconds" << std::endl;
}

void PowerManager::SetMemoryThreshold(size_t mb) {
  memory_threshold_mb_ = mb;
  std::cout << "[AnyWP] [PowerManager] Memory threshold set to " 
            << mb << " MB" << std::endl;
}

void PowerManager::SetCleanupInterval(int minutes) {
  cleanup_interval_minutes_ = minutes;
  std::cout << "[AnyWP] [PowerManager] Cleanup interval set to " 
            << minutes << " minutes" << std::endl;
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
  std::cout << "[AnyWP] [PowerManager] Optimizing memory usage..." << std::endl;
  
  // Trigger garbage collection (Windows will reclaim unused pages)
  SetProcessWorkingSetSize(GetCurrentProcess(), 
                          static_cast<SIZE_T>(-1), 
                          static_cast<SIZE_T>(-1));
  
  std::cout << "[AnyWP] [PowerManager] Memory optimization complete" << std::endl;
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
  
  return true;
}

void PowerManager::StartFullscreenDetection() {
  if (fullscreen_detection_thread_.joinable()) {
    return;  // Already running
  }
  
  stop_fullscreen_detection_ = false;
  
  fullscreen_detection_thread_ = std::thread([this]() {
    std::cout << "[AnyWP] [PowerManager] Fullscreen detection thread started" << std::endl;
    
    while (!stop_fullscreen_detection_) {
      UpdatePowerState();
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    std::cout << "[AnyWP] [PowerManager] Fullscreen detection thread stopped" << std::endl;
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
            std::cout << "[AnyWP] [PowerManager] System suspending" << std::endl;
            break;
          case PBT_APMRESUMESUSPEND:
            std::cout << "[AnyWP] [PowerManager] System resuming from suspend" << std::endl;
            break;
        }
      }
      break;
  }
  
  return DefWindowProc(hwnd, message, wParam, lParam);
}

}  // namespace anywp_engine
