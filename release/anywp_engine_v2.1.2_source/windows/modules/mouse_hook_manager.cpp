#include "mouse_hook_manager.h"
#include <iostream>
#include "../anywp_engine_plugin.h"

namespace anywp_engine {

MouseHookManager* MouseHookManager::instance_ = nullptr;

MouseHookManager::MouseHookManager()
    : hook_(nullptr),
      paused_(false),
      is_mouse_down_(false) {
  instance_ = this;
}

MouseHookManager::~MouseHookManager() {
  Uninstall();
  instance_ = nullptr;
}

bool MouseHookManager::Install() {
  if (hook_) {
    return true;  // Already installed
  }
  
  try {
    std::cout << "[AnyWP] [MouseHook] Installing low-level mouse hook..." << std::endl;
    
    hook_ = SetWindowsHookExW(
      WH_MOUSE_LL,
      LowLevelMouseProc,
      GetModuleHandle(nullptr),
      0
    );
    
    if (hook_) {
      std::cout << "[AnyWP] [MouseHook] Hook installed successfully" << std::endl;
      return true;
    } else {
      DWORD error = GetLastError();
      std::cout << "[AnyWP] [MouseHook] ERROR: Failed to install hook: " << error << std::endl;
      return false;
    }
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [MouseHook] ERROR: Exception in Install: " << e.what() << std::endl;
    return false;
  } catch (...) {
    std::cout << "[AnyWP] [MouseHook] ERROR: Unknown exception in Install" << std::endl;
    return false;
  }
}

void MouseHookManager::Uninstall() {
  if (hook_) {
    std::cout << "[AnyWP] [MouseHook] Uninstalling mouse hook..." << std::endl;
    UnhookWindowsHookEx(hook_);
    hook_ = nullptr;
  }
}

bool MouseHookManager::IsInstalled() const {
  return hook_ != nullptr;
}

void MouseHookManager::SetClickCallback(ClickCallback callback) {
  click_callback_ = callback;
}

void MouseHookManager::SetIframeCallback(IframeCallback callback) {
  iframe_callback_ = callback;
}

void MouseHookManager::SetInstanceCallback(InstanceCallback callback) {
  instance_callback_ = callback;
}

void MouseHookManager::SetHwndCheckCallback(HwndCheckCallback callback) {
  hwnd_check_callback_ = callback;
}

void MouseHookManager::SetPaused(bool paused) {
  paused_ = paused;
}

bool MouseHookManager::IsPaused() const {
  return paused_;
}


LRESULT CALLBACK MouseHookManager::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
  // v2.0.10+ CRITICAL DEBUG: Output IMMEDIATELY to confirm callback is called
  static int total_calls = 0;
  total_calls++;
  
  // ALWAYS output first 20 calls, then every 10th
  static bool first_call_logged = false;
  if (!first_call_logged) {
    std::wcout << L"[MouseHook] *** CALLBACK FIRST CALL *** nCode=" << nCode << L", wParam=" << wParam << std::endl;
    first_call_logged = true;
  }
  
  if (total_calls <= 20 || total_calls % 10 == 0) {
    std::wcout << L"[MouseHook] Callback #" << total_calls << L" nCode=" << nCode << L", wParam=" << wParam << std::endl;
  }
  
  // v2.0.10+ DEBUG: Track early returns (log ALL for debugging)
  static int early_return_ncode = 0;
  static int early_return_paused = 0;
  
