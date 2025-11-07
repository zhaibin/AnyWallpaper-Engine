#include "power_manager.h"
#include <iostream>
#include <psapi.h>

namespace anywp_engine {

PowerManager* PowerManager::instance_ = nullptr;

PowerManager::PowerManager()
    : enabled_(true),
      current_state_(PowerState::ACTIVE),
      last_state_(PowerState::ACTIVE),
      idle_timeout_ms_(5 * 60 * 1000),
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
  enabled_ = enabled;
  if (enabled) {
    std::cout << "[PowerManager] Power saving enabled" << std::endl;
  } else {
    std::cout << "[PowerManager] Power saving disabled" << std::endl;
  }
}

bool PowerManager::IsEnabled() const {
  return enabled_;
}

PowerManager::PowerState PowerManager::GetCurrentState() const {
  return current_state_;
}

void PowerManager::UpdatePowerState() {
  if (!enabled_) return;
  
  PowerState new_state = PowerState::ACTIVE;
  
  // Check conditions
  if (IsFullscreenAppActive()) {
    new_state = PowerState::FULLSCREEN_APP;
  }
  
  if (new_state != current_state_) {
    last_state_ = current_state_;
    current_state_ = new_state;
    
    if (on_state_changed_) {
      on_state_changed_(last_state_, current_state_);
    }
  }
}

void PowerManager::Pause(const std::string& reason) {
  std::cout << "[PowerManager] Pausing: " << reason << std::endl;
  current_state_ = PowerState::PAUSED;
  
  if (on_pause_) {
    on_pause_(reason);
  }
}

void PowerManager::Resume(const std::string& reason, bool force_reinit) {
  std::cout << "[PowerManager] Resuming: " << reason << std::endl;
  current_state_ = PowerState::ACTIVE;
  
  if (on_resume_) {
    on_resume_(reason);
  }
}

void PowerManager::SetIdleTimeout(DWORD timeout_ms) {
  idle_timeout_ms_ = timeout_ms;
}

void PowerManager::SetMemoryThreshold(size_t mb) {
  memory_threshold_mb_ = mb;
}

void PowerManager::SetCleanupInterval(int minutes) {
  cleanup_interval_minutes_ = minutes;
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
  return !is_session_locked_ && !is_remote_session_;
}

size_t PowerManager::GetCurrentMemoryUsage() {
  PROCESS_MEMORY_COUNTERS_EX pmc;
  if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
    return pmc.WorkingSetSize / (1024 * 1024);  // Convert to MB
  }
  return 0;
}

void PowerManager::OptimizeMemoryUsage() {
  std::cout << "[PowerManager] Optimizing memory..." << std::endl;
  // TODO: Implement memory optimization
}

bool PowerManager::IsFullscreenAppActive() {
  // Simplified detection
  return false;
}

void PowerManager::StartFullscreenDetection() {
  stop_fullscreen_detection_ = false;
  // TODO: Start detection thread
}

void PowerManager::StopFullscreenDetection() {
  stop_fullscreen_detection_ = true;
  if (fullscreen_detection_thread_.joinable()) {
    fullscreen_detection_thread_.join();
  }
}

LRESULT CALLBACK PowerManager::PowerSavingWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  return DefWindowProcW(hwnd, message, wParam, lParam);
}

}  // namespace anywp_engine

