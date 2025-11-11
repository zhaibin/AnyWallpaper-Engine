#ifndef ANYWP_ENGINE_MOUSE_HOOK_MANAGER_H_
#define ANYWP_ENGINE_MOUSE_HOOK_MANAGER_H_

#include <windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <functional>
#include <vector>
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

private:
  static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
  static MouseHookManager* instance_;
  
  HHOOK hook_;
  bool paused_;
  bool is_mouse_down_;  // v2.0.4+ Mouse button down state for event tracking
  
  ClickCallback click_callback_;
  IframeCallback iframe_callback_;
  InstanceCallback instance_callback_;
  HwndCheckCallback hwnd_check_callback_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_MOUSE_HOOK_MANAGER_H_

