#ifndef ANYWP_ENGINE_POWER_MANAGER_H_
#define ANYWP_ENGINE_POWER_MANAGER_H_

#include <windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <atomic>
#include <thread>
#include <functional>
#include <vector>
#include <string>

namespace anywp_engine {

// Forward declaration
struct WallpaperInstance;

/**
 * PowerManager - Power saving and performance optimization
 * 
 * Features:
 * - Fullscreen app detection
 * - User idle detection
 * - Screen off detection
 * - Session lock detection
 * - Automatic pause/resume
 * - Memory optimization
 */
class PowerManager {
public:
  enum class PowerState {
    ACTIVE,          // Normal active state
    IDLE,            // User inactive
    SCREEN_OFF,      // Screen is off
    LOCKED,          // System is locked
    FULLSCREEN_APP,  // Fullscreen app in foreground
    PAUSED           // Manually paused
  };

  PowerManager();
  ~PowerManager();

  // Power monitoring
  void Enable(bool enabled);
  bool IsEnabled() const;
  
  // State management
  PowerState GetCurrentState() const;
  void UpdatePowerState();
  
  // Session state (called by external listener)
  void SetSessionLocked(bool locked);
  void SetRemoteSession(bool remote);
  
  // Manual pause/resume
  void Pause(const std::string& reason);
  void Resume(const std::string& reason, bool force_reinit = false);
  
  // v1.4.1+ Phase E: Script execution helpers
  using ScriptExecutor = std::function<void(const std::wstring&)>;
  void ExecutePauseScripts(ScriptExecutor executor);
  void ExecuteResumeScripts(ScriptExecutor executor);
  
  // Configuration
  void SetIdleTimeout(DWORD timeout_ms);
  void SetMemoryThreshold(size_t mb);
  void SetCleanupInterval(int minutes);
  
  // Callbacks
  using StateChangeCallback = std::function<void(PowerState old_state, PowerState new_state)>;
  using PauseCallback = std::function<void(const std::string& reason)>;
  using ResumeCallback = std::function<void(const std::string& reason)>;
  
  void SetOnStateChanged(StateChangeCallback callback);
  void SetOnPause(PauseCallback callback);
  void SetOnResume(ResumeCallback callback);
  
  // Helpers
  bool ShouldWallpaperBeActive() const;
  size_t GetCurrentMemoryUsage();
  void OptimizeMemoryUsage();
  
  // v1.4.0+ Refactoring: Methods made public for delegation support
  bool IsFullscreenAppActive();
  void StartFullscreenDetection();
  void StopFullscreenDetection();

private:
  
  static LRESULT CALLBACK PowerSavingWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  static PowerManager* instance_;
  
  bool enabled_;
  PowerState current_state_;
  PowerState last_state_;
  
  DWORD idle_timeout_ms_;
  size_t memory_threshold_mb_;
  int cleanup_interval_minutes_;
  
  HWND listener_hwnd_;
  std::atomic<bool> stop_fullscreen_detection_;
  std::thread fullscreen_detection_thread_;
  std::atomic<bool> is_session_locked_;
  std::atomic<bool> is_remote_session_;
  
  StateChangeCallback on_state_changed_;
  PauseCallback on_pause_;
  ResumeCallback on_resume_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_POWER_MANAGER_H_

