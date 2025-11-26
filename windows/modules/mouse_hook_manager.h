#ifndef ANYWP_ENGINE_MOUSE_HOOK_MANAGER_H_
#define ANYWP_ENGINE_MOUSE_HOOK_MANAGER_H_

#include <windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <functional>
#include <vector>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "iframe_detector.h"  // For IframeInfo

namespace anywp_engine {

// Forward declarations
struct WallpaperInstance;

/**
 * MouseHookManager - Global mouse hook for desktop click handling
 * 
 * Features:
 * - Low-level mouse hook (WH_MOUSE_LL)
 * - Forward desktop clicks to WebView
 * - Window occlusion detection
 * - Iframe hit-testing
 * - Mouse button state tracking
 * - v2.5.1+ Polling fallback for interference resilience
 * 
 * v2.5.1+ Anti-Interference Design:
 * When another program's mouse hook blocks mousemove events (e.g., lxwp.exe),
 * we use a timer-based polling mechanism as fallback to ensure smooth dragging.
 */
class MouseHookManager {
public:
  MouseHookManager();
  ~MouseHookManager();

  // Hook management
  bool Install();
  void Uninstall();
  bool IsInstalled() const;

  // Callbacks
  using ClickCallback = std::function<void(int x, int y, const char* event_type)>;
  using IframeCallback = std::function<IframeInfo*(int x, int y, WallpaperInstance*)>;
  using InstanceCallback = std::function<WallpaperInstance*(int x, int y)>;
  using HwndCheckCallback = std::function<bool(HWND)>;
  
  void SetClickCallback(ClickCallback callback);
  void SetIframeCallback(IframeCallback callback);
  void SetInstanceCallback(InstanceCallback callback);
  void SetHwndCheckCallback(HwndCheckCallback callback);
  
  // State management
  void SetPaused(bool paused);
  bool IsPaused() const;
  
  // v2.5.1+ Polling fallback configuration
  /**
   * Enable/disable polling fallback mechanism
   * When enabled, a timer will poll mouse position during drag operations
   * to compensate for potentially blocked mousemove events from other hooks
   * 
   * @param enabled True to enable polling fallback (default: true)
   */
  void SetPollingFallbackEnabled(bool enabled);
  
  /**
   * Set polling interval in milliseconds
   * Lower values = smoother dragging but higher CPU usage
   * 
   * @param interval_ms Polling interval (default: 16ms = ~60fps)
   */
  void SetPollingInterval(UINT interval_ms);
  
  /**
   * Set window handle for UI thread timer
   * The timer will process queued events from polling thread
   * 
   * @param hwnd Window handle to attach timer to (should be on UI thread)
   */
  void SetTimerWindow(HWND hwnd);

private:
  static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
  static MouseHookManager* instance_;
  
  // v2.5.1+ Polling fallback implementation (using dedicated thread)
  void StartPollingThread();
  void StopPollingThread();
  void PollingThreadFunc();
  
  // Process pending polled events (must be called from UI thread)
  void ProcessPendingPolledEvents();
  
  HHOOK hook_;
  bool paused_;
  std::atomic<bool> is_mouse_down_;  // v2.0.4+ Mouse button down state for event tracking
  
  // v2.5.1+ Polling fallback state (using dedicated thread for reliability)
  std::unique_ptr<std::thread> polling_thread_;
  std::mutex polling_mutex_;
  std::condition_variable polling_cv_;
  std::atomic<bool> polling_thread_running_;
  std::atomic<bool> polling_thread_should_stop_;
  UINT polling_interval_ms_;
  bool polling_fallback_enabled_;
  POINT last_polled_position_;
  std::atomic<DWORD> last_hook_mousemove_time_;  // Timestamp of last mousemove from hook
  static constexpr DWORD HOOK_TIMEOUT_MS = 50;   // If no hook event for this long, use polling
  
  // v2.5.1+ Thread-safe event queue for cross-thread event delivery
  struct PolledEvent {
    int x;
    int y;
  };
  std::mutex event_queue_mutex_;
  std::vector<PolledEvent> polled_event_queue_;
  static constexpr size_t MAX_QUEUED_EVENTS = 100;  // Prevent unbounded growth
  
  // v2.5.1+ UI thread timer for queue processing
  HWND timer_hwnd_;
  UINT_PTR ui_timer_id_;
  static void CALLBACK UITimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
  
  ClickCallback click_callback_;
  IframeCallback iframe_callback_;
  InstanceCallback instance_callback_;
  HwndCheckCallback hwnd_check_callback_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_MOUSE_HOOK_MANAGER_H_

