#include "state_manager.h"
#include "logger.h"

namespace anywp_engine {

StateManager::StateManager() {
  Logger::Instance().Info("StateManager", "Utility initialized");
}

StateManager::~StateManager() {
  Logger::Instance().Info("StateManager", "Utility destroyed");
}

void StateManager::SetPowerState(PowerState state) {
  std::lock_guard<std::mutex> lock(state_mutex_);
  
  PowerState old_state = current_power_state_;
  last_power_state_ = old_state;
  current_power_state_ = state;

  Logger::Instance().Info("StateManager", 
    "Power state changed: " + PowerStateToString(old_state) + 
    " -> " + PowerStateToString(state));

  if (state_change_callback_) {
    state_change_callback_(old_state, state);
  }
}

StateManager::PowerState StateManager::GetPowerState() const {
  std::lock_guard<std::mutex> lock(state_mutex_);
  return current_power_state_;
}

StateManager::PowerState StateManager::GetLastPowerState() const {
  std::lock_guard<std::mutex> lock(state_mutex_);
  return last_power_state_;
}

void StateManager::SetSessionLocked(bool locked) {
  is_session_locked_.store(locked);
  Logger::Instance().Info("StateManager", 
    "Session " + std::string(locked ? "locked" : "unlocked"));
}

bool StateManager::IsSessionLocked() const {
  return is_session_locked_.load();
}

void StateManager::SetRemoteSession(bool remote) {
  is_remote_session_.store(remote);
  Logger::Instance().Info("StateManager", 
    "Remote session " + std::string(remote ? "active" : "inactive"));
}

bool StateManager::IsRemoteSession() const {
  return is_remote_session_.load();
}

void StateManager::SetPaused(bool paused) {
  is_paused_.store(paused);
  Logger::Instance().Info("StateManager", 
    std::string(paused ? "Paused" : "Resumed"));
}

bool StateManager::IsPaused() const {
  return is_paused_.load();
}

void StateManager::SetStateChangeCallback(StateChangeCallback callback) {
  std::lock_guard<std::mutex> lock(state_mutex_);
  state_change_callback_ = callback;
}

bool StateManager::ShouldWallpaperBeActive() const {
  return !is_session_locked_.load() && !is_remote_session_.load();
}

std::string StateManager::PowerStateToString(PowerState state) const {
  switch (state) {
    case PowerState::ACTIVE: return "ACTIVE";
    case PowerState::IDLE: return "IDLE";
    case PowerState::SCREEN_OFF: return "SCREEN_OFF";
    case PowerState::LOCKED: return "LOCKED";
    case PowerState::FULLSCREEN_APP: return "FULLSCREEN_APP";
    case PowerState::PAUSED: return "PAUSED";
    default: return "UNKNOWN";
  }
}

}  // namespace anywp_engine