  if (nCode < 0 || !instance_) {
    early_return_ncode++;
    // Log first 50 or every 20th
    if (early_return_ncode <= 50 || early_return_ncode % 20 == 0) {
      std::wcout << L"[MouseHook] DEBUG: Early return (nCode=" << nCode 
                 << L", instance=" << (instance_ ? L"OK" : L"NULL")
                 << L"), count: " << early_return_ncode << std::endl;
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }
  
  // Skip if paused (performance optimization)
  if (instance_->paused_) {
    early_return_paused++;
    if (early_return_paused <= 50 || early_return_paused % 20 == 0) {
      std::wcout << L"[MouseHook] DEBUG: Paused, skipping event, count: " << early_return_paused << std::endl;
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }
  
  MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
  POINT pt = info->pt;
  
  // Check if click position is occluded by a top-level application window
  HWND window_at_point = WindowFromPoint(pt);
  
  // Get window class name for debugging
  wchar_t className[256] = {0};
  if (window_at_point) {
    GetClassNameW(window_at_point, className, 256);
  }
  
  // Debug logging (v2.0.10+ log ALL non-mousemove events unconditionally)
  static int debug_count = 0;
  static int mousemove_debug_count = 0;
  bool should_log = (wParam != WM_MOUSEMOVE);  // Always log non-mousemove events
  
  // For mousemove, log every 50th event
  if (wParam == WM_MOUSEMOVE) {
    mousemove_debug_count++;
    should_log = (mousemove_debug_count % 50 == 0);
  }
  
  if (should_log) {
    debug_count++;
    std::wcout << L"[MouseHook] Event: " << wParam 
               << L" at (" << pt.x << L"," << pt.y << L")";
    if (instance_->is_mouse_down_) {
      std::wcout << L" [MOUSE_DOWN]";
    }
    std::wcout << std::endl;
    std::wcout << L"[MouseHook] WindowAtPoint: " << window_at_point 
               << L" ClassName: " << className << std::endl;
  }
  
  // Check if this is a top-level application window (not desktop layer)
  bool is_app_window = false;
  if (window_at_point) {
    // Get the root owner window
    HWND root_window = GetAncestor(window_at_point, GA_ROOT);
    
    // Check if it's a visible top-level window
    if (root_window && IsWindowVisible(root_window)) {
      LONG style = GetWindowLongW(root_window, GWL_STYLE);
      
      // Get root window class name
      wchar_t rootClassName[256] = {0};
      GetClassNameW(root_window, rootClassName, 256);
      
      // v2.0.10+ DEBUG: Log root window class
      if (should_log) {
        std::wcout << L"[MouseHook] Root window: " << root_window 
                   << L" Class: " << rootClassName << std::endl;
      }
      
      // v2.0.10+ CRITICAL FIX: Detect desktop AND desktop icon list
      // SysListView32 is the icon container, we need to forward these events too
      bool is_desktop_window = (
        wcscmp(rootClassName, L"Progman") == 0 ||
        wcscmp(rootClassName, L"WorkerW") == 0 ||
        wcscmp(rootClassName, L"Shell_TrayWnd") == 0 ||  // Taskbar
        wcsstr(rootClassName, L"Xaml") != nullptr ||  // System UI
        wcscmp(className, L"SysListView32") == 0  // Desktop icons (child of Progman)
      );
      
      // Check if it's OUR wallpaper window (via HWND callback)
      bool is_our_window = false;
      if (instance_->hwnd_check_callback_) {
        // Check both the window at point and its root
        bool check_window = instance_->hwnd_check_callback_(window_at_point);
        bool check_root = instance_->hwnd_check_callback_(root_window);
        is_our_window = check_window || check_root;
        
        if (should_log) {
          std::wcout << L"[MouseHook] HWND Check - window_at_point: " << check_window 
                     << L", root_window: " << check_root << std::endl;
        }
      }
      
      // Fallback: Also check if it's our Chrome WebView2 window
      if (!is_our_window && (wcsstr(rootClassName, L"Chrome") != nullptr || wcsstr(className, L"Chrome") != nullptr)) {
        if (instance_->instance_callback_) {
          WallpaperInstance* inst = instance_->instance_callback_(pt.x, pt.y);
          is_our_window = (inst != nullptr);
          
          if (should_log) {
            std::wcout << L"[MouseHook] Chrome window check: " << (inst != nullptr) << std::endl;
          }
        }
      }
      
      if (should_log) {
        std::wcout << L"[MouseHook] is_desktop_window: " << is_desktop_window 
                   << L", is_our_window: " << is_our_window << std::endl;
      }
      
      if (!is_desktop_window && !is_our_window) {
        // Check if it's a normal app window (has caption or is popup)
        bool has_window_style = (style & WS_CAPTION) || (style & WS_POPUP);
        
        if (has_window_style) {
          is_app_window = true;
        }
      }
    }
  }
  
  // v2.0.4+ Set mouse button state BEFORE checking is_app_window
  // Otherwise mousedown on app window will return early and never set is_mouse_down_
  
  // Determine event type and update mouse button state FIRST
  const char* event_type = nullptr;
  
  if (wParam == WM_LBUTTONDOWN) {
    event_type = "mousedown";
    // v2.0.4+ Track mouse button down state - MUST set this before any early returns
    instance_->is_mouse_down_ = true;
    if (should_log) {
      std::wcout << L"[MouseHook] 🖱️ Mouse button down" << std::endl;
    }
  } else if (wParam == WM_LBUTTONUP) {
    event_type = "mouseup";
    // v2.0.4+ Clear mouse button down state
    instance_->is_mouse_down_ = false;
    if (should_log) {
      std::wcout << L"[MouseHook] 🖱️ Mouse button up" << std::endl;
    }
  } else if (wParam == WM_MOUSEMOVE) {
    event_type = "mousemove";
  }
  
  // v2.0.4+ NOW check is_app_window (after mouse button state is set)
  // Don't block events when mouse button is pressed
  if (is_app_window && !instance_->is_mouse_down_) {
    if (should_log) {
      std::wcout << L"[MouseHook] ❌ BLOCKED - is_app_window = true, mouse button not down" << std::endl;
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }
  
  // v2.0.10+ DEBUG: Always log FORWARDING decision
  static int forward_count = 0;
  forward_count++;
  if (should_log || forward_count <= 10) {
    std::wcout << L"[MouseHook] ✅ FORWARDING #" << forward_count << L" event to WebView";
    if (instance_->is_mouse_down_) {
      std::wcout << L" (mouse down)";
    }
    std::wcout << std::endl;
  }
  
  // Get target wallpaper instance (via callback)
  WallpaperInstance* target_instance = nullptr;
  if (instance_->instance_callback_) {
    target_instance = instance_->instance_callback_(pt.x, pt.y);
  }
  
  // Check if click is on an iframe ad (via callback)
  if (wParam == WM_LBUTTONUP && target_instance && instance_->iframe_callback_) {
    IframeInfo* iframe = instance_->iframe_callback_(pt.x, pt.y, target_instance);
    
    if (iframe && !iframe->click_url.empty()) {
      std::cout << "[AnyWP] [MouseHook] Click on iframe: " << iframe->id 
                << " at (" << pt.x << "," << pt.y << ")" << std::endl;
      std::cout << "[AnyWP] [MouseHook] Opening URL: " << iframe->click_url << std::endl;
      
      // Open the ad URL directly
      std::wstring url_wide(iframe->click_url.begin(), iframe->click_url.end());
      ShellExecuteW(nullptr, L"open", url_wide.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
      
      // Don't forward to WebView
      return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }
  }
  
  // Forward to WebView via callback
  if (event_type && instance_->click_callback_) {
    instance_->click_callback_(pt.x, pt.y, event_type);
  }
  
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

}  // namespace anywp_engine
