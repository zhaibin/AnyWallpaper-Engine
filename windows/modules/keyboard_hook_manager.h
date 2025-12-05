#ifndef ANYWP_ENGINE_KEYBOARD_HOOK_MANAGER_H_
#define ANYWP_ENGINE_KEYBOARD_HOOK_MANAGER_H_

#include <windows.h>
#include <functional>
#include <string>

namespace anywp_engine {

/**
 * KeyboardHookManager - Global keyboard hook for desktop keyboard event handling
 * 
 * Features:
 * - Low-level keyboard hook (WH_KEYBOARD_LL)
 * - Forward keyboard events to WebView
 * - Supports keydown and keyup events
 * - Extracts key code and virtual key information
 */
class KeyboardHookManager {
public:
  KeyboardHookManager();
  ~KeyboardHookManager();

  // Hook management
  bool Install();
  void Uninstall();
  bool IsInstalled() const;

  // Callback for keyboard events
  // Parameters: event_type (keydown/keyup), virtual_key_code, scan_code, is_extended_key
  using KeyboardCallback = std::function<void(const char* event_type, int vk_code, int scan_code, bool extended, bool alt_down, bool ctrl_down, bool shift_down)>;
  
  void SetKeyboardCallback(KeyboardCallback callback);
  
  // State management
  void SetPaused(bool paused);
  bool IsPaused() const;

private:
  static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
  static KeyboardHookManager* instance_;
  
  HHOOK hook_;
  bool paused_;
  
  KeyboardCallback keyboard_callback_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_KEYBOARD_HOOK_MANAGER_H_

